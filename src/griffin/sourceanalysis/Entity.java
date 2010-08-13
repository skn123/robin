package sourceanalysis;

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Vector;
import java.util.Iterator;

import sourceanalysis.SourceFile.DeclDefConnection;

/**
 * <p>Base class for all source-analysis components.</p>
 * <p>The common thing about such components is that they hold <i>properties</i>, which
 * are simple string attributes such as "description" or "note". They may arrive from either
 * the source analyzer or the documentation reader; it is up to the implementation to put and
 * fetch any special values.</p>
 */
public abstract class Entity {
	/**
	 * <p>A string attribute attached to an entity.</p>
	 */
	public static class Property {

		/** Initializes an anonymous Property with no value.
		 */
		public Property() {
			m_name = "anonymous";
			m_value = "";
		}

		public Property(String name, String value) {
			m_name = name;
			m_value = value;
		}

		public String getName() {
			return m_name;
		}

		public String getValue() {
			return m_value;
		}

		public boolean isConcealed() {
			// properties beginning with '.' are reserved for specific use
			return (m_name.length() > 0 && m_name.charAt(0) == '.');
		}

		// Private members
		private String m_name;
		private String m_value;
	}


	/**
	 * Empty initializer.
	 * <ul>
	 *  <li>Initializes name to "anonymous".</li>
	 *  <li>Initializes both group and container to <b>null</b>.</li>
	 *  <li>Initializes properties to empty.</li>
	 *  <li>Initializes template parameters to empty, denoting that the Entity
	 *    is not templated.</li>
	 * </ul>
	 * @see java.lang.Object#Object()
	 */
	public Entity()
	{
		super();
		m_name = "anonymous";
		m_properties = new Vector();
		m_hints = new LinkedList();
		m_uplink = null;
		m_group = null;
		m_affiliates = new LinkedList();
		m_templateParameters = new Vector();
		m_external = false;
	}

	/** @name Push API
	 * Methods for filling information related to the Entity.
	 */
	/*@{*/
	
	/**
	 * Sets the name of this Entity. The name is a simple string without
	 * any qualifiers, and is meaningful in the context of the containing
	 * Scope. It is used for printouts and when searching for specific
	 * entities by name.
	 * @param name - name string
	 */
	public void setName(String name)
	{
		m_name = name;
	}
	
	/**
	 * Attaches properties to the Entity. Properties are added one by one;
	 * Entities are never removed by this method, several entities with the same
	 * name may co-exist.
	 * The properties are later referred to in the <b>same order</b> in which
	 * they were inserted.
	 * @param property new Entity.Property object to add
	 * @see propertyIterator()
	 * @see findProperty(String)
	 */
	public void addProperty(Property property)
	{
		m_properties.add(property);
	}
	
	/**
	 * Attaches hints to the entity. Hints are opaque objects implementing Hint,
	 * and are usually used by the back-ends. See some useful hint classes in the
	 * sourceanalysis.hints package. 
	 * @param hint new Hint object to add
	 * @see hintIterator()
	 */
	public void addHint(Hint hint)
	{
		m_hints.add(hint);
	}
	
	/**
	 * Adds a template parameter. The new parameter is appended to the list
	 * of previously added parameters.
	 * @param parameter a parameter to add
	 */
	public void addTemplateParameter(TemplateParameter parameter)
	{
		m_templateParameters.add(parameter);
		// Connect this parameter to the Entity as "contained"
		parameter.connectToContainer(this, parameter);
	}
	
	/**
	 * Sets the entire template-parameter list. Any existing template parameters
	 * are overwritten by the new setting.
	 * @param parameters the entire (ordered) list of parameters
	 * @throws InappropriateKindException one or more of the elements in
	 * 'parameters' is not a TemplateParameter.
	 */
	public void setTemplateParameters(Vector parameters) 
		throws InappropriateKindException
	{
		m_templateParameters = parameters;
		// Connect all members of the parameters' vector to the Entity
		// as "contained"
		Iterator pi = parameters.iterator();
		while (pi.hasNext()) {
			Object pvalue = pi.next();
			// Check that element really is a template parameter
			if (pvalue instanceof TemplateParameter) {
				TemplateParameter parameter = (TemplateParameter)pvalue;
				// - carry out connection
				parameter.connectToContainer(this, parameter);
			}
			else
				throw new InappropriateKindException("expected: TemplateParameter");
		}
	}
	
	/**
	 * Sets the position of where the Entity is declared.
	 * Positions are merely sets of numbers and has no physical relation with
	 * the source, and are often provided as programmers' reference.
	 */
	public void setDeclarationAt(SourceFile source, SourceFile.Position position)
	{
		// Create a connection
		m_declarationAt =
			new SourceFile.DeclDefConnection(source, position, this);
		source.relateDeclaration(m_declarationAt);
	}
	
