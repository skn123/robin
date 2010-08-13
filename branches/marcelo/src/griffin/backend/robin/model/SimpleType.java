package backend.robin.model;

import sourceanalysis.Entity;
import sourceanalysis.TemplateArgument;

public class SimpleType {

	public String redir;
	public TemplateArgument[] templateArgs;
	public Entity base;

	public SimpleType(Entity base)
	{
		this.base = base;
		this.templateArgs = null;
		this.redir = "";
	}
	
	public SimpleType(Entity base, TemplateArgument[] templateArgs, String redir)
	{
		this.base = base;
		this.templateArgs = templateArgs;
		this.redir = redir;
	}
}
