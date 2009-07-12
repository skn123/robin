package sourceanalysis;

public class TypenameTemplateArgument extends TemplateArgument {

	/**
	 * Constructor - sets the type of the argument.
	 */
	public TypenameTemplateArgument(Type value)
	{
		m_value = value;
	}
	
	/**
	 * Sets the argument's value.
	 * @param value a type expression
	 */
	public void setValue(Type value)
	{
		m_value = value;
	}
	
	/**
	 * Returns the argument's value.
	 * @return Type a type expression
	 */
	public Type getValue()
	{
		return m_value;
	}
	
	/**
	 * Returns the C++ representation of the type held in this
	 * TemplateArgument.
	 */
	@Override
	public String toCpp()
	{
		return m_value.formatCpp();
	}

	/**
	 * Returns the string representation of the type held in this
	 * TemplateArgument.
	 */
	@Override
	public String toString()
	{
		return m_value.toString();
	}

	// Private members
	private Type m_value;
}
