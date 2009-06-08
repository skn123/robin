package backend.robin.model;

public class DeleteSelf implements CppExpression {

	public String evaluate(String argument) {
		return "delete &arg0";
	}

}
