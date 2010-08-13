package sourceanalysis;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * Corresponds to a #define directive yielding a preprocessing macro.
 */
public class Macro extends Entity {

	/**
	 * Constructor for Macro.
	 */
	public Macro() {
		super();
		m_parameters = new LinkedList();
	}

	/**
	 * @name Push API
	 * Methods for setting attributes of the macro.
	 */
	/*@{*/
	
	/**
	 * Adds a preprocessing parameter. Macros may have parameters which are
	 * textually replaced when the macro is used - for example:
	 * <p><tt>#define PLUS_ONE(X) ((X)+1)</tt></p>
	 * in which case "X" is a preprocessing parameter. Preprocessing
	 * parameters, in contrast with routine parameters, are not represented by
	 * entity and are not assigned a type.
	 * @param pparam literal of preprocessing parameter
	 */
	public void addPreprocessingParameter(String pparam)
	{
		m_parameters.add(pparam);
	}

	/**
	 * Sets the text string this macro expands to.
	 * @param expansion macro expansion string
	 */
	public void setExpansion(String expansion)
	{
		m_expansion = expansion;
	}
	
	/*@}*/
	
	/**
	 * @name Pull API
	 */
	/*@{*/
	
	/**
	 * Access all the preprocessing parameters this macro accepts.
	 * @return an iterator which iterates over Strings
	 */
	public Iterator preprocesingParameterIterator()
	{
		return m_parameters.iterator();
	}
	
	/**
	 * Provide the text string this macro expands to.
	 * @return expansion string
	 */
	public String getExpansion()
	{
		return m_expansion;
	}
	
	/*@}*/
	
	// Private members - macro attributes
	private String m_expansion;
	private List m_parameters;
}
