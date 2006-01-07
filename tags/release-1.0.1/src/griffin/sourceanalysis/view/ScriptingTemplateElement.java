package sourceanalysis.view;

/**
 * enclosing_type - undocumented.
 */
public abstract class ScriptingTemplateElement implements TemplateElement {

	/**
	 * Constructor for ScriptingTemplateElement.
	 */
	public ScriptingTemplateElement(String script) {
		super();
		m_instructions = script;
	}

	/**
	 * Method getInstructions.
	 * Returns the script instructions held in this element.
	 * @return String
	 */
	protected String getInstructions() 
	{
		return m_instructions;
	}

	private String m_instructions;
}
