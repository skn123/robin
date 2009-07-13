package backend.pydoc;

import java.io.IOException;
import java.io.Writer;

import sourceanalysis.Entity;
import sourceanalysis.MissingInformationException;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.Routine;
import sourceanalysis.view.TemplateBank;
import backend.GenericCodeGenerator;

/**
 * Generates a documentation index for use in the Python interpreter.
 * The output is a Python file which, when executed, constructs a Python
 * instance object containing all the necessary data.
 * <p>This object will be of the Python type 
 * <code>document.TableOfContents</code>. Presence of the approperiate
 * module is assumed - you can find this module in the Robin CVS
 * <code>under src/robin/modules/document.py</code>.</p>
 */
public class CodeGenerator extends GenericCodeGenerator 
{

	/**
	 * 
	 */
	public CodeGenerator(ProgramDatabase program, Writer out, 
		TemplateBank bank) 
	{
		super(program, out, bank);
	}

	
	protected void documentProperties(Entity entity) throws IOException
	{
		m_output.write("general = document.Document(\"General\")\n");
		// Go over the properties of this entity
		for (Entity.Property property: entity.getProperties()) {
			m_output.write("general.newSection(\"");
			m_output.write(property.getName());
			m_output.write("\", [\"\"\"");
			m_output.write(property.getValue());
			m_output.write("\"\"\"])\n");
		}
		m_output.write("document.gopher.append(general)\n");
	}
	
	/**
	 * Build a C++ish representation of a function/method prototype.
	 * @param routine Entity representing this function or method in the
	 * program database. 
	 * @return String textual representation of routine's prototype,
	 * as it appears in C++ source code.
	 * @throws MissingInformationException
	 */
	public String reconstructPrototype(Routine routine)
		throws MissingInformationException
	{
		return backend.Utils.reconstructPrototype(routine);
	}
	
	/**
	 * Generates documentation code for classes. The output is a Python
	 * code module, which, when executing, constructs an instance of the
	 * class document.Gopher, which can then be saved using pickle and
	 * read in the interpreter.
	 * @throws MissingInformationException
	 * @throws IOException
	 */
	void generateClassDocumentation()
		throws MissingInformationException, IOException
	{
		generateClassesSingleDocument("preface", "class");
		generateFunctionsSingleDocument("function");
	}

}
