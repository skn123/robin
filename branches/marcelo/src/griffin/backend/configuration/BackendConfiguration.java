/**
 * 
 */
package backend.configuration;

import java.util.Collection;

import backend.exceptions.configuration.BackendNotFoundException;
import backend.exceptions.configuration.ConfigurationParseException;
import backend.exceptions.configuration.InvalidBackendException;

/**
 * Interface to allow access to the Griffin backend configuration
 * 
 * Provides the user with an ability to query for specific backend properties
 * and metadata about them
 * @author Alex Shapira
 *
 */
public class BackendConfiguration {
   
   
   /**
    * Reads the configuration from java class files
    * @throws ConfigurationParseException 
    *
    */
   public BackendConfiguration() throws ConfigurationParseException {
       configurationStorage = new Storage();
       
       enumerateBackends();
   }
   
   
   /**
    * Returns a specific backend
    * @param backendName backend name
    * @return backend data for the requested backend
    * @throws BackendNotFoundException backend does not exist
    */
   public BackendData getBackend(String backendName) throws BackendNotFoundException {
       return configurationStorage.getBackend(backendName);
   }
   
   /**
    * @return all available backends
    */
   public Collection<BackendData> getBackends() {
       return configurationStorage.getBackends();
   }
   
   /**
    * Enumerate all available backends
    * - backends are all assumed to be in the package &lt;parent&gt;.&lt;backendname&gt;
    * - the filesystem is traversed and information is collected from each subpackage
    * @throws ConfigurationParseException 
    *
    */
   private void enumerateBackends() throws ConfigurationParseException {
       Package thisPackage = this.getClass().getPackage();
       
       
       String thisPackageName = thisPackage.getName();
       
       // common prefix
       // i.e "backend"
       String parentPackagePrefix = thisPackageName.substring(0, thisPackageName.lastIndexOf('.'));
       
       PackageFinder pFinder = new PackageFinder(parentPackagePrefix);
       String[] allPackages = pFinder.getPackages();
       
       // go over all known packages and scan only those with the required prefix
       for(String s : allPackages) {
           processPackage(s);

       }
       
       
   }

   /**
    * Scans a package and determines if it contains a backend implementation
    * - checks if p.MAIN_BACKEND_CLASS_NAME exist and implements Backend
    * @param s package name to scan for existence of backend
    */
   private void processPackage(String s) {
       
       // try loading Launcher class
       try {
           Class backendClass = Class.forName(s + "." + BackendConfiguration.MAIN_BACKEND_CLASS_NAME);
           
           BackendData backendData = new BackendData(backendClass); 
           
           configurationStorage.addBackend(backendData);
           
           
       } catch (ClassNotFoundException e) {
           // not a backend
           return;
       } catch (InvalidBackendException e) {
           // TODO: log warning?
           // not a backend
           return;
       }
   }

   

   /**
    * Storage for the data about backends
    */
   private Storage configurationStorage;
   
   
   private final static String MAIN_BACKEND_CLASS_NAME = "Launcher";
   
}
