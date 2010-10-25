package sourceanalysis;

/**
 * A group of members in a class or namespace.
 */
public class Group extends Entity {

	/**
	 * A Scope which sets the "GroupedIn" attribute of elements inserted
	 * into it rather than "ContainedIn".
	 */
	private class GroupScope extends Scope<Group>
	{
		/**
		 * Convenience constructor - creates a GroupScope and immediately
		 * associates it with the owner Group.
         * @param owner owning group, needed in order for the scope to know
         * which group to relate the members with
		 */
		GroupScope(Group owner) {
            super(owner);
            m_groupScopeOwner = owner;
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
		@Override
		protected void mirrorRelationToMember(Entity contained,
			ContainedConnection<Group, ? extends Entity> connection) {
			// Connect to group
			contained.connectToGroup(m_groupScopeOwner);
		}

		// Private members
		Group m_groupScopeOwner;
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
	
	@Override
	protected void connectToGroup(Group containingGroup) {
		super.connectToGroup(containingGroup);
		if(getName()!= null) {
			fullGroupHierarchyName = getGroup().getFullGroupHierarchyName() + "." + getName();
		}
		
	}
	
	
	@Override
	public void setName(String name) {
		super.setName(name);
		if(getGroup() != null) {
			fullGroupHierarchyName = getGroup().getFullGroupHierarchyName() + "." + name;
		} else {
			fullGroupHierarchyName = name;
		}
	}
	
	public String getFullGroupHierarchyName() {
		return fullGroupHierarchyName;
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
	public Scope<Group> getScope()
	{
		return m_scope;
	}


	/** Group full hierarchy name, it distinguishes subgroups with the same name but different
	 *  father groups (it does not distinguishes groups that belong to different Agreggates).
	 */
	String fullGroupHierarchyName;
	
	// Private members
	Scope<Group> m_scope;


}
