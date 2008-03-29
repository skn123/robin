package backend.configuration;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.Enumeration;
import java.util.LinkedList;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;


import backend.exceptions.configuration.ConfigurationParseException;

/**
 * Finds all available backend packages
 * @author Alex Shapira
 *
 */
public class PackageFinder {
   
   

   /**
    * Creates a package finder with the specific package prefix
    * The package finder will return all packages that start with this prefix
    * @param prefix package prefix of form "aaa.bbb"
    */
   public PackageFinder(String prefix) {
       // convert foo.bar.basepackage to foo/bar/basepackage/
       this.prefix = prefix;
   }
   
   private String jarEntryPrefix()
   {
	   return prefix.replace(".", "/") + "/";
   }
   
   private String classFilePrefix()
   {
	   return prefix.replace(".", SEPARATOR) + SEPARATOR;
   }
   
   /**
    * Gets all available packages with name prefix.*
    * @return array of strings, each string is the name of an available package
    * @throws ConfigurationParseException 
    */
   public String[] getPackages() throws ConfigurationParseException {
       
       String ownPath = getOwnPath();
       
       
       // try to process as jar file
       try {
           JarFile jf = new JarFile(ownPath);
           List<String> packages = new LinkedList<String>();
           
           for(Enumeration<JarEntry> e = jf.entries(); e.hasMoreElements();) {
               JarEntry jfEntry = e.nextElement();
               // if entry starts with prefix, is not the prefix itself and is a directory
               // then it's a package we want
               String name = jfEntry.getName();
               if(name.startsWith(jarEntryPrefix()) && 
                  !name.equals(jarEntryPrefix()) &&
                  jfEntry.isDirectory())
               {
                   // convert foo/bar/baz/ to foo.bar.baz
                   // and add
                   packages.add(name.replace("/", ".").substring(0, name.length()-1));
               }
           }
           return packages.toArray(new String[0]);

       } catch (IOException e) {
           // open the directory on filesystem
           File prefixDirectory = new File(ownPath + SEPARATOR + classFilePrefix());
           
           String[] subDirectories =  prefixDirectory.list(new FilenameFilter() {
               public boolean accept(File dir, String name) {
                   return (new File(dir, name)).isDirectory();
               }
           });
           
           String[] packages = new String[subDirectories.length];
           for(int i=0; i<packages.length; i++) {
               packages[i] = prefix + "." + subDirectories[i];
           }
           
           return packages;
       }

   }
   
   /**
    * Return the classpath that corresponds to this class
    * First jar/directory that has the matching class is returned
    * @return path that this class was loaded from
    * @throws ConfigurationParseException Class not found in any paths
    */
   public String getOwnPath() throws ConfigurationParseException {
       String[] paths = getSearchPaths();
       
       
       // find the jar/directory with us in it
       for(String path : paths) {
           // try to process as jar file
           try {
               JarFile jf = new JarFile(path);
               if(jf.getJarEntry(JARENTRY) != null) {
                   return path;
               }
           } catch (IOException e) {
               // not a jar file
               if(new File(path + SEPARATOR + CLASSFILE).exists()) {
                   return path;
               }
           }
       }
       
       // impossible?!
       throw new ConfigurationParseException("Couldn't find PackageFinder " +
    		   "class (" + CLASSFILE + ")");
   }
   
   /**
    * Gets the class paths to search in
    * @return array of class path locations
    */
   public String[] getSearchPaths() {
       return System.getProperty("java.class.path").split(System.getProperty("path.separator"));
   }
           
   
   /**
    * Prefix for all found packages
    */
   private String prefix;
   
   
   /**
    * File separator
    */
   private final String SEPARATOR = System.getProperty("file.separator");
   /**
    * Own filename
    */
   private final String CLASSFILE = PackageFinder.class.getCanonicalName().replace(
                                   ".", SEPARATOR) + ".class";
   private final String JARENTRY = PackageFinder.class.getCanonicalName().replace(
                                   ".", "/") + ".class";
}
