/**
 * 
 */
package backend.exceptions.configuration;

/**
 * Exception thrown when the backend name specified 
 * in the configuration does not implement the Backend interface
 * @author Alex Shapira
 *
 */
public class InvalidBackendException extends ConfigurationException {

   

   /**
    * 
    */
   public InvalidBackendException() {
       // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    */
   public InvalidBackendException(String message) {
       super(message);
       // TODO Auto-generated constructor stub
   }

   /**
    * @param cause
    */
   public InvalidBackendException(Throwable cause) {
       super(cause);
       // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    * @param cause
    */
   public InvalidBackendException(String message, Throwable cause) {
       super(message, cause);
       // TODO Auto-generated constructor stub
   }
   
   /**
    * 
    */
   private static final long serialVersionUID = 8211953449851133143L;
   

}
