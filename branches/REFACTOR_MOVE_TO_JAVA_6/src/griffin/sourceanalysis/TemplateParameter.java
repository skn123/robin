package sourceanalysis;

import java.util.Iterator;

/**
 * Abstract class, contains nothing. Represents a template parameter which
 * may either be:
 * <ul>
 *  <li><tt>&lt;class T&gt;</tt> (TypenameTemplateParameter)</li>
 *  <li><tt>&lt;int U&gt;</tt> (DataTemplateParameter)</li>
 * </ul>
 */
public abstract class TemplateParameter extends Entity {

	/**
	 * Checks if the template parameter has a default.
	 * @return boolean <b>true</b> for "has default", <b>false</b> for
	 * "value not optional".
	 */
	public abstract boolean hasDefault();

	/**
	 * Retrieves the default value of this parameter.
	 * @return a TemplateArgument derivative which should be placed in place
	 * of this templates, when an actual argument is not provided
	 */
	public abstract TemplateArgument getDefaultValue();
	
	/**
	 * Retrieves the default value of this parameter, taking into account the
	 * values assigned to previous parameters.
	 * @param parameterIterator iterator over template parameters
	 * @param argumentIterator iterator over template arguments already
	 * assigned
	 * @return a TemplateArgument derivative which should be placed in place
	 * of this templates, when an actual argument is not provided
	 */
	public abstract TemplateArgument getDefaultValue(Iterator<TemplateParameter> parameterIterator,
													 Iterator<TemplateArgument> argumentIterator)
		throws InappropriateKindException;
	
	/**
	 * Creates an identical duplicate of the template parameter.
	 */
	public abstract Object clone();
}
