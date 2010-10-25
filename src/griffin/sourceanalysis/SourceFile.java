package sourceanalysis;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * enclosing_type - undocumented.
 */
public class SourceFile extends Entity {

	/**
	 * Inner class of SourceFile. <p>Marks the position within the source file
	 * of declarations and definitions.</p>
	 * <p>A position is expressed by:</p>
	 * <p><tt>(StartLine:StartColumn) - (EndLine:EndColumn)</tt></p>
	 */
	public static class Position {

		/**
		 * Coordinate - undocumented. A simple record containing an X/Y
		 * location in the source file - in the form of (Line:Column).
		 */
		public class Coordinate {
			public int line;
			public int column;
		};

		/**
		 * Constructor for Position with a single line.
		 */
		public Position(int line) {
			super();
			m_start = m_end = new Coordinate();
			// Set attributes of start - applies to end as well via reference
			m_start.line = line;
			m_start.column = 1;
		}
		
		/**
		 * Imminent constructor for Position.
		 * @param start starting location
		 * @param end finish location
		 */
		public Position(Coordinate start, Coordinate end)
		{
			m_start = start; m_end = end;
		}
		
		public Coordinate getStart() { return m_start; }
		public Coordinate getEnd() { return m_end; }

		/* Private - package */
		Coordinate m_start;
		Coordinate m_end;
	}
	
	/**
	 * Expresses a declaration or a definition within a source file.
	 */
	public static class DeclDefConnection
	{
		/**
		 * DeclDefConnection constructor. All the connection data must be
		 * supplied at construction time to make a <b>fulfilled</b> connection;
		 * connections may also be unfulfilled as described below.
		 * @param source an Entity corresponding to the source file at which
		 * the component occurs
		 * @param at the location where the component occurs in the source
		 * file
		 * @param entity an Entity corresponding to the component itself
		 */
		public DeclDefConnection(SourceFile source, Position at, Entity entity)
		{
			// Store the attributes
			m_source = source; m_at = at; m_entity = entity;
		}
		
		/**
		 * Yields an unfulfilled (unidirectional) connection. It contains only
		 * the name of the source file rather than a corresponding entity.
		 * This is useful in cases where there is not a SourceFile entity
		 * associated with a given file.
		 * @param sourceFilename filename of source file where the component
		 * occurs
		 * @param at location in the source file where the component occurs
		 * @param entity an Entity corresponding to the component
		 */
		public DeclDefConnection(String sourceFilename, Position at,
			Entity entity)
		{
			// Store the attributes
			m_source = null; m_at = at; m_entity = entity;
			m_sourceFilename = sourceFilename;
		}
		
		/**
		 * Returns the source file from which the declaration or definition origin.
		 * @return SourceFile a source file of the program
		 * @throws MissingInformationException if this is an unfulfilled
		 * connection
		 */
		public SourceFile getSource() throws MissingInformationException
		{
			if (m_source == null)
				throw new MissingInformationException("unfulfilled DeclDefConnection");
			else
				return m_source;
		}
		
		/**
		 * Returns the name of the source file.
		 * @return String name (including path if this information is
		 * present)
		 */
		public String getSourceFilename()
		{
			if (m_source != null)
				return m_source.getFullName();
			else
				return m_sourceFilename;
		}
		
		/**
		 * Upgrades an unfulfilled connection into a fulfilled connection by
		 * assigning the approperiate SourceFile entity.
		 * @param source a source file entity
		 */
		public void fulfill(SourceFile source)
		{
			m_source = source;
		}
		
		/**
		 * Returns the exact position in the source file of the declaration or
		 * definition.
		 * @return Position a position in terms of rows and columns
		 */
		public Position where()
		{
			return m_at;
		}
		
		/**
		 * Returns a reference to the Entity being declared.
		 * getDeclaredEntity() and getDefinedEntity() do essentially the same,
		 * and are both provided just for semantic clarity.
		 * @return Entity referenced entity
		 */
		public Entity getDeclaredEntity() { return m_entity; }
		
