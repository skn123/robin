/*
 * Created on Jun 24, 2003
 */
package backend;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import javax.swing.tree.DefaultMutableTreeNode;

import sourceanalysis.Aggregate;
import sourceanalysis.Alias;
import sourceanalysis.ContainedConnection;
import sourceanalysis.DataTemplateParameter;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;
import sourceanalysis.Field;
import sourceanalysis.InappropriateKindException;
import sourceanalysis.IncompleteTemplateInstance;
import sourceanalysis.InheritanceConnection;
import sourceanalysis.MissingInformationException;
import sourceanalysis.Namespace;
import sourceanalysis.Parameter;
import sourceanalysis.Primitive;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.Routine;
import sourceanalysis.Scope;
import sourceanalysis.Specifiers;
import sourceanalysis.TemplateArgument;
import sourceanalysis.TemplateEnabledEntity;
import sourceanalysis.TemplateParameter;
import sourceanalysis.Type;
import sourceanalysis.TypenameTemplateArgument;
import sourceanalysis.TypenameTemplateParameter;
import sourceanalysis.Type.TypeNode;
import sourceanalysis.view.Traverse;

/**
 * A class which contains some static methods for the use in backend
 * authorship.
 */
public class Utils {

	/**
	 * Checks whether an entity has a 'description' property.
	 * @param entity
	 * @return <b>true</b> if property exists, <b>false</b> if not.
	 */
	public static boolean hasDescription(Entity entity)
	{
		try { 
			return (entity.findProperty("description") != null);
		}
		catch (ElementNotFoundException e) {
			/* no description */
			return false;
		}
	}
	
	/**
	 * Gets the template entity referenced by a template instantiation 
	 * involved in a type expression.
	 * E.g., for the expression "std::vector&lt;int&gt;", the Entity 
	 * "std::vector" is returned. 
	 * @param root the root node of the type expression to check
	 * @return the templated Aggregate entity which the expression directly
	 * instantiates
	 * @throws InappropriateKindException if the given type-node is
	 * not a NODE_TEMPLATE_INSTANTIATION node.
	 */
	public static Aggregate extractTemplate(Type.TypeNode root) 
	throws InappropriateKindException
	{
		if (root.getKind() == Type.TypeNode.NODE_TEMPLATE_INSTANTIATION) {
			// Get the template
			Enumeration cen = root.children();
			Type.TypeNode basenode = (Type.TypeNode)cen.nextElement();
			return (Aggregate)basenode.getBase();
		}
		else
			throw new InappropriateKindException("expected a template node");
	}
	
	/**
	 * Gets the template arguments indicated in a template instantiation 
	 * involved in a type expression.
	 * E.g., for the expression "std::map&lt;int,long&gt;", the list 
	 * containing the type expressions ["int", "long"] is returned. 
	 * @param root the root node of the type expression to check
	 * @return a List of TemplateArgument instances
	 * @throws InappropriateKindException if the given type-node is
	 * not a NODE_TEMPLATE_INSTANTIATION node.
	 */
	public static List<TemplateArgument> extractTemplateArguments(Type.TypeNode root)
	throws InappropriateKindException
	{
		List<TemplateArgument> targs = new ArrayList<TemplateArgument>();
		
		if (root.getKind() == Type.TypeNode.NODE_TEMPLATE_INSTANTIATION) {
			// Get the template
			Enumeration cen = root.children();
			cen.nextElement();     // - skip base node
			// Get the children
			while (cen.hasMoreElements()) {
				// Get user object of child
				DefaultMutableTreeNode element = 
					(DefaultMutableTreeNode)cen.nextElement();
				targs.add( (TemplateArgument)element.getUserObject() );
			}
			
			return targs;
		}
		else
			throw new InappropriateKindException("expected a template node");
	}
	
	/**
	 * Checks whether a specified entity is <b>globally</b> visible, that is,
	 * it can be accessed from the global scope by simply writing its fully
	 * qualified name, and it's not blocked by "private" declarations. 
	 * @param entity the entity to observe
	 * @return <b>true</b> if the entity is accessible
	 */
	public static boolean isAccessible(Entity entity)
	{
		return (entity instanceof Primitive) 
			|| (!entity.hasContainer())
			|| (entity.getContainerConnection().getVisibility() == Specifiers.Visibility.PUBLIC);
	}
	
	/**
	 * Like isAccessible(Entity), only for template arguments.
	 * @note An orphan (i.e. without a container) template argument is
	 * considered accessible, whereas an orphan Entity is not.
	 * @param templateArg a template argument to observe
	 * @return <b>true</b> if this template argument can be used
	 */
	public static boolean isAccessible(TemplateArgument templateArg)
	{
		if (templateArg instanceof TypenameTemplateArgument) {
			TypenameTemplateArgument tta = (TypenameTemplateArgument)templateArg;
			Type type = tta.getValue();
			return isAccessible(type.getRootNode());
		}
		return true;
	}
	
	/**
	 * Like isAccessible(Entity), only for type expressions.
	 * To determine whether a type expression is accessible, any entity 
	 * occurring in it must be accessible.
	 * @param root the root node of the type expression
	 * @return <b>true</b> if this type expression is usable in a global
	 * scope 
	 */
	public static boolean isAccessible(Type.TypeNode root)
	{
		if (root.getKind() == Type.TypeNode.NODE_TEMPLATE_INSTANTIATION) {
			// Get the template arguments
			try {
				List<TemplateArgument> arguments = extractTemplateArguments(root);
				// Make sure that all the template arguments are accessible
				for (TemplateArgument ta: arguments) {
					if (!isAccessible(ta))
						return false;
				}
				return true;
			}
			catch (InappropriateKindException e) { throw new AssertionError(); }
		}
		else if (root.getKind() == Type.TypeNode.NODE_ARRAY ||
				root.getKind() == Type.TypeNode.NODE_POINTER ||
				root.getKind() == Type.TypeNode.NODE_REFERENCE) {
			// Go down to the child
			Type.TypeNode child = (Type.TypeNode)root.getFirstChild();
			return isAccessible(child);
		}
		else if (root.getKind() == Type.TypeNode.NODE_LEAF) {
			// Check the base type
			try {
				Entity base = root.getBase();
				return isAccessible(base);
			}
			catch (InappropriateKindException e) { throw new AssertionError(); }
		}
		else
			return false;
	}
	
	/**
	 * Checks whether the routine is entirely flat - that is, all the types it
	 * receives and returns are flat.
	 * @return boolean flatness flag
	 */
	public static boolean allAreFlat(Routine routine)
	{
		try {
			// Check flatness of return type
			Type rtype = routine.getReturnType();
			if (rtype.getRoot() == null) {
				if (!routine.isConstructor()) return false;
			}
			else {
				if (!rtype.isFlat()) return false;
			}
			// Iterate parameters and check the flatness of each
			for (Iterator pi = routine.parameterIterator(); pi.hasNext(); ) {
				Parameter param = (Parameter)pi.next(); 
				if (!param.getType().isFlat()) return false;
			}
			return true;
		}
		catch (MissingInformationException e) {
			return false;
		}
	}
	
	/**
	 * Checks whether the given type expression employs any arrays in any
	 * of its parameters.
	 * 
	 * @param root a type expression
	 * @return <b>true</b> if any array specifications were found,
	 * <b>false</b> otherwise.
	 */
	public static boolean hasAnyArrays(Routine routine)
	{
		// Iterate parameters and check the flatness of each
		for (Iterator pi = routine.parameterIterator(); pi.hasNext(); ) {
			Parameter param = (Parameter)pi.next();
			try {
				if (hasAnyArrays(param.getType())) return true;
			}
			catch (MissingInformationException e) { /* pass */ }
		}
		
		return false;
	}
	
	/**
	 * Checks whether the given type expression employs any arrays.
	 * 
	 * @param root a type expression
	 * @return <b>true</b> if any array specifications were found,
	 * <b>false</b> otherwise.
	 */
	private static boolean hasAnyArrays(Type root)
	{
		final boolean found[] = new boolean[] { false };
		
		try {
			/* Observe all type nodes, and look for an array node */
			Type.transformType(root, new Type.Transformation() {
				public TypeNode transform(TypeNode original)
						throws InappropriateKindException {
					// - when found, store in outer variable found[]
					if (original.getKind() == Type.TypeNode.NODE_ARRAY)
						found[0] = true;
					return null;
				}
			});
		}
		catch (InappropriateKindException e) {
			throw new AssertionError();
		}
		
		return found[0];
	}

