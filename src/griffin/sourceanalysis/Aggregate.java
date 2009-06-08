package sourceanalysis;

import java.util.Iterator;
import java.util.List;
import java.util.LinkedList;

/**
 * Represents a class, struct, or union in problem space.
 * Members of the so-called "compound" or "aggregate" type, are accessible
 * through the Scope held inside this object, which you can retrieve using
 * getScope().
 */
public class Aggregate extends TemplateEnabledEntity 
{

	/**
	 * Constructor for Aggregate.
	 * <ul>
	 * <li>Prepares an empty scope,</li>
	 * <li>Sets no base classes (empty bases list),</li>
	 * <li>Sets no specialization.</li>
	 * </ul>
	 */
	public Aggregate() {
		super();
		m_agScope = new Scope<Aggregate>(this);
		m_bases = new LinkedList<InheritanceConnection>();
	}
	
	/** @name Push API
	 * Methods for setting information about the Aggregate.
	 */
	/*@{*/
	
	/**
	 * Declares (another) base class for this class. Structs seldom derive and
	 * unions never do; so it is assumed that this Aggregate is a class in the
	 * program and that the <i>base</i> is also a class. An InheritanceConnection
	 * object is created and inserted to both ends.
	 * @param base the base class
	 * @param baseTemplateArgs template arguments for base class if it's
	 * templated. Use addBase(Aggregate,int) for non-templated bases.
	 * @param visibility protection level for inheritance, taken from
	 *  Specifiers.Visibility.
	 * @return the InheritanceConnection object generated as a result of
	 * this update operation.
	 */
	public InheritanceConnection addBase(Aggregate base,
	    TemplateArgument[] baseTemplateArgs, int visibility)
	{
		InheritanceConnection connection =
			new InheritanceConnection(base, baseTemplateArgs,
			                          visibility, this);
		m_bases.add(connection);
		return connection;
	}

	/**
	 * Declares (another) base class for this class. Structs seldom derive and
	 * unions never do; so it is assumed that this Aggregate is a class in the
	 * program and that the <i>base</i> is also a class. An InheritanceConnection
	 * object is created and inserted to both ends.
	 * @param base the base class
	 * @param visibility protection level for inheritance, taken from
	 *  Specifiers.Visibility.
	 * @return the InheritanceConnection object generated as a result of
	 * this update operation.
	 */
	public InheritanceConnection addBase(Aggregate base, int visibility)
	{
		return addBase(base, null, visibility);
	}

	/*@}*/
	
	
	/** @name Pull API
	 * Methods for retrieving information about the Aggregate.
	 */
	/*@{*/

	/**
	 * Access classes from which this Aggregate is derived.
	 * @return Iterator an iterator over InheritanceConnection; use methods
	 *  of the connection to reveal both the base class and the visibility of the
	 *  inheritance.
	 */
	public Iterator<InheritanceConnection> baseIterator()
	{
		return m_bases.iterator();
	}
	
	/*@}*/	

	/** @name Scoping
	 * Provides access to members of the Aggregate via Scope.
	 */
	/*@{*/
	
	/**
	 * Provides the Scope which contains references to the Aggregate's
	 * members.
	 * @return Scope a Scope object which belongs to the Aggregate. Use
	 * Push/Pull API of Scope to fill or retrieve members.
	 */
	public Scope<Aggregate> getScope()
	{
		return m_agScope;
	}

	/*@}*/

	// Scoping
	private Scope<Aggregate> m_agScope;
	// Inheritance
	private List<InheritanceConnection> m_bases;
}
