package sourceanalysis.dox;

import java.io.Reader;
import java.io.StringReader;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import sourceanalysis.Aggregate;
import sourceanalysis.Alias;
import sourceanalysis.ContainedConnection;
import sourceanalysis.DataTemplateParameter;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;
import sourceanalysis.Field;
import sourceanalysis.Group;
import sourceanalysis.InheritanceConnection;
import sourceanalysis.Macro;
import sourceanalysis.MissingInformationException;
import sourceanalysis.Namespace;
import sourceanalysis.Parameter;
import sourceanalysis.Primitive;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.Routine;
import sourceanalysis.Scope;
import sourceanalysis.SourceFile;
import sourceanalysis.Specifiers;
import sourceanalysis.TemplateArgument;
import sourceanalysis.TemplateEnabledEntity;
import sourceanalysis.TemplateParameter;
import sourceanalysis.Type;
import sourceanalysis.TypenameTemplateParameter;
import sourceanalysis.Type.TypeNode;
import sourceanalysis.dox.DocumentComponentRegistry.RequestedDocument;
import sourceanalysis.xml.XML;
import sourceanalysis.xml.XMLFormatException;
import antlr.RecognitionException;
import antlr.TokenStreamException;

/**
 * <p>Translates Doxygen(r) XML output files into sourceanalysis.* components.</p>
 * <p>A name of a directory containing XML files is passed to the object upon initialization.
 * References and cross-references are then satisfied using the files contained in that
 * directory. Interconnected components are finally represented by references in the
 * resulting program database.</p>
 * 
 * <h1>Using the Doxygen Analyzer</h1>
 * <h2>Preparing the Intermediate Document</h2>
 * <p>The first step is to run <tt>doxygen</tt> in order to generate the XML
 * (called <i>intermediate document</i> in this context). In order to do so,
 * create a Doxyfile describing the program - refer to Doxygen(r)'s online
 * documentation - then run:</p>
 * <tt>% doxygen</tt>
 * 
 * <h2>Acquiring the Program Database</h2>
 * <p>Construct an analyzer object:</p>
 * <tt>DoxygenAnalyzer analyzer = new DoxygenAnalyzer("...");</tt>
 * <p>Replace ... with the location of the intermediate document, optionally
 * suffixed by "/". This refers to the <b>directory</b> where all the XMLs
 * generated from the previous run of Doxygen(r) were created. This directory
 * always contains a file named <tt>index.xml</tt></p>
 * 
 * <p>To extract information, use:</p>
 * <tt>ProgramDatabase p = analyzer.processIndex();</tt>
 * <p>Which will invoke the translation algorithm and build a Program Database.
 * The analyzer will start by reading <tt>index.xml</tt>, accessing any
 * entities referred to by it, then recursively collecting any other entities
 * by following references from entities already encountered.
 * </p>
 * <p>Notice that only classes actively defined in your program will be added
 * to the global namespace; so if you are using <tt>std::string</tt>, you may
 * encounter an Entity representing it in type-expressions (see Type), but
 * going through namespaces and scopes you will see no recognition of it.
 * This is the desired behavior, since the user wants to refer to his/her own
 * classes and not proprietary and third-party classes. To access external
 * entities, use p.getExternals().
 * </p>
 */
public class DoxygenAnalyzer {

	/**
	 * Strings which are used in the XML processing and are externalized
	 * in case they ever change.
	 */
	private static class Tags
	{
		public static final String ID = "id";
		public static final String KIND = "kind";

		public static final String COMPOUND = "compound";
		public static final String FILE = "file";
		public static final String NAMESPACE = "namespace";
		public static final String CLASS = "class";
		public static final String STRUCT = "struct";
		public static final String UNION = "union";
		public static final String TYPEDEF = "typedef";
		public static final String FUNCTION = "function";
		public static final String VARIABLE = "variable";
		public static final String ENUM = "enum";
		public static final String SECTION = "section";
		public static final String MEMBER = "member";
		public static final String PARAM = "param";
		public static final String DEFINE = "define";
		public static final String USER_DEFINED = "user-defined";
		
		public static final String INNERNAMESPACE = "innernamespace";
		public static final String INNERCLASS = "innerclass";

		public static final String NAME = "name";
		public static final String DECLNAME = "declname";
		public static final String DEFNAME = "defname";
		public static final String COMPOUNDNAME = "compoundname";
		public static final String DEFINITION = "definition";
		public static final String HEADER = "header";
		public static final String ENUMVALUE = "enumvalue";
		public static final String BASECOMPOUNDREF = "basecompoundref";
		public static final String TEMPLATEPARAMLIST = "templateparamlist";
		public static final String CLASSTYPE = "class";
		public static final String TYPENAMETYPE = "typename";
		public static final String TYPE_ARRAYDIM = "array";
		public static final String FRIEND = "friend";
		
		public static final String BRIEFDESC = "briefdescription";
		public static final String DETAILEDDESC = "detaileddescription";
		public static final String PARA = "para";
		public static final String SIMPLESECT = "simplesect";
		public static final String TITLE = "title";
		public static final String EXCEPTIONS = "exceptions";
		public static final String PARAMETERLIST = "parameterlist";
		public static final String PARAMETERNAME = "parametername";
		public static final String PARAMETERDESCRIPTION 
			= "parameterdescription";
		
		public static final String LOCATION = "location";
		public static final String FILENAME = "file";
		public static final String LINE = "line";
		public static final String BODYFILE = "bodyfile";
		public static final String BODYLINE = "bodystart";
		
		public static final String REF = "ref";
		public static final String REFID = "refid";
		public static final String REFKIND = "kindref";
		public static final String EXTERNAL = "external";
		public static final String DEF = "def";
		public static String def(String kind) { return kind + DEF; }
		
		public static final String TYPE = "type";
		public static final String DEFVAL = "defval";
		public static final String INITIALIZER = "initializer";
		public static final String ARGSSTRING = "argsstring";
		
		public static final String PROT = "prot"; // access priveleges of member
		public static final String VIRT = "virt";
		public static final String STATIC = "static";
		public static final String CONST = "const";
		public static final String INLINE = "inline";
		public static final String EXPLICIT = "explicit";
		public static final String SPECIALIZED = "specialized";
		
		public static final String PUBLIC = "public";
		public static final String PROTECTED = "protected";
		public static final String PRIVATE = "private";
		
		public static final String VIRT_NORMAL = "normal";
		public static final String VIRT_NONE = "non-virtual";
		public static final String VIRT_VIRTUAL = "virtual";
		public static final String VIRT_PURE = "pure-virtual";
		
		public static final String YES = "yes";
		public static final String NO = "no";
	}
	
	/**
	 * Helps in the resolution of names during type-expression parsing. An
	 * instance of this class is passed to every call to the type-expression
	 * parser.
	 */
	class ScopefulResolution implements EntityNameResolving
	{
		ScopefulResolution(Map hints)
		{
			m_known = hints;
		}
		
		/**
		 * Finds an entity to go with a C++ base-name. First the local map
		 * is scanned, then the global map; the last resort is to create a new
		 * Aggregate with that name.
		 * @param name C++ name string from type-expression; may be
		 * nested such as std::vector but never templated
		 * @return Entity an entity with a proper name, never <b>null</b>
		 */
		public Entity resolve(String name)
		{
			Object hint = m_known.get(name);			// search hints
			if (hint != null) {
				return (Entity)hint;
			}
			else {
				hint = m_local_byname.get(name);		// search locals
				if (hint != null) {
					return (Entity)hint;
				}
				else {
					hint = m_global_byname.get(name);	// search globals
					if (hint != null) {
						return (Entity)hint;
					}
					else {
						// Nothing else to do - create new aggregate
						Aggregate a = new Aggregate();
						a.setName(name);  
						m_global_byname.put(name, a);
						return a;
					}
				}
			}
		}
		
		/**
		 * Finds a primitive entity by name. The global map is searched, and
		 * if the primitive requested is not found, a new one is created and
		 * inserted to the map.
		 * @param name C++ name for primitive
		 * @return Primitive corresponding entity
		 */
		public Primitive resolvePrimitive(String name)
		{
			Object hint = m_global_byname.get(name);
			if (hint != null) {
				return (Primitive)hint;
			}
			else {
				// Get from predefined primitives
				try {
					return Primitive.byName(name);
				}
				catch (ElementNotFoundException e) {
					// Hmm... a new primitive?
					Primitive p = new Primitive();
					p.setName(name);
					m_global_byname.put(name, p);
					return p;
				}
			}
		}
		
		private Map m_known;
	}
	
	/**
	 * This scope is used for source files, which do not contain a scope
	 * of their own, when translating the members.
	 * @see DoxygenAnalyzer.translateCompound().
	 */
	private class GlobalScope extends Scope
	{
        private Scope globals;

