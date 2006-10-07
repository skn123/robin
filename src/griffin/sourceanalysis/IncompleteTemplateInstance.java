package sourceanalysis;

import java.util.Iterator;

/**
 * Represents a template instance which has not yet been instantiated. 
 */
public class IncompleteTemplateInstance extends Aggregate {

	/**
	 * Default Constructor.
	 */
	public IncompleteTemplateInstance() {
		super();
	}
	
	/**
	 * Returns the type holding the template arguments of this template
	 * instance.
	 * 
	 * @return The type holding the template arguments.
	 */
	public Type getType()
	{
		return m_type;
	}
	
	/**
	 * Sets the type holding the template arguments of this template
	 * instance.
	 * 
	 * @param type The type to hold the template arguments.
	 */
	public void setType(Type type)
	{
		m_type = type;
	}
	
	/**
	 * Copies contents of original template scope into the scope of this
	 * incomplete template instance entity.
	 * 
	 * @param templateScope template.getScope() where template is the template
	 * being instantiated
	 */
	public void assimilate(Scope templateScope)
	{
		// Copy inner compounds of given template scope into this scope
		for (Iterator ai = templateScope.aggregateIterator(); ai.hasNext();) {
			ContainedConnection connection =
				(ContainedConnection)ai.next();
			Aggregate inner = (Aggregate)connection.getContained();
			IncompleteTemplateInstance ninner =
				new IncompleteTemplateInstance();
			ninner.setName(inner.getName());
			ninner.assimilate(inner.getScope());
			getScope().addMember(ninner, connection.getVisibility());
		}
		// Copy aliases from there as well
		for (Iterator ai = templateScope.aliasIterator(); ai.hasNext();) {
			ContainedConnection connection =
				(ContainedConnection)ai.next();
			Alias inner = (Alias)connection.getContained();
			IncompleteTemplateInstance ninner =
				new IncompleteTemplateInstance();
			ninner.setName(inner.getName());
			getScope().addMember(ninner, connection.getVisibility());
		}
	}
	
	// The type holding the template arguments for this template instanciations.
	private Type m_type;

}
