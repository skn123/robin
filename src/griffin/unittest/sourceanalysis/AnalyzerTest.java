package unittest.sourceanalysis;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintStream;
import java.io.StringWriter;
import java.io.Writer;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

import javax.swing.tree.DefaultMutableTreeNode;

import junit.framework.TestCase;
import sourceanalysis.Aggregate;
import sourceanalysis.ContainedConnection;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;
import sourceanalysis.MissingInformationException;
import sourceanalysis.Namespace;
import sourceanalysis.Parameter;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.Routine;
import sourceanalysis.Specifiers;
import sourceanalysis.TemplateParameter;
import sourceanalysis.Type;
import sourceanalysis.TypenameTemplateParameter;
import sourceanalysis.dox.DoxygenAnalyzer;

/**
 * To change this generated comment edit the template variable "typecomment":
 * Window>Preferences>Java>Templates.
 * To enable and disable the creation of type comments go to
 * Window>Preferences>Java>Code Generation.
 */
public class AnalyzerTest extends TestCase {

	/**
	 * Constructor for AnalyzerTest.
	 * @param title test's text title string
	 */
	public AnalyzerTest(String title) {
		super(title);
		random = new Random();
	}

	public void testTypeExpressionParser() throws Exception
	{
		int failed = 0;
		ProgramSkeleton skel = new ProgramSkeleton();
		DoxygenAnalyzer dox = new DoxygenAnalyzer();

		for (int times = 0; times < 25; ++times) {
			Type type = skel.randomType(true);
			String typeexpr = type.formatCpp();
			Type parsed = dox.parseType(typeexpr, new HashMap<String, Entity>());
			parsed.toString();
			if (!type.toString().equals(parsed.toString())) {
				System.err.println("" + type + " ?= " + typeexpr + " ?= " + parsed);
				failed++;
			}
		}
		
		assertEquals(failed, 0);
	}
	
	/**
	 * Validates the reading of routines from a flat C source file.
	 * <ol>
	 *  <li>Generate <i>k</i> random routine prototypes</li>
	 *  <li>Print prototypes in C++ format to a temporary file
	 *  (<tt>idl.h</tt>)</li>
	 *  <li>Analyze the resulting file, producing a ProgramDatabase</li>
	 *  <li>Compare the protoypes of the resulting routines with those
	 *  initially generated</li>
	 * </ol>
	 */
	public void testRoutines() throws Exception
	{
		// Prepare a file containing function headers
		List<Routine> routines = new LinkedList<Routine>();
		int nroutines = 5;
		PrintStream fout = new PrintStream(new FileOutputStream("check/idl.h"));
		ProgramSkeleton skel = new ProgramSkeleton();
		
		for (int routineCount = 0; routineCount < nroutines; ++routineCount) {
			Routine rou = skel.randomRoutine(true);
			routines.add(rou);
			verboseRoutine(rou, new OutputStreamWriter(System.err));
			// Write function prototype in file
			printRoutineCpp(rou, new OutputStreamWriter(fout));
			fout.println(";");
		}
		fout.close();
		
		// Run Doxygen and extract analyzed 
		ProgramDatabase p = executeDoxygenAndAnalyze(
			"src/unittest/sourceanalysis/Doxyfile.testRoutines");
		
		// Compared routines from extracted database to originally generated
		// ones
		Iterator<Routine> originaliter = routines.iterator();
		Iterator<ContainedConnection<Namespace, Routine>> processediter = p.getGlobalNamespace().getScope().getRoutines().iterator();
		
		while (originaliter.hasNext() && processediter.hasNext()) {
			// Extract both routines
			Routine originalRoutine = originaliter.next();
			Routine processedRoutine = (processediter.next()).getContained();
			// Compare them using the string representations
			StringWriter original = new StringWriter();
			StringWriter processed = new StringWriter();
			verboseRoutine(originalRoutine, original);
			verboseRoutine(processedRoutine, processed);
		}
		
		assertTrue(!originaliter.hasNext() && !processediter.hasNext());
	}
	