		GlobalScope(Entity owner, Scope globals) {
            super(owner);
            this.globals = globals;
        }
		public void addMember(Routine routine, int visibility,
			int virtuality, int storage)
		{
			// Add member to global scope instead
			globals.addMember(routine, visibility, virtuality, storage);
		}

		public void addMember(sourceanalysis.Enum enume, int visibility)
		{
			// Add member to global scope instead
			globals.addMember(enume, visibility);
		}
		
		public void addMember(Alias alias, int visibility)
		{
			// Add member to... you guessed it! global scope
			globals.addMember(alias, visibility);
		}
		
		public void addMember(Field field, int visibility, int storage)
		{
			m_globalFields_byname.put(field.getName(),
				new ContainedConnection(null, visibility, 
					Specifiers.DONT_CARE, storage, field));
		}
		
		public void addMember(Namespace inner)
		{
			globals.addMember(inner);
		}
		
		protected void mirrorRelationToMember(Entity contained, 
			ContainedConnection connection) {	}
	};

	/**
	 * DoxygenAnalyzer constructor.
	 */
	public DoxygenAnalyzer()
	{
		m_db = null;
		m_registry = new DocumentComponentRegistry();
		m_global_byname = new HashMap();
		m_local_byname = new HashMap();
		m_unfulfilled_declarations = new LinkedList();
		m_files_byname = new HashMap();
		m_routinesForRepair = new LinkedList();
		m_aliasesForRepair = new LinkedList();
		m_fieldsForRepair = new LinkedList();
		m_inheritanceForRepair = new LinkedList();
		m_globalFields_byname = new HashMap();
		logger = Logger.getLogger("sourceanalysis.dox");
	}
	
	/**
	 * DoxygenAnalyzer constructor; redirects input from a directory other
	 * than the default location.
	 * @param inputdir directory containing XML input files
	 */
	public DoxygenAnalyzer(String inputdir)
	{
		this();		
		m_registry.setInputDirectory(inputdir);
	}
	
	/*@}*/	

	/**
	 * @name Translation Algorithm
	 * Routines which keep busy at translating parts of the XML into data
	 * elements and Program Database Entities.
	 */
	/*@{*/
	
	/**
	 * Translates the name element from some XML component.
	 * The name can be stored in one of the following sub-nodes of the
	 * root node:
	 * <ul>
	 *  <li>&lt;name&gt;<i>name</i>&lt;/name&gt;</li>
	 *  <li>&lt;declname&gt;<i>name</i>&lt;/declname&gt;</li>
	 *  <li>&lt;defname&gt;<i>name</i>&lt;/defname&gt;</li>
	 * </ul>
	 * This function extracts the first one which has a value.
	 * @param xmlnode root of XML sub-tree
	 * @return String extracted name from either sub-node
	 * @throws XMLFormatException if no name subnode was found.
	 */
	public String translateName(Node xmlnode) throws XMLFormatException
	{
		String name;
		
		if (xmlnode.getNodeType() == Node.ELEMENT_NODE) {
			// Get Element node
			Element xmlelement = (Element)xmlnode;
			// Get the 'name', 'declname', 'defname' nodes (if any)
			Node plain = XML.subNode(xmlelement, Tags.NAME);
			Node decl = XML.subNode(xmlelement, Tags.DECLNAME);
			Node def = XML.subNode(xmlelement, Tags.DEFNAME);
			// Fetch the first of all the three
			if (plain != null)
				name = XML.collectText(plain);
			else if (decl != null)
				name = XML.collectText(decl);
			else if (def != null)
				name = XML.collectText(def);
			else
				throw new XMLFormatException("name node missing", xmlnode);
		}
		else
			throw new XMLFormatException("node is not an element", xmlnode);
		
		return normalizeName(name);
	}
	
	/**
	 * Removes redundant space after 'operator' keyword, so that
	 * "operator *" becomes "operator*" but "operator int" remains
	 * unchanged.
	 * @param name original name as it appears in the XML input
	 * @return normal-formed name.
	 */
	private String normalizeName(String name)
	{
		String optor = "operator ";
		if (name.startsWith(optor) && name.length() > optor.length()) {
			char l = name.charAt(optor.length());
			if (l == '_' || Character.isLetter(l))
				name = name.replace(" ", "");
		}
		return name;
	}
	
	/**
	 * Translates a literal into the enumerated value which is appointed to
	 * it from Specifiers.Visibility.
	 * <ul>
	 *  <li><tt>public</tt>: Specifiers.Visibility.PUBLIC</li>
	 *  <li><tt>protected</tt>: Specifiers.Visibility.PROTECTED</li>
	 *  <li><tt>private</tt>: Specifiers.Visibility.PRIVATE</li>
	 * </ul>
	 * @param literal a string (normally from the XML attribute) which
	 * describes the value
	 * @return int translated enumerated value from Specifiers.Visibility
	 * @throws XMLFormatException if the literal does not match any of the
	 * above.
	 */
	public int translateVisibility(String literal) throws XMLFormatException
	{
		if (literal.equals(Tags.PUBLIC)) 					// public
			return Specifiers.Visibility.PUBLIC;
		else if (literal.equals(Tags.PROTECTED))	// protected
			return Specifiers.Visibility.PROTECTED;
		else if (literal.equals(Tags.PRIVATE))			// private
			return Specifiers.Visibility.PRIVATE;
		else
			throw new XMLFormatException("invalid visibility: " + literal);
	}
	
	/**
	 * Translates a literal into the enumerated value which is appointed to
	 * it from Specifiers.Virtuality.
	 * <ul>
	 *  <li><tt>normal</tt>: Specifiers.Virtuality.NON_VIRTUAL</li>
	 *  <li><tt>virtual</tt>: Specifiers.Virtuality.VIRTUAL</li>
	 *  <li><tt>pure</tt>: Specifiers.Virtuality.PURE_VIRTUAL</li>
	 * </ul>
	 * @param literal a string (normally from the XML attribute) which
	 * describes the value
	 * @return int translated enumerated value from Specifiers.Virtuality
	 * @throws XMLFormatException if the literal does not match any of the
	 * above.
	 */
	public int translateVirtuality(String literal) throws XMLFormatException
	{
		if (literal.equals(Tags.VIRT_NORMAL) || literal.equals(Tags.VIRT_NONE))
			return Specifiers.Virtuality.NON_VIRTUAL;
		else if (literal.equals(Tags.VIRT_VIRTUAL))
			return Specifiers.Virtuality.VIRTUAL;
		else if (literal.equals(Tags.VIRT_PURE))
			return Specifiers.Virtuality.PURE_VIRTUAL;
		else
			throw new XMLFormatException("invalid virtuality: " + literal);
	}
	
	/**
	 * Translates a boolean literal referring to the storage type of a member
	 * into the enumerated value which is appointed to it from
	 * Specifiers.Storage.
	 * <ul>
	 *  <li><tt>yes</tt>: Specifiers.Storage.STATIC/CLASS_WIDE</li>
	 *  <li><tt>no</tt>: Specifiers.Storage.EXTERN/INSTANCE_OWN</li>
	 * </ul>
	 * @param literal a string (normally from the XML attribute) which
	 * describes the value
	 * @return int translated enumerated value from Specifiers.Storage
	 * @throws XMLFormatException if the literal does not match any of the
	 * above.
	 */
	public int translateStorage(String literal) throws XMLFormatException
	{
		if (literal.equals(Tags.YES))
			return Specifiers.Storage.STATIC;
		else if (literal.equals(Tags.NO))
			return Specifiers.Storage.EXTERN;
		else
			throw new XMLFormatException("invalid storage class: " + literal);
	}
	
	/**
	 * Translates a &lt;param&gt; element into a Parameter entity.
	 * @param xmlnode root of &lt;param&gt; sub-tree
	 * @return Parameter translated entity
	 * @throws XMLFormatException if the sub-tree's composition does not
	 * match expected schema.
	 */
	public Parameter translateParameter(Node xmlnode)
		throws XMLFormatException
	{
		Parameter param = new Parameter();
		// Translate parameter's name
		try {
			param.setName(translateName(xmlnode));
		}
		catch (XMLFormatException e) {
			/* parameters are allowed not to have a name */
		}
		// Translate type
		Node typenode = XML.subNode(xmlnode, Tags.TYPE);
		Node arrnode = XML.subNode(xmlnode, Tags.TYPE_ARRAYDIM);
		if (typenode == null)
			throw new XMLFormatException("param without type", xmlnode);
		try {
			param.setType(parseType(typenode, arrnode));
		}
		catch (XMLFormatException e) {
			System.err.println("*** WARNING: " + e);
			param.setType(new Type(null));
		}
		// Translate default
		Node dv = XML.subNode(xmlnode, Tags.DEFVAL);
		if (dv != null)
			param.setDefault(XML.collectText(dv));
			
		return param;
	}

