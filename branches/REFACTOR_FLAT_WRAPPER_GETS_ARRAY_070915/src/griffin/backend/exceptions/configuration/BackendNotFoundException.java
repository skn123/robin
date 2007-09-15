package backend.exceptions.configuration;


/**
 * Thrown when a backend is not found
 * @author Alex Shapira
 *
 */
public class BackendNotFoundException extends ConfigurationException {


   

   /**
    * 
    */
   public BackendNotFoundException() {
       // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    */
   public BackendNotFoundException(String message) {
       super(message);
       // TODO Auto-generated constructor stub
   }

   /**
    * @param cause
    */
   public BackendNotFoundException(Throwable cause) {
       super(cause);
       // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    * @param cause
    */
   public BackendNotFoundException(String message, Throwable cause) {
       super(message, cause);
       // TODO Auto-generated constructor stub
   }
   
   /**
    * 
    */
   private static final long serialVersionUID = -3387586937659336931L;
   
   
}
