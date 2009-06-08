package backend.website;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import sourceanalysis.Aggregate;
import sourceanalysis.Alias;
import sourceanalysis.ContainedConnection;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Field;
import sourceanalysis.Group;
import sourceanalysis.InheritanceConnection;
import sourceanalysis.MissingInformationException;
import sourceanalysis.Namespace;
import sourceanalysis.Parameter;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.Routine;
import sourceanalysis.Scope;
import sourceanalysis.SourceFile;
import sourceanalysis.Specifiers;
import sourceanalysis.TemplateParameter;
import sourceanalysis.dox.DoxygenAnalyzer;
import backend.Utils;

/**
 * @author baruch
 *
 */
public class XMLWriter {

	/**
	 * Constructor for XMLWriter.
	 * Creates an XMLWriter that will create XML documents from 
	 * one program DB without comparing it.
	 * @param classListFile The file containing the list of classes
	 * to document.
	 * @param PDBDirectory The directory containing the program DB
	 * information.
	 */
	public XMLWriter(String classListFile, String PDBDirectory) 
		throws SAXException, IOException, FileFormatException, ElementNotFoundException {
		
		// Create the list of classes to document.
		m_classes = classList(classListFile);
		
		DoxygenAnalyzer dox = new DoxygenAnalyzer(PDBDirectory);
		m_oldDB = dox.processIndex();		
		
		m_toCompare = false;
	}

	/**
	 * Constructor for XMLWriter.
	 * Creates an XMLWriter that will create XML documents from 
	 * two program DB's while comparing them.
	 * @param classListFile The file containing the list of classes
	 * to document.
	 * @param oldPDBDirectory The directory containing the older 
	 * version of the program DB information.
	 * @param newPDBDirectory The directory containing the newer 
	 * version of the program DB information.
	 */
	public XMLWriter(String classListFile, String oldPDBDirectory, String newPDBDirectory) 
		throws SAXException, IOException, FileFormatException, ElementNotFoundException {
		
		// Create the list of classes to document.
		m_classes = classList(classListFile);
	
		DoxygenAnalyzer doxOld = new DoxygenAnalyzer(oldPDBDirectory);
		m_oldDB = doxOld.processIndex();
		
		DoxygenAnalyzer doxNew = new DoxygenAnalyzer(newPDBDirectory);
		m_newDB = doxNew.processIndex();		
		
		m_toCompare = true;
	}

	/**
	 * This method will return a list of all the classes the user 
	 * wishes to create according to the given XML file containing 
	 * this information.
	 * @param classListFile The XML file containing the list of 
	 * classes.
	 * @return A list of the wanted classes. 
	 * @throws IOException if the file can't be found or read.
	 * @throws SAXException if the format of the file isn't XML format.
	 * @throws FileFormatException if the format of the XML doesn't 
	 * match the wanted format.
	 */
	public String[] classList(String classListFile) 
		throws SAXException, IOException, FileFormatException {
		
		// An XML parser.
		org.apache.xerces.parsers.DOMParser parser = 
			new org.apache.xerces.parsers.DOMParser();
			
		// Parse the XMl file and get the document.
		parser.parse(classListFile);
		Document doc = parser.getDocument();
		
		// Get the list of classes.
		NodeList classes = doc.getElementsByTagName("class");
		int classNum = classes.getLength();
		
		String[] classList = new String[classNum];
		for(int i = 0; i < classNum; ++i) {
			
			Node node = classes.item(i);
			
			if( ! node.hasAttributes()) {
				throw new FileFormatException
					("No attributes to class", classListFile);
			}
		
			NamedNodeMap attributes = node.getAttributes(); 
			Node attr = attributes.getNamedItem("name");
				
			if(attr == null) {
				throw new FileFormatException
					("Missing attribute - name", classListFile);
			}
			
			// Add '::' to the name and remove template.
			String name = "::" + attr.getNodeValue();
			if(name.indexOf("<") != -1) {
				name = name.substring(0, name.indexOf("<"));
			}
			
			classList[i] = name;
		}
			
		return classList;
	}
	
	/**
	 * 
	 */
	public void MODULES(String classListFile) 
		throws SAXException, IOException, FileFormatException {
		
		// An XML parser.
		org.apache.xerces.parsers.DOMParser parser = 
			new org.apache.xerces.parsers.DOMParser();
			
		// Parse the XMl file and get the document.
		parser.parse(classListFile);
		Document doc = parser.getDocument();
		
		// Get the list of classes.
		NodeList modules = doc.getElementsByTagName("module");
		int moduleNum = modules.getLength();
		m_modules = new Module[moduleNum];
		
		for(int i = 0; i < moduleNum; ++i) {
			
			Node node = modules.item(i);
			
			if( ! node.hasAttributes()) {
				throw new FileFormatException
					("No attributes to class", classListFile);
			}
		
			NamedNodeMap attributes = node.getAttributes(); 
			Node attr = attributes.getNamedItem("name");
				
			if(attr == null) {
				throw new FileFormatException
					("Missing attribute - name", classListFile);
			}
			
			Module module = new Module(attr.getNodeValue());
					
			NodeList classes = node.getChildNodes();
			for(int j = 0; j < classes.getLength(); ++j) {
				
				Node classNode = classes.item(i);
			
				if( ! classNode.hasAttributes()) {
					throw new FileFormatException
						("No attributes to class", classListFile);
				}
		
				NamedNodeMap classAttributes = classNode.getAttributes(); 
				Node classAttr = classAttributes.getNamedItem("name");
				
				if(classAttr == null) {
					throw new FileFormatException
						("Missing attribute - name", classListFile);
				}
				
				module.addSource(classAttr.getNodeValue());	
	System.out.println(attr.getNodeValue() + ": " + classAttr.getNodeValue());	
			}
			
			m_modules[i] = module;
		}
	}
	