	/**
	 * Translates a &lt;function&gt; element into a Routine entity. The
	 * parameters (&lt;param&gt; nodes) under the root node are examined
	 * and associated as parameters of this routine.
	 * @param xmlnode root of &lt;function&gt; sub-tree
	 * @return Routine translated entity
	 * @throws XMLFormatException if the sub-tree's composition does not
	 * match expected schema.
	 */
	public Routine translateRoutine(Node xmlnode) throws XMLFormatException
	{
		Routine routine = new Routine();
		// Translate name
		routine.setName(translateName(xmlnode));
		translateLocation(xmlnode, routine);
		// Translate template information
		translateTemplated(xmlnode, routine, null);
		// Translate const and inline attributes
		String constAttr = XML.attribute(xmlnode, Tags.CONST, Tags.NO);
		String inlineAttr = XML.attribute(xmlnode, Tags.INLINE, Tags.NO);
		String explicitAttr = XML.attribute(xmlnode, Tags.EXPLICIT, Tags.NO);
		routine.setConst(constAttr.equals(Tags.YES));
		routine.setInline(inlineAttr.equals(Tags.YES));
		routine.setExplicit(explicitAttr.equals(Tags.YES));
		// Get the definition
		Node definitionNode = XML.subNode(xmlnode, Tags.DEFINITION);
		if (definitionNode == null)
			throw new XMLFormatException("routine definition missing", xmlnode);
		String definition = XML.collectText(definitionNode);
		// Translate return type
		// [*] special care taken with conversion operators
		Type opConversion = parseConversionOperatorType(definition);
		if (opConversion == null) {
			Node typenode = XML.subNode(xmlnode, Tags.TYPE);
			if (typenode == null)
				throw new XMLFormatException("function without return type",
						xmlnode);
			routine.setReturnType(parseType(typenode));
		}
		else {
			routine.setName(Routine.OPERATOR_PREFIX + " "
					+ opConversion.formatCpp());
			routine.setReturnType(opConversion);
		}
		// Translate parameters
		Collection<Node> parameters = XML.subNodes(xmlnode, Tags.PARAM);
		for (Node pnode: parameters) {
			Parameter parameter = translateParameter(pnode);
			try {
				Type type = parameter.getType();
				// - add parameter to routine; ignore the "void" parameter, which
				//   may occur in "int func(void)". 
				if (type != null && type.getRoot() != null
					&& !(type.getRootNode().getKind() == Type.TypeNode.NODE_LEAF 
						 && type.getBaseType() == Primitive.VOID))
					routine.addParameter(parameter);
			}
			catch (MissingInformationException e) { }
		}
		// Translate exceptions
		translateExceptions(xmlnode, routine);

		// Translate description of routine and its parameters
		translateDescription(xmlnode, routine);

		m_routinesForRepair.add(routine);
		return routine;
	}
	
    public boolean dataMemberIsFunction(Node xmlnode) throws XMLFormatException {
        Node typenode = XML.subNode(xmlnode, Tags.TYPE);
        Node arrnode = XML.subNode(xmlnode, Tags.ARGSSTRING);
        if (typenode == null) {
            return false;
        }
        Type type = parseType(typenode, arrnode);
        if (!(type.getBaseType() instanceof Alias)) {
            return false;
        }
        Alias alias = (Alias)type.getBaseType();
        if (alias.getAliasedType().getRootNode().getKind() == TypeNode.NODE_FUNCTION) {
            return true;
        }
        return false;
    }
	
	/**
	 * Translates a data member node into a Field entity.
	 * @param xmlnode root of variable sub-tree
	 * @return Field translated entity
	 * @throws XMLFormatException if the sub-tree's composition does not
	 * match expected schema.
	 */
	public Field translateDataMember(Node xmlnode) throws XMLFormatException
	{
		Field data = new Field();
		// Translate name
		data.setName(translateName(xmlnode));
		translateDescription(xmlnode, data);
		translateLocation(xmlnode, data);
		// Translate type
		Node typenode = XML.subNode(xmlnode, Tags.TYPE);
		Node arrnode = XML.subNode(xmlnode, Tags.ARGSSTRING);
		if (typenode == null)
			throw new XMLFormatException("field with no type", xmlnode);
		data.setType(parseType(typenode, arrnode));
		// Translate initializer
		Node init = XML.subNode(xmlnode, Tags.INITIALIZER);
		Collection<Node> ctor = XML.subNodes(xmlnode, Tags.PARAM);
		if (init != null)
			data.setInitializer("=" + XML.collectText(init));
		else {
			// Add constructor arguments, comma-delimited
			StringBuffer construction = new StringBuffer();
			for (Node element: ctor) {
				if (construction.length() > 0) construction.append(",");
				construction.append(XML.collectText(element));
			}
			data.setInitializer("(" + construction.toString() + ")");
		}
		
		m_fieldsForRepair.add(data);
		
		return data;
	}
	
	/**
	 * Translates a typedef entry from the XML file into an Alias entity.
	 * @param xmlnode root of typedef specification sub-tree
	 * @return Alias translated entity
	 * @throws XMLFormatException if the sub-tree's composition does not
	 * match expected schema.
	 */
	public Alias translateTypedef(Node xmlnode) throws XMLFormatException
	{
		Alias typedef = new Alias();
		// Translate the name
		typedef.setName(translateName(xmlnode));
		selfSubscribe(xmlnode, typedef);
		rememberByName(typedef);
		// Translate properties
		translateDescription(xmlnode, typedef);
		translateLocation(xmlnode, typedef);
		logger.log(Level.INFO, "translating typedef " + typedef.getName());
		// Get the real type
		Node typenode = XML.subNode(xmlnode, Tags.TYPE);
		Node arrnode = XML.subNode(xmlnode, Tags.ARGSSTRING);
		if (typenode == null)
			throw new XMLFormatException("typedef " + typedef.getName() +
				" has no actual type.", xmlnode);
		typedef.setAliasedType(parseType(typenode, arrnode));
		
		m_aliasesForRepair.add(typedef);
		
		return typedef;
	}
	
