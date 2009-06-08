/**
 * 
 */
package unittest.backend.configuration;

import junit.framework.TestCase;
import backend.configuration.BackendData;
import backend.configuration.PropertyData;

/**
 * Tests that backend data is parsed correctly
 * @author Alex Shapira
 *
 */
public class TestBackendData extends TestCase {

   /* (non-Javadoc)
    * @see junit.framework.TestCase#setUp()
    */
   protected void setUp() throws Exception {
       super.setUp();
       bd = new BackendData(unittest.backend.configuration.examplebackend.Launcher.class);
   }

   /* (non-Javadoc)
    * @see junit.framework.TestCase#tearDown()
    */
   protected void tearDown() throws Exception {
       super.tearDown();
   }

   /**
    * Tests that backend name is parsed correctly from the annotation
    * Test method for {@link backend.configuration.BackendData#getBackendName()}.
    */
   public void testGetBackendName() {
       assertEquals("Backend name not parsed correctly", "Test", bd.getBackendName());
   }

   /**
    * Tests that description is passed correctly
    * Test method for {@link backend.configuration.BackendData#getDescription()}.
    */
   public void testGetDescription() {
       assertEquals("Backend name not parsed correctly", "Test backend", bd.getDescription());
   }

   /**
    * Tests that the implementing class is really the class we ran data gather on
    * Test method for {@link backend.configuration.BackendData#getImplementingClassName()}.
    */
   public void testGetImplementingClassName() {
       assertEquals("Different class name",
               unittest.backend.configuration.examplebackend.Launcher.class.getCanonicalName(), 
               bd.getImplementingClassName());
   }

   
   /**
    * Tests that properties are parsed
    * Test method for {@link backend.configuration.BackendData#getPropertyData(java.lang.String)}.
    */
   public void testGetPropertyData() {
       
       PropertyData fooData = bd.getPropertyData("fooProperty");
       
       assertEquals("fooProperty", fooData.getPropertyName());
       
   }
   
   /**
    * Tests that default properties are parsed
    *
    */
   public void testGetDefaultPropertyData() {
       
       PropertyData inputData = bd.getPropertyData(BackendData.DEFAULT_INPUT_PROPERTY.getPropertyName());
       
       assertEquals(1, inputData.getNumArguments());
       
       bd.getPropertyData(BackendData.DEFAULT_MIXINS_PROPERTY.getPropertyName());
       
   }

   /**
    * Tests that all properties are parsed
    * Test method for {@link backend.configuration.BackendData#getPropertiesData()}.
    */
   public void testGetPropertiesData() {
       bd.getPropertiesData();
       assertEquals("Should have 4 properties", 4, bd.getPropertiesData().size());
   }
   
   
   private BackendData bd;

}
