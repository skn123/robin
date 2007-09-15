/**
 * 
 */
package unittest.backend;

import unittest.backend.configuration.TestBackendData;
import unittest.backend.configuration.TestPackageFinder;
import junit.framework.Test;
import junit.framework.TestSuite;

/**
 * Tests the backend launcher
 * @author Alex Shapira
 *
 */
public class BackendTestSuite{

   
   public static Test suite() {
       TestSuite suite = new TestSuite("Backend launcher test");
       suite.addTestSuite(TestPackageFinder.class);
       suite.addTestSuite(TestBackendData.class);
       suite.addTestSuite(TestLauncher.class);
       return suite;
   }
   
}