package unittest.sourceanalysis;

import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;

import junit.framework.TestCase;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.dox.DoxygenAnalyzer;

/**
 * Tests the Front-end using an existing test-suite. The program database
 * resulting from the analysis is dumped and the output compared to the
 * expected representation of the code.
 */
public class AnalyzerTestSuite extends TestCase {

	/**
	 * Constructor for AnalyzerTestSuite.
	 * @param title test to run
	 */
	public AnalyzerTestSuite(String title) {
		super(title);
	}

	public void testSuite() throws Exception
	{
		// Read the program database
		DoxygenAnalyzer dox =
			new DoxygenAnalyzer("check/suite/intermediate/xml/");
		ProgramDatabase p = dox.processIndex();

		// Dump it
		Writer dumpfile = new OutputStreamWriter(new FileOutputStream("gfdump"));
		Dumpster dumpster = new Dumpster();
		dumpster.dump(p.getGlobalNamespace(), dumpfile);
	}
}