	/**
	 * This method will return the wanted Aggregate according to 
	 * its name.
	 * @param name The name of the aggregate.
	 * @param pdb The program DB to look in. 
	 * @return The aggregate itself.
	 * @throws  ElementNotFoundException if the wanted aggregate is 
	 * no where to be found.
	 * @throws IllegalArgumentException if the name is an illegal name. 
	 */
	public static Aggregate getAggregateByName(String name, ProgramDatabase pdb) 
		throws IllegalArgumentException, ElementNotFoundException {
		
		// Get the aggregate using its parents(aggregates \ 
		// namespaces) according to its name.
		// The name of an aggregate is in this format:
		//   "::<parent_1>::<parent_2>:: ... ::<parent_n>::<local_name>"
		if( ! name.matches("::[^\\s]+") ) {
			throw new IllegalArgumentException
				("The name of the aggregate is not in the correct format");  
		}
		
		// Get the global namespace.
		Scope location = pdb.getGlobalNamespace().getScope();
		name = name.substring(2);
		
		while( name.indexOf("::") != -1 ) {
			if( ! name.matches("[^\\s]+::[^\\s]+") ) {
				throw new IllegalArgumentException
					("The name of the aggregate is not in the correct format");  
			}
			
			// Get the parent.
			String parent = name.substring(0, name.indexOf("::"));
			name = name.substring(parent.length() + 2);			
			
			// Find the parent.
			boolean found = false;
			
			// Look for the parent as a namespace.
			for (Iterator iter = location.namespaceIterator(); iter.hasNext();) {
				
				ContainedConnection cc = (ContainedConnection)iter.next();
				Namespace ns = (Namespace)cc.getContained();
				if(ns.getName().equals(parent)) {
					found = true;
					location = ns.getScope();
				}	
			}

			// Look for the parent as an aggregate.
			if( ! found ) {
				for (Iterator iter = location.aggregateIterator(); iter.hasNext();) {
				
					ContainedConnection cc = (ContainedConnection)iter.next();
					Aggregate agg = (Aggregate)cc.getContained();
					if(agg.getName().equals(parent)) {
						found = true;
						location = agg.getScope();
					}
				}	
			}
			
			// Throw an exception if the parent isn't found.
			if( ! found ) {	
				throw new ElementNotFoundException();
			}
				
		}
		
		// Find the entity in the current loction.
		for (Iterator iter = location.aggregateIterator(); iter.hasNext();) {
				
			ContainedConnection cc = (ContainedConnection)iter.next();
			Aggregate agg = (Aggregate)cc.getContained();
			if(agg.getName().equals(name)) {
				return agg;
			}
		}	
		
		// If the aggregate wasn't found in the current location throw 
		// an exception.
		throw new ElementNotFoundException();
	}
	
	/**
	 * This method will compare the two routines and return the answer
	 * 'true' if they are equal and 'false' otherwise.
	 * @param first The first routine to compare.
	 * @param second The second routine to compare.
	 * @return 'true' if they are equal, 'false' otherwise.
	 */
	public static boolean equalRoutines(Routine first, Routine second) 
		throws MissingInformationException {
			
		// Compare return values.
		String firstRet = first.getReturnType().formatCpp();
		String secondRet = second.getReturnType().formatCpp();
		if( ! firstRet.equals(secondRet) ) {
			return false;
		}
			
		// Compare names.
		String firstName = first.getFullName();
		String secondName = second.getFullName();
		if( ! firstName.equals(secondName) ) {
			return false;
		}
			
		// Compare all arguments.
		Iterator fiter = first.parameterIterator();
		Iterator siter = second.parameterIterator();
		while( fiter.hasNext() || siter.hasNext() ) {
			
			if( ! fiter.hasNext() || ! siter.hasNext() ) {
				return false;
			}
			
			Parameter fparam = (Parameter)fiter.next();
			Parameter sparam = (Parameter)siter.next();
			
			String firstArg = fparam.getType().formatCpp() + fparam.getName();
			String secondArg = sparam.getType().formatCpp() + sparam.getName();
			
			if( ! firstArg.equals(secondArg) ) {
				return false;
			}
		}
			
			
		//////////////////////////////////////////////////////	
		//////////////////////////////////STATIC !!! CONST !!!

		return true;
	}
	
	/**
	 * This method will compare the two routine and return the answer
	 * 'true' if the second routine overides the first one or 
	 * 'false' otherwise.
	 * @param first The first routine to compare.
	 * @param second The second routine to compare.
	 * @return 'true' if the second routien overides the first, 
	 * 'false' otherwise.
	 */
	public static boolean overidesRoutine(Routine first, Routine second) 
		throws MissingInformationException {
				
		// Compare names (not full names !!!).
		String firstName = first.getName();
		String secondName = second.getName();
		if( ! firstName.equals(secondName) ) {
			return false;
		}
			
		// Compare all arguments.
		Iterator fiter = first.parameterIterator();
		Iterator siter = second.parameterIterator();
		while( fiter.hasNext() || siter.hasNext() ) {
			
			if( ! fiter.hasNext() || ! siter.hasNext() ) {
				return false;
			}
			
			Parameter fparam = (Parameter)fiter.next();
			Parameter sparam = (Parameter)siter.next();
			
			String firstArg = fparam.getType().toString();// + fparam.getName();
			String secondArg = sparam.getType().toString();// + sparam.getName();
			
			if( ! firstArg.equals(secondArg) ) {
				return false;
			}
		}
						
		//////////////////////////////////////////////////////	
		//////////////////////////////////STATIC !!! CONST !!!

		return true;
	}
	
	/**
	 * This method will check if the given routine exists in the
	 * given aggregate.
	 * @param aggr The aggregate.
	 * @param routine The routine.
	 * @return 'true' if the routine exists in the aggregate, 'false' 
	 * otherwise.
	 */
	public static boolean containsRoutine(Aggregate aggr, Routine routine) 
		throws MissingInformationException {
		
		// Compare to all the routine in the aggregate.
		for (Iterator iter = aggr.getScope().routineIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Routine compare = (Routine)cc.getContained();	
			
			if( equalRoutines(routine, compare) ) {
				return true;
			}
		}
		
		return false;
	}
	
	/**
	 * This method will compare the two fields and return the answer
	 * 'true' if they are equal and 'false' otherwise.
	 * @param first The first field to compare.
	 * @param second The second field to compare.
	 * @return 'true' if they are equal, 'false' otherwise.
	 */
	public static boolean equalFields(Field first, Field second) 
		throws MissingInformationException {
			
		// Compare types.
		String firstRet = first.getType().formatCpp();
		String secondRet = second.getType().formatCpp();
		if( ! firstRet.equals(secondRet) ) {
			return false;
		}
			
		// Compare names.
		String firstName = first.getFullName();
		String secondName = second.getFullName();
		if( ! firstName.equals(secondName) ) {
			return false;
		}
					
		//////////////////////////////////////////////////////	
		//////////////////////////////////STATIC !!! CONST !!!

		return true;
	}
	
	/**
	 * This method will check if the given field exists in the
	 * given aggregate.
	 * @param aggr The aggregate.
	 * @param field The field.
	 * @return 'true' if the field exists in the aggregate, 'false' 
	 * otherwise.
	 */
	public static boolean containsField(Aggregate aggr, Field field) 
		throws MissingInformationException {
		
		// Compare to all the field in the aggregate.
		for (Iterator iter = aggr.getScope().fieldIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Field compare = (Field)cc.getContained();	
			
			if( equalFields(field, compare) ) {
				return true;
			}
		}
		
		return false;
	}
	
