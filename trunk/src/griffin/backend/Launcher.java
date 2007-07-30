package backend;

import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;

import sourceanalysis.ElementNotFoundException;
import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.dox.DoxygenAnalyzer;
import sourceanalysis.mixin.JythonMixIn;
import sourceanalysis.mixin.MixIn;

import backend.configuration.BackendConfiguration;
import backend.configuration.BackendData;
import backend.configuration.PropertyData;
import backend.exceptions.BackendException;
import backend.exceptions.InvalidCommandLineException;
import backend.exceptions.configuration.BackendNotFoundException;
import backend.exceptions.configuration.ConfigurationParseException;

/**
 * Main Griffin launcher.
 * Launches the specified backend, after verifying that the passed arguments
 * answer the backend requirements
 * @author Alex Shapira
 *
 */
public class Launcher {
   
   /**
    * Main entry point
    * @param args
    */
   public static void main(String args[]) {
       try {
           (new Launcher(new BackendConfiguration())).launch(args);
       } catch (Exception e) {
           System.err.println("*** ERROR: " + e.getMessage());
           e.printStackTrace();
       }
   }
   
   /**
    * Creates a Launcher()
    * @param bc Backend configuration data
    * @throws ConfigurationParseException 
    */
   public Launcher(BackendConfiguration bc) throws ConfigurationParseException {
       configurationData = bc;
   }
   
   /**
    * Executes the launcher with the provided arguments
    * @param args arguments
    * @throws BackendException Exception occured during execution of a backend
    */
   protected void launch(String[] args) throws BackendException {
       if(args.length == 0) {
           printBasicLaunchHelp();
           System.exit(2);
       }
       
       
       String backendName = args[0];
       
       // First argument can be a utility flag (list all backends, etc)
       if(backendName.startsWith("--")) {
           handleUtilityFlag(backendName, args);
       } else {
           try {
               try {
                   BackendData backendData = configurationData.getBackend(backendName);
                   
                   PropertyPage backendProperties = parseCommandLine(backendData, args);
                   
                   invokeBackend(backendData, backendProperties);
               }  catch (InvalidCommandLineException e) {
                   System.err.println("Not enough arguments for backend " + backendName);
                   printUsageHelp(backendName);
                   
               } 
           } catch (BackendNotFoundException e) {
               System.err.println("Backend " + backendName + " does not exist.\n" +
                          "Use --" + Launcher.LIST_ALL_FLAG + " to show all available backends");
               System.exit(1);
           }
       }
           
   }


   
   /**
    * Parse command line arguments
    * @param backendData backend data that defines the command line format
    * @param args arguments
    * @return generated property page
    * @throws NotEnoughArgumentsException
    * @throws InvalidCommandLineException 
    */
   protected PropertyPage parseCommandLine(BackendData backendData, String[] args) throws InvalidCommandLineException {
       
       Map<String, List<String>> propertiesMap = new HashMap<String, List<String>>();
       
       // parse the command line arguments array into a map
       // mapping each property to its arguments
       
       String currentProperty = null;
       // args[0] = backend name, skip
       for(int i=1; i<args.length; i++) {
           // new property
           if(args[i].startsWith("--")) {
               currentProperty = args[i].substring(2);
               propertiesMap.put(currentProperty, new LinkedList<String>());
           } else if(currentProperty == null) {
               // if we reached this point and currentProperty is null, we have an erroneous command line
               throw new InvalidCommandLineException("Please use '--property0 arg0 arg1 --property1 arg0 ...' format");
           } else {
               propertiesMap.get(currentProperty).add(args[i]);
           }
       }
       
       // validate the map
       

       
       // - check that all properties exist in the backend
       for(String propName : propertiesMap.keySet()) {
           try {
               backendData.getPropertyData(propName);
           } catch(IllegalArgumentException e) {
               throw new InvalidCommandLineException("Invalid property name for backend " + 
                                                      backendData.getBackendName() + 
                                                      " - " +
                                                      propName);
           }
       }
       
       // - check that all required properties are there
       Collection<PropertyData> backendProperties = backendData.getPropertiesData();
       for(PropertyData pd : backendProperties) {
           // verify existence
           if(pd.isRequired() && propertiesMap.containsKey(pd.getPropertyName()) == false) {
               throw new InvalidCommandLineException("Backend " + backendData.getBackendName() +
                                                     " requires property \"" + pd.getPropertyName() + "\"");
           }
           
           // verify number of arguments
           if(propertiesMap.containsKey(pd.getPropertyName())) {
               if(propertiesMap.get(pd.getPropertyName()).size() != pd.getNumArguments() &&
                       pd.getNumArguments() != PropertyData.ANY_NUMBER_OF_ARGUMENTS) 
               {
                   throw new InvalidCommandLineException("Property " + pd.getPropertyName() + 
                                                         " must have " + pd.getNumArguments() + " arguments");
               }
           }
           
       }
       
       // everything verified, fill the property page and invoke the module
       PropertyPage propPage = new PropertyPage();
       for(PropertyData pd : backendProperties) {
           String propertyName = pd.getPropertyName();
           
           
           // required property or non-required, but exists
           if(propertiesMap.containsKey(propertyName)) {
               List<String> propertyValue = null;
               propertyValue = propertiesMap.get(propertyName);
               if(pd.getNumArguments() == 0) {
                   // flag
                   propPage.addProperty(propertyName, "true");
               } else if(pd.getNumArguments() == 1) {
                   propPage.addProperty(propertyName, propertyValue.get(0));
               } else {
                   propPage.addProperty(propertyName, propertyValue.toArray(new String[0]));
               }
               
           } else {
               // add with default value
               String[] propertyValue = pd.getDefaultValue();
               if(pd.getNumArguments() == 0) {
                   // flag
                   propPage.addProperty(propertyName, propertyValue[0]);
               } else if(pd.getNumArguments() == 1) {
                   propPage.addProperty(propertyName, propertyValue[0]);
               } else {
                   propPage.addProperty(propertyName, propertyValue);
               }
           }
           
           // for properties with single argument or no arguments, just add them and the argument
           // otherwise, add an array of arguments
           
           
       }
       
       return propPage;
   }
   
