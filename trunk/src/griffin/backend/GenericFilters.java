package backend;

import java.util.Iterator;

import sourceanalysis.Entity;
import sourceanalysis.FriendConnection;
import sourceanalysis.SourceFile;

/**
 * Filters that could be used by any backend
 * @author ask
 *
 */
public class GenericFilters {
	/**
	 * Checks whether an entity is actually declared in a header file.
	 * This is used to distinguish between, for instance, global constants
	 * and internally used global variables.
	 * @param entity
	 * @return <b>true</b> if entity is declared in an .h or .inl file; 
	 * <b>false</b> otherwise.
	 */
	public static boolean isDeclared(Entity entity)
	{
		// Look for source declaration
		boolean sourceDecl = false;
		SourceFile.DeclDefConnection connection = entity.getDeclaration();
		sourceDecl = isAllowedToDeclare(connection);
		// Look for friends
		boolean friendDecl = false;
		for (Iterator affi = entity.affiliatesIterator(); 
				affi.hasNext(); ) {
			FriendConnection fconnection = (FriendConnection)affi.next();
			connection = fconnection.getDeclaring().getDeclaration();
			friendDecl = isAllowedToDeclare(connection);
		}
		
		return sourceDecl || friendDecl;
	}
	
	/**
	 * Checks if the declaration is exposed, that is, occurs in a file visible
	 * to the user (a .h, .hh, or .inl file).
	 * @param decl declaration of the observed entity
	 * @return <b>true</b> if the declaration is located in an exposed file,
	 * <b>false</b> otherwise.
	 */
	private static boolean 
		isAllowedToDeclare(SourceFile.DeclDefConnection decl)
	{
		if (decl != null) {
			String filename = decl.getSourceFilename(); 
			return filename.endsWith(".h") || filename.endsWith(".hh") 
				|| filename.endsWith(".hpp")
				|| filename.endsWith(".inl");
		}
		else
			return false;
	}
	
	/**
	 * Decides whether or not it is legal to <code>#include</code> the file
	 * in which a given declaration occurs. As a thumb rule, only 
	 * <code>.h</code> and <code>.hh</code> files are allowed for inclusion.
	 * @param decl declaration of the observed entity
	 * @return <b>true</b> if the declaration can be included,
	 * <b>false</b> otherwise.
	 */
	public static boolean 
		isAllowedToInclude(SourceFile.DeclDefConnection decl)
	{
		if (decl != null) {
			String filename = decl.getSourceFilename(); 
			return filename.endsWith(".h") || filename.endsWith(".hh")
				|| filename.endsWith(".hpp");
		}
		else
			return false;
	}
}