	/**
	 * This method will return the prototype of the given routine with
	 * additional 'link tags' to related classes.
	 * @param routine The routine.
	 * @return The prototype of the routine. 
	 * @throws MissingInformationException if the program DB is 
	 * incomplete.
	 */
	public static String getRoutinePrototype(Routine routine) 
		throws MissingInformationException {

		// Create the prototype:
		// <return_value> <name>( <arguments> ).
		String proto = "";
		
		if(routine.getReturnType().getRoot() == null) { 
			// Constructor or Destructor (no return type).
			proto += Utils.cleanFullName(routine);
		} else {
			proto += Utils.cleanFormatCpp(routine.getReturnType(), 
				Utils.cleanFullName(routine));
		}

		proto += "(";	
		for (Iterator iter = routine.parameterIterator(); iter.hasNext();) {
			Parameter param = (Parameter)iter.next();
			
			proto += Utils.cleanFormatCpp(param.getType(), 
				 param.getName());
			if(param.hasDefault()) {
				proto += " = " + param.getDefaultString();
			}
			if(iter.hasNext()) {
				proto += ", ";
			} 
		}
		
		proto += ")";
		
		// Const.
		if(routine.isConst()) {
			proto = proto + " const";
		}
		
		// Inline.
		if(routine.isInline()) {
			proto = "inline " + proto;
		}
		
		// Virtuality.
		if(routine.getContainerConnection().getVirtuality() == 
			Specifiers.Virtuality.VIRTUAL) {
			proto = "virtual " + proto;
		} else if(routine.getContainerConnection().getVirtuality() == 
			Specifiers.Virtuality.PURE_VIRTUAL) {
			proto = "virtual " + proto + " = 0";
		}
		
		// Static.
		if(routine.getContainerConnection().getStorage() == 
			Specifiers.Storage.STATIC) {
			proto = "static " + proto;
		} 
		
		return proto;
		//    ADD LINKS TO RELATED CLASSES (return element)!!!
		//  HOW TO DECIDE IF THE CLASS EXISTS IN THE LIST???????
	}
	
	/**
	 * This method will return the prototype of the given field.
	 * @param field The field.
	 * @return The prototype of the field. 
	 * @throws MissingInformationException if the program DB is 
	 * incomplete.
	 */
	public static String getFieldPrototype(Field field) 
		throws MissingInformationException {
		// Create the prototype:
		// <type> <name>
		String proto = "";

		proto += Utils.cleanFormatCpp(field.getType(), 
			Utils.cleanFullName(field));
					
		// Static.
		if(field.getContainerConnection().getStorage() == 
			Specifiers.Storage.STATIC) {
			proto = "static " + proto;
		}
		
		return proto; 
	}
	
	/**
	 * 
	 */
	public static Set<SourceFile> getSources(Aggregate aggr) 
		throws MissingInformationException {
		
		Set<SourceFile> sources = new HashSet<SourceFile>();
		for (Iterator iter = aggr.getScope().routineIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Routine routine = (Routine)cc.getContained();
			if(routine.getDeclaration() != null) {
				sources.add(routine.getDeclaration().getSource());
			}
			if(routine.getDefinition() != null) {
				sources.add(routine.getDefinition().getSource());
			}
		}
		return sources;
	}
	
	/**
	 * 
	 */
	public boolean isClassDocumented(String className) {
		
		for(int i = 0; i < m_classes.length; ++i) {
			if(m_classes[i].equals(className)) {
				return true;
			}
		}	
		
		return false;
	} 
	
	/**
	 * This method will write the document in XML format to the 
	 * given output stream.
	 * @param out The output stream to which the document will 
	 * be writtten to.
	 * @param doc The document itself.
	 * @throws TransformerConfigurationException if the transformer 
	 * couldn't have been created.
	 * @throws TransformerException if there was an error writing 
	 * the document. 
	 */
	public static void writeXMLDocument(OutputStream out, Document doc) 
		throws TransformerConfigurationException, TransformerException {

		// Create a transformer to write he document.
		Transformer transformer = 
			TransformerFactory.newInstance().newTransformer();
		
		// Set the transformer to print the document with indentation.
		transformer.setOutputProperty("indent", "yes");		
		
		// Write the documet to the wanted output stream.
		transformer.transform(
		        	new DOMSource((Node)doc),
		        	new StreamResult(out));	
		
	}
		