	/**
	 * Tests a fragment of code generated for this purpose.
	 */
	public void testCode()
		throws Exception
	{
		ProgramSkeleton skel = new ProgramSkeleton();
		// Fill skeleton with some methods
		for (ContainedConnection<Namespace, Aggregate> cc: skel.nsGlobal.getScope().getAggregates()) {
			Aggregate agg = cc.getContained();
			// Add methods to this aggregate
			for (int count = 0; count < 5; count++) {
				Routine ro = skel.randomRoutine(false);
				agg.getScope().addMember(ro, Specifiers.Visibility.PUBLIC, 
					Specifiers.Virtuality.NON_VIRTUAL,
					Specifiers.Storage.INSTANCE_OWN);
			}
		}
		
		// Dump entire compartment into a file
		Writer fout = new FileWriter("check/base.h");
		printHeaderCpp(skel, fout);
		fout.close();

		// Run Doxygen and extract analyzed 
		ProgramDatabase p = executeDoxygenAndAnalyze(
			"src/unittest/sourceanalysis/Doxyfile.testCode");
		
		// Compare analyzed aggregates to original ones	
		for (ContainedConnection<Namespace, Aggregate> connection: skel.nsGlobal.getScope().getAggregates()) {
			Aggregate original = connection.getContained();
			Aggregate analyzed = null;
			// Find other aggregate
			for (ContainedConnection<Namespace, Aggregate> aconnection: p.getGlobalNamespace().getScope().getAggregates()) {
				// Check full name of aggregate
				analyzed = aconnection.getContained();
				if (analyzed.getFullName().equals(original.getFullName()))
					break ;
				else
					analyzed = null;
			}
			
			assertTrue(analyzed != null);
			
			// Compare original and analyzed
			StringWriter originalPhrase = new StringWriter();
			StringWriter analyzedPhrase = new StringWriter();
			verboseAggregate(original, originalPhrase);
			verboseAggregate(analyzed, analyzedPhrase);
			
			assertEquals(originalPhrase.getBuffer().toString(),
				analyzedPhrase.getBuffer().toString());
		}
	}
	
	
	/**
	 * @name Engine
	 * Methods for running the Doxygen analyzer.
	 */
	/*@{*/
	
	/**
	 * Executes the external Doxygen tool with the supplied configuration.
	 * @param config configuration file name
	 */
	private void executeDoxygen(String config) throws IOException
	{
		// Invoke the runtime environment to run external Doxygen executable
		String[] cmd = { DOXYGEN_EXECUTABLE, config };
		Process doxygen = Runtime.getRuntime().exec(cmd);
		// Read output from child
		BufferedReader doxyout = new BufferedReader(
			new InputStreamReader(doxygen.getInputStream()));
		while (doxyout.readLine() != null) ;
		// Wait for child to terminate
		for (boolean done = false; !done; ) {
			try {
				doxygen.waitFor();
				done = true;
			}
			catch (InterruptedException e) { /* continue waiting */ }
		}
	}
	
	/**
	 * Executes the external Doxygen tool with the supplied configuration,
	 * and reads analyzed information into a program database using the
	 * DoxygenAnalyzer.
	 * @param config configuration file name
	 */
	private ProgramDatabase executeDoxygenAndAnalyze(String config)
		throws IOException, ElementNotFoundException
	{
		// Run doxygen
		executeDoxygen(config);
		
		// Analyze output
		DoxygenAnalyzer d = new DoxygenAnalyzer("check/unittest/xml/");
		return d.processIndex();		
	}
		
	/**
	 * @name Verbosity
	 */
	/*@{*/

