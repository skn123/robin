/*
 * Created on Jun 22, 2003
 */
package backend.man;


import java.io.IOException;

import backend.Backend;
import backend.PropertyPage;
import backend.annotations.BackendDescription;
import backend.annotations.PropertyDescription;
import backend.configuration.PropertyData;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.dox.DoxygenAnalyzer;

@BackendDescription(backendName = "man", backendDescription = "Generate manpage documentation for the classes")
public class Launcher implements Backend{

   public void execute(ProgramDatabase program, PropertyPage properties) throws IOException, MissingInformationException {
		
       outputFile = properties.getString("outfile");
		
       autocollected = properties.getBoolean("auto");
		
       collected = properties.getStringArray("collected");
       
       
       CodeGenerator codegen =
           new CodeGenerator(program, outputFile);
       
       
       if(autocollected) {
           codegen.autocollect();
		}
       // Collect targets
       for (int i = 0; i < collected.length; ++i) {
          codegen.collect(collected[i]);
		}

       codegen.generateClassesDocumentation();
		
	}
   
   @PropertyDescription(propertyName = "outfile",
                        propertyDescription = "Output directory name",
                        numberOfArguments = 1,
                        required = false,
                        defaultValue = "./robin_man/")
   private String outputFile;
   
   @PropertyDescription(propertyName = "auto",
            propertyDescription = "Enable auto collection",
            numberOfArguments = 0,
            required = false,
            defaultValue = "false")
   private boolean autocollected;
   
   @PropertyDescription(propertyName = "collected",
            propertyDescription = "list of class names to collect documentation from",
            numberOfArguments = PropertyData.ANY_NUMBER_OF_ARGUMENTS,
            required = false,
            defaultValue = "")
   private String[] collected;
}
