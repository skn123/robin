/*
 * Created on Dec 8, 2003
 *
 */
package backend.robin;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import backend.GenericFilters;
import backend.Utils;
import backend.robin.model.TypeToolbox;
import sourceanalysis.*;

/**
 * This is an auxiliary class for CodeGenerator; it contains some functions
 * for determining which entities to wrap and which to exclude.
 */
public class Filters extends GenericFilters {
	
	static private boolean hasFlag(Entity entity, String flag)
	{
		try {
			Entity.Property robinProp = entity.findProperty(".robin");
			if (robinProp.getValue().indexOf(flag) >= 0) {
				/* Flag criterion fulfilled */
				return true;
			}
			else return false;
		}
		catch (ElementNotFoundException e) {
			/* no ".robin" property - criterion unfulfilled */
			return false;
		}
	}
	
	/**
	 * Checks whether the user explicitly requested exclusion of the
	 * mentioned entity from Robin wrapping using the doxygen element
	 * <code>\@par .robin unavailable</code>.
	 * @param entity
	 * @return
	 */
	static private boolean isExplicitlyExcluded(Entity entity)
	{ 
		return hasFlag(entity, "unavailable");
	}
	
	/**
	 * The "assignment" property indicates that the class has an
	 * implicit assignment operator. The operator will be wrapped to
	 * enable copying of the object in the target language; Griffin
	 * cannot detect the existance of a compiler-generated operator.
	 * Furthermore, not every class that does have such an operator would
	 * like to expose it.
	 * @param entity a class being checked
	 * @return <b>true</b> if the property "assignment" is expressed
	 * for this entity.
	 */
	static /* package */ boolean isAssignmentSupportive(Entity entity)
	{
		return hasFlag(entity, "assignment");
	}
	
	/**
	 * The "clonable" property indicates that a "clone()" function should
	 * be automatically generated using the class' copy constructor. The
	 * user is responsible of supplying such a constructor.
	 * @param entity a class begin checked
	 * @return <b>true</b> if the flag "clonable" is specified for this
	 * entity.
	 */
	static /* package */ boolean isCloneable(Entity entity)
	{
		return hasFlag(entity, "cloneable");
	}

    /**
     * Checks if the class can be extended in any way.
     * @param entity
     */
    static boolean isClassExtendible(Aggregate entity)
    {
        return !Utils.hasOnlyPrivateConstructors(entity);
    }
	
	/**
	 * Determines whether a routine should or should not be wrapped.
	 * A routine may be excluded from the wrapping if one of the following
	 * criteria fulfills:
	 * <ul>
	 * <li>The routine's signature is not flat (contains non-flat types)</li>
	 * <li>The routine is using C-style arrays in its declaration</li>
	 * <li>The routine is a template</li>
	 * <li>The routine is not declared in a header file</li>
	 * <li>The routine is tagged with the doxygen element @par .robin
	 * unavailable</li>
	 * </ul>
	 * @param routine
	 */
	static boolean isAvailable(Routine routine)
	{
		return
			(Utils.allAreFlat(routine)
			 && !Utils.hasAnyArrays(routine)
			 && !routine.isTemplated() 
			 && ((routine.hasContainer() 
					 && routine.getContainer() instanceof Aggregate) 
			 	 || isDeclared(routine))
			 && !isExplicitlyExcluded(routine));
	}

	/**
	 * Determines whether a routine should or should not be wrapped as a
	 * static call.
	 * A non-static routine should be wrapped as a static call when all the
	 * following are fulfilled:
	 * + The routine is available for wrapping (isAvailable)
	 * + The routine is neither a constructor nor a destructor
	 * + The routine is publicly visible
	 * + The routine is virtual, but not pure
	 */
	public static boolean isAvailableStatic(Routine routine)
	{
		ContainedConnection uplink = routine.getContainerConnection();
		
		return (isAvailable(routine)
				&& !routine.isConstructor() && !routine.isDestructor()
				&& uplink.getVisibility() == Specifiers.Visibility.PUBLIC
				&& uplink.getVirtuality() == Specifiers.Virtuality.VIRTUAL);
	}
		