	/**
	 * Translates a namespace, class, struct, union, or source file node
	 * into a Namespace, Aggregate, or SourceFile entity.
	 * @param xmlnode root of compound sub-tree
	 * @param entityClass the Entity to generate, takes either "Namespace",
	 * "Aggregate", or "SourceFile"
	 * @param isExternal 'true' if the compound being translated occurs as
	 * an external reference (this only has any effect when entityClass is
	 * "SourceFile")
	 * @throws XMLFormatException if the sub-tree's composition does not
	 * match expected schema.
	 */
	public Entity translateCompound(Node xmlnode, String entityClass, 
			boolean isExternal) throws XMLFormatException
	{
		Entity compound = null;
		Scope scope = null;
		// Create an entity for 'compound'
		if (entityClass.equals("Namespace")) {
			Namespace ns = new Namespace();
			compound = ns;
			scope = ns.getScope();
		}
		else if (entityClass.equals("Aggregate")) {
			Aggregate ag = new Aggregate();
			compound = ag;
			scope = ag.getScope();
		}
		else if (entityClass.equals("SourceFile")) {
			SourceFile sf = new SourceFile();
			compound = sf;
			/*
			 * For source files - a phony scope is created to put members in
			 * global namespace (a SourceFile is not really a container).
			 */
			scope = new GlobalScope(compound,
					isExternal ? m_db.getExternals() 
					           : m_db.getGlobalNamespace().getScope());
		}
		
		selfSubscribe(xmlnode, compound);
		translateLocation(xmlnode, compound);
		translateDescription(xmlnode, compound);
		
		// Translate the name of the entity
		Node namenode = XML.subNode(xmlnode, Tags.COMPOUNDNAME);
		if (namenode == null)
			throw new XMLFormatException("no compoundname", xmlnode);
		else
			compound.setName(XML.collectText(namenode));

		rememberByName(compound);
		
		logger.log(Level.INFO, "translating compound " + compound.getName());
		
		// Translate template information
		Map previous_locals = m_local_byname;
		(m_local_byname = new HashMap()).putAll(previous_locals);
		if (compound instanceof TemplateEnabledEntity) {
			TemplateEnabledEntity enabled = (TemplateEnabledEntity)compound;
			translateTemplated(xmlnode, enabled,scope);
		}
		
		// Go through inner classes and namespaces
		translateInnerCompounds(xmlnode, compound, scope, Tags.INNERCLASS);
		translateInnerCompounds(xmlnode, compound, scope, Tags.INNERNAMESPACE);
		
		// Go through sections in the declaration
		Collection<Node> sections = XML.subNodes(xmlnode, Tags.def(Tags.SECTION));
		for (Node section: sections) {
			Group group = translateGroup(section, scope);
			// Go through members defined in this section
			Collection<Node> members = XML.subNodes(section, Tags.def(Tags.MEMBER));
			for (Node mbr: members) {
				// Get the attributes
				String mid = XML.attribute(mbr, Tags.ID, null);		// member ID
				String mkind = XML.attribute(mbr, Tags.KIND, null);
				String access = XML.attribute(mbr, Tags.PROT, null);
				String virt = XML.attribute(mbr, Tags.VIRT, null);
				String stat = XML.attribute(mbr, Tags.STATIC, null);
				String spec = XML.attribute(mbr, Tags.SPECIALIZED, null);
				// Skip non-global members in source file entities
				if (scope instanceof GlobalScope && mid.startsWith("namespace"))
					continue;
				// Skip specialization entries (since Doxygen 1.4.0)
				if (spec != null && spec.equals(Tags.YES))
					continue;
				// Check if entity has already been read before (as a result
				// of reference resolutions)
				Entity pre = null;
				boolean hasPre = false;
				if (mid != null && mkind != null) {
					try {
						pre = m_registry.locate(makeLocatorFromNode(mbr));
						hasPre = true;
					}
					catch (ElementNotFoundException e) {
						/* leave original values of pre and hasPre */
					}
				}
				try {
					// Decide which entity is translated
					if (mkind.equals(Tags.FUNCTION)) {    /* function member */
						Routine emember = hasPre ? (Routine)pre : translateRoutine(mbr);
						if (compound instanceof Aggregate && !emember.isTemplated()
								&& XML.subNode(mbr, Tags.TEMPLATEPARAMLIST) != null) {
							continue; // doxygen template specialization bug workaround
						}
						scope.addMember(emember, translateVisibility(access),
							translateVirtuality(virt), translateStorage(stat));
						if (group != null)
							group.getScope().addMember(emember, translateVisibility(access),
								translateVirtuality(virt), translateStorage(stat));
					}
					else if (mkind.equals(Tags.VARIABLE)) {	/* data member */
					    // This code here, discards functions that look to doxygen (and/or griffin) as they
					    // were variables. In the future, we want to wrap this ones as functions.
					    if (dataMemberIsFunction(mbr)) {
					        continue;
					    }
						Field emember = hasPre ? (Field)pre : translateDataMember(mbr);
						scope.addMember(emember, translateVisibility(access),
							translateStorage(stat));
						if (group != null)
							group.getScope().addMember(emember, translateVisibility(access),
								translateStorage(stat));
					}
					else if (mkind.equals(Tags.TYPEDEF)) { /* typedef member */
						Alias emember = hasPre ? (Alias)pre : translateTypedef(mbr);
						scope.addMember(emember, translateVisibility(access));
						if (group != null) 
							group.getScope().addMember(emember, translateVisibility(access));
					}
					else if (mkind.equals(Tags.ENUM)) { /* enum member */
						sourceanalysis.Enum emember = hasPre ? (sourceanalysis.Enum)pre : translateEnum(mbr);
						scope.addMember(emember, translateVisibility(access));
						if (group != null)
							group.getScope().addMember(emember, translateVisibility(access));
					}
					else if (mkind.equals(Tags.DEFINE)) { /* macro in SourceFile */
						if (hasPre) translateDefine(mbr); // - currently unused
					}
					else if (mkind.equals(Tags.FRIEND)) { /* friend function in Aggregate */
						// - make sure friend is a function
						Node typenode = XML.subNode(mbr, Tags.TYPE);
						String typeText = XML.collectText(typenode);
						if (typeText.indexOf(Tags.CLASSTYPE) == -1) {
							// Get friend entity and create connection
							Routine emember = hasPre ? (Routine)pre : translateRoutine(mbr);
							scope.addFriend(emember);
							if (group != null)
								group.getScope().addFriend(emember);
							adjustFriend(compound, emember, isExternal);
						}
					}
					/* Any other members which might occur in the XML are
					 * ignored. */
				}
				catch (XMLFormatException e) {
					System.err.println("*** WARNING: corrupted compound member: "
						+ e);
				}
			}
		}
		
		// Translate inheritance information
		Collection<Node> basenodes = XML.subNodes(xmlnode, Tags.BASECOMPOUNDREF);
		for (Node basenode: basenodes) {
			// Get the ID of the base compound
			String bid = XML.attribute(basenode, Tags.REFID, null);
			if (bid == null)
				throw new XMLFormatException("base without refid", basenode);
			String vis = XML.attribute(basenode, Tags.PROT, Tags.PRIVATE);
			// Follow reference to base
			DocumentComponentRegistry.EntityLocator baseloc =
				makeLocatorFromNode(basenode, "compound");
			try {
				Entity base = followReference(baseloc);
				// Create link to base
				if (base instanceof Aggregate &&
						compound instanceof Aggregate) {
					Aggregate derived = (Aggregate)compound;
					InheritanceConnection inheritance;
					
					if (base.isTemplated()) {
						// Remember template arguments if they are needed
						TemplateArgument[] targs =
							parseType(basenode).getTemplateArguments();
						inheritance =
							derived.addBase((Aggregate)base, targs,
							                translateVisibility(vis));
						// - remember to update inheritance later
						m_inheritanceForRepair.add(inheritance);
					}
					else {
						inheritance =
							derived.addBase((Aggregate)base,
							                translateVisibility(vis));
					}
				}
				else {
					System.err.println("*** WARNING: non-aggregates " +
						"participating in inheritance relationship: ");
					System.err.println("base: " + base.getFullName() +
						", derived: " + compound.getFullName());
				}
			}
			catch (ElementNotFoundException e) {
				System.err.println("*** WARNING: Cannot locate base class of "
					+ compound.getFullName() + ": " + e);
			}
		}
		
		m_local_byname = previous_locals;
		
		return compound;
	}

	/**
	 * Reads a &lt;templateparamlist&gt; tag, if one exists, and adds template
	 * parameters to the translated entity.
	 * @param xmlnode compound element's XML root node
	 * @param entity a translated entity into which template parameters should
	 * be inserted
	 * @param scope a Scope object to insert template parameters as inner
	 * classes in
	 * @throws XMLFormatException if the tag format underneath the node
	 * is unexpected. If the node does not contain a &lt;templateparamlist&gt;
	 * tag, nothing is done and no exception is thrown.
	 */
	private void translateTemplated(Node xmlnode,
		TemplateEnabledEntity entity, Scope scope) throws XMLFormatException
	{
		Node list = XML.subNode(xmlnode, Tags.TEMPLATEPARAMLIST);
		// If any list is present, read parameters
		if (list != null) {
			if (scope == null) {
				scope = new Scope(entity);
			}
			Collection<Node> params = XML.subNodes(list, Tags.PARAM);
			for (Node paramnode: params) {
				// Translate param
				TemplateParameter param =
					translateTemplateParameter(paramnode, scope);
				entity.addTemplateParameter(param);
			}
		}
	}
	
	/**
	 * Reads a template parameter, producing a corresponding TemplateParameter
	 * entity (which may actually be either DataTemplateParameter or
	 * TypenameTemplateParameter).
	 * @param xmlnode parameter root
	 * @param scope for typename parameters, created nested in this scope
	 * @return TemplateParameter translated entity
	 * @throws XMLFormatException if the XML format does not match
	 * expected schema.
	 */
	private TemplateParameter translateTemplateParameter(Node xmlnode,
		Scope scope) throws XMLFormatException
	{
		// Get Type
		Node typenode = XML.subNode(xmlnode, Tags.TYPE);
		if (typenode == null)
			throw new XMLFormatException("parameter without type", xmlnode);
		String typeText = XML.collectText(typenode);
		// Translate name
		String name;
		if ((name = wordWithPrefix(typeText, Tags.CLASSTYPE)) != null)
			typeText = Tags.CLASSTYPE;
		else if ((name = wordWithPrefix(typeText, Tags.TYPENAMETYPE)) != null)
			typeText = Tags.TYPENAMETYPE;
		else
			name = translateName(xmlnode);
		// Branch according to type
		if (typeText.equals(Tags.CLASSTYPE) || typeText.equals(Tags.TYPENAMETYPE)) {
			// Translate as typename template argument
			Aggregate nested = new Aggregate();
			nested.setName(name);
			scope.addMember(nested, Specifiers.Visibility.PRIVATE);
			// - create template parameter entity
			TypenameTemplateParameter param = new TypenameTemplateParameter();
			param.setName(name);
			param.associate(nested);
			m_local_byname.put(name, nested);
			// - try to read a default value
			Node defnode = XML.subNode(xmlnode, Tags.DEFVAL);
			if (defnode != null) {
				Type def = parseType(defnode);
				param.setDefault(def);
			}
			return param;
		}
		else {
			// Translate as data template argument
			DataTemplateParameter param = new DataTemplateParameter();
			param.setName(name);
			// - set the type
			param.setType(parseType(typenode));
			// - try to read a default value
			Node defnode = XML.subNode(xmlnode, Tags.DEFVAL);
			if (defnode != null) {
				param.setDefault(XML.collectText(defnode));
			}
			return param;
		}
	}

