/*
 * Created on Jun 18, 2003
 */
package backend;

import java.io.File;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import sourceanalysis.Aggregate;
import sourceanalysis.Alias;
import sourceanalysis.ContainedConnection;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;
import sourceanalysis.InappropriateKindException;
import sourceanalysis.InheritanceConnection;
import sourceanalysis.MissingInformationException;
import sourceanalysis.Namespace;
import sourceanalysis.Primitive;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.Routine;
import sourceanalysis.Scope;
import sourceanalysis.Specifiers;
import sourceanalysis.TemplateArgument;
import sourceanalysis.TemplateEnabledEntity;
import sourceanalysis.Type;
import sourceanalysis.assist.SupervisedTemplates;
import sourceanalysis.view.TemplateBank;
import sourceanalysis.view.Traverse;

/**
 * Provides a standard basic for code generation functionality. Derive and
 * extend to create specialized code generators.
 */
public class GenericCodeGenerator
	implements sourceanalysis.view.Perspective 
{

	/**
	 * GenericCodeGenerator constructor.
	 */
	public GenericCodeGenerator(ProgramDatabase program, Writer output,
		TemplateBank templates)
	{
		super();
		m_program = program;
		m_output = output;
		m_subjects = new HashSet<Aggregate>();
		m_subjectTemplates = new HashSet();
		m_enums = new HashSet();
		m_typedefs = new HashSet();
		m_globalFuncs = new HashSet();
		m_namespaces = new HashSet();
		m_templates = templates;
		m_separateClassTemplates = false; // if set to true, classes are put
					// in 'subjects' while class templates are put in 
					// 'subjectTemplates', otherwise they are all put
					// in 'subjects'.
		// initialize instances
		m_instanceSet = new HashSet();
		m_instanceMap = new HashMap();
		m_supervisedTemplates = new SupervisedTemplates();
	}
	
	/**
	 * GenericCodeGenerator constructor.
	 * The template bank is assumed to be <b>null</b>.
	 */
	public GenericCodeGenerator(ProgramDatabase program, Writer output)
	{
		this(program, output, null);
	}

	/**
	 * Sets the output directory. This is not always a mandatory attribute,
	 * but some backends do require it. 
	 * @param directory a File object representing the output directory
	 */
	public void setOutputDirectory(File directory)
	{
		// Store directory as absolute
		if (directory.isAbsolute())
			m_outputDirectory = directory;
		else
			m_outputDirectory = directory.getAbsoluteFile();
	}
	
	/**
	 * Sets the output filename. Also sets the output directory to the 
	 * directory containing that file. This is not always a mandatory 
	 * attribute, but some backends do require it. 
	 * @param filename the output filename
	 */
	public void setOutputFilename(String filename)
	{
		File outfile = new File(filename);
		// - set the output directory accordingly
		File outdir = outfile.getParentFile();
		if (outdir == null) outdir = new File(".");
		setOutputDirectory(outdir);
	}
	
	/**
	 * Looks for 'classname' anywhere in the program database.
	 * @param componentname name to look for - this name is recursively 
	 * searched in all scopes.
	 */
	public void collect(String componentname)
	{
		collect(m_program.getGlobalNamespace().getScope(), componentname);
		collect(m_program.getExternals(), componentname);
	}
	
	/**
	 * Looks for 'classname' in the given scope and all sub-scopes.
	 * @param scope a scope to start collecting in 
	 * @param componentname name to look for - this name is recursively 
	 * searched in all scopes.
	 */
	public void collect(Scope scope, String componentname)
	{
		// Find aggregates
		for (Iterator aggiter = scope.aggregateIterator(); aggiter.hasNext(); ) {
			// Find class
			ContainedConnection connection = (ContainedConnection)aggiter.next();
			Aggregate agg = (Aggregate)connection.getContained();
			if (nameMatches(agg, componentname))
				consume(agg);
		}
		// Find enums
		for (Iterator enumiter = scope.enumIterator(); enumiter.hasNext(); ) {
			// Find class
			ContainedConnection connection = (ContainedConnection)enumiter.next();
			sourceanalysis.Enum enume = (sourceanalysis.Enum)connection.getContained();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC
				&& nameMatches(enume, componentname)) {
				m_enums.add( enume );
			}
		}
		// Find typedefs
		for (Iterator aliter = scope.aliasIterator(); aliter.hasNext(); ) {
			ContainedConnection connection = (ContainedConnection)aliter.next();
			Alias alias = (Alias)connection.getContained();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC
				&& nameMatches(alias, componentname)) {
				m_typedefs.add(alias);
			}
		}
		// Find global functions
		for (Iterator funciter = scope.routineIterator(); funciter.hasNext(); ) {
			ContainedConnection connection = 
				(ContainedConnection)funciter.next();
			Routine routine = (Routine)connection.getContained();
			if (!(connection.getContainer() instanceof Aggregate)
				&& nameMatches(routine, componentname)) {
				m_globalFuncs.add(routine);
			}
		}
		// Find namespaces and look in inner namespace scopes
		for (Iterator nsiter = scope.namespaceIterator(); nsiter.hasNext(); ) {
			ContainedConnection connection = (ContainedConnection)nsiter.next();
			Namespace namespace = (Namespace)connection.getContained();
			if (nameMatches(namespace, componentname)) {
				m_namespaces.add(namespace);
				autocollect(namespace.getScope());
			}
			collect(namespace.getScope(), componentname);
		}
	}
	
	/**
	 * Attempts detection of documented classes, by filtering only those
	 * that have a description (that is, a documentation comment-block in
	 * the source).
	 */
	public void autocollect()
	{
		autocollect(m_program.getGlobalNamespace().getScope());
	}
	
	/**
	 * Collects classes with documentation from underneath the given scope.
	 * @param scope start scope
	 */
	private void autocollect(Scope scope)
	{
		// Find aggregates
		for (Iterator aggiter = scope.aggregateIterator(); aggiter.hasNext(); ) {
			// Find class
			ContainedConnection connection = (ContainedConnection)aggiter.next();
			Aggregate agg = (Aggregate)connection.getContained();
			consume(agg);
		}
		// Find global functions
		for (Iterator funciter = scope.routineIterator(); funciter.hasNext();)
		{
			// Find function
			ContainedConnection connection = (ContainedConnection)funciter.next();
			Routine routine = (Routine)connection.getContained();
			m_globalFuncs.add(routine);
		}
		// Find typedefs
		for (Iterator typedefiter = scope.aliasIterator(); typedefiter.hasNext();)
		{
			// Find typedef
			ContainedConnection connection = (ContainedConnection)typedefiter.next();
			Alias alias = (Alias)connection.getContained();
			// Add the typedef
			m_typedefs.add(alias);
		}
		// Look in inner namespace scopes
		for (Iterator nsiter = scope.namespaceIterator(); nsiter.hasNext(); ) {
			ContainedConnection connection = (ContainedConnection)nsiter.next();
			Namespace namespace = (Namespace)connection.getContained();
			autocollect(namespace.getScope());
		}		
	}

	/**
	 * Returns a container of all possible names via which
	 * an entity can be referenced for collection purposes.
	 * @param entity
	 * @return a Collection of Strings
	 */
	protected static Collection allPossibleNames(Entity entity)
	{
		List names = new ArrayList();
		// add the trivial names
		names.add(entity.getName());
		names.add(Utils.cleanFullName(entity));
		// if this is a template instantiation - add the template name as well
		if ((entity instanceof TemplateEnabledEntity) && ((TemplateEnabledEntity)entity).isSpecialized()) {
			names.add(((TemplateEnabledEntity)entity).getGeneralTemplateForSpecialization().getGeneral().getName());
		}
		return names;
	}
	
	protected boolean nameMatches(Entity entity, String name)
	{
		Collection names = allPossibleNames(entity);
		for (Iterator nameIter = names.iterator(); nameIter.hasNext(); ) {
			if (nameIter.next().equals(name))
				return true;
		}
		return false;
	}
	
	/**
	 * Adds an aggregate to either m_subjects or m_subjectTemplates
	 * according to relevance.
	 * @param agg an Aggregate instance
	 */
	private void consume(Aggregate agg)
	{
		if (!agg.isTemplated() || !m_separateClassTemplates)
			m_subjects.add( agg );
		else
			m_subjectTemplates.add( agg );
	}

	/**
	 * Adds classes which are targets of typedef declarations to the
	 * subjects list.
	 */
	public void grabTypedefedClasses()
	{
		for (Alias typedef: m_typedefs) {
			Type type = typedef.getAliasedType();
			// Get the base type of flat types as the target of the typedef
			if (type.isFlat()) {
				Entity base = type.getBaseType();
				if (base instanceof Aggregate && !(base instanceof Primitive) &&
						(type.getTemplateArguments() == null)) {
					m_subjects.add((Aggregate)base); 
				}
				else if (base instanceof sourceanalysis.Enum) {
					m_enums.add(base);
				}
			}
		}
	}
	
	/**
	 * Adds inner constructs of the subjects already taken. This
	 * include inner classes and inner enumerated types.
	 */
	public void grabInnersAsWell()
	{
		List<Aggregate> innerClasses = new LinkedList<Aggregate>();
		
		for (Aggregate subject: m_subjects) {
			grabInnersOf(subject, innerClasses);
		}
		
		m_subjects.addAll(innerClasses);
	}
	
	private void grabInnersOf(Aggregate subject, List innerClasses)
	{
		Scope scope = subject.getScope();
		// Add public enums
		for (Iterator ei = scope.enumIterator(); ei.hasNext();) {
			ContainedConnection connection = (ContainedConnection)ei.next();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC) {
				m_enums.add(connection.getContained());
			}
		}
		// Add public typedefs
		for (Iterator ai = scope.aliasIterator(); ai.hasNext();) {
			ContainedConnection connection = (ContainedConnection)ai.next();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC) {
				m_typedefs.add((Alias)connection.getContained());
			}
		}
		// Add public static methods
		for (Iterator ri = scope.routineIterator(); ri.hasNext();) {
			ContainedConnection connection = (ContainedConnection)ri.next();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC
				 && connection.getStorage() == Specifiers.Storage.STATIC) {
				m_globalFuncs.add((Routine)connection.getContained());
			}
		}
		// Add public inner classes
		for (Iterator ci = scope.aggregateIterator(); ci.hasNext();) {
			ContainedConnection connection = (ContainedConnection)ci.next();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC) {
				Aggregate inner = (Aggregate)connection.getContained();
				innerClasses.add(inner);
				grabInnersOf(inner, innerClasses);
			}
		}
	}
	/**
	 * Provides the template-bank, for use of this class as a Perspective. 
	 */
	public TemplateBank getTemplates()
	{
		return m_templates;
	}
	
	/**
	 * @name Utilities for backends, especially those which exploit Jython 
	 * code.
	 */
	//@{
	
	/**
	 * Fills a template for every property of a given entity. A "proxy"
	 * entity, with properties "name" and "value" is temporarily generated
	 * for each property - and is sent to the template as a variable named
	 * "PROPERTY".
	 * @param entity the entity from which to process properties
	 * @param templateName name of template to be filled for each property
	 * @return Combined filled strings for all properties
	 * @throws ElementNotFoundException if the specified template does not
	 * exist in the template bank determined by getTemplates().
	 */
	public String refillProperties(Entity entity, String templateName)
		throws ElementNotFoundException
	{
		StringBuffer sb = new StringBuffer();
		
		for (Iterator pi = entity.propertyIterator(); pi.hasNext(); ) {
			Entity.Property property = (Entity.Property)pi.next();
			Entity proxy = new Entity() { };
			proxy.addProperty(new Entity.Property("name", property.getName()));
			proxy.addProperty(new Entity.Property("value",property.getValue()));
			// Create a scope containing proxy property entity
			sourceanalysis.view.Scope scope = new sourceanalysis.view.Scope();
			scope.declareMember("PROPERTY", proxy, true);
			// Fill template
			sb.append(getTemplates().fillTemplate(templateName,scope,this));
		}
		return sb.toString();
	}
	
	
	/**
	 * Fills methods using a sourceanalysis.view template. Methods are
	 * filtered according to the visibility criterion.
	 * Template member functions are <b>ignored</b>.
	 * @param templateName name of the template to be filled for each
	 * method
	 * @param elementName name of the element in the sourceanalysis.view.Scope
	 * which is sent to the template on each invocation
	 * @param scope the scope containing routines to be filtered and
	 * processed
	 * @param crit a visibility value (from Specifiers.Visibility) which
	 * serves as a criterion for filtering methods.
	 * @throws ElementNotFoundException if the template referred to by
	 * templateName does not exist.
	 */
	protected String refillMethods(String templateName,
		String elementName, Scope scope, int crit) throws ElementNotFoundException
	{
		StringBuffer sb = new StringBuffer();
		
		for (Iterator ri = scope.routineIterator(); ri.hasNext(); ) {
			// Acquire the connection. This connection lets the loop
			// filter methods by visibility.
			ContainedConnection connection = (ContainedConnection)ri.next();
			int visibility = connection.getVisibility();
			Entity routine = connection.getContained();
			// Fill the given template if visibility is PUBLIC.
			// Skip template member functions.
			if (visibility == crit && !routine.isTemplated()) {
				// Prepare a scope with elementName=routine
				sourceanalysis.view.Scope vscope = 
					new sourceanalysis.view.Scope();
				vscope.declareMember(elementName, routine, true);
				// Fill template referred to by 'templateName'
				sb.append(getTemplates().fillTemplate(templateName, vscope, this));
			}
		}
		
		return sb.toString();
	}

	/**
	 * Filters only methods that are being exposed - that is, only
	 * public methods. For each method, a sourceanalysis.view template is
	 * filled.
	 * @param templateName name of the template to be filled for each
	 * method
	 * @param elementName name of the element in the sourceanalysis.view.Scope
	 * which is sent to the template on each invocation
	 * @param scope the scope containing routines to be filtered and
	 * processed
	 * @throws ElementNotFoundException if the template referred to by
	 * templateName does not exist.
	 */
	public String refillPublicMethods(String templateName,
		String elementName, Scope scope) throws ElementNotFoundException
	{
		return refillMethods(templateName, elementName, scope,
			 Specifiers.Visibility.PUBLIC);
	}

	/**
	 * Filters only private methods. They may be important to instruct SWIG
	 * not to create certain types of forbidden invocations. 
	 * For each method, a sourceanalysis.view template is
	 * filled.
	 * @param templateName name of the template to be filled for each
	 * method
	 * @param elementName name of the element in the sourceanalysis.view.Scope
	 * which is sent to the template on each invocation
	 * @param scope the scope containing routines to be filtered and
	 * processed
	 * @throws ElementNotFoundException if the template referred to by
	 * templateName does not exist.
	 */
	public String refillPrivateMethods(String templateName,
		String elementName, Scope scope) throws ElementNotFoundException
	{
		return refillMethods(templateName, elementName, scope,
			 Specifiers.Visibility.PRIVATE);
	}

	/**
	 * Fills a template over and over again for all the members of
	 * a given collection.
	 * 
	 * @param collection the Collection containing the actual data
	 * @param elementKind the slot in the scope to use for filling,
	 * e.g. "CLASS" or "FUNCTION".
	 * @param elementTemplate the template to invoke upon every element
	 * @throws IOException
	 */
	protected void refillFromContainer(Collection collection,
			String elementKind, String elementTemplate)
		throws IOException
	{
		for (Iterator subjectiter = collection.iterator(); 
		     subjectiter.hasNext();) {
			Entity element = (Entity) subjectiter.next();
			// Create scope with current class
			sourceanalysis.view.Scope scope = new sourceanalysis.view.Scope();
			scope.declareMember(elementKind, element, true);
			// Fill template!
			try {
				m_output.write(m_templates.fillTemplate(elementTemplate,
						                                scope, this));
				m_output.flush();
			}
			catch (ElementNotFoundException e) {
				System.err.println("*** ERROR: template not found: " + e);			
			}
		}
	}
	
	/**
	 * Creates an output in which a section is generated for each class
	 * subject discovered by collect() or autocollect(). 
	 * 
	 * @param prefaceTemplate
	 * @param classTemplate
	 * @throws IOException
	 * @throws MissingInformationException
	 */
	protected void generateClassesSingleDocument(String prefaceTemplate,
		String classTemplate) throws IOException, MissingInformationException
	{
		TemplateBank.setInstance(m_templates);
		// Generate a preface
		sourceanalysis.view.Scope topscope = new sourceanalysis.view.Scope();
		topscope.declareMember("PACKAGE", m_program, true);
		try {
			m_output.write(m_templates.fillTemplate(prefaceTemplate,
					                                topscope, this));
		}
		catch (ElementNotFoundException e) {
			System.err.println("*** ERROR: failed to generate the preface:"
				+ e);		
		}
			
		List sorted_subjects = topologicallySortSubjects(false);

		refillFromContainer(sorted_subjects, "CLASS", classTemplate);
	}
	
	/**
	 * Creates an output in which a section is generated for each
	 * global function discovered by collect() or autocollect().
	 *  
	 * @param funcTemplate the name of the template 
	 * @throws IOException
	 * @throws MissingInformationException
	 */
	protected void generateFunctionsSingleDocument(String funcTemplate)
		throws IOException, MissingInformationException
	{
		refillFromContainer(m_globalFuncs, "FUNCTION", funcTemplate);
	}
	
	//@}

	/**
	 * @name Templates and Template Instantiation
	 */
	//@{
	
	/**
	 * Locates template instances which seem to be a "must" for the
	 * wrapping of the collected classes, and creates instances for
	 * them which are then added to the subjects list.
	 */
	public void investImplicitInstantiations()
	{
		final List<Aggregate> instanceList = new LinkedList<Aggregate>();		

		Traverse.TypeInformationVisitor instantiateVisitor =
			new Traverse.TypeInformationVisitor() {
				public void visit(Type typei) {
					try {
						Type.TypeNode root = typei.getRootNode();
						if (root != null)
							findTemplatesToInstantiate(root, instanceList, true);
					}
					catch (InappropriateKindException e) {
						/* corrupt type */
						System.err.println("*** WARNING: corrupt type " 
							+ typei);
					}
				}
			};
		
		// Find templates to instantiate in classes
		for (Aggregate subject: m_subjects) {
			if(!GenericFilters.isDeclared(subject)) {
				continue;
			}
			for (Iterator baseIter = subject.baseIterator();
				baseIter.hasNext(); ) {
				// Get bases
				InheritanceConnection connection = 
					(InheritanceConnection)baseIter.next();
				Aggregate base = connection.getBase();
				// Specialize templates
				if (base.isTemplated()) {
					TemplateArgument[] targs =
						connection.getBaseTemplateArguments();
					investThisTemplateInstantiation(base, targs, instanceList);
				}
			}
			// Traverse methods
			Traverse t = new Traverse();
			t.traverse(subject.getScope(), instantiateVisitor, false,
					Specifiers.Visibility.PUBLIC);
		}
		
		// Find templates to instantiate in global functions
		for (Iterator subjectIter = m_globalFuncs.iterator();
			subjectIter.hasNext(); ) {
			Routine subject = (Routine)subjectIter.next();
			// don't instantiate templates in functions
			// that weren't declared in headers
			if(!GenericFilters.isDeclared(subject)) {
				continue;
			}
			// Observe function
			if (!subject.isTemplated()) {
				Traverse t = new Traverse();
				t.traverse(subject, instantiateVisitor);
			}
		}

		// Find template instances which are the targets of typedefs
		for (Iterator typedefIter = m_typedefs.iterator();
			typedefIter.hasNext(); ) {
			Alias typedef = (Alias)typedefIter.next();
			// This disables instantiation of templates
			// in typedefs not declared in headers.
			// One could argue that it's useful, but it brings its
			// own share of issues
			if(!GenericFilters.isDeclared(typedef)) {
				continue;
			}
			try {
				findTemplatesToInstantiate(
					typedef.getAliasedType().getRootNode(),
					instanceList, false);
			}
			catch (InappropriateKindException e) {
				/* corrupt type */
				System.err.println("*** WARNING: corrupt type " 
					+ typedef.getAliasedType());
			}
		}
		
		m_subjects.addAll(instanceList);
		if (!instanceList.isEmpty())
			investImplicitInstantiations(); // repeat process
	}

	/**
	 * Commence a template instantiation. A template is only instantiated
	 * if it wasn't instantiated before, so no two identical instantiations
	 * my occur.
	 * @param template class template to instantiate
	 * @param arguments array of template arguments for instantiation
	 * @param instanceList list into which the new instance is inserted,
	 * if instantiation actually takes place.
	 */
	private void investThisTemplateInstantiation(Aggregate template,
		TemplateArgument[] arguments, List instanceList)
	{
		// 'expression' is used to uniquely describe the template
		// instantiation, so that we avoid multiple specialization
		// of the same template with the same arguments

		String expression = Utils.templateExpression(template, arguments);
		if (!m_instanceSet.contains(expression)) {
			try {
				Aggregate instance =
					Utils.instantiateTemplate(template, arguments, m_instanceMap);
				instanceList.add(instance);
			}
			catch (MissingInformationException e) {
				System.err.println("*** WARNING: implicit "
					+ "template instantiation of " + expression
					+ " failed: " + e);
			}
			catch (InappropriateKindException e) {
			}
			m_instanceSet.add(expression);
		}
	}

	/**
	 * Collects template instantiations occuring in a type expression.
	 * @param root root TypeNode of expression
	 * @param instanceList a List into which collected instances are added
	 * @param inhibitForeignTemplates if <b>true</b>, only instances of
	 * templates which are already members of <i>m_subjects</i> are invested;
	 * otherwise, every template instantation is invested regardless of the
	 * identity of the template.
	 */
	private void findTemplatesToInstantiate(Type.TypeNode root, 
		List instanceList, boolean inhibitForeignTemplates)
		throws InappropriateKindException
	{
		if (root.getKind() == Type.TypeNode.NODE_TEMPLATE_INSTANTIATION) {
			if (!Utils.isAccessible(root)) return ;
			
			Aggregate template = Utils.extractTemplate(root);
			TemplateArgument[] arguments = (TemplateArgument[])
				Utils.extractTemplateArguments(root)
					.toArray(new TemplateArgument[0]);
			// Look in supervised templates
			Aggregate supervised = 
				m_supervisedTemplates.getSupervised(template.getFullName());
			// Add instantiation
			if (supervised != null
			    || m_subjectTemplates.contains(template) 
				|| !inhibitForeignTemplates)
				investThisTemplateInstantiation(template, arguments, 
				                                instanceList);
		}
		else {
			Enumeration cen = root.children();
			while (cen.hasMoreElements()) {
				Object element = cen.nextElement();
				if (element instanceof Type.TypeNode) {
					findTemplatesToInstantiate((Type.TypeNode)element,
						instanceList, inhibitForeignTemplates);
				}
			}
		}
	}

	/**
	 * Finds enumerated types which are referenced in the known subjects.
	 */
	public void investImpliedEnums()
	{
		for (Aggregate subject: m_subjects) {
			Traverse tr = new Traverse();
			tr.traverse(subject.getScope(), new Traverse.TypeInformationVisitor()
			 { public void visit(Type typei) { findEnumsToAccomodate(typei); }
			 }, false, Specifiers.Visibility.PUBLIC);
		}
	}

	/**
	 * Detects enumerated types which can be added to the collection of
	 * wrapped enums based on a given type.
	 * @param type if 'type' refers to an enumerated type, and this type
	 * is accessible (public), it is added to the m_enums collection
	 */
	protected void findEnumsToAccomodate(Type type)
	{
		if (type.getRootNode() != null 
			 && type.isFlat() && type.getBaseType() instanceof sourceanalysis.Enum) {
			sourceanalysis.Enum base = (sourceanalysis.Enum)type.getBaseType();
			if (Utils.isAccessible(base) && !m_enums.contains(base))
				m_enums.add(base);
		}
	}

	/**
	 * Sorts the subjects in m_subjects such that no base class occurs
	 * after any class which derives it.
	 * @param considerInstantiations if set to <b>true</b>, the topological-
	 * sort will consider template instances as nodes in the graph; otherwise,
	 * each class template is consider as a single node regardless of how
	 * many instantiations of it exist. 
	 */
	protected List topologicallySortSubjects(boolean considerInstantiations)
	{
		class TopologicalNode
		{
			public TopologicalNode() { entryDegree = 0; exit = new LinkedList(); }
			public int entryDegree;
			public List exit;	
		};
		
		// Translate the inheritance information into a graph
		Map<Aggregate, TopologicalNode> bases = new HashMap<Aggregate, TopologicalNode>();
		
		for (Aggregate subject: m_subjects) {
			bases.put(subject, new TopologicalNode());
		}
		for (Aggregate subject: m_subjects) {
			for (Iterator baseIter = subject.baseIterator(); baseIter.hasNext();) {
				InheritanceConnection connection =
					(InheritanceConnection)baseIter.next();
				Aggregate base = (Aggregate)connection.getBase();
				// Find the specialization if one was generated
				if (base.isTemplated() && considerInstantiations) {
					String expression = Utils.templateExpression
						(base, connection.getBaseTemplateArguments());
					base = (Aggregate)m_instanceMap.get(expression);
				}
				// Update the topo-nodes
				if (base != null && bases.containsKey(base)) {
					((TopologicalNode)bases.get(base)).exit.add(subject);
					((TopologicalNode)bases.get(subject)).entryDegree++;
				}
			}
		}
		
		// Now,
		// Perform a basic topological-sort on the graph formed by the
		// TopologicalNode objects
		
		List sorted_subjects = new LinkedList();
		
		while (!bases.isEmpty()) {
			for (Iterator nodeIter = bases.keySet().iterator(); nodeIter.hasNext();) {
				Aggregate node = (Aggregate)nodeIter.next();
				TopologicalNode topo = (TopologicalNode)bases.get(node);
				// Find a node with a zero entry degree
				if (topo.entryDegree == 0) {
					// Remove node from graph including all edges connected
					// to it
					for (Iterator exitIter = topo.exit.iterator();
						exitIter.hasNext(); ) {
						TopologicalNode exitTarget =
							(TopologicalNode)bases.get(exitIter.next());
						exitTarget.entryDegree--;
					}
					bases.remove(node);
					// Add removed node to new subjects list
					sorted_subjects.add(node);
					break;
				}
			}
		}
		
		return sorted_subjects;
	}

	//@}	
	
	// Protected members
	protected ProgramDatabase m_program;
	protected File m_outputDirectory;
	protected Writer m_output;
	protected Set<Aggregate> m_subjects;
	protected Set m_subjectTemplates;
	protected Set m_enums;
	protected Set<Alias> m_typedefs;
	protected Set<Routine> m_globalFuncs;
	protected Set m_namespaces;
	protected TemplateBank m_templates;
	protected boolean m_separateClassTemplates;

	// Template-related
	protected Set m_instanceSet;
	protected Map m_instanceMap;

	protected SupervisedTemplates m_supervisedTemplates;	
}
