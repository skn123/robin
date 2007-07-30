package backend.configuration;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import backend.exceptions.configuration.BackendNotFoundException;

/**
 * Stores the data contained in the backend configuration
 * @author Alex Shapira
 *
 */
/* package-private */ 
class Storage {
   
   
   public Storage() {
       storageMap = new HashMap<String, BackendData>();
   }
   
   /**
    * Add a new backend
    * @param data backend description
    */
   public void addBackend(BackendData data) {
       storageMap.put(data.getBackendName(), data);
   }
   
   
   /**
    * Gets backend data with the specified name
    * @param backendName backend name
    * @return data for the requested backend
    * @throws BackendNotFoundException Requested backend does not exist
    */
   public BackendData getBackend(String backendName) throws BackendNotFoundException {
       if(storageMap.containsKey(backendName) == false) {
           throw new BackendNotFoundException("Backend " + backendName + " does not exist");
       }
       
       return storageMap.get(backendName);
   }
   
   /**
    * @return all available backends
    */
   public Collection<BackendData> getBackends() {
       return storageMap.values();
   }
   
   
   private Map<String, BackendData> storageMap;
   
}