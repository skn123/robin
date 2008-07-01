package backend.robin.model;

import sourceanalysis.Routine;

public class FunctionCall implements CppExpression {

	private Routine function;

	public FunctionCall(Routine function) {
		this.function = function;
	}
	
	public String evaluate(String argument) {
		return function.getFullName() + "(" + argument + ")";
	}

}
