package backend.configuration;

import java.lang.reflect.Field;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import backend.Backend;
import backend.annotations.BackendDescription;
import backend.annotations.PropertyDescription;
import backend.exceptions.configuration.InvalidBackendException;

/**
 * Class containing information about a backend
 * - properties data
 * - implementing java class name
 * @author Alex Shapira
 *
 */
public class BackendData {
   
   /**
    * Create the backend description
    * @param implementingClassName name of the class that implements the backend
    * @param name user-readable name
    * @param description user-readable description
    * @param properties backend properties
    * @throws InvalidBackendException 
    */
   public BackendData(Class implementingClass) throws InvalidBackendException 
   {
       this.implementingClassName = implementingClass.getCanonicalName();
       
       this.backendInterface = getBackendClass(implementingClass);
       
       this.propertiesMap = new HashMap<String, PropertyData>();
       
       parseClassAnnotations(implementingClass);
       
   }
   
   
   /**
    * @return the backend name
    */
   public String getBackendName() {
       return backendName;
   }

   /**
    * @return the backend description
    */
   public String getDescription() {
       return description;
   }


   /**
    * @return the backend implementing class name
    */
   public String getImplementingClassName() {
       return implementingClassName;
   }

   /**
    * @return the backend interface class
    */
   public Class<Backend> getBackendInterface() {
       return backendInterface;
   }
   
   /**
    * Returns all data for a particular property
    */
   
   public PropertyData getPropertyData(String property) {
       if(this.propertiesMap.containsKey(property) == false) {
           throw new IllegalArgumentException("Property does not exist");
       }
       return this.propertiesMap.get(property);
   }
   
   /**
    * Returns list of PropertyData, describing all properties
    */
   
   public Collection<PropertyData> getPropertiesData() {
       return this.propertiesMap.values();
   }


   /**
    * Gets the backend class specified by the string
    * @param implementingClass implementing class name string (i.e Griffin.backends.robin.Launcher)
    * @return <code>Class&lt;Backend&gt;</code> object that can be invoked upon
    * @throws InvalidBackendException implementingClass does not implement Backend
    */
   @SuppressWarnings("unchecked")
   private Class<Backend> getBackendClass(Class implementingClass) throws InvalidBackendException {
                   
       Class[] cInterfaces = implementingClass.getInterfaces();
       
       // go over all interfaces and check if Backend is in them
       for(int i=0; i<cInterfaces.length; i++) {
           // if exists, cast as backend, return
           if(cInterfaces[i] == Backend.class) {
               return implementingClass.asSubclass(Backend.class);
           }
       }
       // we did not implement Backend
       throw new InvalidBackendException("Class " + implementingClass + " does not implement Backend");

   }
   
   /**
    * Parses the annotations in the implementing class
    * - expects the implementing class to have 1 BackendDescription and
    *   0 or more PropertyDescription 
    * 
    * @param implementingClass class that implements a backend
    * @throws InvalidBackendException Backend does not have a BackendDescription
    */
   private void parseClassAnnotations(Class implementingClass) throws InvalidBackendException {
       // load class
       try {
           Class.forName(implementingClass.getCanonicalName());
       } catch (ClassNotFoundException e) {
           throw new InvalidBackendException("Class not found, should never happen", e);
       }
       
       // for type safety, use the generic Class<Backend> subtype
       BackendDescription description = this.getBackendInterface().getAnnotation(BackendDescription.class);
       
       if(description == null) {
           throw new InvalidBackendException("Backend implements Launcher, but has no BackendDescription");
       }
       
       this.backendName = description.backendName();
       this.description = description.backendDescription();
       
       Field[] backendFields = implementingClass.getDeclaredFields();
       
       // place the default input property
       propertiesMap.put(BackendData.DEFAULT_INPUT_PROPERTY.getPropertyName(), BackendData.DEFAULT_INPUT_PROPERTY);
       // place the default mixins property
       propertiesMap.put(BackendData.DEFAULT_MIXINS_PROPERTY.getPropertyName(), BackendData.DEFAULT_MIXINS_PROPERTY);
       
       
       // scan all class fields for fields that implement property annotation
       for(Field f : backendFields) {
           PropertyDescription propertyDescription = f.getAnnotation(PropertyDescription.class);
           if(propertyDescription == null) {
               // not a property
               continue;
           }
           
           // extract data from a property
           PropertyData pd = extractPropertyData(propertyDescription);
           propertiesMap.put(pd.getPropertyName(), pd);
       }
       
       
       
   }
   
   /**
    * Exctracts property data from property description
    * @param propertyDescription property description annotation
    * @return PropertyData of the described property
    * @throws InvalidBackendException 
    */
   private PropertyData extractPropertyData(PropertyDescription propertyDescription) throws InvalidBackendException {
       try {
           return new PropertyData(propertyDescription.propertyName(),
                                   propertyDescription.propertyDescription(),
                                   propertyDescription.numberOfArguments(),
                                   propertyDescription.required(),
                                   propertyDescription.defaultValue());
       } catch (InvalidBackendException e) {
           throw new InvalidBackendException("In backend " + backendName + ": " + e.getMessage(), e);
       }
   }
   
   
   /**
    * Gets the default input property
    * @return default input property
    */
   private static PropertyData getDefaultInputProperty() {
       PropertyData pd = null;
       try {
           pd = new PropertyData("input",          // name
                           "Input source",         // description
                           1,                  // # of arguments
                           true,               // required
                           new String[] { "" });
       } catch (InvalidBackendException e) {
           // can't happen
       }
       
       return pd;

   }
   
   /**
    * Gets the default mixins property
    * @return default mixins property
    */
   
   private static PropertyData getDefaultMixinsProperty() {
       PropertyData pd = null;
       try {
           pd = new PropertyData("hints",                          // name
                           "List of Jython hints",                     // description
                           PropertyData.ANY_NUMBER_OF_ARGUMENTS,   // # of arguments
                           false,                                  // required
                           new String[] {  });
       } catch (InvalidBackendException e) {
           // can't happen
       }
       
       return pd;
   }

   /**
    * Class name of the implementing backend
    * (i.e Griffin.backend.robin.Launcher)
    */
   private String implementingClassName;
   
   /**
    * Backend interface
    */
   private Class<Backend> backendInterface;
   
   
   /**
    * User-readable backend name
    */
   private String backendName;
   /**
    * Description of the backend
    */
   private String description;
   
   /**
    * Map between property name and its data
    */
   private Map<String, PropertyData> propertiesMap;
   
   
   /**
    * Default input property
    * Can be overridden by a particular backend
    */
   
   public static final PropertyData DEFAULT_INPUT_PROPERTY = BackendData.getDefaultInputProperty();
   
   /**
    * Default mixins (hints) property
    * Can be overriden by a particular backend
    */
   
   public static final PropertyData DEFAULT_MIXINS_PROPERTY = BackendData.getDefaultMixinsProperty();
   
   
       
   

}
