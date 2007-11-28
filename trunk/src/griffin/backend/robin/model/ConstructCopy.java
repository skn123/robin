package backend.robin.model;

import sourceanalysis.Entity;

public class ConstructCopy implements CppExpression {

	private Entity targetType;

	public ConstructCopy(Entity targetType)
	{
		this.targetType = targetType;
	}
	
	public String evaluate(String argument)
	{
		return "new " + targetType.getFullName() + "(" + argument + ")";
	}

}