	/**
	 * This method will create the reference document for a certain
	 * aggregate from the new version compared to the same aggregate
	 * in the older version.
	 * @param oldAggr The aggregate (class) to document in the older 
	 * version.
	 * @param newAggr The aggregate (class) to document in the newer 
	 * version.
	 * @return The reference document.
	 * @throws ParserConfigurationException if the document builder
	 * can't be created properly. 
	 * @throws MissingInformationException if the program DB is 
	 * incomplete.
	 */
	private Document documentAggregate(Aggregate oldAggr, Aggregate newAggr)
		throws ParserConfigurationException, MissingInformationException {
	
		// DOCUMENT IF THE CLASS IS ABSTRACT OR NOT.
	
		// Create the document.
		DocumentBuilderFactory factory = 
			DocumentBuilderFactory.newInstance();
		DocumentBuilder builder = factory.newDocumentBuilder();
		Document doc = builder.newDocument();
		
		// Check the version of the class.
		String version;
		if( ! m_toCompare ) { // There is only one aggregate to document.
			version = "both";
		} else if(oldAggr == null) { // There is only a new version.
			version = "new";
		} else if(newAggr == null) { // There is only an old version.
			version = "old";
		} else { // There are two different versions.
			version = "both";
		}
	
		// Create the reference of the aggregate according to the newest
		// possible version.
		Aggregate aggr;
		if(newAggr != null) {
			aggr = newAggr;
		} else {
			aggr = oldAggr;
		}
		
		// Create the document header.
		Element classElm = doc.createElement("class");
		doc.appendChild(classElm);
		
		// Class name.
		nameDocumentation(aggr, doc, classElm);
	
		// Full name. (needed for lca::xLCA).
		Element fullnameElm = doc.createElement("fullname");
		Text fullnameText = doc.createTextNode(Utils.cleanFullName(aggr));
		classElm.appendChild(fullnameElm);
		fullnameElm.appendChild(fullnameText);	
		
		// Base classes.
		baseClassDocumentation(aggr, doc, classElm);
		
		// Sources.
		Set sources = getSources(aggr);
		Element sourcesElm = doc.createElement("source");
		for(Iterator iter = sources.iterator(); iter.hasNext();) {
			String source = ((SourceFile)iter.next()).getName();
			Element sourceElm = doc.createElement("file");
			Text sourceFile = doc.createTextNode(source);
			sourceElm.appendChild(sourceFile);
			sourcesElm.appendChild(sourceElm);
		}
		classElm.appendChild(sourcesElm);

		// Templates.
		if(aggr.isTemplated()) {
			Element templateElm = doc.createElement("template");
			String proto = "<";
			for (Iterator iter = aggr.templateParameterIterator(); iter.hasNext();) {
				TemplateParameter parameter = (TemplateParameter) iter.next();
				proto += parameter.getName();
				if(iter.hasNext()) {
					proto += ", ";
				}
			}
			proto += ">";
			Text templateText = doc.createTextNode(proto);
			templateElm.appendChild(templateText);
			classElm.appendChild(templateElm);
		}

		// Aliases.
		Element aliasesElm = doc.createElement("aliases");
		Namespace namespace = (Namespace)aggr.getContainer();
		for(Iterator iter = namespace.getScope().aliasIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Alias alias = (Alias)cc.getContained();
			
			// Add the typedef as an alias if it is a
			// typedef to the class. 
			if( alias.getAliasedType().isFlat() && 
				alias.getAliasedType().getBaseType() == aggr) {
			//Utils.cleanFormatCpp(alias.getAliasedType(),"")
			//.indexOf(aggr.getName()) != -1 ) {
				
				Element aliasElm = doc.createElement("typedef");
				// Name.
				nameDocumentation(alias, doc, aliasElm);
				// Prototype.
				String prototype = "typedef " +
					Utils.cleanFormatCpp(alias.getAliasedType(),Utils.cleanFullName(alias)); 
						
				Element protoElm = doc.createElement("prototype");
				Text protoText = doc.createTextNode
					(prototype);
				aliasElm.appendChild(protoElm);
				protoElm.appendChild(protoText);
				
				aliasesElm.appendChild(aliasElm);
			}
		}
		classElm.appendChild(aliasesElm);	
		
		// Class version.
		versionDocumentation(version, doc, classElm);
		
		// Class origin.
		originDocumentation(aggr, doc, classElm);
		
		// Class properties.
		Element paragraphElm = doc.createElement("paragraph");
		propertiesDocumentation(aggr, doc, paragraphElm);
		classElm.appendChild(paragraphElm);
		
		// Create the reference to all the groups and routines inside them.
		for (Iterator iter = aggr.getScope().groupIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Group group = (Group)cc.getContained();	
			
			// Send the older version of the aggregate to the 
			// 'documentGroup' method to use as a comparison if 
			// there is need in one.
			// If there is only one version the comparison won't
			// change a thing.
			documentGroup(group, oldAggr, doc, classElm);
		}
		
		// Create the reference to all routines that don't relate to any
		// group.
		for (Iterator iter = aggr.getScope().routineIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Routine routine = (Routine)cc.getContained();	
			
			// Document the routine only if it's public.
			if(routine.getContainerConnection().getVisibility() == 
				Specifiers.Visibility.PUBLIC) {
			
				// Determine the routine's version.
				String routineVersion;
				// If there are no comparison between aggregates all 
				// routines are with the version 'both'.
				if( ! m_toCompare ) { 
					routineVersion = "both";
					// Otherwise there is a need to check if the routine 
					// exists in both classes or only in one of them.
				} else if( oldAggr == null || newAggr == null) {
					// If the older or newer class doesn't exists the header 
					// itself will show its a new\old class and there is no 
					// need to repeat this information again in every routine.
					routineVersion = "both";
				} else if( containsRoutine(oldAggr, routine) ) {
					// If the both aggregates contains the routine the
					// version is "both".
					routineVersion = "both";
				} else {
					routineVersion = "new";
				}
			
				if(routine.getGroup() == null) {
					documentRoutine(routine, "method" ,routineVersion, doc, classElm);
				}
			}	
		}
		
		// Create a reference to all routines that only existed in the
		// old version.
		if(oldAggr != null && newAggr != null) {
				
			for (Iterator iter = oldAggr.getScope().routineIterator(); iter.hasNext();) {
				ContainedConnection cc = (ContainedConnection)iter.next();
				Routine routine = (Routine)cc.getContained();
			
				if( ! containsRoutine(newAggr, routine) && 
					routine.getContainerConnection().getVisibility() == 
					Specifiers.Visibility.PUBLIC) {
					
					documentRoutine(routine, "method" ,"old", doc, classElm);
					
				}	
			}		
		}
		
		// Document all inherited routines.
		//Element inheritedElm = doc.createElement("inherited");
		documentInheritedRoutines(aggr, doc, new ArrayList(), classElm);
		//classElm.appendChild(inheritedElm);		
		
		// Create the reference to all fields regardless to their 
		// grouping. 
		for (Iterator iter = aggr.getScope().fieldIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Field field = (Field)cc.getContained();	

			// Document the field only if it's public.
			if(field.getContainerConnection().getVisibility() == 
				Specifiers.Visibility.PUBLIC) {
			     
				// Determine the field's version.
				String fieldVersion;
				// If there are no comparison between aggregates all 
				// fields are with the version 'both'.
				if( ! m_toCompare ) { 
					fieldVersion = "both";
					// Otherwise there is a need to check if the field 
					// exists in both classes or only in one of them.
				} else if( oldAggr == null || newAggr == null) {
					// If the older or newer class doesn't exists the header 
					// itself will show its a new\old class and there is no 
					// need to repeat this information again in every field.
					fieldVersion = "both";
				} else if( containsField(oldAggr, field) ) {
					// If the both aggregates contains the field the
					// version is "both".
					fieldVersion = "both";
				} else {
					fieldVersion = "new";
				}
			
				documentField(field, fieldVersion, doc, classElm);	
			}
				
		}
		
		// Create a reference to all fields that only existed in the
		// old version.
		if(oldAggr != null && newAggr != null) {
				
			for (Iterator iter = oldAggr.getScope().fieldIterator(); iter.hasNext();) {
				ContainedConnection cc = (ContainedConnection)iter.next();
				Field field = (Field)cc.getContained();
			
				if( ! containsField(newAggr, field) && 
					field.getContainerConnection().getVisibility() == 
					Specifiers.Visibility.PUBLIC) {
					
					documentField(field, "old", doc, classElm);
					
				}	
			}		
		}
		
		// Blobal functions.
		documentGlobalFunctionsEnumsAndTypedefs(aggr, doc, classElm);
		
		
		//    DOCUMENT ALL DELETED METHODS \ MEMBERS THAT ARE INCLUDED 
		//               IN THE OLDER VERSION.
		
				
		return doc;
	}
	
