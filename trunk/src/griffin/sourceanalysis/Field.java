package sourceanalysis;

/**
 * Represents a data element in the Program. Data elements may be:
 * <ul>
 *  <li>Global variables (in which case they are contained in a Namespace)</li>
 *  <li>Data members, either static or non-static (in both cases they are
 *   contained in an Aggregate).</li>
 * </ul>
 * <p>Use getContainerConnection()
 * [sourceanalysis.Entity#getContainerConnection()] to distinguish between
 * these cases and get the storage type for the Field.</p>
 */
public class Field extends Entity{

	/**
	 * Constructor for Field.
	 * <ul>
	 *  <li>Sets an unknown type.</li>
	 *  <li>Sets no initializer.</li>
	 * </ul>
	 */
	public Field() {
		super();
		m_dataType = null;
		m_initializer = null;
	}

	/** @name Push API
	 * Methods for setting the Field's information.
	 */
	/*@{*/

	/**
	 * Sets the type of this variable using a type expression.
	 * @param type a type-expression to serve as the field's type.
	 */
	public void setType(Type type)
	{
		m_dataType = type;
	}

	/**
	 * Sets the initializer assigned to that field. Initializers are always
	 * taken from the definition, and there may be at most one initializer
	 * per field.
	 * <p>Initializers may take one of two forms:</p>
	 * <ul>
	 *  <li>= <i>expression</i>: if variable was explicitly assigned a single
	 *   at its definition;</li>
	 *  <li>(<i>carg</i>0, <i>carg</i>1, ..., <i>cargn</i>): if variable is
	 *   initialized using a (non-default) constructor.</li>
	 * </ul>
	 * <p>Initializers may be assigned to global variables or to static
	 * members, but never to instance members, even if they have the same
	 * initialization in all the constructors of the class.</p>
	 * @param initString a string describing the initializer, possibly some
	 * constant C expression or a tuple of them.
	 */
	public void setInitializer(String initString)
	{
		m_initializer = initString;
	}

	/*@}*/	
	
	/** @name Pull API
	 * Methods for retreiving information about the Field.
	 */
	/*@{*/
	
	/**
	 * Returns the declared type associated with this variable, as was
	 * previously set by setType().
	 * @return Type a type expression
	 */
	public Type getType() throws MissingInformationException
	{
		if (m_dataType == null)
			throw new MissingInformationException();
		else
			return m_dataType;
	}
	
	/**
	 * Checks whether this field was initialized at its definition. If this call
	 * returns <b>true</b>, it is safe to use getDefaultString() to fetch the
	 * default value.
	 * @return boolean <b>true</b> if the parameter was assigned a default
	 * using setDefault(); <b>false</b> otherwise.
	 */
	public boolean hasInitializer()
	{
		return (m_initializer != null);
	}

	/**
	 * Returns the string expressing the initializer assigned to this
	 * parameter by setInitializer().
	 * @return String the default string. If !hasInitializer(), the return value
	 * is <b>null</b>.
	 * @see setInitializer(String)
	 */
	public String getInitializerString()
	{
		return m_initializer;
	}

	/*@}*/	

	// Private members
	Type m_dataType;
	String m_initializer;
}
