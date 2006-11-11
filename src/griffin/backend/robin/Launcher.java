package backend.robin;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;

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
public class Launcher extends backend.Launcher {

	/**
	 * Constructor for Launcher.
	 */
	public Launcher() {
		super();
	}

	public static void main(String[] args) {
		PropertyPage properties = new PropertyPage();
		properties.addBoolean("interceptors", false);
		
		new Launcher().main("backend.robin.Launcher", args, properties);
	}

	/**
	 * Run Robin back-end and generate code.
	 * 
	 * @param program a program database to operate upon
	 * @param properties operation instructions:
	 *  - outfile: name of file to bear the output
	 *  - classes: an array of class names
	 *  - interceptors: a boolean flag indicating whether interceptors are enabled
	 * @throws IOException if an output exception occurs
	 * @throws MissingInformationException if the program database is 
	 * incomplete
	 */
	public void execute(ProgramDatabase program, PropertyPage properties)
		throws IOException, MissingInformationException 
	{
		String outfile = properties.getString("outfile");
		String[] classnames = properties.getStringArray("classes");
		boolean interceptors = properties.getBoolean("interceptors");
		
		// Execute
		execute(program, classnames, outfile, interceptors);		
	}
	
	/**
	 * Run Robin back-end and generate code.
	 * 
	 * @param program a program database to operate upon
	 * @param classnames an array of class names
	 * @param outfile name of a file to bear the output
	 * @param interceptors <b>true</b> to enable the interceptors feature
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
		if (classnames.length == 0 
				|| classnames[0].equals("*")) {
			codegen.autocollect();
		}
		else {
			for (int i = 0; i < classnames.length; ++i) {
				boolean intercepted = false; 
				if (classnames[i].startsWith("@")) {
					classnames[i] = classnames[i].substring(1);
					intercepted = true;
				}
				codegen.collect(classnames[i]);
                if (intercepted) {
					codegen.investInterceptor(classnames[i]);
                }
			}
		}
        if (interceptors) {
            codegen.autoInvestInterceptor();
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
		codegen.report(classnames);

		cfile.close();
	}

}