	/**
	 * This method will recursively document a group inside a class.
	 * It will document the header of the group, all its related 
	 * routines and all subgroup recursively. 
	 * If there is a need to compare two aggreagtes the method will
	 * use its given aggregate in order to check if every routine
	 * exists in it.
	 * @param group The group to document.
	 * @param otherAggr The other aggregate in the comparison.
	 * @param doc The document itself.
	 * @param parent The element in the document that will contain the 
	 * documentation for the group. 
	 * @throws MissingInformationException if the program DB is 
	 * incomplete.
	 */
	private void documentGroup(Group group, Aggregate otherAggr,
	 	Document doc, Element parent) throws MissingInformationException {
	
		// Add the group documentation in the wanted location. 
		Element groupElm = doc.createElement("group");
		parent.appendChild(groupElm);
		
		// Group name.
		nameDocumentation(group, doc, groupElm);
		
		// Group properties.
		propertiesDocumentation(group, doc, groupElm);
		
		// Group's routines.
		List documentedRoutines = new LinkedList(); 
		for (Iterator iter = group.getScope().routineIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Routine routine = (Routine)cc.getContained();		
			
			// - don't document an already documented routine.
			// (this is a patch due to some bug in doxygen)
			boolean alreadyDocumented = false;
			for(int i = 0; i < documentedRoutines.size(); ++i) {
				if(equalRoutines(routine, (Routine)documentedRoutines.get(i))) {
					alreadyDocumented = true;
				}
			}
			
			if( !alreadyDocumented ) {
				documentedRoutines.add(routine);
				
				// Document thr routine only if its public.
				if(routine.getContainerConnection().getVisibility() == 
					Specifiers.Visibility.PUBLIC) {
				
					// Determine the routine's version.
					String version;
					// If there are no comparison between aggregates all 
					//routines are with the version 'both'.
					if( ! m_toCompare ) { 
						version = "both";
				
					// Otherwise there is a need to check if the routine 
					// exists in the other class or not (is a new version).		
					} else if( otherAggr != null 
							&& containsRoutine(otherAggr, routine) ) {
						version = "both";
					} else {
						version = "new";
					}
				
					documentRoutine(routine, "method", version, doc, groupElm);	
				}
			}
		}
		 
		
		//     WHAT ABOUT PRIVATE METHODS \ MEMBERS ??????????????
		// MEMBERS  ALIASES... ?????????????????????????????????????????
		
		// Sub-Groups.
		for (Iterator iter = group.getScope().groupIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Group subGroup = (Group)cc.getContained();
			
			documentGroup(subGroup, otherAggr, doc, groupElm);
		}
	} 
	
	/**
	 * This method will document a routine.
	 * It will document the header of the routine, and all it's 
	 * properties.
	 * @param routine The routine to document.
	 * @param version The routine's version.
	 * @param doc The document itself.
	 * @param parent The element in the document that will contain the 
	 * documentation for the routine. 
	 * @throws MissingInformationException if the program DB is 
	 * incomplete.
	 */
	private void documentRoutine(Routine routine, String type, String version, 
		Document doc, Element parent) throws MissingInformationException {
		
		Element routineElm = doc.createElement(type);
		parent.appendChild(routineElm);
			
		// Routine's name.
		nameDocumentation(routine, doc, routineElm);
				
		// Routine's prototype.
		Element routineProtoElm = doc.createElement("prototype");
		Text routineProtoText = doc.createTextNode
			(getRoutinePrototype(routine));
		routineElm.appendChild(routineProtoElm);
		routineProtoElm.appendChild(routineProtoText);
				
		// Routine's version.
		versionDocumentation(version, doc, routineElm);
				
		// Routine's origin.
		originDocumentation(routine, doc, routineElm);
				
		// Routine's properties.
		propertiesDocumentation(routine, doc, routineElm);
		
		/////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////
		/////////////////    CHECK IF THE METHOD IS OVERLOADED, 
		/////////////////    IF IT ISN'T AND ITS THE ONLY METHOD WITH THIS
		/////////////////    NAME, ADD IT AN @anchor: (CLASS_NAME).html#(METHOD_NAME) 
		/////////////////////////////////////////////////////////////////////////////////
	}
	
	/**
	 * This method will document a field.
	 * It will document the header of the field, and all it's 
	 * properties.
	 * @param field The field to document.
	 * @param version The field's version.
	 * @param doc The document itself.
	 * @param parent The element in the document that will contain the 
	 * documentation for the field. 
	 * @throws MissingInformationException if the program DB is 
	 * incomplete.
	 */
	private void documentField(Field field, String version, 
		Document doc, Element parent) throws MissingInformationException {
	
		Element fieldElm = doc.createElement("member");
		parent.appendChild(fieldElm);
			
		// Field's name.
		nameDocumentation(field, doc, fieldElm);
				
		// Field's prototype.
		Element fieldProtoElm = doc.createElement("prototype");
		Text fieldProtoText = doc.createTextNode
			(getFieldPrototype(field));
		fieldElm.appendChild(fieldProtoElm);
		fieldProtoElm.appendChild(fieldProtoText);
				
		// Field's version.
		versionDocumentation(version, doc, fieldElm);
				
		// Field's origin.
		originDocumentation(field, doc, fieldElm);
				
		// Field's properties.
		propertiesDocumentation(field, doc, fieldElm);	
	}
	
