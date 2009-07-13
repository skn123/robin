package sourceanalysis;

import java.util.Iterator;

/**
 * enclosing_type - undocumented.
 */
public class DataTemplateParameter extends TemplateParameter{

	/**
	 * Empty constructor for DataTemplateParameter:
	 * <ul>
	 *  <li>Assigns an unknown type (<b>null</b>)</li>
	 *  <li>Sets no default</li>
	 * </ul>
	 */
	public DataTemplateParameter()
	{
		m_dataType = null;
		m_default = null;
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
	 * Sets the default value assigned to that parameter in the template spec
	 * (defaults are always taken from the template declaration, never from
	 * definitions).
	 * @param defaultString a string describing the default value, possibly some
	 * constant C expression.
	 */
	public void setDefault(String defaultString)
	{
		m_default = defaultString;
	}

	/**
	 * @name Pull API
	 * Methods for retrieving information about the parameter.
	 */
	/*@{*/

	/**
	 * Retrieves the type of the data element which serves as the template
	 * parameter for the Routine or Aggregate.
	 * @return Type a type expression describing the parameter's type
	 */
	public Type getType() throws MissingInformationException
	{
		if (m_dataType == null)
			throw new MissingInformationException();
		else
			return m_dataType;
	}

	/**
	 * Checks if the template parameter has a default.
	 * @return boolean
	 * @see sourceanalysis.TemplateParameter#hasDefault()
	 */
	@Override
	public boolean hasDefault() {
		return (m_default != null);
	}
	
	/**
	 * Returns the default for this template parameter.
	 * @return String a string associated as the default for this parameter,
	 * possibly some constant C expression. If !hasDefault(), this value is
	 * <b>null</b>.
	 */
	public String getDefaultString()
	{
		return m_default;
	}

	/**
	 * Creates a TypenameTemplateArgument corresponding to the default
	 * type for this parameter.
	 * @return an instance of DataTemplateArgument holding the default string.
	 */
	@Override
	public TemplateArgument getDefaultValue()
	{
		if (hasDefault())
			return new DataTemplateArgument(getDefaultString());
		else
			return null;
	}

	/**
	 * @see sourceanalysis.TemplateParameter#getDefaultValue(java.util.Iterator, java.util.Iterator)
	 */
	@Override
	public TemplateArgument getDefaultValue(
		Iterator<TemplateParameter> parameterIterator,
		Iterator<TemplateArgument> argumentIterator) 
	{
		if (!hasDefault()) return null;
		
		String defaultString = getDefaultString();
		// Replace occurances in default string as if they were macros
		while (parameterIterator.hasNext() && argumentIterator.hasNext()) {
			TemplateParameter parameter = parameterIterator.next();
			TemplateArgument argument = argumentIterator.next();
			
			if (defaultString.startsWith(parameter.getName())) {
				defaultString = argument.toCpp() 
					+ defaultString.substring(parameter.getName().length());
			}
		}
		// Create a DataTemplateArgument
		return new DataTemplateArgument(defaultString);
	}
	
	/*@}*/


	/**
	 * @name Utilities
	 */
	/*@{*/
	@Override
	public Object clone()
	{
		DataTemplateParameter duplica = new DataTemplateParameter();
		duplica.setName(this.getName());
		duplica.m_dataType = this.m_dataType;
		duplica.m_default = this.m_default;
		return duplica;
	}
	/*@}*/


	// Private members
	String m_default;
	Type m_dataType;
}
