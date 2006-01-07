package sourceanalysis;

/**
 * enclosing_type - undocumented.
 */
public class Namespace extends Entity {

	/**
	 * Constructor for Namespace.
	 */
	public Namespace() {
		super();
		m_nsScope = new Scope();
		m_nsScope.associate(this);
	}
	
	/** @name Scoping
	 * Provides access to members of the Namespace via Scope.
	 */
	/*@{*/
	
	/**
	 * Provides the Scope which contains references to the Namespace's
	 * elements.
	 * @return Scope a Scope object which belongs to the Namespace. Use
	 * Push/Pull API of Scope to fill or retrieve members.
	 */
	public Scope getScope()
	{
		return m_nsScope;
	}

	/*@}*/

	/* Scope */
	Scope m_nsScope;
}
