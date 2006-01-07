package sourceanalysis.assist;

import java.io.File;
import java.io.FilenameFilter;

import sourceanalysis.*;
import sourceanalysis.xml.XMLFormatException;

/**
 * Provides a repository for templates which are defined in some program,
 * exported and may be used by other programs in means of creating new
 * instances.
 * <p>For example, <tt>std::vector</tt> is defined as a template in the
 * program <b>STL</b>. However, any program defining new classes may well
 * create new instances of <tt>std::vector</tt>, and may therefore require
 * access to the complete interface of this class template.
 */
public class SupervisedTemplates {

	/**
	 * Initializes the supervised templates databank by reading all the
	 * supervised template definition documents along the supervised
	 * templates' path.
	 * This path is taken from the system properties.
	 */
	public SupervisedTemplates() {
		super();
		m_analyzer = new InteriorAnalyzer();
		// Explore the supervised template path
		String stpath = System.getProperty(STXML_PATH_PROPERTY);
		if (stpath != null) {
			traversePath(stpath,
				new DefDocumentProcessor() { 
					public void processElement(File documentPath) 
						throws XMLFormatException
					{ m_analyzer.absorbDefinitions(documentPath); }
				}
			);
		}
	}
	
	/**
	 * Looks up a supervised template.
	 * @param templateName full name of the template
	 * @return the template entity from the supervised template collection
	 * if one matches that name; otherwise, <b>null</b>.
	 */
	public Aggregate getSupervised(String templateName)
	{
		Entity container = m_analyzer.lookupContainer(templateName);
		if (container != null && container instanceof Aggregate)
			return (Aggregate)container;
		else
			return null;
	}


	/**
	 * A document processor interface. Used with traversePath() to locate
	 * supervised template definitions along the path.
	 */
	private static interface DefDocumentProcessor
	{
		/**
		 * Called for every element found during processing of the s.t.
		 * path.
		 * @param documentPath an abstract pathname of the document
		 * @throws XMLFormatException if document appears to be corrupted
		 */
		public void processElement(File documentPath) 
			throws XMLFormatException;
	}

	private static void traversePath(String stPath, 
		DefDocumentProcessor processor)
	{
		String pathSep = System.getProperty("path.separator");
		String[] pathElements = stPath.split(pathSep);
		for (int i = 0; i < pathElements.length; i++) {
			File pathElement = new File(pathElements[i]);
			// Get all path elements ending with STXML_EXTENSION
			File[] stElements = pathElement.listFiles(new FilenameFilter() {
				public boolean accept(File dir, String name) 
				{ return name.endsWith(STXML_EXTENSION); }
			});
			if (stElements == null) {
				System.err.println("*** WARNING: supervised template " 
						+ "directory " + pathElement + " is invalid.");
				continue;
			}
			// Process the elements found
			for (int j = 0; j < stElements.length; j++) {
				try {
					processor.processElement(stElements[j]);
				}
				catch (XMLFormatException e) {
					System.err.println("*** WARNING: corrupted supervised "
						+ "template document encountered at path element #"
						+ j + ", " + stElements[j]);
					System.err.println("::: " + e);
				}
			}
		}
	}

	public static void main(String[] args) {
		new SupervisedTemplates();
	}

	// Repository
	private InteriorAnalyzer m_analyzer;
	
	private static final String STXML_EXTENSION = ".st.xml";
	private static final String STXML_PATH_PROPERTY = "griffin.st.path";
}
