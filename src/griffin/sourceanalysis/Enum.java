package sourceanalysis;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * enclosing_type - undocumented.
 */
public class Enum extends Entity {

	/**
	 * An enumerated constant - tuple of (String, int).
	 */
	public static class Constant
	{
		/**
		 * Initializes the constant with all the required information.
		 * @param literal the enumerated constant's literal string (taken
		 *  from the <b>enum</b> clause)
		 * @param value an integral value which is substituted for the
		 *  representation of the enum constant
		 */
		public Constant(String literal, int value)
		{
			m_literal = literal;
			m_value = value;
		}
		
		/**
		 * Gets the enum's name.
		 * @return String a string literal (unqualified) which is used to address
		 * this constant; this string is taken from the <b>enum</b> clause.
		 */
		public String getLiteral() { return m_literal; }
		
		/**
		 * Gets the enum's integral value.
		 * @return int the value used in the representation of this constant
		 */
		public int getValue() { return m_value; }
		
		// Private
		String m_literal;
		int m_value;
	};

	/**
	 * Constructor for Enum.
	 * Builds the Enum to represent an empty <b>enum</b> { } statement, that
	 * is, without any constants.
	 */
	public Enum() {
		super();
		m_constants = new LinkedList<Constant>(); // create empty list
	}

	/**
	 * @name Push API
	 * Methods for defining the Enum.
	 */
	/*@{*/
	
	/**
	 * Adds a new enumeration constant to the enumerated type.
	 * @param c an Enum.Constant which defines the new constant to add.
	 */
	public void introduceConstant(Constant c)
	{
		m_constants.add(c);
	}

	/**
	 * Adds a new enumeration constant to the enumerate type.
	 * @param literal a name string for the constant
	 * @param value an integral value for the representation
	 */
	public void introductConstant(String literal, int value)
	{
		introduceConstant(new Constant(literal, value));
	}
	
	/**
	 * Creates an exact copy of this enum.
	 * @return a replica
	 */
	public Object clone()
	{
		Enum replica = new Enum();
		replica.setName(getName());
		// Replicate constants. Don't need to actually create new
		// Constant instances since they are immutable anyway.
		for (Constant constant: m_constants) {
			replica.introduceConstant(constant);
		}
		return replica;
	}
	
	/*@}*/
	
	/**
	 * @name Pull API
	 * Methods for querying information about this Enum.
	 */
	/*@{*/
	
	/**
	 * Returns an iterator which goes over all the declared constants by
	 * order in which they were introduced.
	 * @return Iterator an iteration over Enum.Constant
	 */
	public Iterator<Constant> constantIterator()
	{
		return m_constants.iterator();
	}
	
	/*@}*/

	// Private representation
	List<Constant> m_constants;
}
