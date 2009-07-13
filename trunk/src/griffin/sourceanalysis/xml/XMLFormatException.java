package sourceanalysis.xml;

/**
 * To change this generated comment edit the template variable "typecomment":
 * Window>Preferences>Java>Templates.
 * To enable and disable the creation of type comments go to
 * Window>Preferences>Java>Code Generation.
 */
public class XMLFormatException extends Exception {

	/**
	 * 
	 */
	private static final long serialVersionUID = 645529827740762692L;

	/**
	 * Constructor for XMLFormatException.
	 */
	public XMLFormatException() {
		super();
	}

	/**
	 * Constructor for XMLFormatException.
	 * @param s error description string
	 */
	public XMLFormatException(String s) {
		super("XML: " + s);
		m_at = null;
	}

	/**
	 * Constructor for XMLFormatException.
	 * @param s error description string
	 * @param atnode XML node at which error was encountered
	 */
	public XMLFormatException(String s, org.w3c.dom.Node atnode) {
		super("XML: " + s);
		m_at = atnode;
	}

	/**
	 * Gets a string describing the location of the node in which the error
	 * occurred.
	 * @return a path string to the errornoeus XML node.
	 */
	public String getErrorPath()
	{
		if (m_at == null) {
			return "top level";
		}
		else {
			// Compute the path
			org.w3c.dom.Node visit = m_at;
			StringBuffer path = new StringBuffer();
			while (visit != null) {
				path.insert(0, "/" + visit.getNodeName());
				visit = visit.getParentNode();
			}
			return path.substring(1);
		}
	}
	
	/**
	 * Returns the error string plus the error path.
	 * @return String a printable string
	 */
	@Override
	public String toString()
	{
		return super.toString() + " (at " + getErrorPath() + ")";
	}

	// Private members
	org.w3c.dom.Node m_at;
}
