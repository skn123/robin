package sourceanalysis;

public class Parameter extends Entity {

	/**
	 * Constructor for Parameter.
	 * <ul>
	 *  <li>Sets an unknown type (trying to access type before setting it will
	 *    raise MissingInformationException).</li>
	 *  <li>Sets no default value.</li>
	 * </ul>
	 */
	public Parameter() {
		super();
	}

	/** @name Push API
	 * Methods for setting the Parameter's information.
	 */
	/*@{*/

	/**
	 * Sets the type of this parameter using a type expression.
	 * @param type a type-expression to serve as the parameter's declared type.
	 */
	public void setType(Type type)
	{
		m_dataType = type;
	}

	/**
	 * Sets the default value assigned to that parameter in the function head
	 * (defaults are taken from the declaration if one exists, otherwise they
	 * are taken from the head at the definition).
	 * @param defaultString a string describing the default value, possibly some
	 * constant C expression.
	 */
	public void setDefault(String defaultString)
	{
		m_default = defaultString;
	}

	/*@}*/	
	
	/** @name Pull API
	 * Methods for retreiving information about the Parameter.
	 */
	/*@{*/
	
	/**
	 * Returns the declared type associated with this parameter, which was
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
	 * Checks whether this parameter was assigned a default value. If this call
	 * returns <b>true</b>, it is safe to use getDefaultString() to fetch the
	 * default value.
	 * @return boolean <b>true</b> if the parameter was assigned a default
	 * using setDefault(); <b>false</b> otherwise.
	 */
	public boolean hasDefault()
	{
		return (m_default != null);
	}

	/**
	 * Returns the string expressing the default value assigned for this
	 * parameter by setDefault().
	 * @return String the default string. If !hasDefault(), the return value
	 * is <b>null</b>.
	 */
	public String getDefaultString()
	{
		return m_default;
	}

	/*@}*/	
	
	// Private members - Parameter type attributes
	Type m_dataType;
	String m_default;
}
