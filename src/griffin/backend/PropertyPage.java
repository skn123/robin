package backend;

import java.util.HashMap;
import java.util.Map;


/**
 * Collection of properties
 * @author Alex Shapira
 *
 */
public class PropertyPage {
   
   public PropertyPage() {
       m_properties = new HashMap();
   }
   

   /**
    * Adds a property
    * @param name property name
    * @param value property value
    */
   public void addProperty(String name, Object value) {
       m_properties.put(name, value);
   }
   
   
   /**
    * Gets a property as boolean value
    * @param name property
    * @return boolean value
    */
   public boolean getBoolean(String name) {
       return Boolean.parseBoolean((String)(m_properties.get(name)));
   }
   
   /**
    * Gets a property as integer value
    * @param name property
    * @return integer value
    */
   public int getInteger(String name) {
       return Integer.parseInt((String)(m_properties.get(name)));
   }
   
   /**
    * Gets a property as double value
    * @param name property
    * @return double value
    */
   public double getDouble(String name) {
       return Double.parseDouble((String)(m_properties.get(name)));
   }
   
   /**
    * Gets a property as string value
    * @param name property
    * @return string value
    */
   public String getString(String name) {
       return (String)(m_properties.get(name));
   }
   
   /**
    * Gets a property as String array
    * @param name property
    * @return String array value
    */
   public String[] getStringArray(String name) {
       return (String[])(m_properties.get(name));
   }
   
   /**
    * Returns true if a property exists
    * @param name property name
    * @return true if a property with the specified name exists
    */
   public boolean hasProperty(String name) {
       return m_properties.containsKey(name);
   }
   
   private Map m_properties;
}
