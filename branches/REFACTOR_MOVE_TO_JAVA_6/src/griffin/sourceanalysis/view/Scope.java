package sourceanalysis.view;

import java.util.HashMap;
import java.util.Map;

import sourceanalysis.ConstCollection;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;

/**
 * enclosing_type - undocumented.
 */
public class Scope implements AbstractScope {

	/**
	 * Constructor for Scope.
	 */
	public Scope() {
		super();
		m_members = new HashMap<String, Entity>();
	}

	public Scope(Map<String, Entity> map_of_members)
	{
		super();
		m_members = new HashMap<String, Entity>();
		m_members.putAll(map_of_members);
	}

	/**
	 * Finds an entity in the current scope. The entity is accessed via its keyname (such as "CLASS" or
	 * "METHOD").
	 * @throws ElementNotFoundException - if an item with the given key is not found.
	 * @see sourceanalysis.view.AbstractScope#getMember(String)
	 */
	public Entity getMember(String key) throws ElementNotFoundException
	{
		Object o = m_members.get(key);
		// Raise an exception if the object was not found.
		if (o == null || !(o instanceof Entity)) {
			throw new ElementNotFoundException();
		}
		else {
			return (Entity)o;
		}
	}
	
	public void declareMember(String key, Entity entity, boolean asThis)
	{
		m_members.put(key, entity);
		if (asThis) {
			m_members.put("THIS", entity);
		}
	}
	
	/**
	 * @see sourceanalysis.view.AbstractScope#cloneScope()
	 */
	public AbstractScope cloneScope() {
		return new Scope(m_members);
	}
	
	/**
	 * Returns an iterator over the declared members. The iterator returns a Map.Entry which
	 * has getKey() and getValue()  (both returning Object).
	 * @see sourceanalysis.view.AbstractScope#declIterator()
	 */
	public ConstCollection<Map.Entry<String, Entity>> getDecls() {
		return new ConstCollection<Map.Entry<String, Entity>>(m_members.entrySet());
	}

	// Implementation using a map
	private Map<String, Entity> m_members;
}