	/**
	 * Checks whether a contained entity is a template parameter's
	 * delegate for the class it is contained in. 
	 */
	public static boolean isATemplateParameter(Entity entity)
	{
        if (!entity.hasContainer())
            return false;

        if (!entity.getContainer().isTemplated())
            return false;

        for (Iterator tpi = entity.getContainer().templateParameterIterator();
            tpi.hasNext(); ) {
            // Check if this is a typename argument, and if this
            // argument's delegate entity is 'entity'
            TemplateParameter templateParameter =
                (TemplateParameter)tpi.next();
            if (templateParameter instanceof TypenameTemplateParameter) {
                // Exploit TypenameTemplateParameter's interface
                TypenameTemplateParameter typename =
                    (TypenameTemplateParameter)templateParameter;
                if (entity == typename.getDelegate()) {
                    return true;
                }
            }
        }

        // Non of the above matched, meaning that 'entity' is not
        // a part of its class' template parameters
        return false;
	}


	/**
	 * Constructs a proper qualified-identifier string, similar to that
	 * returned by Entity.getFullName(), but without the leading "::" which
	 * seem troublesome for certain applications.
	 * @param entity the entity to take name of
	 * @return full name without leading "::" 
	 */
	private static String cleanFullNameBase(Entity entity)
	{
		String cleanName = null;
		
		if ( (!entity.hasContainer()) || Utils.isATemplateParameter(entity)) {
			cleanName = entity.getName();
		}
		else {
			Entity container = entity.getContainer();
			String containerName = cleanFullNameBase(container);
			// Prepend container's name to entity's only if it's not empty,
			// thus avoiding orphan colons.
			cleanName = (containerName.equals("")) ?
				entity.getName() : containerName + "::" + entity.getName();
		}

		// Nonetheless this is a patch
		cleanName = cleanName.replaceAll("< ::", "< ");
		if (cleanName.startsWith("::"))
			cleanName = cleanName.substring(2);
		return cleanName;
	}

	/**
	 * Constructs a proper qualified-identifier string, similar to that
	 * returned by Entity.getFullName(), but without the leading "::" which
	 * seem troublesome for certain applications.
	 * This applies to template arguments, if they occur in the type, as
	 * well. e.g., you'll get "std::vector&lt; Elem &gt;" rather than
	 * "::std::vector&lt; ::Elem &gt;".
	 * @param entity entity to take name of
	 * @return full name without leading "::" 
	 */
	public static String cleanFullName(Entity entity)
	{
		return Type.formatBaseUsing(entity, CLEAN_TYPE_FORMATTER);
	}

	/**
	 * Construct a propery C++ representation of a type, using cleanFullName
	 * for every base-name to avoid the leading "::" in class names.
	 * @param root root node of type expression
	 * @param declname declarator to use in type expression (may be "")
	 * @return C++-formatted type string
	 */
	public static String cleanFormatCpp(Type.TypeNode root, String declname)
	{
		return root.formatCpp(declname, CLEAN_TYPE_FORMATTER);
	}

	/**
	 * Construct a propery C++ representation of a type, using cleanFullName
	 * for every base-name to avoid the leading "::" in class names.
	 * @param type type expression
	 * @param declname declarator to use in type expression (may be "")
	 * @return C++-formatted type string
	 */
	public static String cleanFormatCpp(Type type, String declname)
	{
		if (type.getRoot() == null) {
			return "";
		}
		else { 
			return type.getRootNode().formatCpp(declname, CLEAN_TYPE_FORMATTER);
		}
	}

	/**
	 * Construct and returns new String, which does not
	 * contain any HTML/XML tags.
	 * It replaces every substring of the type &lt;XXX&gt; with
	 * a blank String.
	 * 
	 * @param text a text fragment possibly containing HTML tags
	 * @return the text with tags removed
	 */
	public static String cleanHTMLTags(String text) {
		return text.replaceAll("\\<[^<]*\\>", "");
	}

	/**
	 * Returns the declared number of parameters a function or method has.
	 * @param routine entity representing function or method
	 */
	public static int countParameters(Routine routine)
	{
		int counter = 0;
		// Iterate through parameters and increment counter
		for (Iterator iter = routine.parameterIterator(); iter.hasNext(); ) {
			iter.next();
			++counter;
		}		
		return counter;
	}
	
	/**
	 * Returns the essential number of arguments required to invoke this
	 * function or method - that is, the number of non-optional parameters.
	 * @param routine entity representing function or method
	 */
	public static int minimalArgumentCount(Routine routine)
	{
		int counter = 0;
		// Iterate through parameters and increment counter only if
		// parameter does not have a default value
		for (Iterator iter = routine.parameterIterator(); iter.hasNext(); ) {
			Parameter parameter = (Parameter)iter.next();
			if (!parameter.hasDefault()) ++counter;
		}		
		return counter;
	}
	
	/**
	 * This method will return the prototype of the given routine with
	 * additional 'link tags' to related classes.
	 * @param routine The routine.
	 * @return The prototype of the routine. 
	 * @throws MissingInformationException if the program DB is 
	 * incomplete.
	 */
	public static String reconstructPrototype(Routine routine) 
		throws MissingInformationException 
	{
		// Create the prototype:
		// <return_value> <name>( <arguments> ).
		String proto = "";
		
		if(routine.getReturnType().getRoot() == null) { 
			// Constructor or Destructor (no return type).
			proto += Utils.cleanFullName(routine);
		} else {
			proto += Utils.cleanFormatCpp(routine.getReturnType(), 
				Utils.cleanFullName(routine));
		}

		proto += "(";	
		for (Iterator iter = routine.parameterIterator(); iter.hasNext();) {
			Parameter param = (Parameter)iter.next();
			
			proto += Utils.cleanFormatCpp(param.getType(), 
				 param.getName());
			if(param.hasDefault()) {
				proto += " = " + param.getDefaultString();
			}
			if(iter.hasNext()) {
				proto += ", ";
			} 
		}
		
		proto += ")";
		
		// Const.
		if(routine.isConst()) {
			proto = proto + " const";
		}
		
		// Inline.
		if(routine.isInline()) {
			proto = "inline " + proto;
		}
		
		if (routine.hasContainer()) {
			// Virtuality.
			if(routine.getContainerConnection().getVirtuality() == 
				Specifiers.Virtuality.VIRTUAL) {
				proto = "virtual " + proto;
			} else if(routine.getContainerConnection().getVirtuality() == 
				Specifiers.Virtuality.PURE_VIRTUAL) {
				proto = "virtual " + proto + " = 0";
			}
			// Static.
			if(routine.getContainerConnection().getStorage() == 
				Specifiers.Storage.STATIC) {
				proto = "static " + proto;
			} 
		}
		
		return proto;
	}

    /**
     * Searches if the class has any non private constructor.
     * @param entity
     */
    public static boolean hasOnlyPrivateConstructors(Aggregate entity)
    {
        Scope<Aggregate> scope = entity.getScope();
        boolean hasNoPrivateConstructors = false;

        for (Iterator ri = scope.routineIterator(); ri.hasNext(); ) {
            ContainedConnection connection = (ContainedConnection)ri.next();
            Routine routine = (Routine)connection.getContained();
            if (routine.isConstructor()) {
                if (connection.getVisibility() != Specifiers.Visibility.PRIVATE) {
                    return false;
                }
                hasNoPrivateConstructors = true;
            }
        }
        return hasNoPrivateConstructors;
    }
	

	/**
	 * Searches the scope to see if there is a public default constructor.
	 * @param entity
	 * @return
	 * @throws MissingInformationException
	 */
	public static boolean hasDefaultConstructor(Aggregate entity)
		throws MissingInformationException
	{
		Scope<Aggregate> scope = entity.getScope();
		boolean anyConstructor = false;
	
		if (entity instanceof Primitive) return true;
		if (entity.getDeclaration() == null) return false;
		
		for (Iterator ri = scope.routineIterator(); ri.hasNext(); ) {
			ContainedConnection connection = (ContainedConnection)ri.next();
			Routine routine = (Routine)connection.getContained();
			
			if (routine.isConstructor()
				&& !routine.parameterIterator().hasNext()) {
				// This is a default constructor.
				// Return true if it's public, false otherwise
				return connection.getVisibility() == Specifiers.Visibility.PUBLIC;
			}
			if (routine.isConstructor()) anyConstructor = true;
		}
		
        boolean membersHaveDefaultConstructors = true;

        for(Iterator fi = scope.fieldIterator(); fi.hasNext(); ) {
            ContainedConnection connection = (ContainedConnection)fi.next();
            Field field = (Field)connection.getContained();
            
            Type fieldType = flatUnalias(field.getType());
            Entity fieldBaseType = fieldType.getBaseType();

            if (fieldType.getPointerDegree() > 0) 
            	continue;
            else if (fieldType.isReference()) {
            	membersHaveDefaultConstructors = false;
            }
            else {
            	if (fieldBaseType instanceof Aggregate &&
                      !hasDefaultConstructor((Aggregate)fieldBaseType)) {
                    membersHaveDefaultConstructors = false;
            	}
            }
        }

		// No default constructor found
		return !anyConstructor && membersHaveDefaultConstructors;
	}

