/*
 * Created on Jun 24, 2003
 */
package backend.swig;

import java.io.IOException;
import java.io.Writer;
import java.util.Iterator;

import sourceanalysis.Aggregate;
import sourceanalysis.ContainedConnection;
import sourceanalysis.DataTemplateParameter;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;
import sourceanalysis.Field;
import sourceanalysis.InheritanceConnection;
import sourceanalysis.MissingInformationException;
import sourceanalysis.Namespace;
import sourceanalysis.Parameter;
import sourceanalysis.ProgramDatabase;
import sourceanalysis.Routine;
import sourceanalysis.Scope;
import sourceanalysis.Specifiers;
import sourceanalysis.TemplateArgument;
import sourceanalysis.TemplateEnabledEntity;
import sourceanalysis.TemplateParameter;
import sourceanalysis.Type;
import sourceanalysis.TypenameTemplateArgument;
import sourceanalysis.TypenameTemplateParameter;
import sourceanalysis.view.TemplateBank;
import backend.GenericCodeGenerator;
import backend.Utils;

/**
 * Code generation core for SWIG interface files.
 * 
 * <p>Exploits template: data/swig.tmpl</p>
 */
public class CodeGenerator extends GenericCodeGenerator {

	/**
	 * @param program the program database being processed
	 * @param output output file
	 */
	public CodeGenerator(ProgramDatabase program, Writer output,
		TemplateBank templates) 
	{
		super(program, output, templates);
	}
	
	/**
	 * Build a C++ish representation of a function/method prototype,
	 * with default values stripped away and the entire overloading
	 * spanned. That is, instead of:
	 * <p><tt>void flurry(int color, int flavour = CHOCOLATE)</tt></p>
	 * <p>What you get is:</p>
	 * <p><tt>void flurry(int color); void flurry(int color, int flavour);
	 * </tt></p>
	 * @param routine Entity representing this function or method in the
	 * program database.
	 * @return String a possible textual representation of routine's 
	 * prototype, in C++ code, as explained above.
	 * @throws MissingInformationException
	 */
	public String reconstructPrototype(Routine routine)
		throws MissingInformationException
	{
		int minParams = 0, maxParams = 0;
		
		// Count the number of parameters, esp. the number of parameters with
		// default arguments
		for (Iterator pi = routine.parameterIterator(); pi.hasNext(); ) {
			Parameter parameter = (Parameter)pi.next();
			// If parameter has a default value, it is added to max and not
			// to min; otherwise, it is added to both
			maxParams++;
			if (!parameter.hasDefault()) minParams++;
		}
		
		// Generate prototypes with nParams = 
		//  minParams, minParams+1, ... , maxParams
		StringBuffer protos = new StringBuffer();
		
		for (int nParams = minParams; nParams <= maxParams; ++nParams) {
			protos.append(reconstructPrototype(routine, nParams) + "; ");
		}
		
		return protos.toString();
	}
	
