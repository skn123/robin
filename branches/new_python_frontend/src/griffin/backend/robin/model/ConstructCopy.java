package backend.robin.model;

import sourceanalysis.Entity;
import sourceanalysis.Type;

public class ConstructCopy implements CppExpression {

	private Type targetType;

	public ConstructCopy(Type targetType)
	{
		this.targetType = targetType;
	}
	
	public ConstructCopy(Entity targetType)
	{
		this.targetType = new Type(new Type.TypeNode(targetType));
	}

	public String evaluate(String argument)
	{
		return "new " + targetType.formatCpp() + "(" + argument + ")";
	}

}
