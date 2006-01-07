package sourceanalysis;

/**
 * Represents an argument for a template. These fill the blanks of template
 * parameters and can be either:
 * <ul>
 *  <li>Data items - fill in DataTemplateParameters</li>
 *  <li>Type names - fill in TypeNameParameters</li>
 * </ul>
 */
public abstract class TemplateArgument {

	/**
	 * Formats value as a C++-readable expression.
	 * @return text expression
	 */
	public abstract String toCpp();
	
}
