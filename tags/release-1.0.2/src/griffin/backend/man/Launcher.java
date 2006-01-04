/*
 * Created on Jun 22, 2003
 */
package backend.man;


import sourceanalysis.ElementNotFoundException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.dox.DoxygenAnalyzer;

public class Launcher {

	/**
	 * 
	 */
	public Launcher() {
		super();
	}

	public static void main(String[] args)
	{
		if (args.length < 3) {
			System.err.println("*** ERROR: not enough arguments.");
			System.err.println("    Usage: backend.pydoc.Launcher "
				+ "intermediate output classnames");
			System.exit(1);
		}
		
		
		// Read the program database
		DoxygenAnalyzer dox = new DoxygenAnalyzer(args[0]);
		try {
			ProgramDatabase p = dox.processIndex();
		
			
			CodeGenerator codegen =
				new CodeGenerator(p, args[1]);
				
			// Collect targets
			for (int i = 2; i < args.length; ++i) {
				String arg = args[i];
				if (arg.equals("Auto")) codegen.autocollect();
				else codegen.collect(arg);
			}

			codegen.generateClassesDocumentation();

			
		}
		catch (ElementNotFoundException e) {
			System.err.println("*** ERROR: failed to read index: " + e);
			e.printStackTrace();
		}
		
		
	}
}
