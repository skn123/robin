package backend.robin;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.logging.Level;

import sourceanalysis.dox.DoxygenAnalyzer;
import sourceanalysis.*;

/**
 * Runs the Robin code generator to create wrapping & registration code for
 * Robin.
 * <p><b>Input:</b></p>
 * <ul>
 *   <li>Program database</li>
 *   <li>Array of class names</li>
 * </ul>
 * <p><b>Output:</b></p>
 * <ul>
 *   <li>A C++ file which can be compiled by a C++ compiler</li>
 * </ul>
 */
public class Launcher {

	/**
	 * Constructor for Launcher.
	 */
	public Launcher() {
		super();
	}

	public static void main(String[] args) {
		if (args.length < 3) {
			System.err.println("*** ERROR: not enough arguments.");
			System.err.println(
				"    Usage: backend.robin.Launcher intermediate output classnames");
			System.exit(1);
		}
		// Read the program database
		DoxygenAnalyzer dox = new DoxygenAnalyzer(args[0]);
		dox.logger.setLevel(Level.WARNING);
		try {
			ProgramDatabase p = dox.processIndex();
			// Extract parameters
			int flagcount = 0;
			
			// get outfile
			String outfile = args[1];
			
			// all the flags for robin
			boolean interceptors = false;
			
			// go through all of the command line arguments until we hit one
			// that isn't a flag
			for (flagcount = 0; true; ++flagcount) {
				if (args[2 + flagcount].equals("--interceptors")) {
					interceptors = true;
				}
				else {
					break;
				}
			}
			
			// get the class names
			String[] classnames = new String[args.length - 2 - flagcount];
			System.arraycopy(args, args.length - classnames.length, classnames, 0, classnames.length);
			
			// Execute
			execute(p, classnames, outfile, interceptors);
		} catch (ElementNotFoundException e) {
			System.err.println("*** ERROR: failed to read index: " + e);
		} catch (IOException e) {
			System.err.println("*** ERROR: output error: " + e);
		} catch (MissingInformationException e) {
			System.err.println("*** ERROR: some information is missing.");
			e.printStackTrace();
		}
	}

	/**
	 * Run Robin and generate code.
	 * @param program a program database to operate on
	 * @param classnames an array of class names
	 * @param outfile name of a file to bear the output
	 * @throws IOException if an output exception occurs
	 * @throws MissingInformationException if the program database is 
	 * incomplete
	 */
	public static void execute(ProgramDatabase program,
		String[] classnames, String outfile, boolean interceptors)
		throws IOException, MissingInformationException 
	{
		OutputStream cfile = new FileOutputStream(outfile);
		CodeGenerator codegen =
			new CodeGenerator(program, new OutputStreamWriter(cfile));
		codegen.setOutputFilename(outfile);
		
		// Collect targets
		if (classnames[0].equals("*")) {
			codegen.autocollect();
		}
		else {
			for (int i = 0; i < classnames.length; ++i) {
				if (classnames[i].startsWith("@")) {
					classnames[i] = classnames[i].substring(1);
					codegen.investInterceptor(classnames[i]);
				}
				codegen.collect(classnames[i]);
			}
		}
		
		codegen.investImplicitInstantiations();
		codegen.grabTypedefedClasses();
		codegen.grabInnersAsWell();
		codegen.collectConstants();
		codegen.investImpliedEnums();
		codegen.investImplicitInstantiations();

		codegen.generateIncludeDirectives();
		codegen.generatePreface();
		codegen.generateInterceptors();
		codegen.generateRoutineWrappers();
		codegen.generateConstantWrappers();
		codegen.generateEnumeratedTypeWrappers();
		codegen.generateEntry();
		codegen.report();

		cfile.close();
	}

}