		/**
		 * Returns a reference to the Entity being declared.
		 * getDeclaredEntity() and getDefinedEntity() do essentially the same,
		 * and are both provided just for semantic clarity.
		 * @return Entity referenced entity
		 */
		public Entity getDefinedEntity() { return m_entity; }
		
		// Private members - connection attributes
		private SourceFile m_source;
		private Position m_at;
		private Entity m_entity;
		private String m_sourceFilename;
	}
	
	/**
	 * Constructor for SourceFile.
	 */
	public SourceFile() {
		super();
		// Initialize all relations to empty
		m_declarations = new LinkedList<DeclDefConnection>();
		m_definitions = new LinkedList<DeclDefConnection>();
		m_includes = new LinkedList<SourceFile>();
		m_includedIn = new LinkedList<SourceFile>();
		m_fullPath = null;
	}

	/**
	 * @name Push API
	 * Methods for setting information concerning this source file.
	 */
	/*@{*/
	
	/**
	 * States that this source-file includes another source-file.
	 * @param header the other source-file included (generally this will be a
	 * header file).
	 */
	public void addInclude(SourceFile header)
	{
		m_includes.add(header);
		header.relateIncluded(this);
	}
	
	/*@}*/
	
	/**
	 * @name Protected Push API
	 * Attribute setting methods used by this package alone, and hidden from
	 * the front-end.
	 */
	/*@{*/
	
	/**
	 * Record the fact that this file is included in another.
	 * @param including the file in which this source-file is included.
	 */
	private void relateIncluded(SourceFile including)
	{
		m_includedIn.add(including);
	}
	
	/**
	 * Record the fact that a declaration appears in this source file.
	 * This is a package-access method used by Entity methods.
	 * @param declConnection a connection object connecting the entity
	 * being declared with the position of the declaration in the source file
	 */
	void relateDeclaration(DeclDefConnection declConnection)
	{
		m_declarations.add(declConnection);
	}
	
	/**
	 * Record the fact that a definition appears in this source file.
	 * This is a package-access method used by Entity methods.
	 * @param defConnection a connection object connecting the entity
	 * being defined with the position of the definition in the source file
	 */
	void relateDefinition(DeclDefConnection defConnection)
	{
		m_definitions.add(defConnection);
	}
	
	/**
	 * Sets the exact location of the source file in the file-system.
	 * This value is returned by getFullName(); if no full path is set
	 * for a source file, getFullName() returns the same value as does
	 * getName().  
	 * @param full filename string with complete absolute path
	 */
	public void setFullPath(String full)
	{
		m_fullPath = full;
	}
	
	/*@}*/

	/**
	 * @name Pull API
	 * Methods for accessing the contents of the source file.
	 */
	/*@{*/
	
	/**
	 * Access files included by this file.
	 * @return Iterator iterates over files which this file includes
	 */
	public Iterator<SourceFile> includesIterator()
	{
		return m_includes.iterator();
	}
	
	/**
	 * Access files including this file.
	 * @return Iterator iterates over files which include this file
	 */
	public Iterator<SourceFile> includedInIterator()
	{
		return m_includedIn.iterator();
	}
	
	/**
	 * Access the declarations in this file.
	 * @return Iterator iterates over DeclDefConnection
	 */
	public ConstCollection<DeclDefConnection> getDeclarations() {
		return new ConstCollection<DeclDefConnection>(m_declarations);
	}

	/**
	 * Access the definitions in this file.
	 * @return Iterator iterates over DeclDefConnection
	 */
	public Iterator<DeclDefConnection> definitionIterator()
	{
		return m_definitions.iterator();
	}
	
	/**
	 * Returns the full pathname of the source file.
	 * The path must have been set earlier using setFullPath(); if it wasn't,
	 * the return value is the same as from getName().
	 * @return String full path (e.g., /tmp/karateka/punch.h)
	 */
	@Override
	public String getFullName()
	{
		return m_fullPath == null ? super.getFullName() : m_fullPath;
	}
	
	/*@}*/
	

	// Private members
	private List<DeclDefConnection> m_declarations;
	private List<DeclDefConnection> m_definitions;
	
	private List<SourceFile> m_includes;
	private List<SourceFile> m_includedIn;
	
	private String m_fullPath;
}