	/**
	 * Interprets the attributes of a section and retrieves (or creates) the
	 * corresponding group in the compound entity.
	 * @param xmlnode section node
	 * @param compound scope of compound to find group in or add group 
	 * into.
	 * @return Group the group found or created; the referenced section may
	 * not denote a group, in which case the returned value is <b>null</b>.
	 * @throws XMLFormatException if the format of the section is corrupt
	 */
	private Group translateGroup(Node xmlnode, Scope compound)
		throws XMLFormatException
	{
		String kind = XML.attribute(xmlnode, Tags.KIND, null);
		if (kind == null)
			throw new XMLFormatException("sectiondef with no kind", xmlnode);
		// Check whether section is a user defined section (== group)
		if (kind.equals(Tags.USER_DEFINED)) {
			// Read group's name
			Node header = XML.subNode(xmlnode, Tags.HEADER);
			String headerstring = (header == null) ? 
					"(anonymous)" : XML.collectText(header);
			String[] headers = headerstring.split("!");
			// Look for group in scope
			Scope groupscope = compound;
			Group group = null;
			
			for (int hsi = 0; hsi < headers.length; hsi++) {
				String topic = headers[hsi];
				// Descend into group tree using 'topic' as a key
				try {
					group = groupscope.groupByName(topic);
				}
				catch (ElementNotFoundException e) {
					// Create a new group
					group = new Group();
					group.setName(topic);
					// Add new group to scope and return it
					groupscope.addGroup(group);
				}
				groupscope = group.getScope();
			}
			return group;
		}
		else
			return null;
	}

	/**
	 * Reads inner compound constructs of a compound. The inner constructs
	 * (which may be classes or namespaces) are translated into Entity objects
	 * and added to the scope of the containing compound.
	 * @param xmlnode root node of compound in XML document
	 * @param compound compound object's (half-)translated Entity
	 * @param scope compound object's scope
	 * @param tag XML element name to search for; can be one of:
	 * <tt>Tags.INNERCLASS</tt>, <tt>Tags.INNERNAMESPACE</tt>
	 * @throws XMLFormatException if XML comprehension problems occur
	 * during the translation of any of the inner compounds.
	 */
	private void translateInnerCompounds(Node xmlnode, Entity compound,
		Scope scope, String tag) throws XMLFormatException
	{
		Collection<Node> innerclasses = XML.subNodes(xmlnode, tag);
		for (Node innerclass: innerclasses) {
			// Follow reference to aggregate
			DocumentComponentRegistry.EntityLocator locator =
				makeLocatorFromNode(innerclass, "compound");
			String desc = "inner class " + locator.getComponentID()
				+ " of compound " + compound.getName();
			try {
				Entity innerEntity = followReference(locator);
				trimName(innerEntity, compound);
				// - filter out inner entities that have a name starting with "@"
				boolean anonymous = (innerEntity.getName().charAt(0) == '@');
				// Add aggregate or namespace as member
				if (!innerEntity.hasContainer() && !anonymous) {
					if (innerEntity instanceof Aggregate) {
						String vis = XML.attribute(innerclass, Tags.PROT, "public");
						scope.addMember((Aggregate)innerEntity, 
								translateVisibility(vis));
					}
					else if (innerEntity instanceof Namespace) {
						scope.addMember((Namespace)innerEntity);
					}
					else {
						System.err.println("*** WARNING: " + desc +
							" has an unexpected class '" + 
							innerEntity.getClass().getName() + "'; dropped.");
					}
				}
			}
			catch (XMLFormatException e) {
				System.err.println("*** WARNING: " + desc + 
					" - analysis failed; reason: " + e);
			}
			catch (ElementNotFoundException e) {
				System.err.println("*** WARNING: " + desc + 
					" - analysis failed; reason: " + e);
			}
		}
	}

	/**
	 * Reads exception classes as they appear in the function's throw()
	 * clause in C++ (or throws() in Java) and adds the appropriate
	 * connection to the routine.
	 * @param xmlnode root node for the routine entity
	 * @param routine the routine entity itself
	 */
	private void translateExceptions(Node xmlnode, Routine routine)
	{
		Node exceptions = XML.subNode(xmlnode, Tags.EXCEPTIONS);
		// Analyze exceptions node
		if (exceptions != null) {
			routine.setThrows(true);
			// - collect 'ref' nodes
			Collection<Node> refs = XML.subNodes(exceptions, Tags.REF);
			for (Node element: refs) {
				try {
					Entity exctype = followReference(element);
					if (exctype instanceof Aggregate)
						routine.addThrows((Aggregate)exctype);
				}
				catch (ElementNotFoundException e) {
					logger.warning("// could not resolve reference in throw()"
							+ " clause of " + routine.getFullName() 
							+ ": " + e);
				}
				catch (XMLFormatException e) {
					logger.warning("// error in exception class thrown from "
							+ routine.getFullName() + ": " + e);					
				}
			}
		}
	}
	
	/**
	 * Reads an enumerated type into an Enum entity.
	 * @param xmlnode root node of enum in XML document
	 * @return Enum translated entity
	 * @throws XMLFormatException if the sub-tree's composition does not
	 * match expected schema.
	 */
	public sourceanalysis.Enum translateEnum(Node xmlnode) throws XMLFormatException
	{
		sourceanalysis.Enum enume = new sourceanalysis.Enum();
		// Translate name
		enume.setName(translateName(xmlnode));
		// Translate documentation
		translateLocation(xmlnode, enume);
		translateDescription(xmlnode, enume);
		// Add to document/component registry
		selfSubscribe(xmlnode, enume);
		rememberByName(enume);
		
		// Read constants
		Collection<Node> valueNodes = XML.subNodes(xmlnode, Tags.ENUMVALUE);
		int valueCandidate = 0;
		for (Node valueNode: valueNodes) {			
			// - look for an initializer
			Node initializerNode = XML.subNode(valueNode, Tags.INITIALIZER);
			if (initializerNode != null) {
				try {
					valueCandidate = Integer.parseInt(XML.collectText(initializerNode));
				}
				catch (NumberFormatException e) { 
					/* skip enumerated value */
				}
			}
			// - add constant to enum
			enume.introduceConstant(
				new sourceanalysis.Enum.Constant(translateName(valueNode), valueCandidate));
			valueCandidate++;
		}
		
		return enume;
	}

	/**
	 * Reads the definition of a macro.
	 * @param xmlnode root node of "define" member in XML document.
	 * @return Macro translated entity
	 * @throws XMLFormatException if the sub-tree's composition does not
	 * match expected scheme.
	 */
	public Macro translateDefine(Node xmlnode) throws XMLFormatException
	{
		Macro macro = new Macro();
		// Translate name
		macro.setName(translateName(xmlnode));
		// Translate documentation
		translateDescription(xmlnode, macro);
		// Translate preprocessing parameters
		Collection<Node> paramnodes = XML.subNodes(xmlnode, Tags.PARAM);
		for (Node paramnode: paramnodes) {
			// Create the preprocessing parameter
			String prepParamName = translateName(paramnode);
			macro.addPreprocessingParameter(prepParamName);
		}
		
		return macro;
	}
	
	/**
	 * Translates the documentation section of the component.
	 * The brief description and the detailed description are inserted as
	 * properties "brief" and "description".
	 * @param xmlnode root node for entity
	 * @param entity an entity to which properties are inserted
	 */
	public void translateDescription(Node xmlnode, Entity entity)
	{
		// Get the brief description
		Node briefnode = XML.subNode(xmlnode, Tags.BRIEFDESC);
		if (briefnode != null) {
			Node par = XML.subNode(briefnode, Tags.PARA);
			if (par != null)
				entity.addProperty(new Entity.Property("brief", XML.copyText(par)));
		}
		// Get the detailed description
		Node detailednode = XML.subNode(xmlnode, Tags.DETAILEDDESC);
		if (detailednode != null) {
			Collection<Node> pars = XML.subNodes(detailednode, Tags.PARA);
			// Collect text from all <para> nodes
			StringBuffer paragraph = new StringBuffer();
			for (Node par: pars) {
				// Try to retrieve section
				NodeList descnodes = par.getChildNodes();
				for (int si = 0; si < descnodes.getLength(); ++si) {
					Node fragment = descnodes.item(si);
					// Extract Properties to entity from <simplesect> nodes
					if (fragment.getNodeType() == Node.ELEMENT_NODE &&
						fragment.getNodeName().equals(Tags.SIMPLESECT)) {
						// The <title> node contains property name, whereas the
						// text which is to be the value of the property is placed
						// in a <para> element.
						Node titlenode = XML.subNode(fragment, Tags.TITLE);
						Node subparnode = XML.subNode(fragment, Tags.PARA);
						if (titlenode != null && subparnode != null) {
							entity.addProperty(new Entity.Property(
								XML.collectText(titlenode), XML.copyText(subparnode)));
						}
					}
					else if (fragment.getNodeType() == Node.ELEMENT_NODE &&
						fragment.getNodeName().equals(Tags.PARAMETERLIST)) {
						// Translate parameter descriptions
						if (entity instanceof Routine)
							translateParameterDescriptions(fragment,
									                       (Routine)entity);
					}
					else {
						// Other nodes are added as just text
						if (fragment.getNodeType() == Node.TEXT_NODE)
							paragraph.append(fragment.getNodeValue());
						else
							paragraph.append(XML.copyText(fragment, true));
					}
				}
			}
			if (paragraph.length() > 0)
				entity.addProperty(
					new Entity.Property("description", paragraph.toString()));
		}
	}
	