   /**
    * Invokes a backend
    * @param backendData backend data
    * @param backendProperties properties passed to the backend
    * @throws BackendException any exception in backend occurs
    */
   private void invokeBackend(BackendData backendData, PropertyPage backendProperties) throws BackendException  {
       // process the database
       // Process input files and create the program database
       try {
           DoxygenAnalyzer dox = new DoxygenAnalyzer(
                   backendProperties.getString(BackendData.DEFAULT_INPUT_PROPERTY.getPropertyName()));
           
           // collect mixins
           Collection<JythonMixIn> mixins = new LinkedList<JythonMixIn>();
           
           String[] mixinPaths = 
               backendProperties.getStringArray(BackendData.DEFAULT_MIXINS_PROPERTY.getPropertyName());
           
           for(String mixin : mixinPaths) {
               mixins.add(new JythonMixIn(mixin));
           }
           
           
           dox.logger.setLevel(Level.WARNING);
           ProgramDatabase pdb = dox.processIndex();
           
           
           
           // Apply mix-ins
           for (Iterator mixini = mixins.iterator(); mixini.hasNext(); ) {
               ((MixIn)mixini.next()).apply(pdb);
           }
           
           // create an instance of the backend
       

           Backend backend = backendData.getBackendInterface().newInstance();
           backend.execute(pdb, backendProperties);
           return;
       
       } catch (InstantiationException e) {
           throw new BackendException("Could not instantiate backend " + backendData.getBackendName() + " class", e);
       } catch (IllegalAccessException e) {
           throw new BackendException("Could not instantiate backend " + backendData.getBackendName() + " class", e);
       } catch (IOException e) {
           throw new BackendException("I/O error in backend " + backendData.getBackendName(), e);
       } catch (MissingInformationException e) {
           throw new BackendException("Some information was missing for " + backendData.getBackendName(), e);
       } catch (ElementNotFoundException e) {
           throw new BackendException("Failed to read index ", e);
       }
       
   }

