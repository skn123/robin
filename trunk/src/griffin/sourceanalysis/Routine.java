package sourceanalysis;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Vector;

/**
 * Represents a function or a method.
 * <ul>
 * <li>A Routine is a function if it is contained in a Namespace 
 *   (use <b>getContainer</b>()).</li>
 * <li>A Routine is a method if it is contained in an Aggregate.</li>
 * <li>A method is a static (class-wide) method or an instance method
 *  depending on the <i>storage</i> attribute of the ContainedConnection
 *  connecting it to its container.</li>
 * </ul>
 * @see sourceanalysis.Entity#getContainer()
 */
public class Routine extends TemplateEnabledEntity implements Cloneable {

	/**
	 * Constructor for Method.
	 */
	public Routine()
	{
		super();
		m_formalArguments = new Vector();
		m_hasThrows = false;
		m_throws = new LinkedList();
	}

	/** @name Push API
	 * Methods used to set the signature and other information about the
	 * Routine.
	 */
	/*@{*/

	/**
	 * Sets constness of routine (only applicable to methods, but this
	 * criteria is not forced).
	 * @param constness <b>true</b> for const, <b>false</b> for non-const.
	 */
	public void setConst(boolean constness)
	{
		m_isConst = constness;
	}

	/**
	 * Sets the 'inline' attribute of the routine.
	 * @param inline <b>true</b> to indicate that the method is inlined;
	 * <b>false</b> to indicate that it isn't.
	 */
	public void setInline(boolean inline)
	{
		m_isInline = inline;
	}
	
	/**
	 * Sets the 'explicit' attribute. This call has no visible effect unless
	 * the routine is a constructor.  
	 * @param explicit <b>true</b> to indicate an explicit constructor;
	 * <b>false</b> to indicate a normal (implicit conversion) constructor.
	 */
	public void setExplicit(boolean explicit)
	{
		m_isExplicit = explicit;
	}

	/**
	 * Sets the 'throws' attribute, indicating whether this function has
	 * a throw() clause as part of its declaration.
	 * @param interceptor <b>true</b> to indicate that this method is part of
	 * an interceptor class to implement a pure virtual method.
	 */
	public void setThrows(boolean hasThrows)
	{
		m_hasThrows = hasThrows;
	}
	
	/**
	 * Sets the return type of this Routine.
	 * @param valtype return type-expressions, use <b>null</b> for special
	 *  methods (constructors and destructors), us the type-expression (void)
	 *  for functions which has no return value.
	 */
	public void setReturnType(Type valtype)
	{
		m_returnType = valtype;
	}

	/**
	 * Attaches a new parameter. Call this method for each parameter in the
	 * function's prototype, following the order in which the parameters occur
	 * in the declaration.
	 * <p>The new parameter is up-linked to the Routine using a
	 * ContainedConnection to maintain the symmetry.</p>
	 * @param farg an argument to append at end of function signature
	 */
	public void addParameter(Parameter farg) 
	{
		m_formalArguments.add(farg);
		// Up-link
		farg.connectToContainer(new ContainedConnection(this,
			Specifiers.DONT_CARE, Specifiers.DONT_CARE, 
			Specifiers.DONT_CARE, farg));
	}
	
	/**
	 * Attaches a new exception type thrown by this function.
	 * @param exception aggregate describing the kind of exception
	 * thrown 
	 */
	public void addThrows(Aggregate exception)
	{
		m_throws.add(exception);
	}
	
	/*@}*/
	
	/** @name Pull API
	 * Methods used to retrieve information about the Routine.
	 */
	/*@{*/
	
	/**
	 * Check if the "this" is const in a non-static class method.
	 * @return boolean <b>true</b> for "const thiscall", <b>false</b>
	 * for a non-const signature routine.
	 */
	public boolean isConst()
	{
		return m_isConst;
	}
	
	/**
	 * Check if the function is inlined or not.
	 * @return boolean <b>true</b> for an inline function, <b>false</b>
	 * for a non-inline function.
	 */
	public boolean isInline()
	{
		return m_isInline;
	}
	
	/**
	 * Checks if this routine is a constructor for the class in which
	 * it is contained.
	 * @return boolean <b>true</b> if it's a constructor
	 */
	public boolean isConstructor()
	{
		ContainedConnection contained = getContainer();
		Entity structure;
		
		structure = contained.getContainer();
		// If "structure" is a specialized template instance,
		// compare name of method to name of general template instead
		if (structure instanceof TemplateEnabledEntity) {
			TemplateEnabledEntity enabled = (TemplateEnabledEntity)structure;
			if (enabled.isSpecialized()) {
				structure = enabled.getGeneralTemplateForSpecialization()
					.getGeneral();
			}
		}
		return getName().equals(structure.getName());
	}
	
	/**
	 * Checks if this routine is the destructor of the class in which
	 * it is contained.
	 * @return boolean <b>true</b> if it's a destructor
	 */
	public boolean isDestructor()
	{
		return getName().charAt(0) == '~';
	}
	
	/**
	 * Checks if this routine is an explicit constructor.
	 * @return <b>true</b> if it's both a constructor and signified with
	 * the reserved word <code>explicit</code>. <b>false</b> otherwise.
	 */
	public boolean isExplicitConstructor()
	{
		return isConstructor() && m_isExplicit;
	}
	
	/**
	 * Checks whether this function is an operator.
	 * @return <b>true</b> if it's ann operator
	 */
	public boolean isOperator()
	{
		return getName().startsWith(OPERATOR_PREFIX);	
	}
	
	/**
	 * Checks whether this function is a conversion operator.
	 * @return <b>true</b> if it's an operator
	 */
	public boolean isConversionOperator()
	{
		final String prefix = OPERATOR_PREFIX + " ";
		String name = getName();
		// Make sure the name begins with the prefix followed by at least
		// one letter
		char follow;
		return (name.startsWith(prefix) &&
			(Character.isLetter(follow = name.charAt(prefix.length())) 
			 || follow == '_'));
	}
	
	/**
	 * Checks whether this function has a throw() clause as part of its
	 * declaration.
	 * @return <b>true</b> if such a clause exists, <b>false</b> if not.
	 */
	public boolean hasThrowClause()
	{
		return m_hasThrows;
	}
	
	/**
	 * Method getReturnValue.
	 * @return Parameter
	 */
	public Type getReturnType() throws MissingInformationException
	{
		if (m_returnType == null)
			throw new MissingInformationException();
		else
			return m_returnType;
	}
	
	/**
	 * Access parameters in the function signature.
	 * @return Iterator an iterator over Parameter. Parameters do not have
	 *  extra attributes in this connection.
	 */
	public Iterator parameterIterator()
	{
		return m_formalArguments.iterator();
	}
	
	/**
	 * Access exceptions thrown by the function.
	 * 
	 * @return an iterator over Aggregate. Thrown exceptions do not have
	 * extra attributes in this connection. 
	 */
	public Iterator throwsIterator()
	{
		return m_throws.iterator();
	}
	
	/**
	 * @see java.lang.Object#clone()
	 */
	public Object clone() {
		try {
			return super.clone();
		} catch(CloneNotSupportedException e) {
			return null;
		}
	}
	
	/*@}*/

	// Signature attributes
	private boolean m_isConst;
	private boolean m_isInline;
	private boolean m_isExplicit;      ///< refers to constructors
	private boolean m_hasThrows;
	private Type m_returnType;
	private Vector m_formalArguments;
	private List m_throws;

	public static final String OPERATOR_PREFIX = "operator";
}