package sourceanalysis.view;

import sourceanalysis.ElementNotFoundException;

/**
 * enclosing_type - undocumented.
 */
public interface AbstractScope {

	public sourceanalysis.Entity getMember(String key)  throws ElementNotFoundException;
	public void declareMember(String key, sourceanalysis.Entity element, boolean asThis);
	public AbstractScope cloneScope();
	
	public java.util.Iterator declIterator();
}
