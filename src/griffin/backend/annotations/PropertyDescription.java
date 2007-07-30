package backend.annotations;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Description of a property:
 * - name
 * - description
 * - number of expected parameters
 * - is required?
 * - default value
 * 
 * Should be used on fields in a Backend-implementing class:
 * 
 *<code>
 * class Foo implements Backend {
 *     ...
 *  @PropertyDescription("bar", "Bar property", 3, "true", { "a", "b", "c" })
 *  private Boolean barProperty;
 *</code>
 *
 * Property that has more than 0 arguments and a preset number of them
 * and is not required, must have an initializer of the expected # of arguments size
 * 
 * Property with 0 arguments is assumed to be a flag
 * Default value should therefore be "false"
 * @author Alex Shapira
 *
 */

@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface PropertyDescription {

   /**
    * @return property name exposed to the user
    */
   public String propertyName();
   
   /**
    * @return Property description
    */
   public String propertyDescription();
   
   /**
    * @return number of expected arguments
    */
   public int numberOfArguments();
   
   /**
    * @return is the user required to specify it?
    */
   public boolean required();
   
   /**
    * @return Default value
    */
   public String[] defaultValue();
}
