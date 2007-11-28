/**
 * 
 */
package unittest.backend.configuration.examplebackend;

import java.io.IOException;

import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import backend.Backend;
import backend.PropertyPage;
import backend.annotations.BackendDescription;
import backend.annotations.PropertyDescription;
import backend.configuration.PropertyData;

/**
 * Sample backend for unittest
 * @author Alex Shapira
 *
 */
@BackendDescription(backendName = "Test", backendDescription = "Test backend")
public class Launcher implements Backend {

   /* (non-Javadoc)
    * @see backend.Backend#execute(sourceanalysis.ProgramDatabase, backend.PropertyPage)
    */
   public void execute(ProgramDatabase program, PropertyPage properties)
           throws IOException, MissingInformationException {
       // TODO Auto-generated method stub

   }
   
   
   @PropertyDescription(propertyName = "fooProperty",
                        propertyDescription = "Foo property",
                        numberOfArguments = 1,
                        required = false,
                        defaultValue = "foo")
   private boolean foo;
   
   
   @PropertyDescription(propertyName = "barProperty",
            propertyDescription = "Bar property",
            numberOfArguments = PropertyData.ANY_NUMBER_OF_ARGUMENTS,
            required = true,
            defaultValue = "")
   private boolean bar;

   // to prevent "'...' is never read locally" warning
   void silenceWarnings() { if (foo); if (bar); }
}
