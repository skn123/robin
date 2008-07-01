package backend.annotations;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Basic backend description
 * 
 * Usage example: 
 * 
 * <code>
 * // in the backend.foo package 
 * @BackendDescription(backendDescription = "blah")
 * class Launcher implements Backend {
 * ...
 * }
 * </code>
 * 
 * @author Alex Shapira
 *
 */
@Documented
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface BackendDescription {
   /**
    * @return Human-readable backend name
    */
   public String backendName();
   
   /**
    * @return Human-readable backend description
    */
   public String backendDescription();
}
