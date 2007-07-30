/**
 * 
 */
package backend.exceptions.configuration;

/**
 * Thrown when there's an exception parsing the configuration file
 * @author Alex Shapira
 *
 */
public class ConfigurationParseException extends ConfigurationException {

   

   /**
    * 
    */
   public ConfigurationParseException() {
       
   }

   /**
    * @param message
    */
   public ConfigurationParseException(String message) {
       super(message);
       
   }

   /**
    * @param cause
    */
   public ConfigurationParseException(Throwable cause) {
       super(cause);
       
   }

   /**
    * @param message
    * @param cause
    */
   public ConfigurationParseException(String message, Throwable cause) {
       super(message, cause);
       
   }
   
   /**
    * 
    */
   private static final long serialVersionUID = -664598748854119582L;

}
