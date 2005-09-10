package sourceanalysis;

/**
 * Represents a type alias (typedef) in the Program.
 * <ul>
 *  <li>Use sourceanalysis.Entity#getName() to get the aliased name.</li>
 *  <li>Use getAliasedType() to get the actual type.</li>
 * </ul>
 */
public class Alias extends Entity {

	/**
	 * Constructor for Alias. After construction, the Alias is uninitialized,
	 * attempts to access the aliased type using getAliasedType() will result
	 * in a MissingInformationException.
	 */
	public Alias() {
		super();
	}

	/**
	 * @name Push API
	 * Methods to define the Alias.
	 */
	/*@{*/
	
	/**
	 * Sets the actual type for the alias.
	 * @param actualType the type expression referring to the original type
	 * aliased by the typedef clause.
	 */
	public void setAliasedType(Type actualType)
	{
		m_actualType = actualType;
	}
	
	/*@}*/
	
	/**
	 * @name Pull API
	 * Methods for retrieving information about the Alias.
	 */
	/*@{*/
	
	/**
	 * Retrieves the actual type of the Alias as set by setAliasedType().
	 * @return Type the type expression referring to the original type
	 * aliased by the typedef clause.
	 */
	public Type getAliasedType()
	{
		return m_actualType;
	}

	/*@}*/
	
	// Private members
	Type m_actualType;
}
