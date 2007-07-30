package unittest.backend.testlauncher;

import unittest.backend.configuration.testbackendconfiguration.BackendConfigurationTest;
import backend.Launcher;
import backend.PropertyPage;
import backend.configuration.BackendConfiguration;
import backend.exceptions.BackendException;
import backend.exceptions.InvalidCommandLineException;
import backend.exceptions.configuration.BackendNotFoundException;
import backend.exceptions.configuration.ConfigurationParseException;

public class LauncherTest extends Launcher {

   public LauncherTest() throws ConfigurationParseException, BackendException {
       super(new BackendConfigurationTest());
   }
   
   public PropertyPage parseCommandLine(String[] args) throws BackendNotFoundException, InvalidCommandLineException {
       return parseCommandLine(configurationData.getBackend("Test"), args);
   }
   
}
