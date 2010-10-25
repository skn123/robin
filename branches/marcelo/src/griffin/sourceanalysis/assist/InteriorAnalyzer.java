package sourceanalysis.assist;

import java.io.File;
import java.io.IOException;

import javax.swing.tree.DefaultMutableTreeNode;

import org.apache.xerces.parsers.DOMParser;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import sourceanalysis.Aggregate;
import sourceanalysis.Alias;
import sourceanalysis.ContainedConnection;
import sourceanalysis.DataTemplateParameter;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;
import sourceanalysis.Field;
import sourceanalysis.Namespace;
import sourceanalysis.Parameter;
import sourceanalysis.Primitive;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.Routine;
import sourceanalysis.Scope;
import sourceanalysis.Specifiers;
import sourceanalysis.TemplateParameter;
import sourceanalysis.Type;
import sourceanalysis.TypenameTemplateArgument;
import sourceanalysis.TypenameTemplateParameter;
import sourceanalysis.xml.XML;
import sourceanalysis.xml.XMLFormatException;

/**
 * Analyzes a manually-written XML file holding the description of the
 * program.
 * TODO: document format of input XMLs for the interior analyzer.
 * 
 * The entities accepted by the interior analyzer are:
 * <ul>
 *   <li><b>Primitive</b> - <code>&lt;primitive name="..."/&gt;</code>
 *   </li>
 *   <li><b>Aggregate</b> - <code>&lt;class name="..."&gt;&lt;/class&gt;</code>
 *   </li>
 *   <li><b>Routine</b> - <code>&lt;routine name="..."&gt;</code>
 *   </li>
 *   <li><b>Parameter</b> - <code>&lt;parameter name="..."&gt;</code>
 *   </li>
 *   <li><b>TemplateParameter</b> - <code>&lt;template-parameter name="..."&gt;
 *     </code>
 *   </li>
 *   <li><b>Type</b> and <b>Type.TypeNode</b> -
 *     <ul>
 *       <li><code>&lt;<i>type-container</i>&gt;</code></li>
 *       <li><code>&lt;pointer&gt;...&lt;/pointer&gt;</code></li>
 *       <li><code>&lt;reference&gt;...&lt;/reference&gt;</code></li>
 *       <li><code>&lt;template-instantiation name="<i>classname</i>"&gt;...
 *         arguments...&lt;/template-instantiation&gt;</code></li>
 *     </ul>
 *   </li>
 * </ul>
 */
public class InteriorAnalyzer {

	private static class TypeHolder extends Entity
	{
		public TypeHolder(Type type) { this.type = type; }
		public Type type;
	}

	public InteriorAnalyzer()
	{
		m_program = new ProgramDatabase();
	}

	/**
	 * Reads program information from an external XML definitions file.
	 * @param defFilename name of XML file to open and absorb
	 * @throws XMLFormatException if the XML format is corrupted
	 */
	public void absorbDefinitions(String defFilename) throws XMLFormatException {
		// Parse XML file
		DOMParser parser = new DOMParser();
		try {
			parser.parse(defFilename);
		}
		catch (IOException e) {
			throw new XMLFormatException(defFilename + ": " + e);
		}
		catch (SAXException e) {
			throw new XMLFormatException(defFilename + ": " + e);
		}
		// Traverse document
		Document document = parser.getDocument();
		Node program = document.getFirstChild();
		NodeList directChildren = program.getChildNodes();
		for (int i = 0; i < directChildren.getLength(); ++i) {
			Node child = directChildren.item(i);
			if (child.getNodeType() == Node.ELEMENT_NODE) {
				readEntity(child);
			}
		}
	}

	/**
	 * Reads program information from an external XML definitions file.
	 * @param defFile path of XML file to open and absorb
	 * @throws XMLFormatException if the XML format is corrupted
	 */
	public void absorbDefinitions(File defFile) throws XMLFormatException
	{
		absorbDefinitions(defFile.getPath());
	}
	
