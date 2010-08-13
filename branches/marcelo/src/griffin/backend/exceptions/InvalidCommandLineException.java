/**
 * 
 */
package backend.exceptions;

/**
 * Thrown by launcher when command line is invalid
 * @author Alex Shapira
 *
 */
public class InvalidCommandLineException extends Exception {

   

   /**
    * 
    */
   public InvalidCommandLineException() {
       // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    */
   public InvalidCommandLineException(String message) {
       super(message);
       // TODO Auto-generated constructor stub
   }

   /**
    * @param cause
    */
   public InvalidCommandLineException(Throwable cause) {
       super(cause);
       // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    * @param cause
    */
   public InvalidCommandLineException(String message, Throwable cause) {
       super(message, cause);
       // TODO Auto-generated constructor stub
   }
   
   /**
    * 
    */
   private static final long serialVersionUID = -8311699014866719917L;

}
