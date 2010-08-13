package backend.robin.model;

public class AddressOf implements CppExpression {

	public String evaluate(String argument) {
		return "&" + argument;
	}

}