	/**
	 * Tells whether the specified type is primitive with no
	 * indirections, such as <code>int</code>.
	 * 
	 * @param type
	 * @return
	 */
	static boolean isDirectPrimitive(Type type)
	{
		Entity base = type.getBaseType();
		return (type.isFlat() 
				&& type.getPointerDegree() == 0 && !type.isReference()
				&& (base instanceof Primitive
					|| (base instanceof Alias
						&& isDirectPrimitive((Alias)base)) ) );
	}
	
	/**
	 * Tells whether the given alias is simply a primitive type without
	 * indirections, such as <code>typedef int uid</code>.
	 * 
	 * @param alias
	 * @return
	 */
	static boolean isDirectPrimitive(Alias alias)
	{
		Type type = alias.getAliasedType();
		return isDirectPrimitive(type);
	}

	

	/**
	 * Checks whether a field should be wrapped. A field is included in the
	 * wrapping if:
	 * <ul>
	 *   <li>It is visible (i.e. not static or in an anonymous namespace)</li>
	 *   <li>It's declared in a header file</li>
	 *   <li>Its type is flat and is not an array type</li>
	 * </ul>
	 * @param field
	 * @param up the connection from the field to its container
	 * @return
	 */
	static boolean isAvailable(Field field, ContainedConnection up)
	{
		try {
			return (up.getVisibility() == Specifiers.Visibility.PUBLIC
					&& (up.getContainer() instanceof Aggregate 
						|| isDeclared(field)
							&& up.getStorage() != Specifiers.Storage.STATIC)
					&& field.getType().isFlat()
					&& !field.getType().isArray()
					&& !isExplicitlyExcluded(field));
		}
		catch (MissingInformationException e) {
			return false;
		}
	}

	static boolean isAvailableStatic(Field field, ContainedConnection up)
	{
		try {
			return (up.getVisibility() == Specifiers.Visibility.PUBLIC
					&& (up.getContainer() instanceof Aggregate 
							&& up.getStorage() == Specifiers.Storage.STATIC
						|| up.getContainer() instanceof Namespace
						    && isDeclared(field)
							&& up.getStorage() != Specifiers.Storage.STATIC)
					&& field.getType().isFlat()
					&& !field.getType().isArray()
					&& !isExplicitlyExcluded(field));
		}
		catch (MissingInformationException e) {
			return false;
		}
	}
	
	/**
	 * Checks if the user specifically requested the return value of a
	 * function to be borrowed (which means the CodeGenerator will signify
	 * the return value with '&' instead of '*').
	 * @param routine the function in question
	 * @return <b>true</b> if the borrow flag (<code>returns borrowed</code>)
	 * is specified; <b>false</b> otherwise.
	 */
	public static boolean isForceBorrowed(Routine routine)
	{
		return hasFlag(routine, "returns borrowed");
	}
	
	/**
	 * Checks if the user specifically requested a parameter to be an output
	 * parameter (which means the CodeGenerator will signify its time
	 * with '>' instead of '*' or '&').
	 * @param parameter the parameter in question
	 * @return <b>true</b> if the output flag (<code>[output]</code>) is
	 * specified for this parameter, making it an input/output parameter;
	 * <b>false</b> for a "regular" input-only parameter. 
	 */
	static boolean isOutputParameter(Parameter parameter)
	{
		// check if this is a non const reference
		try {
			Type type = parameter.getType();
			if ( type.isFlat() && type.isReference() &&
			    !type.isConst() && !(type.getBaseType() instanceof Primitive)) {
				return true;
			}
		}
		catch(MissingInformationException e) {
			
		}
		
		// check if the description contains [output]
		try { 
			if (parameter.findProperty("description").getValue()
			    .indexOf("[output]") >= 0) {
				return true;
			}
		}
		catch (ElementNotFoundException e) {
			
		}
		
		return false;
	}
	
	/**
	 * Determines if a <code>return</code> statment is required for a function
	 * returning this type. In other words, whether the given type is 
	 * <code>void</code> (but not, for instance, <code>void*</code>).
	 * @param returnType the function's return type - should be flat
	 * @return <b>true</b> if and only if returnType is <b>not</b> the special
	 * type "<code>void</code>".
	 */
	static boolean needsReturnStatement(Type returnType)
	{
		return !(returnType.getBaseType().equals(Primitive.VOID)
				&& returnType.getPointerDegree() == 0);		
	}
	
