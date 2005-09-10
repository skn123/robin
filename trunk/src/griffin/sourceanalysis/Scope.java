package sourceanalysis;

import java.util.Iterator;
import java.util.List;
import java.util.LinkedList;

/**
 * Holds contained entities in a Container. Containers are:
 * <ul>
 *  <li>Aggregate</li>
 *  <li>Namespace</li>
 *  <li>Group</li>
 * </ul>
 * They all have a corresponding getScope() method which returns an
 * instance of this Scope class. Through this instance, serial access to the
 * contents is provided using the ...Iterator() methods. Elements which may
 * be contained are:
 * <ul>
 *  <li>Aggregate - scope.aggregateIterator()</li>
 *  <li>Namespace - scope.namespaceIterator()</li>
 *  <li>Alias - scope.aliasIterator()</li>
 *  <li>Enum - scope.enumIterator()</li>
 *  <li>Routine - scope.routineIterator()</li>
 *  <li>Field - scope.fieldIterator()</li>
 *  <li>Group - scope.groupIterator()</li>
 * </ul>
 */
public class Scope {

	/**
	 * Constructor for Scope.
	 */
	public Scope() {
		super();
		// Prepare empty lists for all slots
		m_routines = new LinkedList();
		m_fields = new LinkedList();
		m_aggregates = new LinkedList();
		m_namespaces = new LinkedList();
		m_groups = new LinkedList();
		m_enums = new LinkedList();
		m_aliases = new LinkedList();
		m_friends = new LinkedList();
	}

	/** @name Push API
	 * Methods for setting information in the Scope
	 */
	/*@{*/
	
	/** Sets scope ownership. The owner is the Entity which holds the Scope
	 * (must be Aggregate or Namespace) and is required for a proper setting
	 * of connections.
	 */
	public void associate(Entity owner)
	{
		m_owner = owner;
	}
	
	/**
	 * Admits a routine as a global function in a namespace or a member function
	 * of a class.
	 * May set static members as well as non-static (use the storage parameter).
	 * The specifier parameters are meaningless if the owner is a namespace,
	 * use Specifiers.DONT_CARE.
	 * @param routine the new member function
	 * @param visibility access rights, taken from Specifiers.Visibility
	 * @param virtuality type of virtual, taken from Specifiers.Virtuality
	 * @param storage storage specifier, taken from Specifiers.Storage
	 */
	public void addMember(Routine routine, int visibility, int virtuality,	int storage)
	{
		// Create a contained-connection
		ContainedConnection connection = 
			new ContainedConnection(m_owner, visibility, virtuality, storage, routine);
		// Add connection to list of connected routines
		m_routines.add(connection);
		// Connect member to the owner of this scope
		mirrorRelationToMember(routine, connection);
	}
	
	/**
	 * Admits a field (data element) into the Scope.
	 * May set static members as well as non-static (use the storage parameter).
	 * The specifier parameters are meaningless if the owner is a namespace,
	 * use Specifiers.DONT_CARE.
	 * @param field the new member variable
	 * @param visibility access rights, taken from Specifiers.Visibility
	 * @param storage storage specifier, taken from Specifiers.Storage
	 */
	public void addMember(Field field, int visibility, int storage)
	{
		// Create a contained-connection
		ContainedConnection connection = 
			new ContainedConnection(m_owner, visibility, Specifiers.DONT_CARE,
			storage, field);
		// Add connection to list of connected routines
		m_fields.add(connection);
		// Connect member to the owner of this scope
		mirrorRelationToMember(field, connection);
	}
	
	/**
	 * Admits an inner construct (class, struct, union) into the Scope.
	 * @param inner the new inner structure
	 * @param visibility access rights, taken from Specifiers.Visibility. 
	 * Meaningless for namespace scopes, use Specifiers.DONT_CARE.
	 */
	public void addMember(Aggregate inner, int visibility)
	{
		// Create a contained-connection
		ContainedConnection connection = 
			new ContainedConnection(m_owner, visibility, Specifiers.DONT_CARE,
			Specifiers.DONT_CARE, inner);
		// Add connection to list of connected aggregates
		m_aggregates.add(connection);
		// Connect member to the owner of this scope
		mirrorRelationToMember(inner, connection);
	}
	
