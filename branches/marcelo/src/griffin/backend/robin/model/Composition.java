package backend.robin.model;

public class Composition implements CppExpression {

	private CppExpression[] elements;
	
	public Composition(CppExpression... exprs)
	{
		elements = exprs;
	}
	
	public String evaluate(String argument) {
		String product = argument;
		for (int i = elements.length; i > 0; --i) {
			product = elements[i-1].evaluate(product);
		}
		return product;
	}

}
