/**
 * 
 */
package backend.exceptions;

/**
 * Thrown when a backend had an exception
 * @author Alex Shapira
 *
 */
public class BackendException extends Exception {

   

   /**
    * 
    */
   public BackendException() {
       // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    */
   public BackendException(String message) {
       super(message);
       // TODO Auto-generated constructor stub
   }

   /**
    * @param cause
    */
   public BackendException(Throwable cause) {
       super(cause);
       // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    * @param cause
    */
   public BackendException(String message, Throwable cause) {
       super(message, cause);
       // TODO Auto-generated constructor stub
   }
   
   /**
    * 
    */
   private static final long serialVersionUID = 3960209057763262021L;

}
