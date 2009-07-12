/*
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package unittest.backend.robin;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import junit.framework.TestCase;
import sourceanalysis.assist.InteriorAnalyzer;

/**
 */
public class GeneratorTest extends TestCase {

	/**
	 * GeneratorTest constructor.
	 * @param title
	 */
	public GeneratorTest(String title)
	{
		super(title);
	}

	/**
	 * Utility function; reads the entire contents of the input stream
	 * and prints it to stderr.
	 * @param input
	 */
	public static void deplete(InputStream input)
	{
		InputStreamReader reader = new InputStreamReader(input);
		int c;
		try {
			while ((c = reader.read()) != -1) System.err.write(c);
		}
		catch (IOException e) {
		}
	}
	
	/**
	 * Runs a command in a child process.
	 * @param command
	 * @return
	 * @throws IOException
	 */
	public static Process execute(String command) throws IOException
	{
		final Process run = 
			Runtime.getRuntime().exec(command);
		new Thread() { @Override
		public void run() { deplete(run.getInputStream()); }
		}.start();
		new Thread() { @Override
		public void run() { deplete(run.getErrorStream()); }
		}.start();
		// Wait for completion
		boolean finished = false;
		while (!finished) {
			try {
				run.waitFor();
				finished = true;
			}
			catch (InterruptedException ie) {
				/* continue looping */
			}
		}
		return run;
	}

	/**
	 * Tests very simple code generation, for a basic class Count.
	 * @throws Exception
	 */
	public void testSimple() throws Exception
	{
		InteriorAnalyzer analyzer = new InteriorAnalyzer();
		analyzer.absorbDefinitions("check/codegen/robin/testSimple.xml");
		// Launch Robin
		String[] classes = { "Count" };
		backend.robin.Launcher.execute(analyzer.getAnalysis(), classes,
			"check/codegen/robin/count_robin.cc", true);
		try {
			// Compile code
			final Process gmake = execute(gmakeCmd + " -C check/codegen/robin");
			assertEquals(0, gmake.exitValue());
		}
		finally {
			execute(gmakeCmd + " -C check/codegen/robin clean");
		}
	}

	/**
	 * Tests generation of overloaded functions' code.
	 * @throws Exception
	 */
	public void testOverloading() throws Exception
	{
		InteriorAnalyzer analyzer = new InteriorAnalyzer();
		analyzer.absorbDefinitions("check/codegen/robin/testOverloading.xml");
		// Launch Robin
		String[] classes = { "Fraction" };
		backend.robin.Launcher.execute(analyzer.getAnalysis(), classes, 
			"check/codegen/robin/fraction_robin.cc", true);
		String target = "TARGET=fraction";
		try {
			// Compile code
			final Process gmake = execute(gmakeCmd + " -C check/codegen/robin " 
				+ target);
			assertEquals(0, gmake.exitValue());
			// Run products
			final Process element = execute("check/codegen/robin/fraction");
			assertEquals(0, element.exitValue());
		}
		finally {
			execute(gmakeCmd + " -C check/codegen/robin clean " + target);
		}
	}
	
	public void testTemplate() throws Exception
	{
		InteriorAnalyzer analyzer = new InteriorAnalyzer();
		analyzer.absorbDefinitions("check/codegen/robin/testTemplate.xml");
		// Launch Robin
		String[] classes = { "Lemon", "LemonTree" };
		backend.robin.Launcher.execute(analyzer.getAnalysis(), classes, 
			"check/codegen/robin/lemon_robin.cc", true);
	}

	/**
	 * Tests supervised templates by instantiating a vector.
	 * @throws Exception
	 */
	public void testVector() throws Exception
	{
		InteriorAnalyzer analyzer = new InteriorAnalyzer();
		analyzer.absorbDefinitions("check/codegen/robin/testVector.xml");
		// Launch Robin
		String[] classes = { "Element" };
		backend.robin.Launcher.execute(analyzer.getAnalysis(), classes, 
			"check/codegen/robin/element_robin.cc", true);
		String target = "TARGET=element";
		try {
			// Compile code
			final Process gmake = execute(gmakeCmd + " -C check/codegen/robin " 
				+ target);
			assertEquals(0, gmake.exitValue());
			// Run products
			final Process element = execute("check/codegen/robin/element");
			assertEquals(0, element.exitValue());
		}
		finally {
			execute(gmakeCmd + " -C check/codegen/robin clean " + target);
		}
	}

	final String gmakeCmd = "gmake --no-print-directory";
}
