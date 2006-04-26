package sourceanalysis.mixin;

import org.python.util.PythonInterpreter;

import sourceanalysis.ProgramDatabase;

/**
 * A MixIn that uses the power of Jython to flexibly
 * apply hints to the program database.
 * 
 * Writing hints in Jython: a variable named 'pdb' is created in
 * the global context. Use <code>sourceanalysis.view.Traverse</code> with a
 * corresponding visitor by implementing one or more of the 
 * <code>sourceanalysis.view.Traverse.*Visitor</code> interfaces.
 */
public class JythonMixIn implements MixIn {

	private String jythonFilename;
	
	/**
	 * 
	 * @param jythonFilename a Jython file (typically .py or .jy) containing
	 * hints in the Python language.
	 */
	public JythonMixIn(String jythonFilename)
	{
		this.jythonFilename = jythonFilename;
	}
	
	public void apply(ProgramDatabase program) {
		PythonInterpreter jython = new PythonInterpreter();
		jython.set("pdb", program);
		jython.execfile(jythonFilename);
	}

}
