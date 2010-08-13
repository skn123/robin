/*
 * Created on Jun 22, 2003
 */
package backend.pydoc;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

import backend.Backend;
import backend.PropertyPage;
import backend.annotations.BackendDescription;
import backend.annotations.PropertyDescription;
import backend.configuration.PropertyData;

import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.view.TemplateBank;
import sourceanalysis.view.TemplateReader;

@BackendDescription(backendName = "pydoc", backendDescription = "Generate PyDoc documentation for the classes")
public class Launcher implements Backend {


	
	/**
	 * Runs the Python documentation back-end and generates output.
    * 
	 */
	public void execute(ProgramDatabase program, PropertyPage properties)
		throws IOException, MissingInformationException 
	{
       outfile = properties.getString("outfile");
       classnames = properties.getStringArray("classes");
       templatefile = properties.getString("templatefile");
       autocollect = properties.getBoolean("auto");
		
		
       
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
			
       if(autocollect) {
           codegen.autocollect();
       }
		// Collect targets
		for (int i = 0; i < classnames.length; ++i) {
           codegen.collect(classnames[i]);
		}

		// Generate output
		codegen.generateClassDocumentation();

		cfile.close();
	}

   @PropertyDescription(propertyName = "outfile",
            propertyDescription = "Output directory name",
            numberOfArguments = 1,
            required = false,
            defaultValue = "./robin_pydoc/")
   private String outfile;
   
   @PropertyDescription(propertyName = "classes",
            propertyDescription = "list of class names to collect documentation from",
            numberOfArguments = PropertyData.ANY_NUMBER_OF_ARGUMENTS,
            required = false,
            defaultValue = "")
   private String[] classnames;
   
   @PropertyDescription(propertyName = "templatefile",
            propertyDescription = "Template file name",
            numberOfArguments = 1,
            required = true,
            defaultValue = "")
   private String templatefile;
   
   @PropertyDescription(propertyName = "auto",
            propertyDescription = "Enable auto collection",
            numberOfArguments = 0,
            required = false,
            defaultValue = "false")
   private boolean autocollect;
   
   
}
