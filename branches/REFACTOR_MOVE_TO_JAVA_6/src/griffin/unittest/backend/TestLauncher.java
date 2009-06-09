/**
 * 
 */
package unittest.backend;

import junit.framework.TestCase;
import unittest.backend.testlauncher.LauncherTest;
import backend.PropertyPage;
import backend.exceptions.BackendException;
import backend.exceptions.InvalidCommandLineException;
import backend.exceptions.configuration.BackendNotFoundException;
import backend.exceptions.configuration.ConfigurationParseException;

/**
 * @author Alex Shapira
 *
 */
public class TestLauncher extends TestCase {

   /* (non-Javadoc)
    * @see junit.framework.TestCase#setUp()
    */
   protected void setUp() throws Exception {
       super.setUp();
   }

   /* (non-Javadoc)
    * @see junit.framework.TestCase#tearDown()
    */
   protected void tearDown() throws Exception {
       super.tearDown();
   }

   /**
    * Tests that launching a backend with proper number of arguments works
    * Test method for {@link unittest.backend.testlauncher.LauncherTest#LauncherTest()}.
    * @throws BackendException 
    * @throws ConfigurationParseException 
    * @throws InvalidCommandLineException 
    * @throws BackendNotFoundException 
    */
   public void testLauncherTest() throws ConfigurationParseException, BackendException, BackendNotFoundException, InvalidCommandLineException {
       LauncherTest lc = new LauncherTest();
       PropertyPage pp = lc.parseCommandLine("Test --input '.' --fooProperty A --barProperty foo".split(" "));
       
       assertEquals("A", pp.getString("fooProperty"));
       assertEquals(1, pp.getStringArray("barProperty").length);
       assertEquals("foo", pp.getStringArray("barProperty")[0]);
       
   }
   
   /**
    * Tests that launching a backend with some arguments omitted works
    * and omitted arguments are replaced with defaults
    * Test method for {@link unittest.backend.testlauncher.LauncherTest#LauncherTest()}.
    * @throws BackendException 
    * @throws ConfigurationParseException 
    * @throws BackendException 
    * @throws ConfigurationParseException 
    * @throws InvalidCommandLineException 
    * @throws BackendNotFoundException 
    * @throws InvalidCommandLineException 
    * @throws BackendNotFoundException 
    */
   public void testLauncherOmitArgs() throws ConfigurationParseException, BackendException, BackendNotFoundException, InvalidCommandLineException {
       LauncherTest lc = new LauncherTest();
       PropertyPage pp = lc.parseCommandLine("Test --input '.' --barProperty foo".split(" "));
       assertEquals("foo", pp.getString("fooProperty"));
       assertEquals(1, pp.getStringArray("barProperty").length);
       assertEquals("foo", pp.getStringArray("barProperty")[0]);
   }
   
   /**
    * Tests that when not all required arguments are supplied exception is thrown
    * @throws ConfigurationParseException
    * @throws BackendException
    * @throws BackendNotFoundException
    * @throws InvalidCommandLineException
    */
   public void testNotEnoughArgs() throws ConfigurationParseException, BackendException, BackendNotFoundException, InvalidCommandLineException {
       LauncherTest lc = new LauncherTest();
       try {
           lc.parseCommandLine("Test --barProperty foo".split(" "));
       } catch(InvalidCommandLineException e) {
           // good boy
           return;
       }
       
       fail("Not enough arguments, but still parsed");
   }
   
   /**
    * Tests a situation when a property is passed too many arguments
    * @throws ConfigurationParseException
    * @throws BackendException
    * @throws BackendNotFoundException
    */
   public void testTooManyArgs() throws ConfigurationParseException, BackendException, BackendNotFoundException {
       LauncherTest lc = new LauncherTest();
       try {
           lc.parseCommandLine("Test --input '.' --fooProperty foo bar".split(" "));
       } catch(InvalidCommandLineException e) {
           // good boy
           return;
       }
       
       fail("Too many arguments, but still parsed");
   }
   
   /**
    * Tests a situation when launcher is passed unknown arguments
    * @throws ConfigurationParseException
    * @throws BackendException
    * @throws BackendNotFoundException
    * @throws InvalidCommandLineException
    */
   public void testLauncherUnknownArgs() throws ConfigurationParseException, BackendException, BackendNotFoundException, InvalidCommandLineException {
       LauncherTest lc = new LauncherTest();
       try {
           lc.parseCommandLine("Test --input '.' --blah foo --fooProperty A --barProperty foo".split(" "));
       } catch(InvalidCommandLineException e) {
           // good boy
           return;
       }
       fail("Parsed unknown arguments");
   }
   
   /**
    * Tests any number of arguments property
    * @throws ConfigurationParseException
    * @throws BackendException
    * @throws BackendNotFoundException
    * @throws InvalidCommandLineException
    */
   public void testAnyNumberOfArguments() throws ConfigurationParseException, BackendException, BackendNotFoundException, InvalidCommandLineException {
       LauncherTest lc = new LauncherTest();
       lc.parseCommandLine("Test --input '.' --barProperty foo bar".split(" "));   
   }

}
