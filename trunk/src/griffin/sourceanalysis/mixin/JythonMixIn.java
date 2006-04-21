package sourceanalysis.mixin;

import org.python.util.PythonInterpreter;

import sourceanalysis.ProgramDatabase;

/**
 * A MixIn that uses the power of Jython to flexibly
 * apply hints to the program database.
 */
public class JythonMixIn implements MixIn {

	public void apply(ProgramDatabase program) {
		PythonInterpreter jython = new PythonInterpreter();
		jython.set("pdb", program);
		jython.execfile("hint.py");
	}

}
