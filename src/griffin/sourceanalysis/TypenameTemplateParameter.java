package sourceanalysis;

import java.util.Iterator;

import sourceanalysis.Type.TypeNode;

/**
 * Extends TemplateParameter and adds typename-specific attributes.
 * <table>
 *  <tr><td>&lt;</td><td><b>class</b></td><td>T</td><td>=</td><td>int</td><td>&gt;</td></tr>
 *  <tr><td></td><td></td><td><b>getName()</b></td><td></td><td>getDefaultString()</td><td></td></tr>
 * </table>
 */
public class TypenameTemplateParameter extends TemplateParameter {

	public TypenameTemplateParameter()
	{
		m_default = null;
		m_blank = null;
	}

	/**
	 * @name Push API
	 * Methods for setting the typename parameter's attributes.
	 */
	/*@{*/
	
	/**
	 * Sets an actual aggregate entity which is associated with the template
	 * parameter - this entity is used wherever the template parameter is
	 * referenced in the template body.
	 * @param assoc associated aggregate
	 */
	public void associate(Aggregate assoc)
	{
		m_blank = assoc;
	}
	
	/**
	 * Sets a default for this parameter. The default value is a type expression.
	 * @param defaultType
	 */
	public void setDefault(Type defaultType)
	{
		m_default = defaultType;
	}
	
	/*@}*/


	/**
	 * @name Pull API
	 * Methods for accessing template parameter information.
	 */
	/*@{*/
	
	/**
	 * Returns an inner aggregate which is used as the "blank" in places where
	 * the template parameter is used.
	 * @return Aggregate associated entity
	 */
	public Aggregate getDelegate()
	{
		return m_blank;
	}
	
	/**
	 * Checks if the template parameter has a default.
	 * @return boolean
	 */
	public boolean hasDefault()
	{
		return (m_default != null);
	}
	
	/**
	 * Creates a TypenameTemplateArgument corresponding to the default
	 * type for this parameter.
	 * @return a TypenameTemplateArgument holding the default Type.
	 */
	public TemplateArgument getDefaultValue()
	{
		if (hasDefault())
			return new TypenameTemplateArgument(getDefault());
		else
			return null;
	}

	/**
	 * @see sourceanalysis.TemplateParameter#getDefaultValue(java.util.Iterator, java.util.Iterator)
	 */
	public TemplateArgument getDefaultValue(
		Iterator parameterIterator,
		Iterator argumentIterator) throws InappropriateKindException
	{
		if (!hasDefault()) return null;
		
		Type deflt = getDefault();
		
		while (parameterIterator.hasNext() && argumentIterator.hasNext()) {
			final TemplateParameter parameter = (TemplateParameter)parameterIterator.next();
			final TemplateArgument argument = (TemplateArgument)argumentIterator.next();
			// - transform type
			Type.Transformation transformer = new Type.Transformation() {
				public TypeNode transform(TypeNode original)
					throws InappropriateKindException 
				{
					if (original.getKind() == Type.TypeNode.NODE_LEAF) {
						Entity base = original.getBase();
						// - if contained inside the template parameter, replace 
						//   container with argument
						String name = base.getName(), prefix = parameter.getName() + "::";
						if (name.startsWith(prefix)) {
							Aggregate pseudo = new Aggregate();
							pseudo.setName(argument.toCpp() + "::"
							               + name.substring(prefix.length()));
							return new Type.TypeNode(pseudo);
						}
					}
					return null;
				}
			};
			deflt = Type.transformType(deflt, transformer);
		}
		return new TypenameTemplateArgument(deflt);
	}
	
	/**
	 * Returns the default value of this template argument.
	 * @return Type a type expression which is the default for this parameter
	 */
	public Type getDefault()
	{
		return m_default;
	}
	
	/*@}*/
	
	/**
	 * @name Utilities
	 */
	/*@{*/
	public Object clone()
	{
		TypenameTemplateParameter duplica = new TypenameTemplateParameter();
		duplica.setName(this.getName());
		duplica.m_default = this.m_default;
		return duplica;
	}
	/*@}*/
	
	// Private members
	Type m_default;
	Aggregate m_blank;
}