	/**
	 * This method will document all the inherited routines of a certain
	 * aggregate.
	 * It will document the base class and virtuality of every routine.
	 * The method will use a list in order to save all routines in the 
	 * aggregate and avoid documenting reimplemented routines. 
	 * @param aggr The aggregate to docuemnt its inherited routines.
	 * @param doc The document.
	 * @param routines A list of all routines.
	 * @param parent The element to add the documentation to.
	 * @throws MissingInformationException if the program DB isn't 
	 * complete.
	 */
	private void documentInheritedRoutines(Aggregate aggr, 
		Document doc, List routines, Element classElm) throws MissingInformationException {
		
		// Add all the aggregate's routines in order to avoid 
		// documenting reimplemented routines.
		for (Iterator iter = aggr.getScope().routineIterator(); iter.hasNext();) {
			ContainedConnection cc = (ContainedConnection)iter.next();
			Routine routine = (Routine)cc.getContained();
			
			routines.add(routine);
		}
			
		// Go over all the class' base classes.
		for (Iterator iter = aggr.baseIterator(); iter.hasNext();) {
			InheritanceConnection ic = (InheritanceConnection)iter.next();
			
			// Check if the inheritence is public.
			if(ic.getVisibility() == Specifiers.Visibility.PUBLIC) {
				Aggregate base = ic.getBase();
				
				// Document the base class.
				//Element baseElm = doc.createElement("class");
				//baseElm.setAttribute("name", base.getName());
				//parent.appendChild(baseElm); 
				
				// Document the routines from the class.
				for (Iterator iterator = base.getScope().routineIterator();
					iterator.hasNext(); ) {
					
					ContainedConnection cc = (ContainedConnection)iterator.next();
					Routine routine = (Routine)cc.getContained();
					
					// Check if the routine isn't reimplemented.
					boolean reimplemented = false;
					for(int i = 0; i < routines.size(); ++i) {
						if(overidesRoutine(routine, (Routine)routines.get(i))) {
							reimplemented = true;
						}
					}
					
					if( ! reimplemented && ! routine.isConstructor() && 
						cc.getVisibility() == Specifiers.Visibility.PUBLIC) {
					
						Element routineElm = doc.createElement("method");
						nameDocumentation(routine, doc, routineElm);
				
						// Routine's prototype.
						Element routineProtoElm = doc.createElement("prototype");
						Text routineProtoText = doc.createTextNode
							(getRoutinePrototype(routine));
						routineElm.appendChild(routineProtoElm);
						routineProtoElm.appendChild(routineProtoText);
			
						// Routine's origin.
						originDocumentation(routine, doc, routineElm);
				
						// Routine's properties.
						propertiesDocumentation(routine, doc, routineElm);
					
						// Virtuality + Base class.
						String virtuality;
						if(routine.getContainerConnection().getVirtuality() == 
							Specifiers.Virtuality.VIRTUAL) {
							virtuality = "virtual";
						} else if(routine.getContainerConnection().getVirtuality() == 
							Specifiers.Virtuality.PURE_VIRTUAL) {
							virtuality = "pure-virtual";
						} else {
							virtuality = "non-virtual";
						}
						Element inheritenceElm = doc.createElement("inherited");
						inheritenceElm.setAttribute("class", base.getName());
						inheritenceElm.setAttribute("virtuality", virtuality);
						routineElm.appendChild(inheritenceElm);
						
						// Group.
						appendInGroup(routine, routineElm, classElm, doc);
					}
				}		
				
				// Document recursively the base class' base classes.
				documentInheritedRoutines(base, doc, routines, classElm);
			} 			
		}
			
	}	
	
	/**
	 * 
	 */
	private void appendInGroup(Routine routine, Element routineElm, 
		Element classElm, Document doc) {
			
		if(routine.getGroup() == null) { // No group
			// If the routine doesn't belong to any group
			// append it directly to the class.
			classElm.appendChild(routineElm);
							
		} else {
			// Find the group.
			String groupName  = routine.getGroup().getName().trim();
			NodeList groups = classElm.getElementsByTagName("group");
			Element groupElm = null;
			for(int i = 0; i < groups.getLength(); ++i) {
				Element group = (Element)groups.item(i); 
				String tmpName = group.getElementsByTagName("name").item(0).getFirstChild().getNodeValue();
				if( groupName.equals( tmpName ) ) {
					groupElm = group;
				}
			}		
							
			if(groupElm == null) { // The group doesn't exists.
				// If the group wasn't found create it.
				groupElm = doc.createElement("group");
				nameDocumentation(routine.getGroup(), doc, groupElm);
				propertiesDocumentation(routine.getGroup(), doc, groupElm);
				groupElm.appendChild(routineElm);
								
				// Create the full 'path' of groups until this group
				boolean parentGroupExists = false;
				Group parentGroup = routine.getGroup();
				while( (parentGroup = parentGroup.getGroup()) != null) {
					Element tmp = null;
					for(int i = 0; i < groups.getLength(); ++i) {
						Element group = (Element)groups.item(i); 
						String tmpName = group.getElementsByTagName("name").item(0).getFirstChild().getNodeValue();
						if( parentGroup.getName().equals( tmpName ) ) {
							tmp = group;
						}
					}	
									
					if(tmp == null) { // The parent group doesn't exists.
									
						// Create the parent group too.
						tmp = doc.createElement("group");
						nameDocumentation(parentGroup, doc, tmp);
						propertiesDocumentation(parentGroup, doc, tmp);
									
						tmp.appendChild(groupElm);
						groupElm = tmp;
										
					} else { // The parent group exists.
									
						tmp.appendChild(groupElm);
						parentGroupExists = true;
						break;	
					}
				}
								
				// Append the new group to the class element (unless
				// it is already connected to a parent group).
				if( ! parentGroupExists ) {
					classElm.appendChild(groupElm);
				}
								
			} else {
				// If the group was found append the routine to it.
				groupElm.appendChild(routineElm);
			}
		}
	}
	
	/**
	 * 
	 */
	private void documentGlobalFunctionsEnumsAndTypedefs(Aggregate aggr, 
		Document doc, Element parent) throws MissingInformationException {
		
		// A group for global functions.
		Element groupElm = doc.createElement("global");		
			
		// Get all the source files related to the aggregate
		// in order to look for the global functions there.
		Set<SourceFile> sources = getSources(aggr);
		
		// Look for global functions in the sources.
		for (SourceFile source: sources) {
			
			for(Iterator riter = source.declarationIterator(); riter.hasNext();) {
				SourceFile.DeclDefConnection ddc = 
					(SourceFile.DeclDefConnection)riter.next();
				sourceanalysis.Entity declared = ddc.getDeclaredEntity();
			
				if(declared instanceof Routine) { // Function
					Routine routine = (Routine)declared;
					// Check if the routine is a global function.
					// (by checking if its container is a namespace).
					if(routine.hasContainer() && 
						routine.getContainer() instanceof Namespace) {
						
						// This ugly part checks that this is actually a routine 
						// (although is was checked before) because macros (such
						// as xDEBUG_FLAG) can fool doxygen and appear like routines.	
						if( ! getRoutinePrototype(routine).matches("^.*, \\)$")) {
							// Create the element for the global function.
							documentRoutine(routine, "function", "both", doc, groupElm);		
						}
					} 					
				} else if(declared instanceof sourceanalysis.Enum) { // Enum
					sourceanalysis.Enum enume = (sourceanalysis.Enum)declared;
					Element enumElm = doc.createElement("enum");
					// Name.
					nameDocumentation(enume, doc, enumElm);
					// Prototype.
					String prototype = "enum " + 
						Utils.cleanFullName(enume) + "{";
					for(Iterator citer = enume.constantIterator(); citer.hasNext();) {
						sourceanalysis.Enum.Constant constant = (sourceanalysis.Enum.Constant)citer.next();
						prototype += constant.getLiteral();
						
						if(citer.hasNext()) {
							prototype += ", ";
						}
					}
					prototype += "}";
					Element protoElm = doc.createElement("prototype");
					Text protoText = doc.createTextNode
						(prototype);
					enumElm.appendChild(protoElm);
					protoElm.appendChild(protoText);
					
					// Properties.
					propertiesDocumentation(enume, doc, enumElm);
					
					groupElm.appendChild(enumElm);
					
				} else if(declared instanceof Alias) { // Typedef
					Alias typedef = (Alias)declared;
					Element typedefElm = doc.createElement("typedef");
					// Name.
					nameDocumentation(typedef, doc, typedefElm);
					// Prototype.
					String prototype = "typedef " +
						Utils.cleanFormatCpp(typedef.getAliasedType(),Utils.cleanFullName(typedef)); 
						
					Element protoElm = doc.createElement("prototype");
					Text protoText = doc.createTextNode
						(prototype);
					typedefElm.appendChild(protoElm);
					protoElm.appendChild(protoText);
					
					// Propreties.
					propertiesDocumentation(typedef, doc, typedefElm);
					
					groupElm.appendChild(typedefElm);
					
				}
			}
			
			parent.appendChild(groupElm);		
		}	
	}
	
