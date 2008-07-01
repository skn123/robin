package backend.robin.model;

public class NopExpression implements CppExpression {

	public String evaluate(String argument) {
		return argument;
	}

}
