package sourceanalysis;


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
    public void assimilate(Scope<Aggregate> templateScope)
    {
        copyInnerCompoundsFromTemplate(templateScope);
        copyInnerAliasesFromTemplate(templateScope);
    }

    private void copyInnerCompoundsFromTemplate(Scope<Aggregate> templateScope) {
        for (ContainedConnection<Aggregate, Aggregate> connection: templateScope.getAggregates()) {

            copyInnerAggregate( (Aggregate)connection.getContained(), connection.getVisibility() );
        }
    }

    private void copyInnerAliasesFromTemplate(Scope<Aggregate> templateScope) {
        for (ContainedConnection<Aggregate, Alias> connection: templateScope.getAliass()) {
            copyInnerAlias( (Alias)connection.getContained(), connection.getVisibility() );
        }
    }

    /**
     * protected so it can be tested
     */
    protected void copyInnerAggregate(Aggregate inner, int visibility) {
        IncompleteTemplateInstance ninner = new IncompleteTemplateInstance();
        ninner.setName(inner.getName());
        ninner.assimilate(inner.getScope());
        getScope().addMember(ninner, visibility);
    }

    /**
     * protected so it can be tested
     */
    protected void copyInnerAlias(Alias inner, int visibility) {
        IncompleteTemplateInstance ninner = new IncompleteTemplateInstance();
        ninner.setName(inner.getName());
        getScope().addMember(ninner, visibility);
    }
    
    // The type holding the template arguments for this template instanciations.
    private Type m_type;

}