	/**
	 * Sets the position of where the Entity is defined.
	 * Positions are merely sets of numbers and has no physical relation with
	 * the source, and are often provided as programmers' reference.
	 */
	public void setDefinitionAt(SourceFile source, SourceFile.Position position)
	{
		// Create a connection
		m_definitionAt =
			new SourceFile.DeclDefConnection(source, position, this);
		source.relateDefinition(m_definitionAt);
	}

	public void setDeclarationAt(DeclDefConnection declaration) {
		if (declaration == null) return;
		
		SourceFile source = null;
		try {
			source = declaration.getSource();
			m_declarationAt = new SourceFile.DeclDefConnection(source, declaration.where(), this);
			source.relateDefinition(m_declarationAt);
		} catch (MissingInformationException e) {}
	}
	
	public void setDefinitionAt(DeclDefConnection definition) {
		if (definition == null) return;
		
		SourceFile source = null;
		try {
			source = definition.getSource();
			m_definitionAt = new SourceFile.DeclDefConnection(source, definition.where(), this);
			source.relateDefinition(m_definitionAt);
		} catch (MissingInformationException e) {}
	}
	
	/**
	 * Sets the position where the entity is declared by name of file rather
	 * than by full Entity.
	 */
	public void setDeclarationAt(String sourceFilename, SourceFile.Position position)
	{
		m_declarationAt =
			new SourceFile.DeclDefConnection(sourceFilename, position, this);
	}
	
	/**
	 * Sets the position where the entity is defined by name of file rather
	 * than by full Entity.
	 */
	public void setDefinitionAt(String sourceFilename, SourceFile.Position position)
	{
		m_definitionAt =
			new SourceFile.DeclDefConnection(sourceFilename, position, this);
	}

	public void setExternal(boolean external)
	{
		m_external = external;
	}
	
	/*@}*/


	/** @name Protected Push API
	 * These methods are only used to set the uplink of the Entity and are called
	 * by "container" entities only.
	 */
	/*@{*/
	/**
	 * Mentions that this Entity is contained (member-of) another Entity.
	 *  This method is called when the entity is inserted into the Scope of the
	 * containing Entity, and must not be called directly on other occasions.
	 * @param connection uplink connection to container
	 */
	protected void connectToContainer(ContainedConnection connection)
	{
		m_uplink = connection;
	}
    
    /**
     * convenience method when we don't care about the connection's visibility,
     * virtuality, or storage
     */
    protected void connectToContainer(Entity container, Entity contained)
    {
        this.connectToContainer(
            new ContainedConnection(
                container,
                Specifiers.DONT_CARE,
                Specifiers.DONT_CARE,
                Specifiers.DONT_CARE,
                contained
            )
        );
    }
	
	/**
	 * Mentions that this Entity belongs to a Group. This method is called when
	 * the entity is inserted into a Group object, and must not be called directly
	 * on other occasions.
	 * @param containingGroup - the group containing this Entity.
	 */
	protected void connectToGroup(Group containingGroup)
	{
		m_group = containingGroup;
	}

	/**
	 * Mentions that this Entity is declared as a friend by another.. This 
	 * method is called when a FriendConnection is created by a Scope.
	 * @param friend the Entity declaring this Entity as a friend
	 */
	protected void connectToAffiliate(FriendConnection friend)
	{
		m_affiliates.add(friend);
	}
	/*@}*/
	
	/** @name Pull API
	 * Methods for retrieving information related to the Entity.
	 */
	/*@{*/

	/**
	 * Method getName.
	 * @return String
	 */
	public String getName()
	{
		return m_name;
	}
	
	/**
	 * Returns fully-qualified name.
	 * <ul>
	 *  <li>Adds names of containing classes or namespace, separated by
	 *       "::", as a prefix.</li>
	 *  <li>Adds template specialization arguments (if any) as a suffix.</li>
	 * </ul>
	 * @return String
	 */
	public String getFullName()
	{
        if (hasContainer())
            return getContainer().getFullName() + "::" + getName();
        else
            return getName();
	}
	
	/**
	 * Finds a property by its name.
	 * @param name property's name
	 * @return Property
	 * @throws ElementNotFoundException no property by that name
	 * exists in the Entity
	 */
	public Property findProperty(String name) throws ElementNotFoundException
	{
		for (Iterator i = m_properties.iterator(); i.hasNext(); ) {
			Property property = (Property)i.next();
			if (property.getName().equals(name)) return property;
		}
		throw new ElementNotFoundException();
	}
	
	/**
	 * Access to all the Entity's properties. Returns an Iterator (java.util.Iterator)
	 * which iterates over Entity.Property items.
	 * @return Iterator
	 */
	public Iterator propertyIterator()
	{
		return m_properties.iterator();
	}
	
