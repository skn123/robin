package sourceanalysis;

import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

/**
 * <p>Saves a bunch of classes and other types in an acessible structure.</p>
 * <p>The <b>ProgramDatabase</b> is the main class in the Griffin API provided
 * for back-end writers, and serves as a starting point to accessing all the
 * information of the program which is represented by Entities in the Program
 * Database.</p>
 * 
 * <h1>Acquiring the Program Database</h1>
 * <p>
 * Usually, a program using Griffin does not create the ProgramDatabase object
 * itself. Instead, its is generally the <b>front-end</b>'s part to create it,
 * by analyzing some input given to it for the purpose of collecting the vast
 * amount of information needed for creating such a structure.
 * </p>
 * <p>Currently, only one front-end exists, and it is described just below.
 * </p>
 * <ul><li>Doxygen Analyzer - uses Doxygen(c) XML capabilities to extract
 *   information read by Doxygen(c) from source files. The input for this
 *   front-end is a directory containing .xml files created by Doxygen(c).
 *   <br />See: sourceanalysis.dox.DoxygenAnalyzer
 * </li>
 * </ul>
 * 
 * <h1>Using the Program Database</h1>
 * <p>Once you have a ProgramDatabase object, you can access information
 * in it in one of several ways:</p>
 * <ul>
 *  <li>Start with the global namespace and reach for its contents using a
 *   Scope. This way you can find programming components by descending
 *   down the tree fromed by the ContainedConnection.<br />
 *   To do this, use ProgramDatabase.getGlobalNamespace().
 *  </li>
 *  <li>Go through the source files of the Program. Each source is represented
 *   by a SourceFile entity and it has connections to programming elements
 *   declared or defined in it.<br />
 *   To do this, use ProgramDatabase.sourceFileIterator().
 *   </li>
 *   <li>Iterate the list of preprocessing macros (#defines) in the program.
 *   Each macro is represented by a Macro entity.<br />
 *   To do this, use ProgramDatabase.macroIterator().
 *   </li>
 * </ul>
 * <p></p>
 * <p><b>Always remember</b> to consult the documentation of the Entity
 * classes themselves:</p>
 * <p>[Entity] [Aggregate] [Namespace] [Routine] [Parameter] [Alias] [Enum]
 * [SourceFile] [Macro] </p>
 */
public class ProgramDatabase extends Entity {

	/**
	 * Empty constructor - makes everything empty:
	 * <ul>
	 *  <li>Prepares an empty sources list,</li>
	 *  <li>Prepares an empty global namespace,</li>
	 *  <li>Sets the name to "Package Database".</li>
	 * </ul>
	 */
	public ProgramDatabase()
	{
		super();
		m_sources = new LinkedList<SourceFile>();
		m_macros = new LinkedList<Macro>();
		m_globalNamespace = new Namespace();
		m_globalNamespace.setName("");
		m_externalsNamespace = new Namespace();
		m_externalsNamespace.setName("");
		setName("Package Database");
	}


	/**
	 * States that a source file belongs to the program.
	 * The file is appended to the list of previously enlisted sources, which is
	 * accessible through sourceFileIterator().
	 * @param source a new SourceFile object
	 */
	public void enlistSourceFile(SourceFile source)
	{
		m_sources.add(source);
		source.connectToContainer(this, source);
	}
	
	/**
	 * States that a macro definition exists in the program.
	 * The macro is appended to a program-wide list of macros, accessible
	 * through macroIterator().
	 * @param macro a new Macro entity
	 */
	public void enlistMacro(Macro macro)
	{
		m_macros.add(macro);
	}
	
	/**
	 * Access elements which are described by this Program Database.
	 * This is done by receiving an anonymous "global namespace" object
	 * which serves as a root. It is accessed, like any normal workspace,
	 * through the Scope, which contains references to top-level Entities.
	 * Container entities may contain references to more entities and so
	 * forth.
	 * @return Namespace a namespace object containing a Scope which holds
	 * all top-level Entities
	 */
	public Namespace getGlobalNamespace()
	{
		return m_globalNamespace;
	}
	
	/**
	 * Access elements which are not directly part of the analyzed program,
	 * but are referenced by it and reside in external dependencies, such
	 * as libraries. 
	 * @return a Scope which can be enumerated to access external entities
	 */
	public Scope<Namespace> getExternals()
	{
		return m_externalsNamespace.getScope();
	}

	/**
	 * Access all the source files in the program.
	 * @return Iterator iterates over SourceFile objects.
	 */
	public Iterator<SourceFile> sourceFileIterator()
	{
		return m_sources.iterator();
	}

	/**
	 * Searches for a source file with the given filename.
	 * @param filename name of the source file (path not required)
	 * @return a SourceFile object
	 * @throws ElementNotFoundException if a source file with the given name 
	 * does not exist in the current program database
	 */
	public SourceFile lookupSourceFile(String filename) 
		throws ElementNotFoundException
	{
		for (SourceFile source: m_sources) {
			if (source.getName().equals(filename) 
					|| source.getFullName().endsWith("/" + filename)) 
				return source;
		}
		throw new ElementNotFoundException("file", filename);
	}
	
	/**
	 * Access the macro definitions in the program.
	 * @return Iterator iterates over Macro objects/
	 */
	public Iterator<Macro> macroIterator()
	{
		return m_macros.iterator();
	}

	// Private members
	private Namespace m_globalNamespace;
	private List<SourceFile> m_sources;
	private List<Macro> m_macros;

	private Namespace m_externalsNamespace;
}
