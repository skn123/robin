package backend.configuration;

import backend.exceptions.configuration.InvalidBackendException;

/**
 * Configuration data about a property
 * - name, description, expected number of arguments, required flag, default value
 * @author Alex Shapira
 *
 */
public class PropertyData {
   
   

   /**
    * Creates a new description data about a property
    * @param propertyName property name
    * @param propertyDescription property description
    * @param numArguments number of arguments (set to ANY_NUMBER_OF_ARGUMENTS for unknown)
    * @param required true if the property is required by the backend
    * @param defaultValue default value
    */
   public PropertyData(String propertyName,
                      String propertyDescription,
                      int numArguments,
                      boolean required,
                      String[] defaultValue) throws InvalidBackendException
   {
       
       if(required &&
          this.numArguments != 0 && 
          this.numArguments != ANY_NUMBER_OF_ARGUMENTS && 
          this.numArguments != defaultValue.length) 
       {
           throw new InvalidBackendException("Property " + propertyName + 
                                             " default value length does not match expected number of parameters");    
       }
           
       this.propertyName = propertyName;
       this.propertyDescription = propertyDescription;
       this.numArguments = numArguments;
       this.required = required;
       this.defaultValue = defaultValue;
   }
   
   public String[] getDefaultValue() {
       return defaultValue;
   }

   public int getNumArguments() {
       return numArguments;
   }

   public String getPropertyDescription() {
       return propertyDescription;
   }

   public String getPropertyName() {
       return propertyName;
   }

   public boolean isRequired() {
       return required;
   }
   
   
   /**
    * Property name
    */
   private String propertyName;
   
   /**
    * User-friendly property description
    */
   private String propertyDescription;
   
   /**
    * Number of expected arguments
    */
   private int numArguments;
   
   /**
    * Is the property required?
    */
   private boolean required;
   
   /**
    * Default value
    */
   private String[] defaultValue; 
   
   
   /**
    * Integer constant for infinite number of arguments
    */
   public static final int ANY_NUMBER_OF_ARGUMENTS = -1;
}