/*
 * Created on Jun 22, 2003
 */
package backend.pydoc;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.view.TemplateBank;
import sourceanalysis.view.TemplateReader;

public class Launcher extends backend.Launcher {

	/**
	 * 
	 */
	public Launcher() {
		super();
	}

	public static void main(String[] args)
	{
		PropertyPage properties = new PropertyPage();
		properties.addString("templatefile", null);
		
		new Launcher().main("backend.pydoc.Launcher", args, properties);
	}
	
	/**
	 * Runs the Python documentation back-end and generates output.
	 */
	public void execute(ProgramDatabase program, PropertyPage properties)
		throws IOException, MissingInformationException 
	{
		String outfile = properties.getString("outfile");
		String[] classnames = properties.getStringArray("classes");
		String templatefile = properties.getString("templatefile");
		
		if (templatefile == null) {
			System.err.println("*** ERROR: template filename not provided"
					+ " (specify --templatefile=)");
			System.exit(1);
		}
		
		execute(program, templatefile, outfile, classnames);
	}
	
	/**
	 * Runs the Python documentation back-end and generates output.
	 * 
	 * @param program
	 * @param templatefile
	 * @param outfile
	 * @param classnames
	 * @throws IOException
	 * @throws MissingInformationException
	 */
	private void execute(ProgramDatabase program, String templatefile, String outfile, String[] classnames)
		throws IOException, MissingInformationException
	{
		// Read templates
		TemplateBank templates = null;
		try {
			templates = TemplateReader.readTemplatesFromFile(templatefile);
		}
		catch (IOException e) {
			System.err.println("*** FATAL: cannot read template definition"
				+ " file: " + e);
			System.exit(1);
		}

		// Create code generator
		OutputStream cfile = new FileOutputStream(outfile);
		CodeGenerator codegen =
			new CodeGenerator(program, new OutputStreamWriter(cfile), 
			                  templates);
			
		// Collect targets
		for (int i = 0; i < classnames.length; ++i) {
			String classname = classnames[i];
			if (classname.equals("Auto")) codegen.autocollect();
			else codegen.collect(classname);
		}

		// Generate output
		codegen.generateClassDocumentation();

		cfile.close();
	}
}