   /**
    * Handles a utility flag passed to Griffin
    * @param flag utility flag (help, etc)
    */
   private void handleUtilityFlag(String flag, String args[]) {
       // we support 2 flags -
       // --help prints general usage information
       // --help <backendname> prints usage information for a specific backend
       // --listbackends prints launch help for all backends
       
       if(flag.equals("--" + Launcher.HELP_FLAG)) {
           if(args.length > 1) {
               // --help backend
               try {
                   printUsageHelp(args[1]);
               } catch (BackendNotFoundException e) {
                   System.err.println("Backend " + args[1] + " not found!");
               }
           } else {
               // --help
               printBasicLaunchHelp();
           }
       } else if(flag.equals("--" + Launcher.LIST_ALL_FLAG)) {
           printBackendsUsageHelp();
       }
       
   }

   
   /**
    * Prints basic usage help
    *
    */
   private void printBasicLaunchHelp() {
       System.out.println("Usage:");
       
       System.out.println("griffin --" + Launcher.HELP_FLAG);
       System.out.println("\tshow this help");
       
       System.out.println("griffin --" + Launcher.HELP_FLAG + " <backendname>");
       System.out.println("\tprint help for backend <backendname>");
       
       System.out.println("griffin --" + Launcher.LIST_ALL_FLAG);
       System.out.println("\tshow usage help for all backends");
       
       System.out.println("griffin <backendname> --prop1 arg0 arg1 --prop2 arg0 arg1 ...");
       System.out.println("\tLaunch backend <backendname> with those arguments. See help for specific backend.");
       
   }
   
   /**
    * Prints usage help for all backends
    *
    */
   private void printBackendsUsageHelp() {
       for(BackendData b : configurationData.getBackends()) {
           try {
               printUsageHelp(b.getBackendName());
           } catch (BackendNotFoundException e) {
               // can't happen
           }
       }
       
   }
   
   /**
    * Prints usage help for a specific backend
    * @param backendName backend name
    * @throws BackendNotFoundException backend not found
    */
   private void printUsageHelp(String backendName) throws BackendNotFoundException {
           BackendData bData = configurationData.getBackend(backendName);
           
           System.out.println("----------------------------------------------------------");
           System.out.println("Help for backend \"" + bData.getBackendName() + "\"");
           System.out.println("\n");
           System.out.println("Backend description: " + bData.getDescription());
           System.out.println("Properties:\n");
           
           for(PropertyData pd : bData.getPropertiesData()) {
               System.out.println("\"" + pd.getPropertyName() + "\"");
               
               System.out.println("\tProperty description:");
               System.out.println("\t\t" + pd.getPropertyDescription());
               
               System.out.println("\tNumber of arguments:");
               if(pd.getNumArguments() == PropertyData.ANY_NUMBER_OF_ARGUMENTS) {
                   System.out.println("\t\tany");
               } else {
                   System.out.println("\t\t" + pd.getNumArguments());
               }
               
               System.out.println("\tRequired:");
               System.out.println("\t\t" + pd.isRequired());
               
               if(pd.isRequired() == false) {
                   System.out.println("\tDefault value:");
                   String[] defValue = pd.getDefaultValue();
                   
                   System.out.print("\t\t");
                   for(int i=0; i<defValue.length; i++) {
                       System.out.print("\"" + defValue[i] + "\" ");
                   }
                   System.out.println();
               }
               System.out.println();
       
           }
   
   }


   /**
    * Backend configuration data
    */
   protected BackendConfiguration configurationData;
   
   
   private static final String HELP_FLAG = "help";
   private static final String LIST_ALL_FLAG = "backends";

}