	/**
	 * Build a C++ish representation of a function/method prototype.
	 * @param routine Entity representing this function or method in the
	 * program database. 
	 * @param nParams number of parameters from function prototype (not
	 * including *this) to actually include in generated prototype. This
	 * value is used when generating an interface involving default arguments.  
	 * @return String textual representation of routine's prototype,
	 * as it appears in C++ source code.
	 * @throws MissingInformationException
	 */
	public String reconstructPrototype(Routine routine, int nParams)
		throws MissingInformationException
	{
		if (!backend.Utils.allAreFlat(routine))
			return "/* " + routine.getName() + ": non-flat */";
		
		StringBuffer sb = new StringBuffer();
		
		// Acquire virtuality information 
		int virt;
		if (routine.hasContainer()) {
			virt = routine.getContainerConnection().getVirtuality();
		}
		else {
			virt = Specifiers.Virtuality.NON_VIRTUAL;
		}
		
		if (virt == Specifiers.Virtuality.VIRTUAL 
			|| virt == Specifiers.Virtuality.PURE_VIRTUAL) {
			sb.append("virtual ");
		}
		
		// Rebuild function name declaration with return type
		if (routine.isConstructor()) {
			Aggregate clas = (Aggregate)routine.getContainer();
			sb.append(cleanFullName(clas));
		}
		else {
			Type returnType = routine.getReturnType();
			sb.append(cleanFormatCpp(returnType, routine.getName()));
		}
		
		// Rebuild parameters
		boolean first = true;
		int paramCount = 0;
		sb.append("(");
		
		for (Iterator pi = routine.parameterIterator();
					pi.hasNext() && paramCount < nParams; ) {
			Parameter parameter = (Parameter)pi.next();	
			// Obtain parameter information		
			String name = parameter.getName(); 			
			Type type = parameter.getType();
			// Rebuild parameter declaration
			if (!first) sb.append(", ");
			sb.append(cleanFormatCpp(type, name));
			// Stay in control
			first = false;
			paramCount++;
		}
		
		sb.append(")");
		
		// Attach const/pure-virtual suffix
		if (routine.isConst()) {
			sb.append(" const");
		}
		if (virt == Specifiers.Virtuality.PURE_VIRTUAL) {
			sb.append(" = 0");
		}
		
		return sb.toString();
	}

	/**
	 * Builds a <tt>template&lt;...&gt;</tt> declaration header suitable
	 * for 
	 * @param entity
	 * @return
	 */
	public String reconstructTemplateHeader(TemplateEnabledEntity entity)
		throws MissingInformationException
	{	
		if (entity.isTemplated()) {
			StringBuffer sb = new StringBuffer("template < ");
			boolean first = true;
			
			for (Iterator tpi = entity.templateParameterIterator(); 
				tpi.hasNext(); ) {
				if (!first) sb.append(", ");
				// Get next template parameter
				TemplateParameter templateParameter =
					(TemplateParameter)tpi.next();
				// Observe whether template parameter is typename or data
				if (templateParameter instanceof TypenameTemplateParameter) {
					sb.append("class " + templateParameter.getName());
				}
				else if (templateParameter instanceof DataTemplateParameter) {
					DataTemplateParameter data =
						(DataTemplateParameter)templateParameter;
					cleanFormatCpp(data.getType(), data.getName());					
				}
				first = false;
			}
			sb.append(" >");
			
			return sb.toString();
		}
		else
			return "";
	}

	/**
	 * Builds an inheritance string for a given class.
	 * The inheritance string is that part of the C++ class declaration
	 * block where the base classes are specified. It looks something like
	 * this:
	 * <p><tt>: public Container, protected Factory</tt></p>
	 * @param clas class name for which to generate an inheritance string
	 * @return the inheritance string in normal C++ form
	 */
	public String reconstructInheritance(Aggregate clas)
	{
		StringBuffer sb = new StringBuffer();
		boolean first = true;
		
		// Go over base classes
		for (Iterator bi = clas.baseIterator(); bi.hasNext(); ) {
			InheritanceConnection connection =
				(InheritanceConnection)bi.next();
			// Print delimiter
			if (first) sb.append(": "); else sb.append(", ");
			// Print the visibility
			int visibility = connection.getVisibility();
			if (visibility == Specifiers.Visibility.PUBLIC)
				sb.append("public ");
			else if (visibility == Specifiers.Visibility.PROTECTED)
				sb.append("protected ");
			else if (visibility == Specifiers.Visibility.PRIVATE)
				sb.append("private ");
			// Print base class name
			sb.append(cleanFullName(connection.getBase()));
			// Print template arguments for base
			TemplateArgument[] targs = connection.getBaseTemplateArguments();
			if (targs != null) {
				sb.append("<");
				for (int i = 0; i < targs.length; i++) {
					TemplateArgument argument = targs[i];
					if (i>0) sb.append(", ");
					// Add argument, formatted
					if (argument instanceof TypenameTemplateArgument)
						sb.append(cleanFormatCpp(((TypenameTemplateArgument)argument).getValue(), ""));
					else
						sb.append(argument.toString());
				}
				sb.append(" >");
			}
			first = false;
		}
		
		return sb.toString();
	}
	
