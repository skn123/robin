package backend.robin.model;

import sourceanalysis.Routine;

public class StaticMethodCall implements CppExpression {

	private Routine method;

	public StaticMethodCall(Routine method) {
		this.method = method;
	}
	
	public String evaluate(String argument) {
		assert argument.startsWith("arg0");
		int comma = argument.indexOf(",");
		argument = (comma >= 0) ? argument.substring(comma+1) : "";
		return "arg0." + method.getFullName() + "(" + argument + ")";
	}

}
