package sourceanalysis;

import javax.swing.tree.DefaultMutableTreeNode;

/**
 * enclosing_type - undocumented.
 */
public class InheritanceConnection {

	/**
	 * Constructor for InheritanceConnection. All the fields of the connection
	 * must be initialized at the time it is built.
	 * @param base the base class entity
	 * @param visibility protection for the inheritance, taken from
	 *  Specifiers.Visibility
	 * @param derived the derived class entity
	 * @param templateArgs template arguments for base class (meaningless if
	 * base is not a template)
	 */
	public InheritanceConnection(Aggregate base, 
	                             TemplateArgument[] templateArgs,
	                             int visibility, Aggregate derived) {
		super();
		m_base = base;
		m_baseTemplateArgs = templateArgs;
		m_visibility = visibility;
		m_derived = derived;
	}

	/**
	 * Changes the base type this connection holds by interpreting a type
	 * expression. This is useful for repairing an unresolved reference.
	 * @param baseType the type being extended through this inheritance
	 */
	public void setBaseFromType(Type baseType) {
		if (baseType.isFlat()) {
			Entity base = baseType.getBaseType();
			TemplateArgument[] templ = baseType.getTemplateArguments();
			// - store information in this connection
			if (base instanceof Aggregate) {
				m_base = (Aggregate)base;
				m_baseTemplateArgs = templ;
			}
		}
	}
	
	/**
	 * Constructor for InheritanceConnection. All the fields of the connection
	 * must be initialized at the time it is built.
	 * @param base the base class entity
	 * @param visibility protection for the inheritance, taken from
	 *  Specifiers.Visibility
	 * @param derived the derived class entity
	 */
	public InheritanceConnection(Aggregate base, int visibility, Aggregate derived) {
		this(base, null, visibility, derived);
	}
	
	/**
	 * Retrieves the base class of the inheritance connection.
	 * @return Aggregate base class' entity
	 */
	public Aggregate getBase()
	{
		return m_base;
	}

	/**
	 * Retrieves the template arguments for the base class. This is only
	 * meaningful if the base class is a template - use Entity.isTemplated()
	 * to check this.
	 * @return Array of template argument objects which are given to base
	 * class upon derivation. For example, if the header is:
	 * <p><tt>class Series : public Container&lt;int&gt;  </tt></p>
	 * Then the return value is [ Entity(int) ].
	 */
	public TemplateArgument[] getBaseTemplateArguments()
	{
		return m_baseTemplateArgs;
	}
	
	/**
	 * Returns information about the base type and possibly template arguments
	 * bundled in a type expression.
	 * @return a Type object describing the exact element being extended. 
	 */
	public Type getBaseAsType()
	{
		Aggregate base = getBase();
		TemplateArgument[] templ = getBaseTemplateArguments();
		// Check if there acutally are template arguments
		if (templ != null && templ.length > 0) {
			// Create an approperiate template instantiation type expression
			Type.TypeNode instantiation = 
				new Type.TypeNode(Type.TypeNode.NODE_TEMPLATE_INSTANTIATION);
			// - add base
			instantiation.add(new Type.TypeNode(base));
			// - add template arguments
			for (int i = 0; i < templ.length; i++) {
				instantiation.add(new DefaultMutableTreeNode(templ[i]));
			}
			// - bundle in a Type object
			return new Type(instantiation);
		}
		else {
			// Create a simple, atomic type expression
			// - return base alone
			return new Type(new Type.TypeNode(base));
		}
	}
	
	/**
	 * Retrieves the visibility level of the inheritance (private, protected, public).
	 * @return int protection for the inheritance, taken from
	 * Specifiers.Visibility
	 */
	public int getVisibility()
	{
		return m_visibility;
	}
	
	/**
	 * Retrieves the derived class in the inheritance connection.
	 * @return Aggreate the derived class' entity
	 */
	public Aggregate getDerived()
	{
		return m_derived;
	}

	// Private members (connection attributes)
	Aggregate m_base;
	int m_visibility;
	Aggregate m_derived;
	TemplateArgument[] m_baseTemplateArgs;
}