	private void translateParameterDescriptions(Node xmlnode, Routine routine)
	{
		// Here, <parametername> nodes are translated into
		// descriptions to the parameters of the entity (if
		// it is a routine); translate them.
		NodeList paramnodes = xmlnode.getChildNodes();
		String current = null;
		
		for (int pi = 0; pi < paramnodes.getLength(); ++pi) {
			Node paramnode = paramnodes.item(pi);
			if (paramnode.getNodeType() == Node.ELEMENT_NODE) {
				if (paramnode.getNodeName().equals(Tags.PARAMETERNAME))
					current = XML.collectText(paramnode);
				else if (paramnode.getNodeName()
						            .equals(Tags.PARAMETERDESCRIPTION)
						&& current != null) {
					// - get the description
					String desc = XML.collectText(paramnode);
					// - find a parameter baring the name 'current' and add
					//   the description to it
					for (Iterator rpi = routine.parameterIterator(); 
						rpi.hasNext(); ) {
						Parameter param = (Parameter)rpi.next();
						if (param.getName().equals(current)) {
							param.addProperty(
								new Entity.Property("description", desc));
						}
					}
				}
			}
		}
	}
	
	/**
	 * Reads the location of some entity from the XML and connects the
	 * entity with the approperiate source file using a DeclDefConnection.
	 * @param xmlnode root of XML subtree containing entity
	 * @param entity translated entity
	 * @throws XMLFormatException if the sub-tree's composition does not match
	 * expected schema. The location attribute may be missing altogether, in
	 * which case nothing is done and no exception is thrown.
	 */
	public void translateLocation(Node xmlnode, Entity entity)
		throws XMLFormatException
	{
		Node location = XML.subNode(xmlnode, Tags.LOCATION);
		// Analyze location node
		if (location != null) {
			String file = XML.attribute(location, Tags.FILENAME, null);
			String at = XML.attribute(location, Tags.LINE, null);
			String bodyfile = XML.attribute(location, Tags.BODYFILE, null);
			String bodyat = XML.attribute(location, Tags.BODYLINE, null);
			if (file == null || (at == null && !(entity instanceof SourceFile)))
				throw new XMLFormatException("incomplete location", xmlnode);
			// Store information
			if (entity instanceof SourceFile) {
				// Insert file to map by-name
				m_files_byname.put(file, entity);
				((SourceFile)entity).setFullPath(file);
			}
			else {
				// Prepare an (unfulfilled) connection
				SourceFile.Position declposition =
						new SourceFile.Position(Integer.parseInt(at));
				entity.setDeclarationAt(file, declposition);
				m_unfulfilled_declarations.add(entity.getDeclaration());
				// Set the location of the definition (fulfilled)
				if (bodyfile != null) {
					if (bodyat == null) bodyat = at;
					SourceFile.Position defposition =
						new SourceFile.Position(Integer.parseInt(bodyat));
					entity.setDefinitionAt(bodyfile, defposition);
					m_unfulfilled_declarations.add(entity.getDefinition());
				}
			}
		}
	}
	
	/**
	 * Translate a generic node; the decision which of the parts of the
	 * translation algorithm to put to action is based on the value of the
	 * 'kind' attribute.
	 * @param xmlnode root node of entity subtree
	 * @param isExternal 'true' indicates that the entity being translated 
	 * belongs to an external reference (this only has any effect when this
	 * entity is a source file - i.e. kind is Tags.FILE) 
	 * @throws XMLFormatException if a 'kind' attribute is missing or invalid,
	 * or the sub-tree's composition does not match expected schema.
	 */
	public Entity translationSwitch(Node xmlnode, boolean isExternal) throws XMLFormatException
	{
		// Get kind
		String kind = XML.attribute(xmlnode, Tags.KIND, null);
		if (kind == null) {
			throw new XMLFormatException("no kind mentioned", xmlnode);
		}
		else if (kind.equals(Tags.NAMESPACE)) {
			return translateCompound(xmlnode, "Namespace", isExternal);
		}
		else if (kind.equals(Tags.CLASS) || kind.equals(Tags.STRUCT) 
			|| kind.equals(Tags.UNION)) {
			return translateCompound(xmlnode, "Aggregate", isExternal);
		}
		else if (kind.equals(Tags.FUNCTION)) {
			return translateRoutine(xmlnode);
		}
		else if (kind.equals(Tags.TYPEDEF)) {
			return translateTypedef(xmlnode);
		}
		else if (kind.equals(Tags.VARIABLE)) {
			return translateDataMember(xmlnode);
		}
		else if (kind.equals(Tags.ENUM)) {
			return translateEnum(xmlnode);
		}
		else if (kind.equals(Tags.FILE)) {
			return translateCompound(xmlnode, "SourceFile", isExternal);
		}
		else {
			// Invalid or unrecognized kind
			throw new XMLFormatException("invalid kind: " + kind, xmlnode);
		}
	}
	
	/**
	 * Reads a type-expression from a C++ string representation into a tree representation.
	 * @param expr C++ string for type
	 * @param names a mapping from textual names to Entities
	 * @return Type translated into type-expression form; if the expression
	 * is empty, the returned value is Type(<b>null</b>).
	 * @throws XMLFormatException if the string is malformed.
	 */
	public Type parseType(String expr, Map names) throws XMLFormatException
	{
		if (expr.length() == 0) return new Type(null);
		if (expr.equals("virtual")) return new Type(null); /* bug workaround */
		
		// Build the parser
		Reader in = new StringReader(expr);
		antlr.CharScanner lexer = new TypeExpressionLexer(in);
		TypeExpressionParser parser = new TypeExpressionParser(lexer);
		parser.assignYellowPages(new ScopefulResolution(names));
		
		// Parse expression and return resulting type (as expression tree)
		try {
			Type translated = new Type(parser.typeexpr());
			translated.normalize();
			if (parser.errorOccurred())
				System.err.println("*** WARNING: error in type-expression '" +
					expr + "': " + parser.getErrorMessages());
			return translated;
		}
		catch (TokenStreamException e) {
			throw new XMLFormatException("error in type-expression " + expr);
		}
		catch (RecognitionException e) {
			throw new XMLFormatException("error in type-expression " + expr);
		}
	}
	
	/**
	 * Reads a type-expression from an XML node.
	 * Normally, the text of the node is taken and parsed, however, if it has
	 * any XML &lt;ref&gt; tags they are first extracted and the proper
	 * references saved in a dictionary so that they can be used later on.
	 * @param xmlnode root node of XML fragment containing type
	 * @param arrnode an optional node containing array dimension information.
	 * If <b>null</b>, it is assumed that this is not an array type.
	 * @return Type translated into type-expression form
	 * @throws XMLFormatException if the text in the node is malformed.
	 */
	public Type parseType(Node xmlnode, Node arrnode) throws XMLFormatException
	{
		// Step 1. Collect references from node
		Map reference_map = new HashMap();
		collectReferences(xmlnode, reference_map);
		// Step 2. Actually parse text in node
		String exprtext = XML.collectText(xmlnode);
		if (arrnode != null) {
			String arrtext = XML.collectText(arrnode);
			if (arrtext.startsWith("[") || arrtext.startsWith(")"))
				exprtext += arrtext;
			// This is most probably a function typedef
			if (arrtext.startsWith("(") && arrtext.endsWith(")") && !exprtext.endsWith("*")) {
				return new Type(new TypeNode(TypeNode.NODE_FUNCTION));
			}
		}
		return parseType(exprtext, reference_map);
	}
	
	public Type parseType(Node xmlnode) throws XMLFormatException
	{
		return parseType(xmlnode, null);
	}
	