	/**
	 * Determines if a <code>return</code> statement requires a touchup for
	 * the return type, such as a special casting of the return value, and
	 * return the new type if it does.
	 * @param returnType the function's return type
	 * @return the new type to return after the touchup, or null if no touchup
	 * is required.
	 */
	static Type getTouchup(Type returnType)
	{
		if (getTouchupsMap().containsKey(returnType)) {
			return ((Touchup)getTouchupsMap().get(returnType)).m_newType;
		}
		return null;
	}
	
	static Map getTouchupsMap() {
		return m_touchups;
	}
	
	/**
	 * Checks if a field entity can be assigned to.
	 * Currently only primitive type fields support assignment.
	 * 
	 * @param field the field to check
	 * @return <b>true</b> if the field can accept assignment, 
	 * <b>false</b> if not.
	 */
	static boolean hasSetter(Field field)
	{
		try {
			Type type = field.getType();
			type.getBaseType(); // - make sure base type is present
			
			type = Utils.flatUnalias(type);
			
			
			return (field.hasContainer()
					&& field.getContainer() instanceof Aggregate)
					&& isDirectPrimitive(type)
					&& !type.isConst();
		}
		catch (MissingInformationException e) {
			return false;
		}
	}
	
	/**
	 * This supporting class represents the touchup code required for specialized
	 * types, such as the special requirements of the float type in casting.
	 */
	static class Touchup {
		public Touchup(Type newType, String touchupCode) {
			m_newType = newType;
			m_touchupCode = touchupCode;
		}
		
		public Type m_newType;
		public String m_touchupCode;
	}
	// The map from function return type to the new type and the touchup code
	private static Map m_touchups = new HashMap();

	// TODO: should be package protected?
	public static boolean isPrimitive(Entity base)
	{
		return (base instanceof Primitive);
	}

	public static boolean isSmallPrimitive(Entity base)
	{
		return (base instanceof Primitive) &&
		       (!base.getName().equals("double")) &&
		       (!base.getName().equals("long long")) &&
			   (!base.getName().equals("unsigned long long"));
	}

	/**
	 * An aggregate (non-primitive) type needs at least one level of pointer
	 * reference. This also applies for primitive types larger then the size
	 * of a pointer.
	 * @param type argument or return type
	 * @return boolean
	 */
	static public boolean needsExtraReferencing(Type type)
	{
		int pointers = type.getPointerDegree();
		boolean reference = type.isReference();
		Entity base = TypeToolbox.getOriginalType(type).getBaseType();
		
		if ((!reference && pointers == 0) && 
			needsExtraReferencing(base)) return true;
		else
			return false;
	}
	
	static public boolean needsExtraReferencing(Entity base)
	{
		return !(isSmallPrimitive(base) || base instanceof sourceanalysis.Enum);
	}
	
	public static boolean needsEncapsulation(Alias alias)
	{
		return needsEncapsulation(alias, false);
	}
	
	/**
	 * Determines whether a simple typedef requires encapsulation - such
	 * typedefs are "hidden" using a proxy wrapper class, which has a
	 * constructor from the aliased type.
	 * @param alias
	 * @param ignoreSize true if the size of the primitive should be ignored (if it's a typedef)
	 * @return <b>true</b> if encapsulation is required - <b>false</b> if
	 * not.
	 */
	public static boolean needsEncapsulation(Alias alias, boolean ignoreSize)
	{
		if (alias.getAliasedType().isFlat() && 
			alias.getAliasedType().getBaseType() instanceof Primitive && 
				(!ignoreSize && !Filters.isSmallPrimitive(alias.getAliasedType().getBaseType()) ||
				 alias.getAliasedType().getPointerDegree() != 0))
			return true;
		else
			return false;
	}

	public static boolean isArray(Type type) {
		return type.isArray() || Utils.flatUnalias(type).isArray();  
	}
}
