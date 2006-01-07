/*
 * Created on Jun 22, 2003
 */
package backend.pydoc;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.logging.Level;

import sourceanalysis.ElementNotFoundException;
import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.dox.DoxygenAnalyzer;
import sourceanalysis.view.TemplateBank;
import sourceanalysis.view.TemplateReader;

public class Launcher {

	/**
	 * 
	 */
	public Launcher() {
		super();
	}

	public static void main(String[] args)
	{
		if (args.length < 4) {
			System.err.println("*** ERROR: not enough arguments.");
			System.err.println("    Usage: backend.pydoc.Launcher "
				+ "template-file intermediate output classnames");
			System.exit(1);
		}
		// Read templates
		TemplateBank templates = null;
		try {
			templates = TemplateReader.readTemplatesFromFile(args[0]);
		}
		catch (IOException e) {
			System.err.println("*** FATAL: cannot read template definition"
				+ " file: " + e);
			System.exit(1);
		}
		// Read the program database
		DoxygenAnalyzer dox = new DoxygenAnalyzer(args[1]);
		dox.logger.setLevel(Level.WARNING);
		try {
			ProgramDatabase p = dox.processIndex();
		
			OutputStream cfile = new FileOutputStream(args[2]);
			CodeGenerator codegen =
				new CodeGenerator(p, new OutputStreamWriter(cfile), 
				                  templates);
				
			// Collect targets
			for (int i = 3; i < args.length; ++i) {
				String arg = args[i];
				if (arg.equals("Auto")) codegen.autocollect();
				else codegen.collect(arg);
			}

			codegen.generateClassDocumentation();

			cfile.close();
		}
		catch (ElementNotFoundException e) {
			System.err.println("*** ERROR: failed to read index: " + e);
		}
		catch (IOException e) {
			System.err.println("*** ERROR: output error: " + e);
		}
		catch (MissingInformationException e) {
			System.err.println("*** ERROR: some information is missing.");
		}
	}
}