	/**
	 * Shows a routine on a console using a readable format.
	 * @param routine routine to print out.
	 * @param out output stream (Writer)
	 */
	private void verboseRoutine(Routine routine, Writer out)
		throws MissingInformationException, IOException
	{
		out.write(routine.getReturnType().toString());
		out.write(" ");
		out.write(routine.getName());
		out.write("(");
		// Verbose parameters
		boolean first = true;
		for (Parameter param: routine.getParameters()) {
			if (!first) out.write(" , ");
			first = false;
			out.write(param.getType().toString());
		}
		out.write(")\n");
		out.flush();
	}
	
	/**
	 * Shows a readable representation of a class.
	 */
	private void verboseAggregate(Aggregate agg, Writer out)
		throws MissingInformationException, IOException
	{
		out.write(agg.getName()) ; out.write(":\n");
		for (ContainedConnection<Aggregate, Routine> connection: agg.getScope().getRoutines()) {
			Routine routine = connection.getContained();
			verboseRoutine(routine, out);
		}
		out.flush();
	}
	
	/**
	 * Produces a C++ output format of a routine.
	 * @param routine routine to print out.
	 * @param out output stream (Writer)
	 */
	private void printRoutineCpp(Routine routine, Writer out)
		throws MissingInformationException, IOException
	{
		out.write(routine.getReturnType().formatCpp(routine.getName()));
		out.write("(");
		// Verbose parameters
		boolean first = true;
		for (Parameter param: routine.getParameters()) {
			if (!first) out.write(" , ");
			first = false;
			out.write(param.getType().formatCpp(param.getName()));
		}
		out.write(")\n");
		out.flush();
	}
	
	/**
	 * Produces a prototype of the classes or class templates held in the
	 * entity in C++ form.
	 * @param proto class definitions (as Aggregate objects)
	 */
	private void printClassCpp(Aggregate klass, Writer out)
		throws MissingInformationException, IOException
	{
		if (klass.isTemplated()) {
			out.write("template <");
			boolean first = true;
			for (TemplateParameter tp: klass.getTemplateParameters()) {
				if (!first) {
					out.write(" , ");
				}
				first = false;
				out.write("class ");
				out.write(tp.getName());
			}
			out.write(">\n");
		}
		out.write("class ");
		out.write(klass.getName());
		out.write("\n{\n");
		
		for (ContainedConnection<Aggregate, Routine> connection: klass.getScope().getRoutines()) {
			Routine method = connection.getContained();
			// - express visibility
			int vis = connection.getVisibility();
			if (vis == Specifiers.Visibility.PUBLIC)
				out.write("public:\n");
			else	if (vis == Specifiers.Visibility.PRIVATE)
				out.write("private:\n");
			else if (vis == Specifiers.Visibility.PROTECTED)
				out.write("protected:\n");
				
			printRoutineCpp(method, out);
			out.write(";\n");
		}
		
		out.write("};\n");
		out.flush();
	}

	/**
	 * Produces a printout of the program skeleton.
	 */
	private void printHeaderCpp(ProgramSkeleton skel, Writer out)
		throws MissingInformationException, IOException
	{
		for (ContainedConnection<Namespace, Aggregate> cc: skel.nsGlobal.getScope().getAggregates()) {
			Aggregate agg = cc.getContained();
			printClassCpp(agg, out);
		}
	}

	/*@}*/
	
	
	private Random random;

	private static final String[] intrinsicNames = { "int", "char", "long", "double", "unsigned int" };
	private static final String[] classNames = { "Socket", "Plug", "Circuit", "Fuse", "PowerSupply" };
	private static final String[] classTemplateNames = { "Connector", "Container" };
	private static final String[] memberNames = { "voltage", "weld", "major", "minor", "flaw" };
		
	private static int unique = 0; // used by randomName
		

	/**	
	 */
	private class ProgramSkeleton
	{
		public final Entity[] intrinsics;
		public final Entity[] classes;
		public final Entity[] templates;
		public final Namespace nsGlobal;
	
