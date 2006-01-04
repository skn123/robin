package sourceanalysis;

/**
 * Thrown when a requested element is not found in the input directory or in the database.
 */
public class ElementNotFoundException extends Exception {

	public ElementNotFoundException()
	{
		super();
	}
	
	public ElementNotFoundException(String kind, String name)
	{
		super();
		m_element_kind = kind;
		m_element_name = name;
	}
	
	public String toString()
	{
		if (m_element_kind == null || m_element_name == null) {
			return "A requested element was not found.";
		}
		else {
			return "Requested " + m_element_kind + " \"" + m_element_name + "\" is missing.";
		}
	}
	
	// Error details
	private String m_element_kind;
	private String m_element_name;
}
