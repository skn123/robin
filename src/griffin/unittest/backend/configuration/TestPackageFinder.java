package unittest.backend.configuration;

import backend.configuration.PackageFinder;
import backend.exceptions.configuration.ConfigurationParseException;
import junit.framework.TestCase;
/**
 * Tests package finder
 * **NOTE**: Assumes that backend.configuration.testfinder is available
 * @author Alex Shapira
 *
 */
public class TestPackageFinder extends TestCase {

   protected void setUp() throws Exception {
       super.setUp();
       pf = new PackageFinder("unittest.backend.configuration.testfinder");
   }

   protected void tearDown() throws Exception {
       super.tearDown();
   }

   /**
    * Checks that getPackages can find at least .configurations
    * Since getPackages does not assume anything about package structure,
    * it should be enough to show that it found everything else too
    * 
    * Also verify that parent package is not returned
    * @throws ConfigurationParseException
    */
   public void testGetPackages() throws ConfigurationParseException {
       String packages[] = pf.getPackages();
       
       assertEquals("There is only one package under unittest.backend.configuration.testfinder, but found more", 1, packages.length);
           
       
       assertEquals("Didn't find examplebackend", "unittest.backend.configuration.testfinder.test", packages[0]);
       
   }

   /**
    * Checks that PackageFinder can find itself
    * Passes if no exception is thrown
    * @throws ConfigurationParseException
    */
   public void testGetOwnPath() throws ConfigurationParseException {
       pf.getOwnPath();
   }
   
   private PackageFinder pf;
   
   

}
