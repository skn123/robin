package backend.man;

import java.io.*;
import java.util.*;


import sourceanalysis.*;
import sourceanalysis.Scope;
import sourceanalysis.view.*;
import backend.*;

public class CodeGenerator extends GenericCodeGenerator {
	String m_dir;
	private static final int MAX_LINE_LENGTH = 50;
	/**
	 * @param program
	 * @param output
	 * @param templates
	 */
	public CodeGenerator(
		ProgramDatabase program,
		Writer output,
		TemplateBank templates) {
		super(program, output, templates);
	}

	/**
	 * @param program
	 * @param output
	 */
	public CodeGenerator(ProgramDatabase program, String dir) {
		super(program, null);
		this.m_dir = dir;
		if (!m_dir.endsWith(File.separator)) {
			m_dir += File.separator;
		}

	}

	/**
	 * generates class documentation
	 */
	
	public void generateClassesDocumentation() {
		Namespace globalNS = m_program.getGlobalNamespace();
		Scope globalScope = globalNS.getScope();	
		Iterator i = globalScope.aggregateIterator();
		while (i.hasNext()) {
			ContainedConnection con = (ContainedConnection) i.next();
			try {
				generateSingleClassDocumentation((Aggregate)con.getContained());
			} catch (IOException e) {
				System.err.println("*** ERROR: failed to create man: " + e);
				e.printStackTrace();
			}
		}

	}

	/**
	 * @param entity
	 */
	private void generateSingleClassDocumentation(Aggregate agg)
		throws IOException {
		String name = agg.getName();		
		
		System.out.println("Handle :"+name);
		Writer w =
			new BufferedWriter(
				new FileWriter(m_dir + agg.getName() + ".3z"));
		writeHeader(agg, w);
		writeName(agg, w);
		writeDescription(agg, w);
		writeGroups(agg, w);
		w.close();
	}

	/**
	 * @param entity
	 * @param w
	 */
	private void writeGroups(Aggregate agg, Writer w) throws IOException {
		Iterator i = agg.getScope().groupIterator();
		while (i.hasNext()) {
			ContainedConnection c = (ContainedConnection)i.next();
			Group g = (Group)c.getContained();
			writeGroup(g, w);
		}
		
	}

	/**
	 * @param g
	 * @param w
	 */
	private void writeGroup(Group g, Writer w) throws IOException {
		w.write(".SH "+g.getName()+"\n"+
				".BR\n");
		Scope s = g.getScope();
		writeMethods(s, w);				
	}

	/**
	 * @param entity
	 * @param w
	 */
	private void writeMethods(Scope scope, Writer w) throws IOException  {		
		Iterator i = scope.routineIterator();			
		while(i.hasNext()) {
	
			ContainedConnection c = (ContainedConnection)i.next();
			Routine r = (Routine)c.getContained();						
			try {
				writeMethod(r, w);
			
			} catch (MissingInformationException e) {			
				e.printStackTrace();
			}
		}
		
		
	}

	/**
	 * @param r
	 * @param w
	 */
	private void writeMethod(Routine r, Writer w) throws IOException, MissingInformationException {
		String prototype = Utils.reconstructPrototype(r);
		writePrototype(prototype, w);
	
		String description = findProperty(r, "description").trim();		
		description = Utils.cleanHTMLTags(description);
		description = description.replaceAll("\\.\\s*", ".\n");		
		description += "\n";		
		w.write(description);
	}
	
	

	/**
	 * Writes the prototype of a a method.
	 * 
	 * It is important to break the prototype line into smaller 
	 * lines because ".SS" does not deal with long lines very well 
	 * on some platforms.
	 *  
	 * @param prototype
	 * @param w
	 */
	private void writePrototype(String prototype, Writer w) throws IOException {
		StringTokenizer st = new StringTokenizer(prototype);
		int length = 0;
		w.write("\n.SS ");
		while(st.hasMoreTokens()) {
			String token = st.nextToken();
			length += token.length();
			if(length >= MAX_LINE_LENGTH) {
				w.write("\n.SS ");
				length = token.length();
			}
			w.write(token+" ");			
		}
		w.write("\n");
		
	}

	/**
	 * @param entity
	 * @param w
	 */
	private void writeDescription(Entity entity, Writer w) throws IOException {
		String description = findProperty(entity, "description");
		description = Utils.cleanHTMLTags(description);
		w.write(".SH Description\n\n"+
				".LP\n"+
				description+"\n");
	}

	protected String findProperty(Entity entity, String name) {
		try {
			return entity.findProperty(name).getValue();
		} catch (ElementNotFoundException e) {
			// Property not present
			return "";			
		}
	}

	/**
	 * @param entity
	 * @param w
	 */
	private void writeName(Entity entity, Writer w) throws IOException {
		w.write(".SH NAME\n");
		w.write(entity.getName() + " Documentation\n");
	}

	/**
	 * @param entity
	 * @param w
	 */
	private void writeHeader(Entity entity, Writer w) throws IOException {
		w.write(".TH " + entity.getName() + " Z3\n");
	}
}

