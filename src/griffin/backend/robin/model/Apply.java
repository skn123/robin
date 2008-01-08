package backend.robin.model;

public class Apply implements CppExpression {

	String verb;
	
	public Apply(String verb)
	{
		this.verb = verb;
	}
	
	public String evaluate(String argument) 
	{
		return verb + "(" + argument + ")";
	}

}
