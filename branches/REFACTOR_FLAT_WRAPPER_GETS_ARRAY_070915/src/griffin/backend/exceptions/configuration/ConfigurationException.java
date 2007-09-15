/**
 * 
 */
package backend.exceptions.configuration;

/**
 * General configuration exception, abstract to prevent direct creation
 * @author Alex Shapira
 *
 */
public abstract class ConfigurationException extends Exception {

   /**
    * 
    */
   public ConfigurationException() {
   }

   /**
    * @param message
    */
   public ConfigurationException(String message) {
       super(message);
   }

   /**
    * @param cause
    */
   public ConfigurationException(Throwable cause) {
       super(cause);
   }

   /**
    * @param message
    * @param cause
    */
   public ConfigurationException(String message, Throwable cause) {
       super(message, cause);
   }

}
