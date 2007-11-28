package backend.robin.model;

import sourceanalysis.Routine;

public class MethodCall implements CppExpression {

	private Routine method;

	public MethodCall(Routine method) {
		this.method = method;
	}
	
	public String evaluate(String argument) {
		return "self->" + method.getName() + "(" + argument + ")";
	}

}