	/**
	 * Extracts the result of the analysis (all prior invocations of
	 * absorbDefinitions()). 
	 * @note further calls to absorbDefinitions() may render the returned 
	 * object invalid.
	 * @return a program database containing processed entities
	 */	
	public ProgramDatabase getAnalysis()
	{
		return m_program;
	}

	/**
	 * Retrieves the 'name' attribute of an XML node.
	 * @param xmlNode a node
	 * @return the value of the name attribute
	 * @throws XMLFormatException if the required attribute does not exist.
	 */
	private static String getNameAttr(Node xmlNode) throws XMLFormatException
	{
		String name = XML.attribute(xmlNode, "name", null);
		if (name == null)
			throw new XMLFormatException("item must have a name", xmlNode);
		else
			return name;
	}

	/**
	 * Splits components of a qualified name into separate identifiers.
	 * @param id a string of the form "a::b::c"
	 * @return an array of separated components, such as "a", "b", "c"
	 */
	private static String[] parseIdentifier(String id)
	{
		return id.split("::");
	}

	/**
	 * Reads an element from XML, which may be one of several entity types.
	 * @param xmlNode node which is a root of the entity to read
	 * @return the entity referenced or described by the element
	 * @throws XMLFormatException if any component under xmlNode violates the
	 * expected format.
	 */	
	private Entity readEntity(Node xmlNode) throws XMLFormatException
	{
		String kind = xmlNode.getNodeName();
		if (kind.equals("primitive")) {
			return readPrimitive(xmlNode);
		}
		else if (kind.equals("class")) {
			return readClass(xmlNode);
		}
		else if (kind.equals("routine")) {
			return readRoutine(xmlNode);
		}
		else if (kind.equals("field")) {
			return readField(xmlNode);
		}
		else if (kind.equals("parameter")) {
			return readParameter(xmlNode);
		}
		else if (kind.equals("template-parameter")) {
			return readTemplateParameter(xmlNode);
		}
		else if (kind.equals("returns") || kind.equals("extends")) {
			return new TypeHolder(readType(xmlNode));
		}
		else {
			throw new XMLFormatException("unexpected kind: " + kind, xmlNode);
		}
	}
	
	/**
	 * Reads a primitive data type.
	 * @param xmlNode
	 * @return
	 * @throws XMLFormatException
	 */
	private Primitive readPrimitive(Node xmlNode) throws XMLFormatException
	{
		String name = getNameAttr(xmlNode);
		try {
			return Primitive.byName(name);
		}
		catch (ElementNotFoundException e) {
			throw new XMLFormatException(e.toString(), xmlNode);
		}
	}
	
	/**
	 * Reads class information from an XML node, including all the elements
	 * contained in it. 
	 * @param xmlNode root node of information tree
	 * @return the class rooted at this node (it may be a new class or an
	 * existing class - since several references to the same element may
	 * occur in the document)
	 * @throws XMLFormatException if the XML structure under xmlNode violates
	 * the expected format.
	 */
	private Aggregate readClass(Node xmlNode) throws XMLFormatException
	{
		String name = getNameAttr(xmlNode);
		Aggregate element = touchClass(parseIdentifier(name));
		// Read elements inside
		NodeList children = xmlNode.getChildNodes();
		for (int i = 0; i < children.getLength(); ++i) {
			Node child = children.item(i);
			// - skip text nodes, read only elements from element nodes
			if (child.getNodeType() == Node.ELEMENT_NODE) {
				Entity inner = readEntity(child);
				int vis = Specifiers.Visibility.PUBLIC;  // TODO: visibility
				int virt = Specifiers.Virtuality.NON_VIRTUAL;
				int stor = Specifiers.Storage.EXTERN;
				// - now follow according to the type of the element
				//   read
				if (inner instanceof Namespace) {
					element.getScope().addMember((Namespace)inner);
				}
				else if (inner instanceof Aggregate) {
					element.getScope().addMember((Aggregate)inner, vis);
				}
				else if (inner instanceof Routine) {
					element.getScope().addMember((Routine)inner, vis,
						virt, stor);
				}
				else if (inner instanceof Field) {
					element.getScope().addMember((Field)inner, vis, stor);
				}
				else if (inner instanceof Alias) {
					element.getScope().addMember((Alias)inner, vis);
				}
				else if (inner instanceof sourceanalysis.Enum) {
					element.getScope().addMember((sourceanalysis.Enum)inner, vis);
				}
				else if (inner instanceof TypenameTemplateParameter) {
					TypenameTemplateParameter tp =
						(TypenameTemplateParameter)inner;
					// - create delegate for template parameter 
					Aggregate delegate = new Aggregate();
					delegate.setName(tp.getName());
					tp.associate(delegate);
					element.addTemplateParameter(tp);
				}
				else if (inner instanceof TemplateParameter) {
					element.addTemplateParameter((TemplateParameter)inner);
				}
				else if (inner instanceof TypeHolder) {
					Type baseType = ((TypeHolder)inner).type;
					if (baseType.isFlat()) {
						element.addBase((Aggregate)baseType.getBaseType(),
							baseType.getTemplateArguments(), 
							Specifiers.Visibility.PUBLIC);
					}
				}
			}
		}
		return element;
	}
	
