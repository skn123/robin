package sourceanalysis.view;

/**
 * To change this generated comment edit the template variable "typecomment":
 * Window>Preferences>Java>Templates.
 * To enable and disable the creation of type comments go to
 * Window>Preferences>Java>Code Generation.
 */
public class Template {

	/**
	 * Constructor for Template.
	 */
	public Template() {
		super();
		m_elements = new java.util.Vector<TemplateElement>();
	}

	public void addElement(TemplateElement element)
	{
		m_elements.add(element);
	}
	
	public java.util.Iterator<TemplateElement> getElementIterator()
	{
		return m_elements.iterator();
	}

	/**
	 * Fills the blanks in the template using a content-sensitive scope
	 * containing program components.
	 * @param context scope describing current location in program
	 * @param perspective view parameters containing global objects
	 * @return String filled text
	 */
	public String fill(AbstractScope context, Perspective perspective)
	{
		java.util.Iterator element = getElementIterator();
		StringBuffer text = new StringBuffer();
		// Extract the text from all elements	
		while (element.hasNext()) {
			TemplateElement templateelement = (TemplateElement)element.next();
			text.append(templateelement.extractText(context, perspective));
		}
		return text.substring(0);
	}

	// Elements
	private java.util.Vector<TemplateElement> m_elements;
}
