package sourceanalysis;

/**
 * A group of members in a class or namespace.
 */
public class Group extends Entity {

	/**
	 * A Scope which sets the "GroupedIn" attribute of elements inserted
	 * into it rather than "ContainedIn".
	 */
	private class GroupScope extends Scope
	{
		/**
		 * Convenience constructor - creates a GroupScope and immediately
		 * associates it with the owner Group.
		 * @param owner owning group
		 */
		GroupScope(Group owner) {
            super(owner);
            associate(owner);
        }
		
		/**
		 * Binds the GroupScope with the Group that contains it.
		 * This is needed in order for the scope to know which group to
		 * relate the members with.
		 * @param owner owning group
		 */
		public void associate(Group owner)
		{
			super.associate(owner);
			m_owner = owner;
		}
		
		/**
		 * Calls contained.connectToGroup() instead of
		 * contained.connectToContainer().
		 * @param contained the member being inserted
		 * @param connection the connection which was added to the scope
		 *  by sourceanalysis.Scope#addMember().
		 * @see sourceanalysis.Scope#mirrorRelationToMember(Entity, ContainedConnection)
		 * @see sourceanalysis.Entity#connectToGroup()
		 */
		protected void mirrorRelationToMember(Entity contained,
			ContainedConnection connection) {
			// Connect to group
			contained.connectToGroup(m_owner);
		}

		// Private members
		Group m_owner;
	}

	/**
	 * Constructor for Group.
	 * <ul>
	 *  <li>Initializes group with an empty scope (no methods and fields).</li>
	 * </ul>
	 */
	public Group() {
		super();
		m_scope = new GroupScope(this);
	}
	
	/**
	 * @name Scoping API
	 * Methods for accessing the contents of the Group.
	 */
	/*@{*/
	
	/**
	 * Retrieves the Scope which stores members of this group.
	 * @return Scope a scope (actually a Group.GroupScope instance)
	 * @see sourceanalysis.Entity#getScope()
	 */
	public Scope getScope()
	{
		return m_scope;
	}

	// Private members
	Scope m_scope;
}
