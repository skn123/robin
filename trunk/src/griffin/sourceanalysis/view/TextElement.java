package sourceanalysis.view;

/**
 * enclosing_type - undocumented.
 */
public class TextElement implements TemplateElement {

	/**
	 * Constructor for TextElement.
	 */
	public TextElement(String text) {
		super();
		m_text = text;
	}

	/**
	 * @see sourceanalysis.view.TemplateElement#extractText(AbstractScope)
	 */
	public String extractText(AbstractScope context, Perspective perspective)
	{
		return m_text;
	}

	// Private members
	private String m_text;
}