	/**
	 * Comprehens the type involved in a conversion operator, parses it and
	 * returns a type-expression.
	 * @param operatorName operator method definition string, e.g.
	 * MyClass::operator long.
	 * @return conversion target type as a Type instance. If the name passed
	 * is obviously not the name of a conversion operator, <b>null</b> is
	 * returned.
	 * @throws XMLFormatException
	 */
	public Type parseConversionOperatorType(String operatorDef)
		throws XMLFormatException
	{
		final String prefix = Routine.OPERATOR_PREFIX + " ";
		// Look for the operator prefix inside the definition string
		int opIndex = operatorDef.indexOf(prefix);
		int typeIndex = opIndex + prefix.length();
		
		if (opIndex >= 0 &&
			Character.isLetter(operatorDef.charAt(typeIndex))) {
			// Parse type name after prefix
			String typename = operatorDef.substring(typeIndex);
			return parseType(typename, new HashMap());
		}
		else
			return null;
	}
	
	/*@}*/
	
	/**
	 * @name Overall
	 */
	/*@{*/
	
	/**
	 * Extracts all the elements in the index and adds them to the anonymous
	 * namespace.
	 * @param global a scope to put translated entities in
	 * @param index the index XML document
	 * @param isExternal 'true' indicates that all compound entities analyzed
	 *   should have the external flag set
	 * @throws ElementNotFoundException if the index document is
	 * unavailable.
	 */
	public void processIndex(Scope global, Document index, boolean isExternal)
			throws ElementNotFoundException
	{
		Node indexRoot = index.getFirstChild();
		// Translate compounds
		Collection<Node> compoundnodes = XML.subNodes(indexRoot, Tags.COMPOUND);
		List<Entity> compounds = new LinkedList<Entity>();
		for (Node element: compoundnodes) {
			String name = XML.collectText(XML.subNode(element, Tags.NAME));
			// - filter out entities that have a name starting with "@"
			boolean anonymous = (name.charAt(0) == '@');
			// Translate compound entity
			try {
				DocumentComponentRegistry.EntityLocator locator =
					makeLocatorFromNode(element);
				try {
					if (name.indexOf("::") == -1 && !anonymous &&
							locator.getKind().equals(Tags.COMPOUND)) {
						Entity comp = followReference(locator);
						if (!comp.hasContainer())
							comp.setExternal(isExternal);
						compounds.add(comp);
					}
				}
				catch (XMLFormatException xe) {
					System.err.println("*** WARNING: top-level entity " +
						locator.getComponentID() + " is corrupt: " + xe);
				}
				catch (ElementNotFoundException ee) {
					System.err.println("*** WARNING: top-level entity " +
						locator.getComponentID() + " cannot be found: " + ee);
				}
			}
			catch (XMLFormatException e) {
				System.err.println("*** WARNING: top-level locator " +
					"syntax is invalid: " + e);
			}
		}
		// Add compounds to global namespace
		for (Entity element: compounds) {
			// - check whether element is class or namespace
			if (!element.hasContainer()) {
				if (element instanceof Aggregate) {
					global.addMember((Aggregate)element, Specifiers.Visibility.PUBLIC);
				}
				else if (element instanceof Namespace) {
					global.addMember((Namespace)element);
				}
				else if (element instanceof SourceFile) {
					m_db.enlistSourceFile((SourceFile)element);
				}
			}
		}
		// Fill in any source connections that may have been encountered
		// and remained unfulfilled
		fulfillSourceConnections();
		// Fill any broken type links which may be fixed
		repairDamagedReferences();
		// Put global variables in global scope
		fillGlobalVariables(global);
	}
	
	/**
	 * Reads the entire range of accessible data from the index, building a
	 * complete program database.
	 * @return ProgramDatabase
	 */
	public ProgramDatabase processIndex() throws ElementNotFoundException
	{
		ProgramDatabase program = new ProgramDatabase();
		m_db = program;
		Scope globals = program.getGlobalNamespace().getScope();
		List<RequestedDocument> indices =
				m_registry.locateAllDocuments("index");
		for (RequestedDocument index : indices) {
			boolean isExternal = m_registry.isExternal(index); 
			Scope scope =  isExternal ? program.getExternals() : globals;
			processIndex(scope, index.getDocument(), isExternal);
		}
		return program;
	}
	
	/*@}*/

	/**
	 * @name Structuring Aid
	 * Methods for dealing with the constructs in the micro level and for
	 * maintaining inter-relations. 
	 */
	/*@{*/

	/**
	 * Attempts to locate the Entity which is referred to by a &lt;ref&gt;
	 * node. The referenced locator is observed and the registry sought to
	 * find the element; if it is not found, the XML document may have to be
	 * opened and scanned through to locate the referenced component.
	 * @param locator locator for Entity to search for - a pair of
	 * document-name and component-id.
	 * @return Entity referenced entity
	 * @throws ElementNotFoundException the Entity which is 
	 * referenced cannot be found in the registry, nor can it be read from
	 * the corresponding XML file.
	 * @throws XMLFormatException the referenced Entity was found in the
	 * XML document to which the reference directs, but it could not be read
	 * because the format of the document is invalid.
	 */
	public Entity followReference(DocumentComponentRegistry.EntityLocator locator)
		throws ElementNotFoundException, XMLFormatException
	{
		try {
			return m_registry.locate(locator);
		}
		catch (ElementNotFoundException e) {
			// Open XML document
			RequestedDocument document =
				m_registry.locateDocument(locator.getDocumentName(), locator.getRealm());
			// Scan all <...def> tags (with corresponding kind) to find that with
			// the requested id
			NodeList refs = document.getDocument()
				.getElementsByTagName(Tags.def(locator.getKind()));
			Node found = null;
			for (int refi = 0; refi < refs.getLength(); ++refi) {
				Node refnode = refs.item(refi);
				// Compare node's id with that in the locator
				if (XML.attribute(refnode, Tags.ID, null).equals(
					locator.getComponentID())) {
					found = refnode;
					break;
				}
			}
			
			if (found == null)	throw new 
				ElementNotFoundException(locator.getKind(), locator.unique());
			
			// Translate the node that was found into an entity
			return translationSwitch(found, m_registry.isExternal(document));
		}
	}

	/**
	 * Oft are nodes carriers of references to other entities. This is usually
	 * done by placing the locator information in the <b>kind</b> and
	 * <b>refid</b> attributes of the node.
	 * This method follows the node directly - the EntityLocator is created
	 * automatically.
	 * @param referrer the node containing the reference
	 * @return Entity retrieved entity
	 * @throws XMLFormatException if the required attributes do not
	 * appear in the referrer node, or if an XML format error was encountered
	 * while accessing the referenced element.
	 * @throws ElementNotFoundException if the element being referenced
	 * by this referrer is missing.
	 */
	public Entity followReference(Node referrer)
		throws ElementNotFoundException, XMLFormatException
	{
		DocumentComponentRegistry.EntityLocator locator =
			makeLocatorFromNode(referrer);
		// Access registry to extract entity
		return followReference(locator);
	}	
	
	/**
	 * Oft are nodes carriers of references to other entities. This is usually
	 * done by placing the locator information in the <b>kind</b> and
	 * <b>refid</b> attributes of the node.
	 * This method uses that information to create an EntityLocator.
	 * @param referrer the node containing the reference
	 * @return DocumentComponentRegistry.EntityLocator translated locator
	 * @throws XMLFormatException if the required attributes do not appear
	 * in the referrer node.
	 */
	public DocumentComponentRegistry.EntityLocator
		makeLocatorFromNode(Node referrer)
		throws ElementNotFoundException, XMLFormatException
	{
		// Get referring attributes
		String kind = XML.attribute(referrer, Tags.KIND, null);
		String refkind = XML.attribute(referrer, Tags.REFKIND, null);
		if (kind == null)
			kind = refkind;
		if (kind == null)
			throw new XMLFormatException("reference without kind", referrer);
		return makeLocatorFromNode(referrer, kind);
	}
	
	private DocumentComponentRegistry.EntityLocator
		makeLocatorFromNode(Node referrer, String kind)
		throws XMLFormatException
	{
		// Get referring attributes
		String id = XML.attribute(referrer, Tags.ID, null);
		String refid = XML.attribute(referrer, Tags.REFID, null);
		String ext = XML.attribute(referrer, Tags.EXTERNAL, null);
		DocumentComponentRegistry.Realm realm =
			(ext == null) ? DocumentComponentRegistry.Realm.DONT_CARE
					: DocumentComponentRegistry.Realm.EXTERNAL;
		if (id == null)
			id = refid;
		if (id == null)
			throw new XMLFormatException("reference without refid", referrer);
		if (kind.equals(Tags.CLASS) 
				|| kind.equals(Tags.STRUCT) || kind.equals(Tags.UNION)
				|| kind.equals(Tags.NAMESPACE) || kind.equals(Tags.FILE))
			kind = Tags.COMPOUND;
		// Create a locator object
		return
			new DocumentComponentRegistry.EntityLocator(kind, id, realm);
	}