	/**
	 * Access to all of the Entity's attached hints. 
	 * @return a java.util.Iterator which iterates over Hint items.
	 */
	public Iterator hintIterator()
	{
		return m_hints.iterator();
	}
	
	/**
	 * Checks whether a specific kind of hint is applied to this entity.
	 * @param kind the Hint subclass to look for
	 * @return true if a hint of this type was found. 
	 */
	public boolean hasHint(Class kind)
	{
		for (Iterator hi = hintIterator(); hi.hasNext();) {
			Hint hint = (Hint) hi.next();
			if (kind.isInstance(hint)) return true;
		}
		return false;
	}
	
	/**
	 * Tries to find a hint belonging to the given hint class. 
	 * @param hintClass a class implementing Hint
	 * @return a Hint object which is an instance of hintClass if
	 * such exists. Otherwise, <b>null</b>.
	 */
	public Hint lookForHint(Class hintClass)
	{
		for (Iterator hintIter = hintIterator(); hintIter.hasNext(); ) {
			Hint hint = (Hint)hintIter.next();
			if (hintClass.isInstance(hint))
				return hint;
		}
		return null;
	}
	
	/**
	 * A simplistic string representation of Entity base on common attributes:
	 * contains Entity kind and name.
	 * @return String
	 */
	public String toString()
	{
		return getClass().getName() + "(" + getName() + ")";
	}

	/*@}*/

	/** @name Container Pull API
	 * methods related to the fact that Entities may be contained in other
	 * Entities.
	 */
	/*@{*/

	/**
	 * Retrieves the Entity in which this Entity is contained. Returns
	 * <b>null</b> if this is a top-level Entity, otherwise the return value is
	 * a ContainedConnection object, from which the container and other
	 * related attributes can be extracted using 
	 * ContainedConnection.getContainer and other methods.
	 * @return ContainedConnection
	 */
	public ContainedConnection getContainerConnection()
	{
		return m_uplink;
	}

    public Entity getContainer() {
        assert hasContainer();
        return getContainerConnection().getContainer();
    }

    public boolean hasContainer() {
        return getContainerConnection() != null;
    }
	
	/**
	 * Retrieves the Group in which this entity is grouped. Returns 
	 * <b>null</b> for entities which exist in an anonymous (base) group of
	 * a class or namespace. 
	 * <p>Grouping is an internal partitioning of members
	 * if a class or namespace and provides extra information over that
	 * already in plain programming-language files. Grouping may be expressed
	 * using documentation comments, but this doesn't have to be the case.
	 * </p>
	 * @return Group
	 */
	public Group getGroup()
	{
		return m_group;
	}
	
	/*@}*/
	
	/** @name Templation Pull API
	 * methods related to template specification.
	 */
	/*@{*/
	
	/**
	 * Checks to see if an Entity is in fact a template.
	 * <p>Entities which may be templated are:</p>
	 * <ul><li>Aggregates (class, struct, union)</li>
	 *       <li>Routines (functions, methods)</li>
	 * </ul>
	 * @return boolean
	 */
	public boolean isTemplated()
	{
		return (m_templateParameters.size() > 0);
	}
	
	/**
	 * Access all the template parameters.
	 * @return Iterator iterates over TemplateParameters
	 */
	public Iterator templateParameterIterator()
	{
		return m_templateParameters.iterator();
	}

	/*@}*/	
	
	
	/**
	 * @name Source-file related Pull API
	 * Methods for retreiving information about declarations and definitions.
	 */
	/*@{*/
	
	/**
	 * Get entities which declare this entity as a friend.
	 * @return an Iterator over FriendConnection.
	 */
	public Iterator affiliatesIterator()
	{
		return m_affiliates.iterator();
	}
	
	/**
	 * Get the location in the source where the Entity is declared.
	 * @return SourceFile.DeclDefConnection connection to source file where
	 * the entity is declared. The connection include line and column numbers
	 * information.
	 */
	public SourceFile.DeclDefConnection getDeclaration()
	{
		return m_declarationAt;
	}
	
	/**
	 * Get the location in the source where the Entity is defined.
	 * @return SourceFile.DeclDefConnection connection to source file where
	 * the entity is defined. The connection include line and column numbers
	 * information.
	 */
	public SourceFile.DeclDefConnection getDefinition()
	{
		return m_definitionAt;
	}

	/**
	 * Indicates whether this entity originates from an external module
	 * (such as a library dependency).
	 * @return boolean if 'true', this entity is external
	 */
	public boolean isExternal()
	{
		return m_external;
	}
	
	/*@}*/

	// Private members
	private String m_name;
	private Vector m_properties;
	private Collection m_hints;
	
	private ContainedConnection m_uplink;
	private Group m_group;
	private List m_affiliates;
	
	private Vector m_templateParameters;
	
	private SourceFile.DeclDefConnection m_declarationAt;
	private SourceFile.DeclDefConnection m_definitionAt;
	private boolean m_external;
}
