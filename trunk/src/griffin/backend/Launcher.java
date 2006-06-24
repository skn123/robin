package backend;

import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;

import sourceanalysis.ElementNotFoundException;
import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.dox.DoxygenAnalyzer;
import sourceanalysis.mixin.JythonMixIn;
import sourceanalysis.mixin.MixIn;

public abstract class Launcher {

	public static class PropertyPage {
		
		public PropertyPage() {
			m_properties = new HashMap();
		}
		
		public void addBoolean(String name, boolean defaultValue) {
			m_properties.put(name, new Boolean(defaultValue));
		}
		
		public void setBoolean(String name, boolean newValue) {
			m_properties.put(name, new Boolean(newValue));
		}
		
		public boolean getBoolean(String name) {
			return ((Boolean)m_properties.get(name)).booleanValue();
		}
		
		public void addString(String name, String defaultVal) {
			m_properties.put(name, defaultVal);
		}
		
		public void setString(String name, String value) {
			m_properties.put(name, value);
		}
		
		public String getString(String name) {
			return (String)(m_properties.get(name));
		}
		
		public void setStringArray(String name, String[] value) {
			m_properties.put(name, value);
		}
		
		public String[] getStringArray(String name) {
			return (String[])(m_properties.get(name));
		}
		
		public boolean hasProperty(String name) {
			return m_properties.containsKey(name);
		}
		
		private Map m_properties;
	}
	
	/**
	 * Main back-end entry point.
	 * 
	 * @param program a program database to operate upon
	 * @param properties a set of attributes describing the actions
	 *  to be taken, with at least the following members:
	 *  - outfile: name of a file to direct the output into
	 *  - classes: an array of class names
	 * @throws IOException if an output exception occurs
	 * @throws MissingInformationException if the program database is 
	 * incomplete
	 */
	public abstract void execute(ProgramDatabase program, PropertyPage properties)
		throws IOException, MissingInformationException; 

	/**
	 * Separates the command-line arguments into boolean properties,
	 * names of classes for analysis and mix-ins to apply.
	 * @param args command-line arguments
	 * @param properties property page to be filled
	 * @param classnames a collection to be filled with class names
	 * @param mixins a collection to be filled with requested mix-ins
	 */
	private void parseAttributesAndMixIns(String[] args, 
			PropertyPage properties, 
			Collection classnames, 
			Collection mixins)
	{
		for (int flagIndex = 2; flagIndex < args.length; ++flagIndex) {
			if (args[flagIndex].startsWith("--hints=")) {
				mixins.add(new JythonMixIn(args[flagIndex].substring(8)));
			}
			else if (args[flagIndex].startsWith("--")) {
				String flag = args[flagIndex].substring(2);
				if (properties.hasProperty(flag)) {
					properties.setBoolean(flag, true);
				}
			}
			else {
				classnames.add(args[flagIndex]);
			}
		}
	}
	
	/**
	 * Reads arguments from the command line according to the
	 * specified property page, and builds the ProgramDatabase to
	 * work on using DoxygenAnalyzer.
	 * 
	 * @param args command line arguments in the following format:
	 *   1st argument - location of intermediate input files
	 *   2nd argument - output file name
	 *   3rd to last  - class names and optional (back-end specific)
	 *                  flags
	 * @param properties
	 * @return a fully analyzed ProgramDatabase
	 */
	protected ProgramDatabase processCmdlineParameters(String[] args,
			PropertyPage properties)
	{
		try {
			List mixins = new LinkedList();
			List classnames = new LinkedList();

			// get outfile
			properties.setString("outfile", args[1]);
			
			// get flag values and class names
			parseAttributesAndMixIns(args, properties, classnames, mixins);

			// store the class names
			properties.setStringArray("classes", 
					(String[])classnames.toArray(new String[0]));
			
			// Process input files and create the program database
			DoxygenAnalyzer dox = new DoxygenAnalyzer(args[0]);
			dox.logger.setLevel(Level.WARNING);
			ProgramDatabase pdb = dox.processIndex();
			
			// Apply mix-ins
			for (Iterator mixini = mixins.iterator(); mixini.hasNext(); ) {
				((MixIn)mixini.next()).apply(pdb);
			}
			
			return pdb;
			
		} catch (ElementNotFoundException e) {
			System.err.println("*** ERROR: failed to read index: " + e);
			return null;
		}
	}
	
	protected void main(String backendName, String[] args, 
			PropertyPage properties) 
	{
		if (args.length < 2) {
			System.err.println("*** ERROR: not enough arguments.");
			System.err.println(
				"    Usage: " + backendName + " intermediate output classnames");
			System.exit(1);
		}
		
		try {
			ProgramDatabase p = processCmdlineParameters(args, properties);
	
			execute(p, properties);
		} catch (IOException e) {
			System.err.println("*** ERROR: output error: " + e);
		} catch (MissingInformationException e) {
			System.err.println("*** ERROR: some information is missing.");
			e.printStackTrace();
		}
	}
}

