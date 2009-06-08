package sourceanalysis;

import java.util.Vector;

/**
 * An entity with implied template-capability:
 * <ul>
 *  <li>Classes (Aggregates)</li>
 *  <li>Functions and methods (Routines)</li>
 * </ul>
 * The inheritance subtree under this class is not supposed to be exposed
 * to the user of these subclasses, it is only meant to refrain from duplicating
 * the following methods and their documentation.
 */
public abstract class TemplateEnabledEntity extends Entity {

	/**
	 * Constructor for TemplateEnabledEntity.
	 */
	public TemplateEnabledEntity() {
		super();
		m_general4specialization = null;
	}

	/**
	 * @name Push API
	 * Methods for setting template specifications.
	 */
	/*@{*/
	
	/**
	 * States that this is a specialization and provides the general template for
	 * it. The <i>general</i> should be a class template. A SpecializationConnection
	 * is created and inserted in both the specific and the general sides.
	 * @param general the general form of the template which is specialized
	 * @param specArgs actual arguments for this concrete specialization
	 */
	public void setGeneralTemplateForSpecialization(TemplateEnabledEntity general, 
		Vector<TemplateArgument> specArgs)
	{
		// Build a specialization connection
		SpecializationConnection conn = 
			new SpecializationConnection(general, specArgs, this);
		// Connect
		setGeneralTemplateForSpecialization(conn);
	}
	
	/**
	 * States that this is a specialization and provides a connection to the
	 * general template.
	 * @param connection a SpecializaedConnection with <b>this</b> as the
	 *  <i>specific</i>.
	 */
	private void setGeneralTemplateForSpecialization(SpecializationConnection connection)
	{
		m_general4specialization = connection;
	}

	/*@}*/

	/**
	 * @name Pull API
	 * Methods for retrieving information about the template.
	 */
	/*@{*/

	/**
	 * Adds a suffix containing the specialized arguments in angle brackets to
	 * the value returned by Entity.<b>getName</b>(). This is done in order to
	 * avoid name collisions, because specializations essentially have the same
	 * name as the general template which they specialize.
	 * @see sourceanalysis.Entity#getName()
	 */
	public String getName()
	{
		if (isSpecialized()) {
			Vector specArgs = getGeneralTemplateForSpecialization().getSpecificArguments();
			// Add args to string buffer
			String specArgsString =
				Type.formatTemplateArguments(specArgs, 
				                             Type.SIMPLE_TYPE_FORMATTER);
			return super.getName() + specArgsString;
		}
		else
			return super.getName();
	}
	
	/**
	 * Returns whether or not this Aggregate is a specialization of another
	 * template.
	 * @return boolean <b>true</b> if specialized, <b>false</b> if not.
	 */
	public boolean isSpecialized()
	{
		return m_general4specialization != null;
	}
	
	/**
	 * Retrieves the template which serves as basis for this specialization, if this
	 * is indeed a specialization. Otherwise, the return value is <b>null</b>.
	 * @return SpecializationConnection can be null if the referenced item is not
	 * specialized.
	 */
	public SpecializationConnection getGeneralTemplateForSpecialization()
	{
		return m_general4specialization;
	}

	/*@}*/

	// Template specification
	private SpecializationConnection m_general4specialization;
}
