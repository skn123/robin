/**
 * 
 */
package backend;

import java.io.IOException;

import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;

/**
 * Implements a common backend interface for all backends
 * Every backend is required to implement it in order for the
 * main launcher to recognize it
 * 
 * Note:
 * - the backends should be placed in subdirectories of backend.
 * - the backends should have a Launcher class, implementing this interface
 * - the backend configuration data will be read from it, using annotations
 * @author Alex Shapira
 *
 */
public interface Backend {
   
   /**
    * Main back-end entry point.
    * 
    * @param program a program database to operate upon
    * @param properties a set of attributes describing the actions
    *  to be taken. The properties set should contain all properties marked as "required"
    *  in the specific backend annotations.
    * @throws IOException if an output exception occurs
    * @throws MissingInformationException if the program database is 
    * incomplete
    */
   public abstract void execute(ProgramDatabase program, PropertyPage properties)
       throws IOException, MissingInformationException; 
   
}
