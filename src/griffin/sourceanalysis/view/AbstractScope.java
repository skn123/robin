package sourceanalysis.view;

import java.util.Map;

import sourceanalysis.ConstCollection;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;

/**
 * enclosing_type - undocumented.
 */
public interface AbstractScope {

	sourceanalysis.Entity getMember(String key)  throws ElementNotFoundException;
	void declareMember(String key, sourceanalysis.Entity element, boolean asThis);
	AbstractScope cloneScope();
	
	ConstCollection<Map.Entry<String, Entity>> getDecls();
}