	/**
	 * Reads a routine entity.
	 * The format of a routine entity in the XML document is:
	 * <code>&lt;routine name="..."&gt;<br/>
	 * &lt;returns&gt;---return type---&lt;/returns&gt;<br/>
	 * &ltparameter name="..."&gt;---parameter (one or more) ---&lt;/parameter&gt;
	 * <br/>
	 * &lt;/routine&gt;
	 * </code>
	 * @param xmlNode
	 * @return
	 * @throws XMLFormatException
	 */
	private Routine readRoutine(Node xmlNode) throws XMLFormatException
	{
		Routine routine = new Routine();
		routine.setReturnType(NULL_TYPE);
		// Read its name
		String name = getNameAttr(xmlNode);
		routine.setName(name);
		// Read elements inside
		NodeList children = xmlNode.getChildNodes();
		for (int i = 0; i < children.getLength(); ++i) {
			Node child = children.item(i);
			// - skip text nodes, read only elements from element nodes
			if (child.getNodeType() == Node.ELEMENT_NODE) {
				Entity element = readEntity(child);
				// - continue according to the type of entity read
				if (element instanceof Parameter) {
					routine.addParameter((Parameter)element);
				}
				else if (element instanceof TypeHolder) {
					routine.setReturnType(((TypeHolder)element).type);
				}
			}
		}		
		return routine;
	}
	
	/**
	 * Reads a field entity.
	 * The format of a field entity is:
	 * <code>&lt;field name="..."&gt;&lt;type&gt;...&lt;/type&gt;&lt;/field&gt;</code>
	 * @param xmlNode
	 * @return
	 * @throws XMLFormatException
	 */
	private Field readField(Node xmlNode) throws XMLFormatException
	{
		Field field = new Field();
		// Read its name
		String name = getNameAttr(xmlNode);
		field.setName(name);
		// Read its type
		Node typeNode = XML.subNode(xmlNode, "type");
		Type type = readType(typeNode);
		field.setType(type);
		return field;
	}
	
	/**
	 * Reads a parameter (consisting of name, type, and default value if any)
	 * from the XML document.
	 * @param xmlNode a &lt;parameter&gt; element containing a parameter 
	 * declaration
	 * @return a Parameter entity
	 * @throws XMLFormatException if the format of any of the nodes violates
	 * the expected format.
	 */
	private Parameter readParameter(Node xmlNode) throws XMLFormatException
	{
		Parameter parameter = new Parameter();
		// Read its name
		String name = getNameAttr(xmlNode);
		parameter.setName(name);
		// Read its type
		Node typeNode = XML.subNode(xmlNode, "type");
		Type type = readType(typeNode);
		parameter.setType(type);
		// Read a default value
		Node defaultNode = XML.subNode(xmlNode, "default");
		if (defaultNode != null) {
			String defaultString = XML.collectText(defaultNode);
			parameter.setDefault(defaultString);
		}
		return parameter;
	}
	