	/**
	 * Admits an enumerated type into the namespace.
	 * @param enume enumerated type entity
	 * @param visibility access rights, taken from Specifiers.Visibility
	 */
	public void addMember(sourceanalysis.Enum enume, int visibility)
	{
		// Create a contained-connection
		ContainedConnection connection = 
			new ContainedConnection(m_owner, visibility, Specifiers.DONT_CARE,
			Specifiers.DONT_CARE, enume);
		// Add connection to list of connected enums
		m_enums.add(connection);
		// Connect member to the owner of this scope
		mirrorRelationToMember(enume, connection);
	}
	
	/**
	 * Admits an alias (typedef) into the scope.
	 * @param alias new alias entity
	 * @param visibility access rights, taken from Specifiers.Visibility.
	 */
	public void addMember(Alias alias, int visibility)
	{
		// Create a contained-connection
		ContainedConnection connection = 
			new ContainedConnection(m_owner, visibility, Specifiers.DONT_CARE,
			Specifiers.DONT_CARE, alias);
		// Add connection to list of connected aliases
		m_aliases.add(connection);
		// Connect member to the owner of this scope
		mirrorRelationToMember(alias, connection);
	}

	/**
	 * Admits an inner namespace. This is only applicable for namespace scopes,
	 * since they are the only ones which may actually contain namespaces;
	 * it is the front-end developer's responsibility not to break this constraint.
	 * @param inner the new namespace
	 */
	public void addMember(Namespace inner)
	{
		// Create a contained-connection
		ContainedConnection connection = 
			new ContainedConnection(m_owner, Specifiers.DONT_CARE,
			Specifiers.DONT_CARE, Specifiers.DONT_CARE, inner);
		// Add connection to list of connected inner namespaces
		m_namespaces.add(connection);
		// Connect member to the owner of this scope
		mirrorRelationToMember(inner, connection);
	}
	
	/**
	 * Adds a Group under this scope. Groups bind together members of the
	 * Scope and should arrive from the documentation comments. Adding groups
	 * does <b>not</b> replace addmissions of the grouped members to this
	 * scope <b>as well as</b> to the relevant Group's scope.
	 */
	public void addGroup(Group group)
	{
		// Create a contained-connection
		ContainedConnection connection = 
			new ContainedConnection(m_owner, Specifiers.DONT_CARE,
			Specifiers.DONT_CARE, Specifiers.DONT_CARE, group);
		// Add connection to list of connected groups
		m_groups.add(connection);
		// Connect member to the owner of this scope
		mirrorRelationToMember(group, connection);
	}

	/**
	 * Adds the Entity 'friend' as a friend of the Entity owning this scope.
	 * @param friend an entity declared as <code>friend</code> inside the
	 * declaration of the class
	 */
	public void addFriend(Entity friend)
	{
		// Create a friend-connection
		FriendConnection connection = new FriendConnection(m_owner, friend);
		// Add connection to list of friends
		m_friends.add(connection);
		// Connect friend to this as affiliate
		mirrorRelationToMember(friend, connection);
	}

	/*@}*/


	/** @name Pull API
	 * Methods for accessing the contents of the Scope. Members of the
	 * scope are accessed via the ContainedConnection; use this object to
	 * reveal more information about the contained Entity, such as the visibility
	 * or storage specifiers (when applicable).
	 */
	/*@{*/
	
	/**
	 * Access routines contained in the Scope.
	 * @return Iterator an iterator over ContainedConnection, referring to
	 * each member routine in turn.
	 */
	public Iterator routineIterator()
	{
		return m_routines.iterator();
	}

	/**
	 * Access fields of the Scope.
	 * @return Iterator and iterator over ContainedConnection, referring to
	 * each member field in turn.
	 */
	public Iterator fieldIterator()
	{
		return m_fields.iterator();
	}
	