	/**
	 * Scans an entire XML subtree and finds all the &lt;ref&gt; nodes within
	 * it. These are extracted and the corresponding entities fetched; these
	 * are stored to the registry and also placed in the 'references' map,
	 * keyed by the text portion of each &lt;ref&gt; node.
	 * @param xmlnode root of XML subtree
	 * @param references a map which is filled with visited ref information
	 */
	private void collectReferences(Node xmlnode, Map references)
	{
		if (xmlnode.getNodeName() == Tags.REF) {
			// Retrieve the reference
			try {
				DocumentComponentRegistry.EntityLocator locator =
					makeLocatorFromNode(xmlnode);
				Entity referenced = followReference(locator);
				String name = XML.collectText(xmlnode);
				// Make sure that the name is coherent. This is needed because
				// Doxygen tends to foolishness when redirecting typedefs
				if (canBeTrusted(name, referenced))
					references.put(name, referenced);
			}
			catch (ElementNotFoundException e) {
				System.err.println("*** WARNING: reference to undefined " +
					"entity encountered while parsing type expression: " + e);
			}
			catch (XMLFormatException e) {
				System.err.println("*** WARNING: referenced entity " +
					XML.repr(xmlnode) + " is corrupted: " + e);
			}
		}
		else {
			// Recursively descend into children
			NodeList children = xmlnode.getChildNodes();
			for (int i = 0; i < children.getLength(); ++i) {
				collectReferences(children.item(i), references);
			}
		}
	}
	
	/**
	 * Checks if the basename of two (possibly qualified) names is the same.
	 * 
	 * @param name1 first name
	 * @param name2 second name
	 * @return <b>true</b> if the last name element matches, e.g. 
	 *  <code>a::cool</code> and <code>b::c::cool</code>
	 */
	private static boolean namesMatch(String name1, String name2)
	{
		return (name1.equals(name2) 
				|| name1.endsWith(":" + name2) || name2.endsWith(":" + name1));
	}
	
	/**
	 * It is sometimes the case, that Doxygen misdirects a reference embedded 
	 * within a type expression. This is because typedefs are followed up to 
	 * their origin, and when the origin is a class template, this results in
	 * loss of template argument information.
	 * 
	 * For example, if the code contains
	 *   <code>typedef std::pair&lt;int,int&gt CoPair;</code>
	 * Then Doxygen redirects <code>CoPair</code> to <code>std::pair</code>
	 * whenever it occurs. As a result, the &lt;int,int&gt; tag cannot be 
	 * recovered.
	 * 
	 * Another instance occurs with pointers-to types, like so:
	 *   <code>typedef MyClass *Ptr;</code>
	 * In which case Doxygen redirects <code>Ptr</code> references to
	 * <code>MyClass</code>, losing the information that it is a pointer
	 * type.
	 * 
	 * @param name the name via which an entity is referenced
	 * @param referenced an entity which Doxygen redirects that name to
	 * @return
	 */
	private static boolean canBeTrusted(String name, Entity referenced)
	{
		return namesMatch(name, referenced.getName());
	}
	
	/**
	 * Names of compounds generated by Doxygen may be fully qualified, with a
	 * prefix comprising of all containing namespaces or classes. These have
	 * to be stripped off before the compound is inserted into its container.
	 * @param inner inner compound with its 'name' set to long name
	 * @param container containing compound (Aggregate or Namespace)
	 */
	private void trimName(Entity inner, Entity container)
	{
		String cover = container.getFullName() + "::";
		String inname = inner.getName();
		// Remove the prefix
		if (inname.startsWith(cover))
			inner.setName(inname.substring(cover.length()));
	}
	
	/**
	 */
	private String dequalifyName(String name)
	{
		int start=0, next;
		next = name.indexOf("::");
		while (next != -1) {
			start = next+2;
			next = name.indexOf("::", start);
		}
		return name.substring(start);
	}
	
	/**
	 * Auxiliary function; if text starts with prefix word, return the
	 * rest of the text.
	 * A space must follow the prefix string in the main text for it to be
	 * considered an occurrance.
	 * @param text the text
	 * @param prefix the prefix to be sought
	 * @return Text following prefix if it was matched, null otherwise.
	 */
	private static String wordWithPrefix(String text, String prefix)
	{
		if (text.startsWith(prefix + " "))
			return text.substring(prefix.length() + 1);
		else
			return null;
	}
	
	/**
	 * Adds a newly created entity to the document/component registry.
	 * @param xmlnode root node
	 * @param entity newly constructed Entity instance
	 */
	private void selfSubscribe(Node xmlnode, Entity entity)
	{
		try {
			// Create a locator and subscribe entity
			DocumentComponentRegistry.EntityLocator locator =
				makeLocatorFromNode(xmlnode, "any");
			m_registry.subscribe(entity, locator);
		}
		catch (XMLFormatException e) {
			/* skip it */
		}
	}

	/**
	 * Puts the given entity by name in the m_globals_byname map, for later
	 * use by the DoxygenHandyman.
	 * @param entity
	 */
	private void rememberByName(Entity entity)
	{
		m_global_byname.put(entity.getName(), entity);
		m_global_byname.put(dequalifyName(entity.getName()), entity);
	}
	
	/**
	 * Corrects properties of a friend function to a class. 
	 * @param compound the class containing the friend declaration
	 * @param emember the routine declared as friend
	 * @param isExternal if the class belongs to an external reference
	 */
	private void adjustFriend(Entity compound, Routine emember, boolean isExternal)
	{
		// Add class' template parameters of container
		for (Iterator ti = compound.templateParameterIterator();
				ti.hasNext(); ) {
			TemplateParameter parameter = 
			(TemplateParameter)(ti.next());
			emember.addTemplateParameter((TemplateParameter)parameter.clone());
		}
		// Create connection to global scope
		if (!isExternal) {
			m_db.getGlobalNamespace().getScope().addMember(
					emember, Specifiers.DONT_CARE, 
					Specifiers.DONT_CARE, Specifiers.DONT_CARE);
		}
	}

	/**
	 * Attempts to fill information which is missing and appears as unfulfilled
	 * connections in the declaration and definition attributes of an entity.
	 */
	private void fulfillSourceConnections()
	{
		for (SourceFile.DeclDefConnection unfulfilled: m_unfulfilled_declarations) {
			// Look for filename in map
			String filename = unfulfilled.getSourceFilename();
			SourceFile source = (SourceFile)m_files_byname.get(filename);
			// Update connection as neccessary
			if (source != null) {
				// - check if this is a definition or a declaration
				boolean isDeclaration = 
					(unfulfilled.getDeclaredEntity().getDeclaration() 
						== unfulfilled);
				// - recreate proper connection
				if (isDeclaration)
					unfulfilled.getDeclaredEntity()
						.setDeclarationAt(source, unfulfilled.where());
				else
					unfulfilled.getDeclaredEntity()
						.setDefinitionAt(source, unfulfilled.where());
			}
		}
	}
	
	/**
	 * Runs a handyman over the routines listed in "routines for repair",
	 * the alias (typedef) entities listed in "aliases for repair", and
	 * inheritance connections listed in "inheritance for repair".
	 */
	private void repairDamagedReferences()
	{
		DoxygenHandyman handy = new DoxygenHandyman(m_db);
		// Run on all routines
		for (Routine routine: m_routinesForRepair) {
			handy.repair(routine);
		}
		// Run on aliases
		for (Alias alias: m_aliasesForRepair) {
			handy.repair(alias);
		}
		// Run on fields
		for (Field field: m_fieldsForRepair) {
			handy.repair(field);
		}
		// Run on compound classes involved in inheritance
		for (InheritanceConnection inheritance: m_inheritanceForRepair) {
			handy.repair(inheritance);
		}
	}
	
	/**
	 * Inserts information collected about global fields (variables) into
	 * the global namespace in the program database.
	 */
	private void fillGlobalVariables(Scope globalScope)
	{
		DoxygenHandyman.transferFields(globalScope,
			m_globalFields_byname);
		m_globalFields_byname.clear();
	}
	
	/*@}*/

	/**
	 * Debug utility - returns the registry.
	 * @return DocumentComponentRegistry internal registry compartment
	 */
	public DocumentComponentRegistry getInternalRegistry()
	{
		return m_registry;
	}

	// Private members
	private ProgramDatabase m_db;
	private DocumentComponentRegistry m_registry;
	private Map m_global_byname;
	private Map m_local_byname;
	
	private Map m_files_byname;
	private List<SourceFile.DeclDefConnection> m_unfulfilled_declarations;
	
	private List<Routine> m_routinesForRepair;
	private List<Alias> m_aliasesForRepair;
	private List<Field> m_fieldsForRepair;
	private List<InheritanceConnection> m_inheritanceForRepair;
	private Map m_globalFields_byname;

	public Logger logger;
}