		/**
		 * ProgramSkeleton constructor.
		 */
		public ProgramSkeleton()
		{
			nsGlobal = new Namespace();
			nsGlobal.setName("");
			
			intrinsics = new Entity[intrinsicNames.length];
			for (int i = 0; i < intrinsicNames.length; ++i) {
				(intrinsics[i] = new Aggregate()).setName(intrinsicNames[i]);
			}
			classes = new Entity[classNames.length];
			for (int i = 0; i < classNames.length; ++i) {
				Aggregate agg;
				(classes[i] = agg = new Aggregate()).setName(classNames[i]);
				nsGlobal.getScope().addMember(agg, Specifiers.Visibility.PUBLIC);
			}
			templates = new Entity[classTemplateNames.length];
			for (int i = 0; i < classTemplateNames.length; ++i) {
				Aggregate agg;
				(templates[i] = agg = new Aggregate()).setName(classTemplateNames[i]);
				nsGlobal.getScope().addMember(agg, Specifiers.Visibility.PUBLIC);
			}
			templates[0].addTemplateParameter(genTypenameTemplateParameter("Rs"));
			templates[0].addTemplateParameter(genTypenameTemplateParameter("Cr"));
			templates[1].addTemplateParameter(genTypenameTemplateParameter("Oj"));
		}

		/**
		 * @name Randomization
		 */
		/*@{*/
		
		/**
		 * Generates a random name string.
		 */
		public String randomName(String[] names)
		{
			int idx = random.nextInt(names.length);
			unique += random.nextInt(3) + 1;
			return names[idx] + unique;
		}
	
		/**
		 * Generates a random routine signature.
		 * @return Routine
		 */
		private Routine randomRoutine(boolean complexity)
		{
			Routine r = new Routine();
			r.setName(randomName(memberNames));
			r.setReturnType(randomType(false));
			int nparameters = random.nextInt(5);
			for (int i = 0; i < nparameters; ++i) {
				Parameter p = new Parameter();
				p.setName("p" + i);
				p.setType(randomType(complexity));
				r.addParameter(p);
			}
			return r;
		}
		
		/**
		 * Generates a random type expression.
		 * @param complexity when set to <b>false</b>, prevents arrays
		 * from occurring in the selected type - they make expressions
		 * more complicated. 
		 * @return Type
		 */
		private Type randomType(boolean complexity)
		{
			if (random.nextBoolean()) {
				// Return a basic type
				if (random.nextBoolean()) {
					// Return an intrinsic type
					int idx = random.nextInt(intrinsics.length);
					return new Type(new Type.TypeNode(intrinsics[idx]));
				}
				else {
					// Return an aggregate type
					int idx = random.nextInt(classes.length);
					return new Type(new Type.TypeNode(classes[idx]));
				}
			}
			else {
				int deref = random.nextInt(complexity ? 3 : 2);
				Type.TypeNode ptr;
				
				if (deref == 0) {
					ptr = new Type.TypeNode(Type.TypeNode.NODE_POINTER);
					ptr.add((Type.TypeNode)randomType(complexity).getRoot());
				}
				else if (deref == 1) {
					ptr = new Type.TypeNode(Type.TypeNode.NODE_REFERENCE);
					ptr.add((Type.TypeNode)randomType(complexity).getRoot());
				}
				else {
					ptr = new Type.TypeNode(Type.TypeNode.NODE_ARRAY);
					ptr.add((Type.TypeNode)randomType(complexity).getRoot());
					ptr.add(new DefaultMutableTreeNode(new Integer(random.nextInt(10))));
				}
				
				return new Type(ptr);
			}
		}

		/*@}*/
			
	}
	
	/**
	 * Assistant - creates a new template parameter
	 */
	private static TemplateParameter genTypenameTemplateParameter(String name)
	{
		TemplateParameter tp = new TypenameTemplateParameter();
		tp.setName(name);
		return tp;
	}
	
	private static final String DOXYGEN_EXECUTABLE =
		"TODO: doxygen path";
}