	public String writeRequiredTemplateInstantiations(Aggregate clas)
	{
		if (clas.isTemplated()) return "";
		
		StringBuffer sb = new StringBuffer();
		
		// Go over base classes
		for (Iterator bi = clas.baseIterator(); bi.hasNext(); ) {
			InheritanceConnection connection =
				(InheritanceConnection)bi.next();
				
			Aggregate baseClass = connection.getBase();
			if (baseClass.isTemplated()) {
				sb.append("%template(ti_"
					 + clas.hashCode() + baseClass.hashCode() + ") ");
				// Print base class name
				sb.append(cleanFullName(baseClass));
				// Print template arguments for base
				TemplateArgument[] targs = connection.getBaseTemplateArguments();
				if (targs != null) {
					sb.append("<");
					for (int i = 0; i < targs.length; i++) {
						TemplateArgument argument = targs[i];
						if (i>0) sb.append(", ");
						// Add argument, formatted
						if (argument instanceof TypenameTemplateArgument)
							sb.append(cleanFormatCpp(((TypenameTemplateArgument)argument).getValue(), ""));
						else
							sb.append(argument.toString());
					}
					sb.append(" >");
				}
				sb.append(";\n");
			}
		}
		return sb.toString();		
	}

	/**
	 * Searches the scope to see if there is a public default constructor.
	 * @param entity
	 * @return
	 * @throws MissingInformationException
	 */
	public boolean hasDefaultConstructor(Aggregate entity)
		throws MissingInformationException
	{
		return Utils.hasDefaultConstructor(entity);
	}

	/**
	 * Searches the global function base for an output operator applicable
	 * to some entity.
	 * @param entity
	 * @return <b>true</b> if an output operator was found
	 * @throws MissingInformationException
	 */
	public boolean hasOutputOperator(Entity entity) 
		throws MissingInformationException
	{
		return Utils.hasOutputOperator(entity, m_program);
	}

	public static String cleanFullName(Entity e)
	{
		return Utils.cleanFullName(e);
	}

	public static String cleanFormatCpp(Type type, String declname)
	{
		return Utils.cleanFormatCpp(type, declname);
	}

	/**
	 * Creates a SWIG interface file for the collected classes.
	 * @throws ElementNotFoundException
	 * @throws MissingInformationException
	 * @throws IOException
	 */
	public void generateClassInterface()
		throws ElementNotFoundException, MissingInformationException, IOException
	{
		generateClassesSingleDocument("preface", "class");
	}

	/**
	 * Creates a SWIG interface for public global variables in the given 
	 * program database.
	 * @throws IOException when failing to write to output stream
	 * @throws ElementNotFoundException if the 'field' template is not found
	 * @throws MissingInformationException if a field's type is missing because
	 * the database was incomplete.
	 */
	public void generateGlobalVariableInterface()
		throws IOException, ElementNotFoundException, MissingInformationException
	{
		Scope<Namespace> global =  m_program.getGlobalNamespace().getScope();
		for (Iterator gfi = global.fieldIterator(); gfi.hasNext(); ) {
			// Get field
			ContainedConnection connection =
				(ContainedConnection)gfi.next();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC
				&& connection.getStorage() != Specifiers.Storage.STATIC) {
				Entity field = connection.getContained();
				if (!((Field)field).getType().isFlat()) continue;
				// Create a scope
				sourceanalysis.view.Scope scope =
					new sourceanalysis.view.Scope();
				scope.declareMember("FIELD", field, true);
				// Fill template for field 
				m_output.write(m_templates.fillTemplate("field",scope,this));
			}
		}
		
		m_output.flush();
	}
}