	/**
	 * Reads a template parameter, which may be a TypenameTemplateParameter
	 * or a DataTemplateParameter.
	 * @param xmlNode the node defining the parameter
	 * @return a TemplateParameter derivative
	 * @throws XMLFormatException if the structure of the parameter node
	 * violates the expected format.
	 */
	private TemplateParameter readTemplateParameter(Node xmlNode)
		throws XMLFormatException
	{
		String name = getNameAttr(xmlNode);
		String kind = XML.attribute(xmlNode, "filler", "typename");
		if (kind.equals("typename")) {
			// - read a type-name template parameter
			TypenameTemplateParameter parameter =
				new TypenameTemplateParameter();
			parameter.setName(name);
			return parameter;
		}
		else if (kind.equals("data")) {
			// - read a data template parameter
			DataTemplateParameter parameter =
				new DataTemplateParameter();
			parameter.setName(name);
			Node typeNode = XML.subNode(xmlNode, "type");
			if (typeNode != null) {
				parameter.setType(readType(typeNode));
			}
			return parameter;
		}
		else {
			throw new XMLFormatException("invalid kind for template parameter"
				+ " - " + kind);
		}
	}
	
	/**
	 * Reads a type expression.
	 * @param xmlNode
	 * @return a new Type object
	 * @throws XMLFormatException if the type's structure violates the
	 * expected format.
	 */
	private Type readType(Node xmlNode) throws XMLFormatException
	{
		Type.TypeNode root = readTypeNode(XML.firstChildElement(xmlNode));
		return new Type(root);
	}
	
	/**
	 * Reads information from the XML node which is parallel to that held
	 * by a type node, constructing the appropriate TypeNode from it.
	 * @param xmlNode root node of type information
	 * @return a new Type.TypeNode object
	 * @throws XMLFormatException if the type's structure violates the
	 * expected format.
	 */
	private Type.TypeNode readTypeNode(Node xmlNode) throws XMLFormatException
	{
		String kind = xmlNode.getNodeName();
		// Follow according to the type of the node.
		if (kind.equals("pointer") || kind.equals("reference")) {
			// - create a pointer node
			Type.TypeNode pointed = 
				readTypeNode(XML.firstChildElement(xmlNode));
			Type.TypeNode pointer = 
				new Type.TypeNode(kind.equals("pointer") ? 
				  Type.TypeNode.NODE_POINTER : Type.TypeNode.NODE_REFERENCE);
			pointer.add(pointed);
			return pointer;
		}
		if (kind.equals("template-instantiation")) {
			// - create a template-instantiation node
			Aggregate template = 
				touchClass(parseIdentifier(getNameAttr(xmlNode)));
			Type.TypeNode templateRoot =
				new Type.TypeNode(Type.TypeNode.NODE_TEMPLATE_INSTANTIATION); 
			templateRoot.add(new Type.TypeNode(template));
			// - get template arguments
			NodeList argNodes = xmlNode.getChildNodes();
			for (int i = 0; i < argNodes.getLength(); ++i) {
				Node argNode = argNodes.item(i);
				if (argNode.getNodeType() == Node.ELEMENT_NODE) {
					// - read type
					Type argValue = new Type(readTypeNode(argNode));
					// - create TypenameTemplateArgument and add to tree
					TypenameTemplateArgument arg = 
						new TypenameTemplateArgument(argValue);
					templateRoot.add(new DefaultMutableTreeNode(arg)); 
				}
			}
			return templateRoot;
		}
		else {
			// - create a leaf node
			Entity base = readEntity(xmlNode);
			return new Type.TypeNode(base);
		}
	}
	
	/**
	 * Tries to locate a class by its given long name (starting at the
	 * global namespace). If the class or any of the containing entities
	 * does not exists, they are created - missing path links assumed to be
	 * namespaces.
	 * @param name components of the long name
	 * @return a class with the full name as specified
	 */
	private Aggregate touchClass(String[] name)
	{
		Entity container = lookupContainer(name, name.length, true);
		if (container == null)
			throw new AssertionError("container==null unexpected");
		return (Aggregate)container;
	}
	
