/*
 * Created on Sep 2, 2003
 */
package sourceanalysis;

/**
 * Connects an entity declared in a friend declaration, to the entity
 * (probably an Aggregate) declaring that it is a friend of.
 * <p>For example, in:</p>
 * <p><tt>class G { friend void o(); }</tt></p>
 * <p>The Aggregate 'G' is connected to the routine 'o()' using a
 * FriendConnection originating in 'G'. Here, 'G' is referred to as the
 * "declaring" side of the connection and 'o()' as the "declared".
 */
public class FriendConnection {

	/**
	 * Constructs a friend connection. 
	 * @param declaring the entity (probably an Aggregate) in which the
	 * friend declaration is mentioned
	 * @param declared the entity referenced in the friend declaration
	 */
	public FriendConnection(Entity declaring, Entity declared) {
		super();
		m_declaring = declaring;
		m_declared = declared;
	}
	
	/**
	 * Returns the "declaring" side of the entity (See connection class'
	 * description for information).
	 * @return declaring entity
	 */
	public Entity getDeclaring()
	{
		return m_declaring;
	}
	
	/**
	 * Returns the "declared" side of the entity (See connection class'
	 * description for information).
	 * @return declared entity
	 */
	public Entity getDeclared()
	{
		return m_declared;
	}
	
	// Connection attributes
	private Entity m_declaring;
	private Entity m_declared;

}
