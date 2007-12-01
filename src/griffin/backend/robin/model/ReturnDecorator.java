package backend.robin.model;

public class ReturnDecorator implements CppExpression {

	private CppExpression inner;
	
	public ReturnDecorator(CppExpression inner)
	{
		this.inner = inner;
	}
	public String evaluate(String argument) {
		return "return " + inner.evaluate(argument);
	}

}