	/**
	 * Access classes, structs, and unions in this Scope.
	 * @return Iterator an iterator over ContainedConnection, referring to
	 * each inner construct in turn.
	 */
	public Iterator aggregateIterator()
	{
		return m_aggregates.iterator();
	}

	/**
	 * Access enumerated types in this Scope.
	 * @return Iterator an iterator over ContainedConnection.
	 */
	public Iterator enumIterator()
	{
		return m_enums.iterator();
	}

	/**
	 * Access alias objects in this Scope.
	 * @return Iterator an iterator over ContainedConnection.
	 */
	public Iterator aliasIterator()
	{
		return m_aliases.iterator();
	}

	/**
	 * Access inner namespaces in a namespace Scope. If this is not a namespace
	 * Scope, the returned iterator is empty, unless the front-end has violated
	 * inclusion constraints.
	 * @return Iterator an iterator over ContainedConnection, referring to
	 * each inner namespace in turn.
	 */
	public Iterator namespaceIterator()
	{
		return m_namespaces.iterator();
	}
	
	/**
	 * Access groups in this scope. Groups tie together elements which are
	 * normally accessed in a "flat" manner using the other *Iterator methods.
	 * Grouped elements appear both directly in the Scope and in their Group.
	 * @return Iterator an iterator over ContainedConnection, referring to
	 * each inner Group in turn, but only those directly under this scope, not
	 * nested groups.
	 */
	public Iterator groupIterator()
	{
		return m_groups.iterator();
	}
	
	/**
	 * 
	 * 
	 */
	public Iterator friendIterator()
	{
		return m_friends.iterator();
	}
	
	/**
	 * Finds a group in this scope according to its name.
	 * @param groupName group's name
	 * @return Group existing group in that scope
	 * @throws ElementNotFoundException if no group by that name exists.
	 */
	public Group groupByName(String groupName) throws ElementNotFoundException
	{
		for (Iterator groupiter = groupIterator(); groupiter.hasNext(); ) {
			ContainedConnection conn = (ContainedConnection)groupiter.next();
			Group group = (Group)conn.getContained();
			// Check name of group
			if (group.getName().equals(groupName)) {
				return group;
			}
		}
		// Group was not found		
		throw new ElementNotFoundException("Group", 
			m_owner.getName()+ ":" + groupName);
	}
	
	/*@}*/		
	
	/** @name Protected Push API
	 * Functions for maintaining the link from container to contained as well
	 * as the other way.
	 */
	/*@{*/
	
	/**
	 * Adjusts correct setting of the connection in the contained. Called by all
	 * the <b>addMember</b> functions, and sets the applicable relation from
	 * the member to the container. For scopes of namespace, class, struct,
	 * union - this method calls Entity.<b>connectToContainer</b>(). For
	 * scopes of groups - it will call Entity.<b>connectToGroup</b>(). This is
	 * done by deriving Scope in an inner class of Group.
	 * @param contained the newly introduced member
	 * @param connection the newly created ContainedConnection for it
	 */
	protected void mirrorRelationToMember(Entity contained, 
		ContainedConnection connection)
	{
		contained.connectToContainer(connection);
	}
	
	/**
	 * Adjusts correct settings of the entity declared as friend by registering
	 * the owner of this scope as one of its affiliates. Called by addFriend().
	 * @param friend the so-declared friend
	 * @param connection the newly created FriendConnection for it
	 */
	protected void mirrorRelationToMember(Entity friend, 
		FriendConnection connection)
	{
		friend.connectToAffiliate(connection);
	}
	
	/*@}*/
	
	// Ownership - entity which holds this scope
	private Entity m_owner;

	// Private members - internal representation of Scope's contents
	private List m_routines;
	private List m_fields;
	private List m_aggregates;
	private List m_namespaces;
	private List m_enums;
	private List m_aliases;
	private List m_groups;
	private List m_friends;
}
