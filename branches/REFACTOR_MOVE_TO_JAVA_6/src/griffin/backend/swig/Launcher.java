/*
 * Created on Jun 22, 2003
 */
package backend.swig;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

import sourceanalysis.ElementNotFoundException;
import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.view.TemplateBank;
import sourceanalysis.view.TemplateReader;
import backend.Backend;
import backend.PropertyPage;
import backend.annotations.BackendDescription;
import backend.annotations.PropertyDescription;
import backend.configuration.PropertyData;

@BackendDescription(backendName = "swig", backendDescription = "Generate SWIG interface from classes")
public class Launcher implements Backend {


   public void execute(ProgramDatabase program, PropertyPage properties) throws IOException, MissingInformationException {
       
		try {
           outfile = properties.getString("outfile");
           classnames = properties.getStringArray("classes");
           templatefile = properties.getString("templatefile");
           autocollect = properties.getBoolean("auto");
           
           OutputStream cfile = new FileOutputStream(outfile);
           TemplateBank templates = TemplateReader.readTemplatesFromFile(templatefile);

			CodeGenerator codegen =
               new CodeGenerator(program, new OutputStreamWriter(cfile), 
				                  templates);
           
           if(autocollect) {
               codegen.autocollect();
			}
           
           for (int i = 0; i < classnames.length; ++i) {
               codegen.collect(classnames[i]);
           }

			codegen.generateClassInterface();
			codegen.generateGlobalVariableInterface();

			cfile.close();
       } catch (ElementNotFoundException e) {
           throw new MissingInformationException("Some information is missing");
		}
       
       
       
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
