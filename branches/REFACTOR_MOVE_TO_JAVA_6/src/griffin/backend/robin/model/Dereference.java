package backend.robin.model;

public class Dereference implements CppExpression {

	public String evaluate(String argument) {
		return "*" + argument;
	}

}
