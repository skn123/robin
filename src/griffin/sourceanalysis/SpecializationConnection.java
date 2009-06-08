package sourceanalysis;

import java.util.Vector;

/**
 * Models a specialization by connecting the general template which is being
 * specialized to the specific implementation which refers to that template
 * with specific arguments applied.
 * <p>For example: std::vector&lt;bool&gt; is a specialization of the general
 * template std::vector&lt;T&gt; with the specific arguments {bool}.
 */
public class SpecializationConnection {

	/**
	 * Constructor for SpecializationConnection. All the information about
	 * the specialization must be given when this connection object is built.
	 * @param general the general template entity
	 * @param specArgs actual values to template parameters of the general
	 * template. The vector must contain only TemplateArgument objects, and
	 * their order must correspond to the order of the formal declaration of
	 * template parameters there.
	 * @param specific the specific implementation with reference to these
	 * arguments
	 */
	public SpecializationConnection(TemplateEnabledEntity general, Vector<TemplateArgument> specArgs, 
		TemplateEnabledEntity specific) 
	{
		super();
		// Set attributes
		m_general = general;
		m_specific = specific;
		m_specArgs = specArgs;
	}
	
	/**
	 * Returns the general template in this specialization.
	 * @return TemplateEnabledEntity
	 */
	public TemplateEnabledEntity getGeneral()
	{
		return m_general;
	}

	/**
	 * Returns the specific arguments in this specialization.
	 * @return Vector of TemplateArgument
	 */
	public Vector<TemplateArgument> getSpecificArguments()
	{
		return m_specArgs;
	}
	
	/**
	 * Returns the Entity which represents the specific template
	 * implementation.
	 * @return TemplateEnabledEntity
	 */
	public TemplateEnabledEntity getSpecific()
	{
		return m_specific;
	}

	// Private members - connection attributes
	private TemplateEnabledEntity m_general;
	private TemplateEnabledEntity m_specific;
	private Vector<TemplateArgument> m_specArgs;
}
