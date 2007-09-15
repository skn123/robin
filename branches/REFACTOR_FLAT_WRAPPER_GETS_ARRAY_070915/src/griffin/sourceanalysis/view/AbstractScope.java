package sourceanalysis.view;

import sourceanalysis.ElementNotFoundException;

/**
 * enclosing_type - undocumented.
 */
public interface AbstractScope {

	sourceanalysis.Entity getMember(String key)  throws ElementNotFoundException;
	void declareMember(String key, sourceanalysis.Entity element, boolean asThis);
	AbstractScope cloneScope();
	
	java.util.Iterator declIterator();
}