	/**
	 * 
	 */
	private void baseClassDocumentation(Aggregate aggr, Document doc,
		Element parent) {
	
		Element extendsElm = doc.createElement("extends");
		for (Iterator iter = aggr.baseIterator(); iter.hasNext();) {
			InheritanceConnection ic = (InheritanceConnection)iter.next();
			
			if(ic.getVisibility() == Specifiers.Visibility.PUBLIC) {
				
				Aggregate base = ic.getBase();
				Element baseElm = doc.createElement("class");
				baseElm.setAttribute("name", base.getName());
				extendsElm.appendChild(baseElm);
				
				baseClassDocumentation(base, doc, baseElm);
			}
		}
		parent.appendChild(extendsElm);		
	}
	
	/**
	 * This method will create and return an Element containing the
	 * documentation of an entity's name.
	 * @param entity The entity to document.
	 * @param doc The document.
	 * @param parent The element in the document that will contain the 
	 * documentation. 
	 */
	private void nameDocumentation(sourceanalysis.Entity entity, 
		Document doc, Element parent) {
		
		Element nameElm = doc.createElement("name");
		Text nameText = doc.createTextNode(entity.getName().trim());
		parent.appendChild(nameElm);
		nameElm.appendChild(nameText);	
	}
	
	/**
	 * This method will create and return an Element containing the
	 * documentation of a version.
	 * @param version The version to write in the documentation.
	 * @param doc The document.
	 * @param parent The element in the document that will contain the 
	 * documentation. 
	 */
	private void versionDocumentation(String version, Document doc, 
		Element parent) {
		
		Element versionElm = doc.createElement("version");
		Text versionText = doc.createTextNode(version);
		parent.appendChild(versionElm);
		versionElm.appendChild(versionText);	
	}
	
	/**
	 * This method will create and return an Element containing the
	 * documentation of an entity's origin.
	 * @param entity The entity to document.
	 * @param doc The document.
	 * @param parent The element in the document that will contain the 
	 * documentation.   
	 */
	private void originDocumentation(sourceanalysis.Entity entity, 
		Document doc, Element parent) {
		
		originDocumentationEx(entity.getDeclaration(), doc, parent);
		originDocumentationEx(entity.getDefinition(), doc, parent);
	}
	
	private void originDocumentationEx(SourceFile.DeclDefConnection source,
		Document doc, Element parent) {
	
		Element originElm = doc.createElement("origin");
		Element fileElm = doc.createElement("file");
		Element lineElm = doc.createElement("line");
		if(source != null) {
			try {
			Text fileText = doc.createTextNode
				(source.getSource().getName());
			Text lineText = doc.createTextNode
				(Integer.toString(source.where().getStart().line));
			parent.appendChild(originElm);
			originElm.appendChild(fileElm);
			originElm.appendChild(lineElm);
			fileElm.appendChild(fileText);
			lineElm.appendChild(lineText);
			} catch(MissingInformationException e) { /* Ignore */ }
		}			
	}
	
	/**
	 * This method will create and return an Element containing the
	 * documentation of an entity's properties.
	 * @param entity The entity to document.
	 * @param doc The document.
	 * @param parent The element in the document that will contain the 
	 * documentation. 
	 */
	private void propertiesDocumentation(sourceanalysis.Entity entity, 
		
		////////////////////////////////////////////////////////
		//     CHECK ALL THE PROPERTIES !!!!!!!!!!!!!!!!
		////////////////////////////////////////////////////////
		
		Document doc, Element parent) {
		
		org.apache.xerces.parsers.DOMParser parser = 
			new org.apache.xerces.parsers.DOMParser();
		
		for (Iterator iter = entity.propertyIterator(); iter.hasNext();) {
			sourceanalysis.Entity.Property prop = 
				(sourceanalysis.Entity.Property)iter.next();
			
			if (prop.isConcealed()) continue; // skip reserved properties
			
			// The content of the property may be an element by
			// itself, so it needs to be parsed. 
			String name = prop.getName().trim().replace(' ', '-');
			
			String property = new String("<" + name + ">" + 
				prop.getValue() + "</" + name + ">");
			
			Node propNode;
			try {
				// A common error in the doc-blocks is writing 'xTVector<x>' without escaping.
				// Here I'll do the escaping automatically.
				Pattern pattern = Pattern.compile("(xTVector<(( < )?[^/]*?( > )?)>)");
				Matcher matcher = pattern.matcher(property);

				while(matcher.find()) {
					String in = matcher.group(2);
					property = property.substring(0,matcher.start(1)) + "xTVector&lt;" + 
									in + "&gt;" + property.substring(matcher.end(1));
					matcher = pattern.matcher(property);
				}
 
				// Try to parse the property.
				parser.parse(new InputSource(new StringReader(property)));
				Document tmp = parser.getDocument();
				propNode = tmp.getFirstChild();
			} catch(Exception e) { 
				System.err.println(" --- Warning: " + name + 
					" element isn't written legally!");
				System.err.println(" --- " + property);
				
				
				//e.printStackTrace();	
				
				propNode = doc.createElement(name);
				Text propText = doc.createTextNode(prop.getValue());
				propNode.appendChild(propText);
			}
			Node cloned =
				doc.importNode(propNode, true);
			//Element propElm = doc.createElement(name);
			//Text propText = doc.createTextNode(prop.getValue());
			//propElm.appendChild(propText);
			parent.appendChild(cloned);
		} 	
	}
	