	/**
	 * Searches the global function base for an output operator applicable
	 * to some entity.
	 * @param entity the entity in which to search the operator
	 * @param program a Program Database in which the applicable global
	 * operator function may be found
	 * @return <b>true</b> if an output operator was found
	 * @throws MissingInformationException if the program database does not
	 * contain enough information to make the decision.
	 */
	public static boolean hasOutputOperator(Entity entity,
		ProgramDatabase program) throws MissingInformationException
	{
		// Check whether given entity is a specialization
		Entity metaEntity = seekOriginalTemplate(entity);
		
		// Scan each and every routine in the global namespace
		for (Iterator globalri = program.getGlobalNamespace()
				.getScope().routineIterator(); 
			 globalri.hasNext() ; )
		{
			ContainedConnection connection = 
				(ContainedConnection)globalri.next();
			Routine fcn = (Routine)connection.getContained();
			
			// Filter "operator<<"
			if (fcn.getName().equals("operator<<")) {
				Iterator pi = fcn.parameterIterator();
				if (pi.hasNext()) {
					// Obtain left operand type
					Parameter left = (Parameter)pi.next();
					if (pi.hasNext()) {
						// Obtain right operand type
						Parameter right = (Parameter)pi.next();
						// Check types
						if (left.getType().isFlat()
							&& left.getType().getBaseType()
								.getFullName().endsWith("ostream") /*@@@*/
							&& right.getType().isFlat()
							&& (areCongruent(right.getType().getBaseType(), entity)
								|| areCongruent(right.getType().getBaseType(), metaEntity)))
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}
	
	/**
	 * Looks for global operators in which the requested entity occurs as a 
	 * left operand.
	 * @param with left operand type to seek for
	 * @param program a program database containing global functions to scan
	 * @return List a list of matching operators
	 * @throws MissingInformationException if information contained in the
	 * program database is insufficient for carrying out this operation.
	 */
	public static List<Routine> findGloballyScopedOperators(Entity with, 
											ProgramDatabase program)
		throws MissingInformationException
	{
		// Scan each and every routine in the global namespace
		List<Routine> matches = new LinkedList<Routine>();
		if (with.isTemplated()) return matches;
		
		for (Iterator globalri = program.getGlobalNamespace()
				.getScope().routineIterator(); 
				globalri.hasNext() ; )
		{
			ContainedConnection connection = 
				(ContainedConnection)globalri.next();
			Routine fcn = (Routine)connection.getContained();
			
			// Filter operators
			if (fcn.isOperator() && !fcn.isTemplated()) {
				Iterator pi = fcn.parameterIterator();
				if (pi.hasNext()) {
					// Obtain left operand type
					Parameter left = (Parameter)pi.next();
					// Check types
					if (left.getType().isFlat()
							&& left.getType().getBaseType() == with
							&& (areCongruent(left.getType().getBaseType(), with) /*
									|| matches(left.getType().getBaseType(), metaEntity)*/))
					{
							matches.add(fcn);
					}
				}
			}		
		}
		return matches;
	}

	/**
	 * Checks whether a class is abstract - that is, it has at least
	 * one pure virtual method.
	 * @param entity an Aggregate representing class to be checked
	 * @param instanceMap a map containing mappings from template expressions
	 * (in the format returned by templateExpression) to Aggregate entities
	 * that have been instantiated for them
	 * @return <b>true</b> if at least one method here is pure virtual;
	 * <b>false</b> if none are.
	 */
	public static boolean isAbstract(Aggregate entity, Map<String, Aggregate> instanceMap)
		throws MissingInformationException
	{
		return ! unimplementedMethods(entity, instanceMap).isEmpty();
	}

	/**
	 * Checks whether a class is abstract - that is, it has at least
	 * one pure virtual method.
	 * <p>This version assumes Utils.defaultInstanceMap for the instance map.
	 * </p>
	 * @note This function is not thread-safe, unless it is guarenteed that
	 * the defaultInstanceMap does not change.
	 * @param entity an Aggregate representing class to be checked
	 * @return <b>true</b> if at least one method here is pure virtual;
	 * <b>false</b> if none are.
	 */
	public static boolean isAbstract(Aggregate entity)
		throws MissingInformationException
	{
		return isAbstract(entity, defaultInstanceMap);
	}
	
	/**
	 * Checks whether the given aggregate is polymorphic, by checking if
	 * any of its methods is either virtual or pure virtual, and doing
	 * this check recursively for all of its ancestors as well.
	 * 
	 * @param subject the aggregate to check
	 * @return <b>true</b> is the aggregate is polymorphic, <b>false</b>
	 * otherwise
	 * @throws MissingInformationException if there is not enough information
	 * in the program database to make this decision.
	 */ 
	public static boolean isPolymorphic(Aggregate subject)
		throws MissingInformationException
	{
		// Go over the methods of the aggregate and for each one, check if
		// it is virtual or pure virtual
		for (Iterator methodIter = subject.getScope().routineIterator();
			 methodIter.hasNext(); ) {
			ContainedConnection connection =
				(ContainedConnection)methodIter.next();
			if (connection.getVirtuality() != Specifiers.Virtuality.NON_VIRTUAL)
				return true;
		}
		// Perform check recursively for all base classes
		for (Iterator baseIter = subject.baseIterator();
			 baseIter.hasNext(); ) {
			InheritanceConnection connection =
				(InheritanceConnection)baseIter.next();
			if (isPolymorphic(connection.getBase()))
				return true;
		}
		// None of the previous checks produced a positive result
		return false;
	}
	
	/**
	 * Instantiates a template contained in a type
	 * @throws InappropriateKindException 
	 * @throws MissingInformationException 
	 */
	public static Entity instantiateTemplate(Type templateType, Map<String, Aggregate> existingInstancesMap) 
		throws MissingInformationException, InappropriateKindException 
	{
		
		Entity typeEntity = templateType.getBaseType();
		TemplateArgument[] typeTemplateArguments = templateType.getTemplateArguments();
		
		
		
		if(!typeEntity.isTemplated() || ! (typeEntity instanceof Aggregate) || typeTemplateArguments == null) {
			return typeEntity;
		}
		
		Aggregate typeAggregate = (Aggregate)typeEntity;
		
		
		
		return instantiateTemplate(typeAggregate, typeTemplateArguments, existingInstancesMap);
	}
	
	
	/**
	 * Generates a specialization for a template class, given actual
	 * values for the template parameters.
	 * @param template a templated Aggregate entity
	 * @param arguments array of template arguments to place in template,
	 * <i>must</i> be of the same length as the list of template parameters in
	 * 'template'
	 * @param institute a map of Entity to Type which instructs which
	 * elements are known to require replacement - this usually carries
	 * values when
	 * @param existingInstancesMap map of existing template instantiations
	 * @return A <b>new</b> Aggregate object
	 * @throws MissingInformationException when some information inside the
	 * template object is incomplete
	 * @throws InappropriateKindException when a malformed type expression
	 * is encountered
	 */
	public static Aggregate instantiateTemplate(Aggregate template,
		TemplateArgument[] arguments, Map<String, Aggregate> existingInstancesMap)
		throws MissingInformationException, InappropriateKindException
	{
		return instantiateTemplate(template, arguments, null, null, existingInstancesMap);
	}
	
	/**
	 * Generates a specialization for a template class, given actual
	 * values for the template parameters.
	 * @param template a templated Aggregate entity
	 * @param arguments array of template arguments to place in template,
	 * <i>must</i> be of the same length as the list of template parameters in
	 * 'template'
	 * @param institute a map of Entity to Type which instructs which
	 * elements are known to require replacement - this usually carries
	 * values when the class template being instantiated is inner in some
	 * greater template - in other cases, <b>null</b> will suffice.
	 * @param d a clean, ready instance of Aggregate into which to add
	 * the instantiated elements. If <b>null</b>, a new instance is
	 * generated in the template's container scope.
	 * @param existingInstancesMap map of existing templates instantiations
	 * @return A <b>new</b> Aggregate object
	 * @throws MissingInformationException when some information inside the
	 * template object is incomplete
	 * @throws InappropriateKindException when a malformed type expression
	 * is encountered
	 */
	public static Aggregate instantiateTemplate(Aggregate template,
		TemplateArgument[] arguments, Map<Entity, Type> institute, Aggregate d, Map<String, Aggregate> existingInstancesMap)
		throws MissingInformationException, InappropriateKindException
	{
		// Don't instantiate the same template twice
		String existingExpression =
			templateExpression((d==null) ? template : d, arguments);
		if(existingInstancesMap.containsKey(existingExpression)) {
			return (Aggregate)existingInstancesMap.get(existingExpression);
		}
		
		
		Map<Entity, Type> substitution = (institute != null) ? institute : new HashMap<Entity, Type>();
		Map<String, TemplateArgument> macros = new HashMap<String, TemplateArgument>();
	    //System.err.println("Started instantiation for " + template + " with arguments " + java.util.Arrays.toString(arguments));
		// - these final variables are for use inside anonymous classes
		final Aggregate fin_template = template;
		final Map<Entity, Type> fin_substitution = substitution;
		final List<TemplateArgument> fin_targs = new ArrayList<TemplateArgument>();
		final Map<String, Aggregate> fin_existingInstancesMap = existingInstancesMap;
		
		// ---------------------------
		// Create instantiations for template arguments
		// ---------------------------
		for(int i=0; i<arguments.length; ++i) {
			TemplateArgument arg = arguments[i];
			if(arg instanceof TypenameTemplateArgument)
			instantiateTemplate(((TypenameTemplateArgument)arg).getValue(), existingInstancesMap);
		}
		
		// ---------------------------
		// Create the substitution map
		// ---------------------------
		// 1. Collect template parameters and their actual agruments
		int argIndex = 0;
		for (Iterator tpi = template.templateParameterIterator(); 
			 tpi.hasNext(); ++argIndex) {
			// - get current parameter and argument (use default if needed)
			TemplateParameter templateParameter = (TemplateParameter)tpi.next();
			TemplateArgument templateArgument = argIndex < arguments.length ? 
				arguments[argIndex] : templateParameter
					.getDefaultValue(template.templateParameterIterator(),
										fin_targs.iterator());
			fin_targs.add(templateArgument);
			// - if not enough arguments were supplied and there is no
			//   appropriate default value, raise exception
			if (templateArgument == null) 
				throw new MissingInformationException(
					"not enough template arguments for instantiation of "
					+ template.getFullName());
			// - list mappings for typename template parameters
			if (templateParameter instanceof TypenameTemplateParameter) {
				TypenameTemplateParameter typenameParameter =
					(TypenameTemplateParameter)templateParameter;
				// - get the replaced and the replacing
				Entity replace = typenameParameter.getDelegate();
				Type with = ((TypenameTemplateArgument)templateArgument).getValue();
				substitution.put(replace, with); 
			}
			else if (templateParameter instanceof DataTemplateParameter) {
				macros.put(templateParameter.getName(), templateArgument);
			}
		}
		// 2. Locate possible use of traits - that is, expressions where the
		//    template parameter is used as a container (Traits::SIZE)
		Traverse.TypeInformationVisitor typeVisitor = 
		new Traverse.TypeInformationVisitor() {
			public void visit(Type typei) {
				if (typei.getRoot() != null)
					collectInnersOfTemplateParameter(typei,
						fin_template, 
						fin_targs, 
						fin_substitution,
						fin_existingInstancesMap);
			}
		};
		new Traverse().traverse(template, typeVisitor, true, 
				Specifiers.Visibility.PUBLIC);
		
		// ---------------------------------
		// Create an entity for the instance
		// ---------------------------------
		Aggregate templateInstance = (d == null) ? new Aggregate() : d;
		templateInstance.setName(template.getName());
		if (arguments.length > 0) {
			templateInstance.setGeneralTemplateForSpecialization(template, 
					new Vector(Arrays.asList(arguments)));
		}
		if (template.getDeclaration() != null) {
			// - copy declaration locator
			try {
				templateInstance.setDeclarationAt(
					template.getDeclaration().getSource(),
					template.getDeclaration().where());
			}
			catch (MissingInformationException e) {
				// tough...
			}
		}
		// - copy properties
		for (Iterator propi = template.propertyIterator(); propi.hasNext();)
		{
			Entity.Property property = (Entity.Property)propi.next();
			templateInstance.addProperty(property);
		}
		// - insert entity to the same namespace where the template lives
		if (!templateInstance.hasContainer()) {
			if (template.hasContainer()) {
				Entity container = template.getContainer();
				if (container instanceof Namespace) {
					((Namespace)container).getScope().addMember(
							templateInstance,
                            template.getContainerConnection().getVisibility());
				}
			}
		}
		// - add new instance to substitution map as well
		substitution.put(template, 
			new Type(new Type.TypeNode(templateInstance)));
		
		// ----------------------------
		// Copy inheritance information
		// ----------------------------
		for (Iterator bi = template.baseIterator(); bi.hasNext(); ) {
			InheritanceConnection connection =
				(InheritanceConnection)bi.next();
			TemplateArgument[] baseTemplateArgs =
				connection.getBaseTemplateArguments();
			// Possibly substitute template arguments in base type-expression
			if (baseTemplateArgs != null)
				baseTemplateArgs = 
					instantiateTemplate(baseTemplateArgs, substitution, macros);
			templateInstance.addBase(connection.getBase(), baseTemplateArgs,
				connection.getVisibility());
		}
		// ------------------------------------
		// Instantiate inner class declarations
		// ------------------------------------
		List<Aggregate[]> inners = new LinkedList<Aggregate[]>();
		for (Iterator ci = template.getScope().aggregateIterator(); 
		      ci.hasNext(); ) {
			ContainedConnection connection = 
				(ContainedConnection)ci.next();
			Aggregate innerClass = 
				(Aggregate)connection.getContained();
			if (!isATemplateParameter(innerClass)) {
				// Create a class by the same name in templateInstance
				Aggregate instance = new Aggregate();
				instance.setName(innerClass.getName());
				templateInstance.getScope().addMember(
						instance, connection.getVisibility());
				// - register inner class for substitution
				substitution.put(
						innerClass, new Type(new Type.TypeNode(instance)));
				inners.add(new Aggregate[] { innerClass, instance });
			}
		}
		// --------------------------
		// Instantiate inner typedefs
		// --------------------------		
		for (Iterator ai = template.getScope().aliasIterator();
			ai.hasNext(); ) {
			// - get an inner
			ContainedConnection connection = (ContainedConnection)ai.next();
			Alias alias = (Alias)connection.getContained();
			Alias instance = instantiateTemplate(alias, substitution, macros);
			templateInstance.getScope().addMember(
				instance, connection.getVisibility());
			// - register alias for substitution
			substitution.put(alias, new Type(new Type.TypeNode(instance)));
		}
		// -----------------------
		// Instantiate inner enums
		// -----------------------
		for (Iterator ei = template.getScope().enumIterator();
			ei.hasNext(); ) {
			// - get an inner
			ContainedConnection connection = (ContainedConnection)ei.next();
			sourceanalysis.Enum enume = (sourceanalysis.Enum)connection.getContained();
			sourceanalysis.Enum instance = (sourceanalysis.Enum)enume.clone();
			templateInstance.getScope().addMember(
				instance, connection.getVisibility());
			// - register for substitution
			substitution.put(enume, new Type(new Type.TypeNode(instance)));
		}
		// ----------------------------
		// Instantiate member functions
		// ----------------------------
		for (Iterator mi = template.getScope().routineIterator(); 
			mi.hasNext();) {
			// - get a method
			ContainedConnection connection = (ContainedConnection)mi.next();
			Routine method = (Routine)connection.getContained();
			Routine methodinst = new Routine();
			methodinst.setName(method.getName());
			methodinst.setConst(method.isConst());
			// - copy properties
			for (Iterator propi = method.propertyIterator(); propi.hasNext();)
			{
				Entity.Property property = (Entity.Property)propi.next();
				methodinst.addProperty(property);
			}
			// - copy template arguments if any
			if (method.isTemplated()) {
				for (Iterator templi = method.templateParameterIterator(); 
					templi.hasNext(); ) {
					TemplateParameter tparameter = 
						(TemplateParameter)templi.next();
					methodinst.addTemplateParameter(
						(TemplateParameter)tparameter.clone());
				}
			}
			for (Iterator propi = method.propertyIterator(); propi.hasNext();)
			{
				Entity.Property property = (Entity.Property)propi.next();
				methodinst.addProperty(property);
			}
			// - substitute in return type of method
			methodinst.setReturnType(instantiateTemplate(method.getReturnType(),substitution,macros));
			if (methodinst.isConversionOperator()) {
				methodinst.setName(Routine.OPERATOR_PREFIX + " " + 
					cleanFormatCpp(methodinst.getReturnType(), ""));
			}
			// - substitute in parameter types 
			for (Iterator pi = method.parameterIterator(); pi.hasNext();) {
				Parameter parameter = (Parameter)pi.next();
				Parameter parameterinst = new Parameter();
				parameterinst.setName(parameter.getName());
				parameterinst.setType(instantiateTemplate(parameter.getType(), substitution,macros));
				if (parameter.hasDefault()) {
					parameterinst.setDefault(parameter.getDefaultString());										
				}
				methodinst.addParameter(parameterinst);
			}
			// - add instantiated method to new class
			templateInstance.getScope().addMember(methodinst, 
				connection.getVisibility(),
				connection.getVirtuality(), 
				connection.getStorage());
		}
		// ------------------------
		// Instantiate data members
		// ------------------------
		for (Iterator mi = template.getScope().fieldIterator(); 
			mi.hasNext();) {
			// - get a field
			ContainedConnection connection = (ContainedConnection)mi.next();
			Field field = (Field)connection.getContained();
			Field instance = new Field();
			instance.setName(field.getName());
			// - copy properties
			for (Iterator propi = field.propertyIterator(); propi.hasNext();)
			{
				Entity.Property property = (Entity.Property)propi.next();
				instance.addProperty(property);
			}
			// - substitute in variable type
			instance.setType(
				instantiateTemplate(field.getType(), substitution, macros));
			// - add field to template instance
			templateInstance.getScope().addMember(instance,
				connection.getVisibility(), connection.getStorage());
		}
		// -------------------------------
		// Fill inner class instantiations
		// -------------------------------
		for (Aggregate[] element: inners) {
			Aggregate innerClass = (Aggregate)element[0];
			Aggregate instance = (Aggregate)element[1];
			instantiateTemplate(innerClass, new TemplateArgument[0],
					substitution, instance, existingInstancesMap);
		}
		
		if (existingInstancesMap != null) {
			String expression = templateExpression(template, arguments);
			existingInstancesMap.put(expression, templateInstance);
		}
		
		return templateInstance;
	}
	
	/**
	 * Substitutes template parameters for actual arguments in a
	 * type expression.
	 * @param type
	 * @param substitution type substitution map (Entity -&gt; Type)
	 * @param macros textual substitution map (String -&gt; String)
	 * @return transformed type
	 * @throws InappropriateKindException
	 */
	private static Type instantiateTemplate(Type type, Map substitution, Map macros)
		throws InappropriateKindException
	{
		final Map final_substitution = substitution;
		final Map final_macros = macros;
		
		Type.ExtendedTransformation ttransform = 
		new Type.ExtendedTransformation() {
			public TypeNode transform(TypeNode original)
				throws InappropriateKindException {
				// Transform all other leaf nodes according to substitution
				// map
				if (original.getKind() == Type.TypeNode.NODE_LEAF) {
					// Make sure I am not the class template itself in a legal
					// instantiation (that is, never transform 'A' in 'A<X>')
					if (original.getParent() != null
						&& ((Type.TypeNode)original.getParent()).getKind()
							== Type.TypeNode.NODE_TEMPLATE_INSTANTIATION 
						&& original.getParent().getChildAt(0) == original) {
						return null;
					}
					Entity base = original.getBase();
					Object replaced = final_substitution.get(base);
					if (replaced != null) {
						Type.TypeNode transformed = Type.transformType
										((Type)replaced, this).getRootNode();
						transformed.setCV(original.getCV());
						return transformed;
					}
					else						
						return null;
				}
				else
					return null;
			}

			/**
			 * Replace the template argument "K" with an appropriate argument
			 * from the macros map, if such substitution is provided; otherwise,
			 * transform argument as if it were a type.
			 */
			public TemplateArgument transform(TemplateArgument original) {
				if (original instanceof TypenameTemplateArgument) {
					// notice that it may be also be a data template argument
					// that was not recognized by the source analyzer
					Type type = ((TypenameTemplateArgument)original).getValue();
					try {
						// - attempt substitution from macros 
						if (type.getRootNode().getKind() == Type.TypeNode.NODE_LEAF) {
							Entity base = type.getRootNode().getBase();
							Object substitution = final_macros.get(base.getName());
							if (substitution != null && 
									substitution instanceof TemplateArgument) {
								return (TemplateArgument)substitution;
							}
						}
						// - attempt type transformation
						Type xformed = Type.transformType(type, this);
						if (xformed != type)
							return new TypenameTemplateArgument(xformed);
					} catch (InappropriateKindException e) {
						/* fail and fall back */
					}
				}
				return original;
			}
		};
		return Type.transformType(type, ttransform); 
	}
	
	private static TemplateArgument[] instantiateTemplate
		(TemplateArgument[] types, Map substitution, Map macros) 
		throws InappropriateKindException
	{
		TemplateArgument[] transformedTypes = 
			new TemplateArgument[types.length];
		// Transform TypenameTemplateArgument elements, copy the rest
		for (int index = 0; index < types.length; ++index) {
			TemplateArgument element = (TemplateArgument)types[index];
			if (element instanceof TypenameTemplateArgument) {
				// Substitute T in argument
				Type value = ((TypenameTemplateArgument)element).getValue();
				// - substitute macros
				Object macroReplacement = macros.get(value.formatCpp());
				if (macroReplacement != null 
					&& macroReplacement instanceof TemplateArgument) {
					transformedTypes[index] = (TemplateArgument)macroReplacement;
				}
				else {
					// - regular substitution
					Type transformedValue = instantiateTemplate(value, 
						substitution, macros);
					transformedTypes[index] = 
						new TypenameTemplateArgument(transformedValue);
				}
			}
			else
				transformedTypes[index] = element;
		}
			
		return transformedTypes;
	}
	
	/**
	 * Creates a template instance for an inner typedef element.
	 * @param inner
	 * @param substitution
	 * @return
	 * @throws InappropriateKindException
	 */
	private static Alias instantiateTemplate(Alias inner, Map substitution, 
		Map macros)
		throws InappropriateKindException
	{
		Alias instance = new Alias();
		instance.setName(inner.getName());
		instance.setAliasedType(
			instantiateTemplate(inner.getAliasedType(), substitution, macros));
		return instance;
	}

	/**
	 * Utility function. Collects all occurances of inner types of one of 
	 * the template parameters of the given class, and adds them to
	 * the substitution map.
	 * @param type
	 * @param template class which holds template parameters
	 * @param substitution accumulates the inner entities collected
	 */
	private static void collectInnersOfTemplateParameter(Type type, 
		Aggregate template, List arguments, Map<Entity, Type> substitution, Map<String, Aggregate> existingInstancesMap)
	{
		final Aggregate fin_template = template;
		final Map<Entity, Type> fin_substitution = substitution;
		final TemplateArgument[] fin_targs =
			(TemplateArgument[])arguments.toArray(new TemplateArgument[] { });
		final Map<String, Aggregate> fin_existingInstancesMap = existingInstancesMap;
		
		Type.Transformation collect = new Type.Transformation() {
			public TypeNode transform(TypeNode typeNode) 
				throws InappropriateKindException 
			{
				// This "transform" method does not really change the type
				// expression tree, it only scans it
				if (typeNode.getKind() == Type.TypeNode.NODE_LEAF) {
					String name = typeNode.getBase().getFullName();
					// - check whether 'name' begins with one of the template
					//   parameters of 'template'
					int index = 0;
					for (Iterator tpi = fin_template.templateParameterIterator();
						 tpi.hasNext(); ++index) {
						// - get names of template parameter and argument
						TemplateParameter parameter = 
							(TemplateParameter)tpi.next();
						String parameterName = parameter.getName();
						TemplateArgument argument = fin_targs[index];
						// - check criteria
						if (name.startsWith(parameterName + "::")) {
							// - replace with actual argument string and 
							//  create a substitution in the map
							String traitSpec = name.substring(parameterName.length() + 2);
							Entity entity = trait(argument, traitSpec, fin_existingInstancesMap);
							fin_substitution.put(typeNode.getBase(), 
								new Type(new Type.TypeNode(entity)));
						}
					}
				}
				return null;
			}
		};
		// Transform the given type using the type transformation utility 
		// in sourceanalysis.Type
		try {
			Type.transformType(type, collect);
		}
		catch (InappropriateKindException ike) {
			// ignore
		}
	}

	private static Entity trait(TemplateArgument traitsHolder, String field, Map<String, Aggregate> existingInstancesMap)
	{
		if (traitsHolder instanceof TypenameTemplateArgument) {
			Type val = ((TypenameTemplateArgument)traitsHolder).getValue();
			Entity traitsEntity = val.getBaseType();
			TemplateArgument[] targs = val.getTemplateArguments();
			// - if the referenced entity is a class, look for a member there
			if (traitsEntity instanceof Aggregate) {
				Aggregate traitsClass = (Aggregate)traitsEntity;
				boolean foundInstantiation = false;
				if (targs != null) {
					String expression = templateExpression(traitsClass, targs);
					if (existingInstancesMap != null 
							&& existingInstancesMap.containsKey(expression)) {
						traitsClass = (Aggregate)existingInstancesMap.get(expression);
						foundInstantiation = true;
						//System.err.println("Found instantiation for " + expression + ": " + traitsClass);
					}
				}
				if(targs == null || foundInstantiation) {
					Entity element = lookup(traitsClass.getScope(), field);
					if (element != null)
						return element;
				}
			}
		}
		// - fallback to creating a new orphan entity using macro replacement
		String replaced = traitsHolder.toCpp() + "::" + field; 
		Aggregate entity = new Aggregate(); 
		entity.setName(replaced);
		return entity;
	}
	
	/**
	 * Looks for an Entity contained in a parent scope, by name. The search is
	 * shallow, that is, does <b>not</b> descend to sub-scopes.
	 * @param scope container scope
	 * @param componentname name of component to look for
	 * @return and Entity by the given name. If no such entity is found, 
	 * <b>null</b> is returned.
	 */
	public static Entity lookup(Scope scope, String componentname)
	{
		// Find aggregates
		for (Iterator aggiter = scope.aggregateIterator(); aggiter.hasNext(); ) {
			ContainedConnection connection = (ContainedConnection)aggiter.next();
			Aggregate agg = (Aggregate)connection.getContained();
			if (agg.getName().equals(componentname))
				return agg;
		}
		// Find namespaces
		for (Iterator nsiter = scope.namespaceIterator(); nsiter.hasNext(); ) {
			ContainedConnection connection = (ContainedConnection)nsiter.next();
			Namespace ns = (Namespace)connection.getContained();
			if (ns.getName().equals(componentname))
				return ns;
		}
		// Find enums
		for (Iterator enumiter = scope.enumIterator(); enumiter.hasNext(); ) {
			ContainedConnection connection = (ContainedConnection)enumiter.next();
			sourceanalysis.Enum enume = (sourceanalysis.Enum)connection.getContained();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC
					&& enume.getName().equals(componentname))
				return enume;
		}
		// Find typedefs
		for (Iterator aliter = scope.aliasIterator(); aliter.hasNext(); ) {
			ContainedConnection connection = (ContainedConnection)aliter.next();
			Alias alias = (Alias)connection.getContained();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC
					&& alias.getName().equals(componentname))
				return alias;
		}
		// - not found
		return null;
	}
	
	/**
	 * Looks for an Entity contained in a container entity, by name. The search is
	 * shallow, that is, does <b>not</b> descend to sub-scopes.
	 * @param entity entity to search in
	 * @param componentname name of component to look for
	 * @return and Entity by the given name. If no such entity is found, 
	 * <b>null</b> is returned.
	 */
	public static Entity lookup(Entity entity, String componentname)
	{
		if (entity instanceof Aggregate) {
			return lookup(((Aggregate)entity).getScope(), componentname);
		}
		else if (entity instanceof Namespace) {
			return lookup(((Namespace)entity).getScope(), componentname);
		}
		else if (entity instanceof Alias) {
			Entity unalias = naiveUnalias(entity);
			if (unalias == entity) return null;
			return lookup(unalias, componentname);
		}
		else {
			return null;
		}
	}
	
	/**
	 * Describes a template instance using a simple (human-readable) string.
	 * @param base class template being used
	 * @param targs argument for template
	 * @return a textual expression
	 */
	public static String templateExpression(Aggregate base, TemplateArgument[] targs)
	{
		StringBuffer expressionbuf = new StringBuffer();
		expressionbuf.append("template-instantiation(");
		expressionbuf.append(base.getFullName());
		for (int i = 0; i < targs.length; i++) {
			TemplateArgument argument = targs[i];
			expressionbuf.append(",");
			expressionbuf.append(argument.toString()); 
		}
		expressionbuf.append(")");
		return expressionbuf.toString();
	}
	
	/**
	 * Describes a template instance using C++ syntax.
	 * @param base class template being used
	 * @param targs argument for template
	 * @return a textual expression
	 */
	public static String templateCppExpression(Aggregate base, TemplateArgument[] targs)
	{
		StringBuffer expressionbuf = new StringBuffer();
		expressionbuf.append(Utils.cleanFullName(base));
		expressionbuf.append("< ");
		for (int i = 0; i < targs.length; i++) {
			TemplateArgument argument = targs[i];
			if (i>0) expressionbuf.append(",");
			// Add a C++ representation of the template argument
			if (argument instanceof TypenameTemplateArgument) {
				Type type = ((TypenameTemplateArgument)argument).getValue();
				expressionbuf.append(Utils.cleanFormatCpp(type,""));
			}
			else
				expressionbuf.append(argument.toString()); 
		}
		expressionbuf.append(" >");
		return expressionbuf.toString();
	}

	/**
	 * Gets a base class name for the inheritance, also applying the
	 * contents of template arguments.
	 * @param connection the InheritanceConnection containing the
	 * information
	 * @return a formatted name string
	 */
	public static String actualBaseName(InheritanceConnection connection)
	{ 
		Aggregate base = connection.getBase();
		if (base.isTemplated())
			return templateCppExpression(base,
				connection.getBaseTemplateArguments());
		else
			return cleanFullName(base);
	}
	
	/**
	 * Gets a base type, also applying the contents of template arguments.
	 * @param type a type expression containing the information - should
	 * be flat
	 * @return a formatted name string
	 */
	public static String actualBaseName(Type type)
	{
		Entity base = type.getBaseType();
		if (type.getTemplateArguments() != null) {
			return templateCppExpression((Aggregate)base,
				type.getTemplateArguments());
		}
		else {
			return cleanFullName(base);
		}
	}

	/**
	 * This method will compare the two routines and return the answer
	 * 'true' if they are identical signature-wise and 'false' otherwise.
	 * @param first The first routine to compare.
	 * @param second The second routine to compare.
	 * @return 'true' if the signatures are identical, 'false' otherwise.
	 */
	public static boolean hasCompatibleSignatures(Routine first, Routine second) 
		throws MissingInformationException {
			
		// Compare names.
		String firstName = first.getName();
		String secondName = second.getName();
		if( ! firstName.equals(secondName) ) {
			return false;
		}
						
		// Compare constness
		if (first.isConst() != second.isConst())
			return false;

		// Compare all arguments types
		Iterator fiter = first.parameterIterator();
		Iterator siter = second.parameterIterator();
		while( fiter.hasNext() || siter.hasNext() ) {
			
			if( ! fiter.hasNext() || ! siter.hasNext() ) {
				return false;
			}
			
			Parameter fparam = (Parameter)fiter.next();
			Parameter sparam = (Parameter)siter.next();
			
			String firstArg = fparam.getType().formatCpp();
			String secondArg = sparam.getType().formatCpp();
			
			if( ! firstArg.equals(secondArg) ) {
				return false;
			}
		}

		return true;
	}
	
	/**
	 * Returns the narrower visibility access (protected->public, etc) 
	 * @param current visibility level
	 * @return the next more strict visibility level 
	 */
	public static int getNextVisibilityLevel(int visibility) {
		switch(visibility) {
		case Specifiers.Visibility.PUBLIC:
			return -1;
		case Specifiers.Visibility.PROTECTED:
			return Specifiers.Visibility.PUBLIC;
		case Specifiers.Visibility.PACKAGE:
			return Specifiers.Visibility.PROTECTED;
		case Specifiers.Visibility.PRIVATE:
			return Specifiers.Visibility.PROTECTED;
		default:
			return -1;
		}
	}
	
	public static boolean isVisible(int minimumVisibility, int memberVisibility) {
		switch(minimumVisibility) {
		case Specifiers.Visibility.PUBLIC:
			return memberVisibility == Specifiers.Visibility.PUBLIC;
		case Specifiers.Visibility.PROTECTED:
			return memberVisibility == Specifiers.Visibility.PUBLIC ||
				   memberVisibility == Specifiers.Visibility.PROTECTED;
		case Specifiers.Visibility.PACKAGE:
			return memberVisibility == Specifiers.Visibility.PACKAGE ||
				   memberVisibility == Specifiers.Visibility.PROTECTED ||
				   memberVisibility == Specifiers.Visibility.PUBLIC;
		case Specifiers.Visibility.PRIVATE:
			return memberVisibility == Specifiers.Visibility.PACKAGE ||
			   memberVisibility == Specifiers.Visibility.PROTECTED ||
			   memberVisibility == Specifiers.Visibility.PUBLIC ||
			   memberVisibility == Specifiers.Visibility.PRIVATE;
		default:
			return false;
		}
		
	}
	
	
	/**
	 * Collects all of the public inheritance-accessible fields in the given class
	 */
	public static Collection<Field> accessibleFields(Aggregate subject, Map instanceMap, 
			int minimumAllowedVisibility)
		throws MissingInformationException
	{
		
		List<Field> accessible = new LinkedList<Field>();
		
		for(Iterator subjectFieldIter = subject.getScope().fieldIterator(); 
			subjectFieldIter.hasNext();) 
		{
			ContainedConnection fieldConnection = (ContainedConnection)subjectFieldIter.next();
			Field myField = (Field)fieldConnection.getContained();
			
			if(isVisible(minimumAllowedVisibility, fieldConnection.getVisibility())) {
				accessible.add(myField);
			}
			
		}
		
		// Get virtual methods from base classes
		for (Iterator baseIter = subject.baseIterator(); baseIter.hasNext(); )
		{
			InheritanceConnection bconnection =
				(InheritanceConnection)baseIter.next();
			Aggregate base = bconnection.getBase();
			TemplateArgument targs[] = bconnection.getBaseTemplateArguments();
			if (targs != null) {
				String expr = templateExpression(base, targs);
				if (instanceMap != null) 
					base = (Aggregate)instanceMap.get(expr);
			}
			if (base == null) continue;
			
		
			
			// we need to fetch the fields based on allowedVisibility
			// and our inheritance connection type
			int baseMinimumAllowedVisibility = -1;
			
			
			
			switch(bconnection.getVisibility()) {
			case Specifiers.Visibility.PUBLIC:
				// private -> private
				// protected -> protected
				// public -> public
				baseMinimumAllowedVisibility = minimumAllowedVisibility;
				break;
			case Specifiers.Visibility.PROTECTED:
				// public -> protected
				// protected -> private
				// private -> (not inherited)
				
				if(minimumAllowedVisibility != Specifiers.Visibility.PUBLIC) {
					baseMinimumAllowedVisibility = getNextVisibilityLevel(minimumAllowedVisibility);
				}
				
				break;
			case Specifiers.Visibility.PRIVATE:
				// nothing 
				break;
			case Specifiers.Visibility.PACKAGE:
				// nothing too
				break;
			}
			
			// we can't get any new fields from this base
			if(baseMinimumAllowedVisibility == -1) {
				continue;
			}
			
			Collection<Field> baseVisibleFields = accessibleFields(base, instanceMap, baseMinimumAllowedVisibility);
			for (Field baseField: baseVisibleFields) {
                // - ensure that a compatible method has not previously 
                //  occurred
				boolean found = false;
				
				for (Field childField: accessible) {
					if (childField.getName().equals(baseField.getName())) {
						found = true;
						break;
					}
				}
				
				if(!found) {
					accessible.add(baseField);
				}
            }
		}
		
		return accessible;
	}
	
	/**
	 * Collects all the virtual methods that are in the given class - including
	 * those inherited from base classes.
	 * @param subject a class to observe
	 * @param instanceMap a map containing mappings from template expressions
	 * (in the format returned by templateExpression) to Aggregate entities
	 * that have been instantiated for them
	 * @return a collection of Routine objects which are declared virtual in 
	 * the subject, or in any of it's ancestors
	 * @throws MissingInformationException
	 */
	public static Collection<Routine> virtualMethods(Aggregate subject,
			Map<String, Aggregate> instanceMap, boolean withDestructors) throws MissingInformationException
	{
		List<Routine> virtual = new LinkedList<Routine>();
		
		// Add virtual methods declared in this class
		for (Iterator subjectMethodIter = subject.getScope().routineIterator();
		     subjectMethodIter.hasNext(); ) {

			ContainedConnection rconnection =
				(ContainedConnection)subjectMethodIter.next();
			Routine myMethod = (Routine)rconnection.getContained();

			if (rconnection.getVirtuality() 
			    != Specifiers.Virtuality.NON_VIRTUAL &&
				(withDestructors || !myMethod.isDestructor())) {
			    virtual.add(myMethod); 
			}
		}
		
		// Get virtual methods from base classes
		for (Iterator baseIter = subject.baseIterator(); baseIter.hasNext(); )
		{
			InheritanceConnection bconnection =
				(InheritanceConnection)baseIter.next();
			Aggregate base = bconnection.getBase();
			TemplateArgument targs[] = bconnection.getBaseTemplateArguments();
			if (targs != null) {
				String expr = templateExpression(base, targs);
				if (instanceMap != null) 
					base = (Aggregate)instanceMap.get(expr);
			}
			if (base == null) continue;
			// - recursively fetch the virtual methods in the base class
			Collection<Routine> baseVirtual = virtualMethods(base, instanceMap, withDestructors);
			for (Routine baseMethod: baseVirtual) {

                // - ensure that a compatible method has not previously 
                //  occurred
				boolean found = false;
				
				for (Routine virtMethod: virtual) {
					if (virtMethod.isCompatible(baseMethod)) {
						found = true;
						break;
					}
				}
				
				if (!found) {
				    virtual.add(baseMethod);
                }
            }
		}

		return virtual;
	}
	
	/**
	 * Collects all the pure-virtual methods that are not supplied with a body
	 * in the given class - including those inherited from abstract base 
	 * classes. 
	 * @param subject a class to observe
	 * @param instanceMap a map containing mappings from template expressions
	 * (in the format returned by templateExpression) to Aggregate entities
	 * that have been instantiated for them
	 * @return a collection of Routine objects which are declared are pure
	 * virtual in some ancestor of subject, and not implemented in subject or
	 * any of its ancestors.
	 * @throws MissingInformationException
	 */
	public static Collection unimplementedMethods(Aggregate subject,
			Map instanceMap) throws MissingInformationException
	{
		List unimplemented = new LinkedList();
		
		// Get unimplemented methods from base classes
		for (Iterator baseIter = subject.baseIterator(); baseIter.hasNext(); )
		{
			InheritanceConnection bconnection =
				(InheritanceConnection)baseIter.next();
			Aggregate base = bconnection.getBase();
			TemplateArgument targs[] = bconnection.getBaseTemplateArguments();
			if (targs != null) {
				String expr = templateExpression(base, targs);
				if (instanceMap != null) 
					base = (Aggregate)instanceMap.get(expr);
			}
			if (base == null) continue;
			// - recursively fetch the unimplemented methods in the base class
			Collection<Routine> baseUnimplemented = unimplementedMethods(base, instanceMap);
			for (Routine baseMethod: baseUnimplemented) {
				// - search this method in subject to check that the method is
				//   still unimplemented in derived class
				boolean found = false;
				
				for (Iterator subjectMethodIter = subject.getScope()
					.routineIterator(); subjectMethodIter.hasNext(); ) {
					ContainedConnection rconnection =
						(ContainedConnection)subjectMethodIter.next();
					Routine myMethod = (Routine)rconnection.getContained();
					// - compare unimplemented method in parent and method in
					//   derived
					if (rconnection.getVirtuality() 
						  != Specifiers.Virtuality.PURE_VIRTUAL 
					 	&& hasCompatibleSignatures(baseMethod, myMethod)) {
						found = true;
						break;
					}
				}
				
				if (!found) {
					unimplemented.add(baseMethod);
				}
			}
		}

		// Add pure virtual methods declared in this class
		for (Iterator subjectMethodIter = subject.getScope()
			.routineIterator(); subjectMethodIter.hasNext(); ) {

			ContainedConnection rconnection =
				(ContainedConnection)subjectMethodIter.next();
			Routine myMethod = (Routine)rconnection.getContained();
	
			if (rconnection.getVirtuality() 
				== Specifiers.Virtuality.PURE_VIRTUAL) {
				unimplemented.add(myMethod); 
			}
		}
		
		return unimplemented;
	}
	
	/**
	 * Collects all the pure-virtual methods that are not supplied with a body
	 * in the given class - including those inherited from abstract base 
	 * classes.
	 * <p>This version assumes Utils.defaultInstanceMap for the instance map.
	 * </p>
	 * @note This function is not thread-safe, unless it is guarenteed that
	 * the defaultInstanceMap does not change.
	 * @param subject a class to observe
	 * @return a collection of Routine objects which are declared are pure
	 * virtual in some ancestor of subject, and not implemented in subject or
	 * any of its ancestors.
	 * @throws MissingInformationException if there is not enough information
	 * in the program database to process the request.
	 */
	public static Collection unimplementedMethods(Aggregate subject)
			throws MissingInformationException
	{
		return unimplementedMethods(subject, defaultInstanceMap);
	}
	
	/**
	 * Checks whether the two given entities are compatible, that is, refer
	 * to the same actual entity in the program - this may happen for non-
	 * identical entities in case of typedefs in the program. 
	 * @param first the first entity for comparison
	 * @param second the second entity for comparison
	 * @return <b>true</b> if entities refer to the same entity (eventually); 
	 * <b>false</b> otherwise.
	 */
	private static boolean areCongruent(Entity first, Entity second)
	{
		return (naiveUnalias(first) == naiveUnalias(second));
	}
	
	/**
	 * Resolves typedefs in rather a silly way - goes down the aliasing chain
	 * as long as all the types are flat, taking the base type of each (thus 
	 * <b>ignoring</b> pointer and reference modifiers).
	 * @param entity the original entity to reference
	 * @return the final entity reached via this procedure
	 */
	private static Entity naiveUnalias(Entity entity)
	{
		while (entity instanceof Alias) {
			Type type =  ((Alias)entity).getAliasedType();
			if (!type.isFlat()) return entity; // cannot handle non-flat types
			entity = type.getBaseType();
			if (entity.isTemplated()) {
				IncompleteTemplateInstance instance = 
					new IncompleteTemplateInstance();
				instance.setName(type.formatCpp());
				instance.setType(type);
				
				if (entity instanceof Aggregate) {
					instance.assimilate(((Aggregate)entity).getScope());
				}
				entity = instance;
			}
		}
		return entity;
	}
	
	/**
	 * Resolves typedefs for flat types. Only works if the base type itself is
	 * an alias without qualifiers - in that case, the returned value is the 
	 * aliased type; otherwise, no resolution is made and the original type is
	 * returned.
	 * @param type original type expression
	 * @return resolved type expression (may be the original type if no
	 * resolution takes place).
	 */
	public static Type flatUnalias(Type type)
	{
		Entity base = type.getBaseType();
		if (!type.isReference() &&type.getPointerDegree() == 0 
				&& base instanceof Alias)
			return ((Alias)base).getAliasedType();
		else
			return type;
	}
	
	/**
	 * Looks for an original template entity from which the requested entity
	 * is deduced.
	 * @param entity the entity (possibly templated) to check
	 * @return a templated entity from which 'entity' originates, if is does
	 * originate from one. <b>null</b> otherwise. 
	 */
	private static Entity seekOriginalTemplate(Entity entity)
	{
        if (!(entity instanceof TemplateEnabledEntity))
            return null;

		// Follow specialization relationships
        TemplateEnabledEntity templateEnabled = (TemplateEnabledEntity)entity;
        if (templateEnabled.isSpecialized())
            return templateEnabled.getGeneralTemplateForSpecialization().getGeneral(); 
        return null;
	}

    /**
     * Looks for the lowest parent declaration which is equivalent to the
     * given routine, i.e. that the given routine is an overriding 
     * implementation thereof.
     * @param routine a routine in a derived class. Must be contained in
     *   an Aggregate entity.
     * @return a compatible routine in a parent class
     * @throws ElementNotFoundException if such a declaration cannot be
     *   located
     * @throws MissingInformationException if information about base classes
     *   is insufficient to determine presence of such a declaration
     */
    public static Routine seekBaseDeclaration(Routine routine) 
        throws ElementNotFoundException, MissingInformationException
    {
        Entity container = routine.getContainer();
        if (!(container instanceof Aggregate))
            throw new ElementNotFoundException("routine", routine.getName());

        Aggregate derived = (Aggregate)container;

        for (Iterator baseIter = derived.baseIterator(); baseIter.hasNext(); ) {
            Aggregate base = ((InheritanceConnection)baseIter.next()).getBase();
            for (Iterator routineIter = base.getScope().routineIterator();
                 routineIter.hasNext(); ) {
                ContainedConnection conn = 
                    (ContainedConnection)routineIter.next();
                Routine candidate = (Routine)conn.getContained();
                //System.err.println("candidate " + candidate.getFullName());
                if (routine.isCompatible(candidate))
                    return candidate;
            }
        }
        throw new ElementNotFoundException("routine", routine.getName());
    }
	
	public static class FileTools {
		/**
		 * Breaks a path to elements. The first element is the root directory,
		 * followed by lower path elements.
		 * @param file a file
		 * @return a List of File objects
		 */
		public static List pathElements(File file)
		{
			List elements = new LinkedList();
			// Scan the tree towards the top
			File fileRef = null;
			
			try {
				fileRef = file.getCanonicalFile();
			}
			catch (IOException e) {
				fileRef = file.getAbsoluteFile();
			}
			
			while (fileRef != null) {
				elements.add(0, fileRef);
				// - go up
				fileRef = fileRef.getParentFile();
			}
			return elements;
		}
		
		/**
		 * Transforms an absolute path into a relative path starting from a
		 * specified directory.
		 * @param target target (absolute-path) File
		 * @param directory starting directory
		 * @return a relative-path File object
		 */
		public static File absoluteToRelative(File target, File directory)
		{
			List targetPathElements = pathElements(target);
			List directoryPathElements = pathElements(directory);
			directoryPathElements.add(new File("."));
			// Find the longest common prefix of the two paths
			Iterator targetIter = targetPathElements.iterator();
			Iterator directoryIter = directoryPathElements.iterator();
			File last = new File("/");
			while (targetIter.hasNext() && directoryIter.hasNext()
					&& (last = (File)targetIter.next()).equals(directoryIter.next())) ;
			// Write ".." elements for the rest of directoryIter
			StringBuffer relative = new StringBuffer();
			while (directoryIter.hasNext()) {
				relative.append("../");
				directoryIter.next();
			}
			// Return the rest of the elements in targetIter
			relative.append(last.getName());
			while (targetIter.hasNext()) {
				File element = (File)targetIter.next();
				relative.append("/" + element.getName());
			}
			return new File(relative.toString());
		}
		
		/**
		 * Transforms an absolute path into a relative path starting from a
		 * specified directory.
		 * @param target target (absolute) filename
		 * @param directory starting directory
		 * @return a relative path string
		 */
		public static String absoluteToRelative(String target, File directory)
		{
			return absoluteToRelative(new File(target), directory).getPath();
		}
	}
	
	// A formatter which uses cleanFullName() instead of Entity.getFullName()
	// for formatting base-names.
	static public final Type.BaseTypeFormatter CLEAN_TYPE_FORMATTER =
		new Type.BaseTypeFormatter() {
			public String formatBase(Entity e) { return cleanFullNameBase(e); }
		};
		
	static public Map<String, Aggregate> defaultInstanceMap = null;

	public static String getTypeHash(Type type) {
		return Integer.toHexString(type.formatCpp().hashCode());
	}
}