	/**
	 * Looks up a class or namespace by their fully qualified name.
	 * @param fullName a '::' separated string
	 * @return the Entity corresponding to fullName. If such an entity
	 * does not exist, this value is <b>null</b>.
	 */
	public Entity lookupContainer(String fullName)
	{
		String[] name = parseIdentifier(fullName);
		return lookupContainer(name, name.length, false);
	}
	
	/**
	 * Looks up a class or namespace by their fully qualified name.
	 * @param name separated qualified name of entity
	 * @param level the number of elements from name[] to consider
	 * @param force if entities along the path that name[0..level-1] points
	 * out does not exist, take initiative and create them (much like
	 * <code>mkdir -p</code>)
	 * @return the Entity corresponding to name. If such an entity does not
	 * exist and <code>force==false</code>, this value is <b>null</b>.
	 */
	private Entity lookupContainer(String[] name, int level, boolean force)
	{
		Entity container = m_program.getGlobalNamespace();
		Scope<? extends Entity> containerScope = m_program.getGlobalNamespace().getScope();
		
		for (int i = 0; i < level; ++i) {
			if (name[i].equals("")) continue;
			// - find a container in current scope
			Entity subcontainer = findContainer(containerScope, name[i]);
			if (subcontainer == null && container instanceof Aggregate)
				subcontainer = findTypenameTemplateParameter(container, name[i]);
			// - check if container exists, if not, create one
			if (subcontainer != null) {
				// - The entity found must be an aggregate or a namespace
				container = subcontainer;
				if (container instanceof Aggregate)
					containerScope = ((Aggregate)container).getScope();
				else
					containerScope = ((Namespace)container).getScope();
			}
			else if (force)
			{
				// - any elements along the route are considered to be
				//   namespaces (unless a class by that name exists);
				//   the last is expected to be a class.
				if (i < name.length - 1) {
					Namespace ns = new Namespace();
					ns.setName(name[i]);
					containerScope.addMember(ns);
					container = ns;
					containerScope = ns.getScope();
				}
				else {
					Aggregate ag = new Aggregate();
					ag.setName(name[i]);
					containerScope.addMember(ag, Specifiers.Visibility.PUBLIC);
					container = ag;
					containerScope = ag.getScope();
				}
			}
			else
				return null;
		}
		return container;
	}

	/**
	 * Finds a container (class or namespace) in the midst of a given
	 * scope.
	 * @param inside scope to search in
	 * @param name expected (short) name of container
	 * @return requested container, if found, <b>null</b> otherwise.
	 */
	private static Entity findContainer(Scope<? extends Entity> inside, String name)
	{
		// Search for classes
		for (ContainedConnection<? extends Entity, Aggregate> connection: inside.getAggregates()) {
			Aggregate aggregate = connection.getContained();
			if (aggregate.getName().equals(name)) {
				return aggregate;
			}
		}
		// Search for namespaces
		for (ContainedConnection<? extends Entity, Namespace> connection: inside.getNamespaces()) {
			Namespace ns = connection.getContained();
			if (ns.getName().equals(name)) {
				return ns;
			}
		}
		// Not found
		return null;
	}

	/**
	 * Looks for a tempate parameter by name.
	 * @param inside
	 * @param name
	 * @return
	 */
	private static Aggregate findTypenameTemplateParameter(Entity inside, 
		String name)
	{
		for (TemplateParameter parameter: inside.getTemplateParameters()) {
			if (parameter instanceof TypenameTemplateParameter &&
				parameter.getName().equals(name)) {
				Aggregate delegate = ((TypenameTemplateParameter)parameter)
					.getDelegate();
				return delegate;
			}
		}
		return null;
	}
	// Repository
	private ProgramDatabase m_program;
	
	static private final Type NULL_TYPE = new Type(null);
}
