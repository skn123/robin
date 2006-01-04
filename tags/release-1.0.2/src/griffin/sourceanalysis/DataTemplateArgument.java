package sourceanalysis;

/**
 * Represent a data argument to a template instantiation.
 */
public class DataTemplateArgument extends TemplateArgument {

	/**
	 * Constructor - sets value of data template argument (as string).
	 * @param value value string
	 */
	public DataTemplateArgument(String value)
	{
		setValue(value);
	}

	/**
	 * Sets the value of this template argument
	 * @param value value string
	 */
	public void setValue(String value)
	{
		m_value = value;	
	}

	/**
	 * Gets the value of the argument.
	 * @return String the value string, possibly a constant C expression.
	 */
	public String getValueString()
	{
		return m_value;
	}
	
	/**
	 * Returns the value string.
	 */
	public String toCpp()
	{
		return m_value;
	}
	
	/**
	 * Returns the value string.
	 */
	public String toString()
	{
		return m_value;
	}
	
	// Private members
	private String m_value;
}
