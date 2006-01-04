package sourceanalysis.xml;

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;

/**
 * Binds some XML processing utility functions for easy reusability.
 */
public class XML {

	/**
	 * Finds an attribute of an XML node.
	 * @param xmlnode node to be read
	 * @param attr attribute name
	 * @param default_value value to use if attribute not found
	 * @return String attribute value, or the 'default_value' if no such 
	 * attribute exists.
	 */
	public static String attribute(Node xmlnode, String attr,
		String default_value)
	{
		// Retrieve the attribute node
		NamedNodeMap attrs = xmlnode.getAttributes();
		Node attrnode = attrs.getNamedItem(attr);
		// Get string value
		if (attrnode != null)
			return attrnode.getNodeValue();
		else
			return default_value;
	}

	public static Node firstChildElement(Node xmlnode)
		throws XMLFormatException
	{
		NodeList children = xmlnode.getChildNodes();
		for (int i = 0; i < children.getLength(); ++i) {
			Node child = children.item(i);
			if (child.getNodeType() == Node.ELEMENT_NODE)
				return child;
		}
		throw new XMLFormatException("node is not permitted to be empty",
			xmlnode);
	}

	/**
	 * @name XML Comprehension Aids
	 * Methods which ease up accessing the XML document (using DOM).
	 */
	
	/**
	 * Extracts the entire text which lies under an XML node. Tags and
	 * attributes are scarfed out, and text is recursively collected to form
	 * a string of long text.
	 * @return String the text under the root node, in the order in which it
	 * occurs in the XML document
	 */
	public static String collectText(Node root)
	{
		NodeList children = root.getChildNodes();
		StringBuffer acc = new StringBuffer();
		// Go over sub-nodes; collect those which are text, recursively
		// descend into those which are elements
		for (int idx = 0; idx < children.getLength(); ++idx) {
			Node child = children.item(idx);
			// Branch disposition according to node type
			if (child.getNodeType() == Node.TEXT_NODE) {
				// accumulate text
				Text txtchild = (Text)child;
				acc.append(txtchild.getData());
			}
			else if (child.getNodeType() == Node.ELEMENT_NODE) {
				// recursively descend
				acc.append(collectText(child));
			}
		}
		// Return entire length of text
		return acc.toString();
	}

	/**
	 * Copies an entire XML fragment and returns it, as a string containing
	 * all the XML tags in their textual form (with angle brackets).
	 * @param fragment node to read text from
	 * @param surround if <b>true</b>, the element tags will be added before
	 * and after the text, like so: <code>&lt;tag&gt;text&lt;/tag&gt;</code>
	 * @return String tagged copy of fragment
	 */
	public static String copyText(Node fragment, boolean surround)
	{
		NodeList children = fragment.getChildNodes();
		StringBuffer acc = new StringBuffer();
		// Write open tag of element
		if (surround) {
			acc.append("<"); acc.append(fragment.getNodeName());
			// - add attributes
			NamedNodeMap attributes = fragment.getAttributes();
			for (int attri = 0; attri < attributes.getLength(); ++attri) {
				Node attribute = attributes.item(attri);
				acc.append(" "); acc.append(attribute.getNodeName());
				acc.append("=\""); acc.append(attribute.getNodeValue());
				acc.append("\"");
			}
			acc.append(">");		
		}
		// Go over sub-nodes; collect those which are text, recursively
		// descend into those which are elements
		for (int idx = 0; idx < children.getLength(); ++idx) {
			Node child = children.item(idx);
			// Branch disposition according to node type
			if (child.getNodeType() == Node.TEXT_NODE) {
				// accumulate text
				Text txtchild = (Text)child;
				acc.append(txtchild.getData());
			}
			else if (child.getNodeType() == Node.ELEMENT_NODE) {
				// recursively descend
				acc.append(copyText(child,true));
			}
		}
		// Write close tag of element
		if (surround) {
			acc.append("</"); acc.append(fragment.getNodeName());
			acc.append(">");
		}
		// Return entire length of text
		return acc.toString();
	}

	/**
	 * Copies an entire XML fragment and returns it, as a string containing
	 * all the XML tags in their textual form (with angle brackets).
	 * @param fragment node to read text from
	 * @return String tagged copy of fragment
	 */
	public static String copyText(Node fragment) {
		return copyText(fragment, false);
	}

	/**
	 * Finds a direct sub-node with a specific name.
	 * @param xmlnode root node
	 * @param subname name of sub-node to search for
	 * @return Node the sub-node with specified name, if one exists;
	 * otherwise, <b>null</b>.
	 */
	public static Node subNode(Node xmlnode, String subname)
	{
		NodeList children = xmlnode.getChildNodes();
		// Go through children
		for (int child = 0; child < children.getLength(); ++child) {
			Node childnode = children.item(child);
			if (childnode.getNodeName().equals(subname))
				return childnode;
		}
		// Not found
		return null;
	}

	/**
	 * Finds direct sub-nodes with a specific name.
	 * @param xmlnode root node
	 * @param subname name of sub-nodes to search for
	 * @return Collection all the sub-nodes which have the requested name
	 */
	public static Collection subNodes(Node xmlnode, String subname)
	{
		List matching = new LinkedList();
		// Iterate children and add them all to the list
		NodeList children = xmlnode.getChildNodes();
		// Go through children
		for (int child = 0; child < children.getLength(); ++child) {
			Node childnode = children.item(child);
			if (childnode.getNodeName().equals(subname))
				matching.add(childnode);
		}
		return matching;
	}

}