	/**
	 * This method will create the XML documents in the given directory.
	 * @param The target directory for the XML documents.
	 */
	public void write(String targetDir) 
		throws IOException { 
		
		// EXCEPTIONS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
		// Add a seperator  to the end of the target dir in 
		// order to prevent the problem that will accour if 
		// there was no seperetor in the end (the writer will
		// try to save the files outside the directory).
		targetDir += File.separator; 
	
		// Check the the target directory is a valid one.
		File dir = new File(targetDir);
		if( ! dir.isDirectory() ) {
			throw new IOException("Illegal target directory");
		}
		
		// Create two lists to log the created documents.
		List<String> createdDocs = new ArrayList<String>();
		List<String> nonCreatedDocs = new ArrayList<String>();
		
		// Go over all the classes.
		for(int i = 0; i < m_classes.length; ++i) {
			
			// Find the aggregate (class) in both versions (if possible).
			Aggregate oldAggr = null;
			Aggregate newAggr = null;
			
			try{
				oldAggr = getAggregateByName(m_classes[i], m_oldDB);	
			} catch(ElementNotFoundException enfe) {
				oldAggr = null;
			} catch(IllegalArgumentException iae) {
				// If the name of the class is incorrect add it to
				// the failed classes.
				nonCreatedDocs.add(new String(m_classes[i] + ".xml" +
					" (illegal name)"));
				continue; 
			}
			
			if(m_toCompare) {
				try{
					newAggr = getAggregateByName(m_classes[i], m_newDB);	
				} catch(ElementNotFoundException enfe) {
					newAggr = null;
				} catch(IllegalArgumentException iae) {
					// This exception can't be thrown because the name of
					// the class is surely a legal name (otherwise it 
					// would have thrown an exception in the last call to
					// 'getAggregateByName'.
					// Therefore if this exception is thrown it is a major
					// error and the program must stop.
					System.err.println(iae.getMessage());
					System.exit(1); 
				}
			}
			
			// If the class doesn't exist in both versions, add it to
			// the non documented classes list. Otherwise document it.
			if(newAggr == null && oldAggr == null) {
				nonCreatedDocs.add(new String(m_classes[i] + ".xml" +
					" (not found)"));
			} else {
				
				try{
				
					Document doc = documentAggregate(oldAggr, newAggr);
					FileOutputStream out = 
						new FileOutputStream(targetDir + m_classes[i].substring(2) + ".xml");
					
					//       FIX FILE NAME TO WHAT ??? (::)
					
					writeXMLDocument(out, doc);
					
					// Add the written document to the list.
					createdDocs.add(m_classes[i] + ".xml");
					
				} catch(Exception e) {
					// Every exception thrown from this methods is a 
					// fatal one and therefore the program must end.
					e.printStackTrace();
					System.err.println(e.getMessage());
					System.exit(1); 
				}
			}
			
			// CREATE A FILE FOR EACH MODULE CONTAINING THE MODULES'S
			// GLOBAL FUNCTIONS.
			
		}
			
		// Print a log of all created \ non created classes.
		System.out.println(" --------------------------------------- ");
		if( createdDocs.size() > 0 ) {
			System.out.println(" |    Documents written:");
			for(int i = 0; i < createdDocs.size(); ++i) {	
				System.out.println(" | " + createdDocs.get(i));
			}
		}
		if( nonCreatedDocs.size() > 0 ) {
			System.out.println(" |    Documents failed:");
			for(int i = 0; i < nonCreatedDocs.size(); ++i) {	
				System.out.println(" | " + nonCreatedDocs.get(i));
			}
		}
		System.out.println(" --------------------------------------- ");
		System.out.println(" |    Total: " + createdDocs.size() + 
			" created,   " + nonCreatedDocs.size() + 
			" failed.");
		System.out.println(" --------------------------------------- ");
	}
	
	/**
	 * This method will print the instructions for using the 
	 * XMLWriter.
	 */
	public static void printInstructions() {
		
		System.out.println(" ");
		System.out.println("                      --- Instructions ---");
		System.out.println(" ");
		System.out.println("This program operates in two different modes:");
		System.out.println(" (1) Create a documentation to a single version of a program.");
		System.out.println("     Use - XMLWriter <class_list_file(xml)> <source_directory>");
		System.out.println("                     <target_directory>");
		System.out.println(" ");
		System.out.println(" (2) Create a documentation from a comparison of two versions");
		System.out.println("     of the program.");
		System.out.println("     Use - XMLWriter <class_list_file(xml)> <old_source_directory>");	
		System.out.println("                     <new_source_directory> <target_directory>");
		System.out.println(" ");
	}	
		
	public static void main(String[] args) {
		
		// The main function can operate in two different modes.
		// In any case the first argument is the path of the file 
		// containing the list of classes.
		// The first mode creates the XML documentation from a single
		// program DB and recieves one more argument - the path
		// of the directory containing the information for the program
		// DB.
		// The second mode create the XML documentation from two
		// different versions of the program DB by comparing them
		// and therefore recieves two pathes of directories with 
		// program DB inforamtion - for the old and the new version
		// respectivly.
		// The third \ fourth and last argument is the target directory
		// to which the XML documents will be written.


		// Print instructions the amount of arguments is wrong.
		if(args.length != 3 && args.length != 4) {
			printInstructions();
			System.exit(1);
		} 
		
		
		XMLWriter writer;
		String targetDir;
		try {
		
			if(args.length == 4) { // Compare between two different versions.
				writer = new XMLWriter(args[0], args[1], args[2]);
				targetDir = args[3];
				
			} else { // Do not compare. 
				writer = new XMLWriter(args[0], args[1]);
				targetDir = args[2];
			}
		
			// EXCEPTIONS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
			writer.write(targetDir);
		
		} catch(Exception e) {
			e.printStackTrace(); /////////////////////////////////
			/////////////////////////////////////////////////
			System.err.println(e.getMessage());
			System.err.println("No targets were made.");	
			System.exit(1);
		}
	}
	
	/** The older version of the program DB */
	private ProgramDatabase m_oldDB;
	/** The newer version of the program DB */
	private ProgramDatabase m_newDB;
	
	/** Whether or not to compare two different DB's */
	private boolean m_toCompare;
	
	/** The list of classes to document */
	private String[] m_classes;	
	
	/** The list of modules */
	private Module[] m_modules;
	
	///////////////////////////////////////////////////////////////
	///////////////////////// -- CLASS -- /////////////////////////
	///////////////////////////////////////////////////////////////
	
	
	/**
	 * 
	 */
	private class Module {
		
		/**
		 * 
		 */
		public Module(String name) {
			
			m_name = name;
			m_sources = new LinkedList<String>();
		}	
		
		/**
		 * 
		 */
		public void addSource(String sourceName) {
			m_sources.add(sourceName);	
		}
		
		public String getName() { return m_name; }
	
		/** The name of the module */
		private String m_name;
		
		/** The list of source files */
		private List<String> m_sources;
	}
}

