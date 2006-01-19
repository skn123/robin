package backend.robin;

import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import sourceanalysis.*;
import backend.Utils;

/**
 * Generates flat wrappers and registration code for Robin.
 */
public class CodeGenerator extends backend.GenericCodeGenerator {

	/**
	 * Constructor for CodeGenerator.
	 * @param program the program database to work on
	 * @param output target for generated wrappers and registration data
	 */
	public CodeGenerator(ProgramDatabase program, Writer output) {
		super(program, output);
		m_separateClassTemplates = true;
		m_uidMap = new HashMap();
		m_uidNext = 0;
		m_globalDataMembers = new LinkedList();
		m_interceptorMethods = new HashSet();
		m_downCasters = new LinkedList();
		m_interceptors = new LinkedList();
		
		// Register touchups for special types
		Type voidptr = new Type(new Type.TypeNode(Type.TypeNode.NODE_POINTER));
		voidptr.getRootNode().add(new Type.TypeNode(Primitive.VOID));
		Filters.getTouchupsMap().put(
				new Type(new Type.TypeNode(Primitive.FLOAT)),
				new Filters.Touchup(
						voidptr,
						"void *touchup(float val)\n{\n" +
						"\tunion { void *word; float f; } u; u.f = val;\n" +
						"\treturn u.word;\n" +
						"}\n" +
						"float touchdown(void* val)\n{\n" +
						"\tunion { void *word; float f; } u; u.word = val;\n" +
						"\treturn u.f;\n" +
						"}\n"));
		Type doubleptr = new Type(new Type.TypeNode(Type.TypeNode.NODE_POINTER));
		doubleptr.getRootNode().add(new Type.TypeNode(Primitive.DOUBLE));
		Filters.getTouchupsMap().put(
				new Type(new Type.TypeNode(Primitive.DOUBLE)),
				new Filters.Touchup(
						doubleptr,
						"double *touchup(double val)\n{\n" +
						"\treturn new double(val);\n" +
						"}\n" +
						"double touchdown(double* val)\n{\n" +
						"\treturn *std::auto_ptr<double>(val);\n" +
						"}\n"));
		Type longlongptr = new Type(new Type.TypeNode(Type.TypeNode.NODE_POINTER));
		longlongptr.getRootNode().add(new Type.TypeNode(Primitive.LONGLONG));
		Filters.getTouchupsMap().put(
				new Type(new Type.TypeNode(Primitive.LONGLONG)),
				new Filters.Touchup(
						longlongptr,
						"long long *touchup(long long val)\n{\n" +
						"\treturn new long long(val);\n" +
						"}\n" +
						"long long touchdown(long long* val)\n{\n" +
						"\treturn *std::auto_ptr<long long>(val);\n" +
						"}\n"));
		Type ulonglongptr = new Type(new Type.TypeNode(Type.TypeNode.NODE_POINTER));
		ulonglongptr.getRootNode().add(new Type.TypeNode(Primitive.ULONGLONG));
		Filters.getTouchupsMap().put(
				new Type(new Type.TypeNode(Primitive.ULONGLONG)),
				new Filters.Touchup(
						ulonglongptr,
						"unsigned long long *touchup(unsigned long long val)\n{\n" +
						"\treturn new unsigned long long(val);\n" +
						"}\n" +
						"unsigned long long touchdown(unsigned long long* val)\n{\n" +
						"\treturn *std::auto_ptr<unsigned long long>(val);\n" +
						"}\n"));
	}
	
	/**
	 * Returns an object identifier - for any n invocations of uid(o) with
	 * the same object o, uid obligates to return the same identifier. For
	 * any two unidentical objects s,t, uid guarantees that uid(s)!=uid(t).
	 * @param o
	 * @return
	 */
	private int uid(Object o) {
		//return System.identityHashCode(o); <-- this seems not to do well
		Object got = m_uidMap.get(o);
		if (got == null) {
			m_uidNext++;
			m_uidMap.put(o, new Integer(m_uidNext));
			return m_uidNext;
		}
		else
			return ((Integer)got).intValue();  
	}

	/**
	 * Finds the given class within the global namespace, listing it for
	 * interceptor creation later.
	 *
	 * @param classname the name of the class to create an interceptor for
	 */
	public void investInterceptor(String classname)
	{
		investInterceptor(m_program.getGlobalNamespace().getScope(), classname);
	}

	/**
	 * Finds the given class within the given scope, listing it for
	 * interceptor creation later.
	 *
	 * @param scope the scope in which to search for the class
	 * @param classname the name of the class to create an interceptor for
	 */
	public void investInterceptor(Scope scope, String classname)
	{
		// Go through all of the Subjects, searching for the given name
		for (Iterator subjectiter = m_subjects.iterator(); subjectiter.hasNext(); ) {
			Aggregate agg = (Aggregate)subjectiter.next();
			if (agg.getName().equals(classname)) {
				// Add to interceptors
				try {
					if ((!agg.isTemplated() || !m_separateClassTemplates)
							&& backend.Utils.isAbstract(agg)) {
						m_interceptors.add( agg );
					}
				}
				catch (MissingInformationException e) { }
			}
		}
	}

	/**
	 * Collects all the constants of the program.
	 */
	public void collectConstants()
		throws IOException, MissingInformationException
	{
		// collect constants in the global namespace
		collectConstants(m_program.getGlobalNamespace().getScope());
		// collect constants in subject classes
		for (Iterator si = m_subjects.iterator(); si.hasNext();) {
			Aggregate aggr = (Aggregate) si.next();
			collectConstants(aggr.getScope());
		}
	}
	
	/**
	 * Collects constants of the program.
	 * 
	 * @param scope program scope inside of which to search for constant.
	 * Search will descend to inner scopes of this one as well.
	 */
	protected void collectConstants(Scope scope)
		throws IOException, MissingInformationException
	{
		// Collect fields in current scope
		for (Iterator fi = scope.fieldIterator(); fi.hasNext();) {
			ContainedConnection connection =
				(ContainedConnection)fi.next();
			Field field = (Field)connection.getContained();

			if (Filters.isAvailableStatic(field, connection)) {
				m_globalDataMembers.add(field);
			}
		}
		// Trace constants in innermore namespaces
		for (Iterator ni = scope.namespaceIterator(); ni.hasNext(); ) {
			ContainedConnection connection =
				(ContainedConnection)ni.next();
			Namespace namespace = (Namespace)connection.getContained();
			// - go to recursion
			collectConstants(namespace.getScope());
		}
	}

	/**
	 * Creates a common beginning which is required for all Robin
	 * flat-wrapping-self-registrating files.
	 * @throws IOException
	 */
	public void generatePreface() throws IOException
	{
		m_output.write("#include <memory>\n");
		
		m_output.write("struct RegData\n");
		m_output.write("{\n");
		m_output.write("	const char *name;\n");
		m_output.write("	const char *type;\n");
		m_output.write("	const RegData *prototype;\n");
		m_output.write("	void *sym;\n");
		m_output.write("};\n\n");
		m_output.write("struct PascalString\n");
		m_output.write("{\n");
		m_output.write("	unsigned long size;\n");
		m_output.write("	const char *chars;\n");
		m_output.write("	char buffer[1];\n");
		m_output.write("};\n\n");
		// TODO autoconf specific
		m_output.write("#ifdef SPECIAL_CONVERSION_OPERATION\n" +
			"# define CONVOP(type,self) static_cast<type>(*self)\n" +
			"#else\n" +
			"# define CONVOP(type,self) self->operator type()\n" +
			"#endif\n\n");
		// Win32 specific
		m_output.write("#ifdef _WINDLL\n" +
				"# define EXPORT __declspec(dllexport)\n" +
				"#else\n# define EXPORT\n#endif\n\n");
		
		m_output.write("typedef void* basic_block;\n");
		m_output.write("typedef void* scripting_element;\n\n");
		
		m_output.write("extern basic_block (*__callback)(" +
				"scripting_element twin, RegData *signature, basic_block args[]);\n");
		m_output.write("basic_block (*__callback)(" +
				"scripting_element twin, RegData *signature, basic_block args[]) = 0;\n\n");
		
		// Special touchup function named same
		m_output.write("template <class T>\n" +
			"struct SameClass {\n" +
			"\tstatic T same(T i) { return i; }\n" +
			"};\n\n");
		
		// Generate touchup code
		Iterator i = Filters.getTouchupsMap().values().iterator();
		while (i.hasNext()) {
			m_output.write(((Filters.Touchup)i.next()).m_touchupCode + "\n");
		}
	}
	
	/**
	 * Writes <code>#include &lt;...&gt;</code> directives to add the required
	 * cofiles to the generated compilation unit.
	 * @throws IOException
	 */
	public void generateIncludeDirectives()
		throws IOException
	{
		List decldefs = new LinkedList();
		Set headers = new HashSet();
		
		// Collect class declarations
		for (Iterator subjectIter = m_subjects.iterator(); subjectIter.hasNext();) {
			Aggregate subject = (Aggregate) subjectIter.next();
			decldefs.add(safeGetDeclaration(subject));
		}
		
		// Collect global routine declarations
		for (Iterator funcIter = m_globalFuncs.iterator(); funcIter.hasNext(); ) {
			Routine routine = (Routine) funcIter.next();
			if (Filters.isAvailable(routine))
				decldefs.add(safeGetDeclaration(routine));
		}
		
		// Collect typedef declarations
		for (Iterator aliasIter = m_typedefs.iterator(); aliasIter.hasNext();) {
			Alias alias = (Alias)aliasIter.next();
			decldefs.add(safeGetDeclaration(alias));
		}
		
		// Collect declarations of constants
		for (Iterator fieldIter = m_globalDataMembers.iterator(); fieldIter.hasNext();) {
			Field field = (Field)fieldIter.next();
			decldefs.add(safeGetDeclaration(field));
		}
		
		// Collect the header files that should be included
		for (Iterator declIter = decldefs.iterator(); declIter.hasNext(); ) {			
			// Try to get location of the declaration
			SourceFile.DeclDefConnection decl = 
				(SourceFile.DeclDefConnection)declIter.next();
			if (decl != null && Filters.isAllowedToInclude(decl)) {
				// Try to get the SourceFile entity for the declaration
				try {
					SourceFile header = decl.getSource();
					headers.add(header.getFullName());
				}
				catch (MissingInformationException e) {
					// Issue a comment indicating the absence of a SourceFile
					m_output.write("// ");
					m_output.write(decl.getSourceFilename());
					m_output.write(" may be missing?\n");
				}
			}
		}		

		// Generate #include directives
		for (Iterator headerIterator = headers.iterator(); 
			headerIterator.hasNext();) {
			m_output.write("#include \"");
			m_output.write(Utils.FileTools.absoluteToRelative(
					(String)headerIterator.next(),
					m_outputDirectory));
			m_output.write("\"\n");
		}
	}
	
	/**
	 * Creates definitions of interceptor classes to wrap the classes marked
	 * for interceptor creation, so that they can be implemented or extended
	 * in the frontend.
	 * @throws IOException
	 * @throws MissingInformationException
	 */
	public void generateInterceptors()
		throws IOException, MissingInformationException
	{
		// New classes to add
		Set newSubjects = new HashSet();
		
		// Generate interceptor class decleration
		for (Iterator subjectIter = m_interceptors.iterator(); subjectIter.hasNext();) {
			Aggregate subject = (Aggregate) subjectIter.next();
			
			// Check if the class is a template instantiation, and if so skip it
			if (subject.isSpecialized()) continue;
			
			// Get the virtual method list of the class
			Collection virtualMethods = Utils.virtualMethods(subject, m_instanceMap);
			if (virtualMethods.isEmpty()) continue;
			
			// This counter counts how many functions come before the unimplemented ones
			int funcCounter = 0;
			// Add the interceptor class to the subjects to wrap
			Aggregate interceptor = new Aggregate();
			// TODO Change the name to "I + uid(subject)" and the regdata name to I + name
			String name = "I" + subject.getName();
			interceptor.setName(name);
			// Add the inheritance from the original interface
			interceptor.addBase(subject, Specifiers.Visibility.PUBLIC);
			++funcCounter;
			// Add the _init function
			Routine _init = new Routine();
			_init.setName("_init");
			Primitive scripting_element = new Primitive();
			scripting_element.setName("scripting_element");
			Parameter _init_imp = new Parameter();
			_init_imp.setName("imp");
			_init_imp.setType(new Type(new Type.TypeNode(scripting_element)));
			_init.addParameter(_init_imp);
			_init.setReturnType(new Type(new Type.TypeNode(Primitive.VOID)));
			interceptor.getScope().addMember(
					_init, Specifiers.Visibility.PUBLIC, 
					Specifiers.Virtuality.NON_VIRTUAL, Specifiers.Storage.EXTERN);
			++funcCounter;
			
			m_output.write("// Interceptor for " + subject.getFullName() + "\n");
			m_output.write("extern RegData scope_" + interceptor.getScope().hashCode() + "[];\n");
			m_output.write("class " + name + " : public " + subject.getFullName() + "\n");
			m_output.write("{\n");
			m_output.write("public:\n");
			m_output.write("\tvirtual ~" + name + "() {}\n\n");
			
			// TODO Add base constructor calls in this class
			for (Iterator ctorIter = subject.getScope().routineIterator(); ctorIter.hasNext();) {
				ContainedConnection connection = (ContainedConnection) ctorIter.next();
				Routine ctor = (Routine) connection.getContained();
				if (! ctor.isConstructor()) continue;
				
				Routine newCtor = (Routine) ctor.clone();
				newCtor.setName(name);
				interceptor.getScope().addMember(
						newCtor, Specifiers.Visibility.PUBLIC, 
						Specifiers.Virtuality.NON_VIRTUAL, Specifiers.Storage.EXTERN);
				++funcCounter;
				
				m_output.write("\t" + name + "(");
				for (Iterator argIter = ctor.parameterIterator(); argIter.hasNext();) {
					Parameter param = (Parameter) argIter.next();
					m_output.write(param.getType().formatCpp(param.getName()));
					if (argIter.hasNext()) m_output.write(", ");
				}
				m_output.write(") : " + subject.getName() + "(");
				for (Iterator argIter = ctor.parameterIterator(); argIter.hasNext();) {
					Parameter param = (Parameter) argIter.next();
					m_output.write(param.getName());
					if (argIter.hasNext()) m_output.write(", ");
				}
				m_output.write(") {}\n\n");
			}
			
			m_output.write("\tvoid _init(scripting_element imp) { twin = imp; }\n\n");
			
			// Write functions in interceptor class, and add them to the griffin class
			int i = 0;
			for (Iterator funcIter = virtualMethods.iterator();
				funcIter.hasNext(); ++i) {
				Routine routine = (Routine) funcIter.next();
				
				// Add the routine to the griffin class
				Routine newRoutine = (Routine) routine.clone();
				m_interceptorMethods.add(newRoutine);
				interceptor.getScope().addMember(
						newRoutine, Specifiers.Visibility.PUBLIC, 
						Specifiers.Virtuality.NON_VIRTUAL, Specifiers.Storage.EXTERN);

				// If the current function has some default arguments, increment
				// the function pointer to the last function
				int defaultArgumentCount = 
					Utils.countParameters(routine) -
					Utils.minimalArgumentCount(routine);
				funcCounter += defaultArgumentCount;
				
				// Write the function header
				m_output.write("\tvirtual ");
				m_output.write(routine.getReturnType().formatCpp());
				m_output.write(" " + routine.getName() + "(");
				for (Iterator argIter = routine.parameterIterator(); argIter.hasNext();) {
					Parameter param = (Parameter) argIter.next();
					m_output.write(param.getType().formatCpp(param.getName()));
					if (argIter.hasNext()) m_output.write(", ");
				}
				m_output.write(")");
				if (newRoutine.isConst()) m_output.write(" const");
				// Write a throw() clause if required by the interface
				if (routine.hasThrowClause()) {
					m_output.write(" throw(");
					boolean first = true;
					for (Iterator ei = routine.throwsIterator(); ei.hasNext(); ) {
						if (!first) m_output.write(", ");
						m_output.write(((Entity)ei.next()).getFullName());
					}
					m_output.write(")");
				}
				m_output.write(" {\n");
				
				// Write the function's basic_block argument array
				m_output.write("\t\tbasic_block args[] = {\n");
				for (Iterator argIter = routine.parameterIterator(); argIter.hasNext();) {
					Parameter param = (Parameter) argIter.next();
					m_output.write("\t\t\t" +
							"(" +
							"(" +
							"basic_block (*)" +
							"(" + param.getType().formatCpp() + ")" +
							")");
					Type touchupType = Filters.getTouchup(param.getType());
					if (touchupType == null) { 
						m_output.write("SameClass< " + param.getType().formatCpp() + " >" +
							"::same");
					}
					else {
						m_output.write(" (" + 
							touchupType.formatCpp() + 
							" (*)(" + 
							param.getType().formatCpp() + ")) ");
						m_output.write("touchup");
					}
					m_output.write(")" +
							"(" + param.getName() + ")");
					if (argIter.hasNext()) m_output.write(",");
					m_output.write("\n");
				}
				m_output.write("\t\t};\n");
				
				// Write the call to __callback
				m_output.write("\t\t");
				if (! routine.getReturnType().equals(
						new Type(new Type.TypeNode(Primitive.VOID)))) {
					m_output.write("basic_block result = ");
				}
				m_output.write("__callback(twin, ");
				m_output.write("scope_" + 
						interceptor.getScope().hashCode() + 
						" + " +
						funcCounter + 
						", ");
				m_output.write("args);\n");
				
				// Write the return statement
				if (! routine.getReturnType().equals(
						new Type(new Type.TypeNode(Primitive.VOID)))) {
					Type returnType = routine.getReturnType();
					Type touchupType = Filters.getTouchup(returnType);
					if (touchupType != null) {
						returnType = touchupType;
					}
					
					int parenNest = 0;
					
					m_output.write("\t\treturn ");
				
					if (needsExtraReferencing(returnType)) {
						m_output.write("*std::auto_ptr< ");
						m_output.write(returnType.formatCpp());
						m_output.write(" >(((");
						m_output.write(routine.getReturnType().formatCpp() + " * (*)(basic_block)) ");
						++parenNest;
					}
					else {
						m_output.write("( (" + routine.getReturnType().formatCpp() + " (*)(basic_block)) ");
					}
					
					if (touchupType != null) {
						m_output.write("(" + 
								routine.getReturnType().formatCpp() + 
								" (*)(" + 
								touchupType.formatCpp() + ")) ");
						m_output.write("touchdown)(");
					}
					else {
						m_output.write("SameClass< basic_block >::same)(");
					}

					m_output.write("result)");
					
					for (int paren = 0; paren < parenNest; ++paren)
						m_output.write(")");
					
					m_output.write(";\n");
				}
				m_output.write("\t}\n\n");

				// Increment the function pointer counter
				funcCounter++;
			}
			
			// Write private sections of class
			m_output.write("private:\n");
			m_output.write("\tscripting_element twin;\n");
			m_output.write("};\n\n");
			
			// Add the class
			newSubjects.add(interceptor);
		}
		
		// Add all of the new subjects to the subjects set
		m_subjects.addAll(newSubjects);
	}
	
	/**
	 * Create wrappers for clasess that were found during collect().
	 * @throws IOException
	 * @throws MissingInformationException
	 */
	public void generateRoutineWrappers()
		throws IOException, MissingInformationException
	{ 
		// Generate routine wrappers for class methods
		for (Iterator subjectIter = m_subjects.iterator(); subjectIter.hasNext();) {
			Aggregate subject = (Aggregate) subjectIter.next();
			boolean isAbstractClass = Utils.isAbstract(subject, m_instanceMap);
			boolean mustHaveCtor = Utils.hasDefaultConstructor(subject);
			boolean ctors = false;
			boolean hasOutput = Utils.hasOutputOperator(subject, m_program);
			boolean hasAssignment = Filters.isAssignmentSupportive(subject);
			boolean hasClone = Filters.isCloneable(subject);
			List additional = Utils.findGloballyScopedOperators(subject, m_program);
			// Create upcasts
			generateUpDownCastFlatWrappers(subject);
			// Grab all the routines from subject aggregate
			for (Iterator routineIter = subject.getScope().routineIterator();
				routineIter.hasNext(); ) {
				// Generate wrapper for routine
				ContainedConnection connection = (ContainedConnection)routineIter.next();
				Routine routine = (Routine)connection.getContained();
				if (routine.isConstructor() && isAbstractClass) continue;
				if (connection.getVisibility() == Specifiers.Visibility.PUBLIC
					&& !routine.isDestructor()
					&& Filters.isAvailable(routine)) {
					// - align the number of arguments
					int minArgs = Utils.minimalArgumentCount(routine),
						maxArgs = Utils.countParameters(routine);
					for (int nArguments = minArgs; nArguments <= maxArgs;
							++nArguments) {
						if (! m_interceptorMethods.contains(routine))
							generateFlatWrapper(routine, nArguments, true);
						generateRegistrationPrototype(routine, nArguments, true);
					}
					ctors = ctors || routine.isConstructor();
				}
			}
			// Grab members from subject aggregate
			for (Iterator fieldIter = subject.getScope().fieldIterator();
				fieldIter.hasNext(); ) {
				// Generate wrapper for field
				ContainedConnection connection = (ContainedConnection)fieldIter.next();
				Field field = (Field)connection.getContained();
				if (Filters.isAvailable(field, connection))
					generateFlatWrapper(field, true);
			}
			// More special stuff
			if (!ctors && mustHaveCtor && !isAbstractClass) {
				generateSpecialDefaultConstructorWrapperAndPrototype(subject);
			}
			if (hasAssignment) {
				generateSpecialAssignmentOperator(subject);
			}
			if (hasClone) {
				generateSpecialCloneMethod(subject);
			}
			if (hasOutput) {
				generateSpecialOutputOperator(subject);
				generateSpecialStringConverter(subject);
			}
			for (Iterator addi = additional.iterator(); addi.hasNext(); ) {
				Routine routine = (Routine)addi.next();
				if (m_globalFuncs.indexOf(routine) == -1 
						&& Filters.isAvailable(routine))
					m_globalFuncs.add(routine);
			}
			generateSpecialDestructor(subject);
		}
		
		// Generate wrappers for encapsulated aliases
		for (Iterator aliasIter = m_typedefs.iterator(); aliasIter.hasNext();)
		{
			Alias alias = (Alias)aliasIter.next();
			if (needsEncapsulation(alias)) {
				generateFlatWrapper(alias);
				generateRegistrationPrototype(alias);
			}
		}
		
		// Generated global function wrappers
		for (Iterator funcIter = m_globalFuncs.iterator(); funcIter.hasNext() ;)
		{
			Routine func = (Routine)funcIter.next();
			if (Filters.isAvailable(func)) {
				// - align the number of arguments
				int minArgs = Utils.minimalArgumentCount(func),
					maxArgs = Utils.countParameters(func);
				for (int nArguments = minArgs; nArguments <= maxArgs;
						++nArguments) {
					generateFlatWrapper(func, nArguments, false);
					generateRegistrationPrototype(func, nArguments, false);
				}
			}
		}
	}

	/**
	 * Creates wrappers for all the constants previously found during
	 * collectConstants().
	 */
	public void generateConstantWrappers()
		throws IOException, MissingInformationException
	{
		for (Iterator ci = m_globalDataMembers.iterator(); ci.hasNext(); ) {
			Field global = (Field)ci.next();
			generateFlatWrapper(global, false);
		}
	}
	
	/**
	 * Creates wrappers for enumerated types that were found during collect().
	 * @throws IOException
	 */
	public void generateEnumeratedTypeWrappers()
		throws IOException
	{
		for (Iterator enumIter = m_enums.iterator(); enumIter.hasNext();) {
			sourceanalysis.Enum subject = (sourceanalysis.Enum) enumIter.next();
			// Generate a fine prototype here
			generateFlatWrapper(subject);
			generateRegistrationPrototype(subject);
		}
	}

	public void generateEntry()
		throws IOException, MissingInformationException
	{
		List sorted_subjects = topologicallySortSubjects(true);
		
		for (Iterator subjectIter = m_subjects.iterator();
			 subjectIter.hasNext();) {
			Aggregate subject = (Aggregate) subjectIter.next();
			// Generate the interface for the class
			generateRegistrationPrototype(subject.getScope(),
				Utils.cleanFullName(subject),
				subject.baseIterator(), 
				Utils.isAbstract(subject, m_instanceMap),
				Utils.hasDefaultConstructor(subject),
				Filters.isAssignmentSupportive(subject),
				Filters.isCloneable(subject),
				Utils.hasOutputOperator(subject, m_program), true, true,
				Utils.findGloballyScopedOperators(subject, m_program));
		}
		// Generate main entry point
		m_output.write("extern \"C\" EXPORT RegData entry[];\n\n");
		m_output.write("RegData entry[] = {\n");
		// - enter enumerated types (these should all appear BEFORE function protos)
		for (Iterator enumIter = m_enums.iterator(); enumIter.hasNext();) {
			sourceanalysis.Enum subject = (sourceanalysis.Enum) enumIter.next();
			m_output.write("\t{\"");
			m_output.write(Utils.cleanFullName(subject));
			m_output.write("\", \"enum\", ");
			m_output.write("enumerated_" + subject.hashCode());
			m_output.write("},\n");
		}
		// - enter typedefs
		for (Iterator typedefIter = m_typedefs.iterator(); typedefIter.hasNext();) {
			Alias subject = (Alias)typedefIter.next();
			Type aliased = subject.getAliasedType();
			if (aliased.isFlat()) {
				m_output.write("\t{\"");
				m_output.write(Utils.cleanFullName(subject));
				if (needsEncapsulation(subject)) {
					m_output.write("\", \"class\", alias_" + uid(subject));
					m_output.write("},\n");
				}
				else {
					m_output.write("\", \"=");
					m_output.write(Utils.actualBaseName(aliased));
					m_output.write("\", 0},\n");
				}
			}
		}
		// - enter classes
		for (Iterator subjectIter = sorted_subjects.iterator();
			 subjectIter.hasNext();) {
			Aggregate subject = (Aggregate) subjectIter.next();
			m_output.write("\t{\"");
			m_output.write(Utils.cleanFullName(subject));
			m_output.write("\", \"class\", ");
			m_output.write("scope_" + subject.getScope().hashCode());
			m_output.write("},\n");
		}
		// - enter global functions
		for (Iterator funcIter = m_globalFuncs.iterator(); funcIter.hasNext(); ) {
			Routine subject = (Routine)funcIter.next();
			if (Filters.isAvailable(subject)) {
				generateRegistrationLine(subject, 0, false);
			}
		}
		// - enter global data members
		for (Iterator fieldIter = m_globalDataMembers.iterator();
			fieldIter.hasNext(); ) {
			// Create an entry for each public data member
			Field field = (Field)fieldIter.next();
			if (Filters.isAvailable(field, field.getContainer()))
				generateRegistrationLine(field, false);
		}
		// - enter downcast functions
		for (Iterator downIter = m_downCasters.iterator(); 
		     downIter.hasNext(); ) {
			m_output.write("{ " + downIter.next() + " },\n");
		}
		m_output.write(END_OF_LIST);
		m_output.flush();
	}
	
	/**
	 * Writes the name of the base type from a <b>flat</b> type given. C++
	 * formatting of base type, including template arguments, is written to
	 * the CodeGenerator's output.
	 * @param type flat type to be written out
	 * @throws IOException if errors occur during output operation
	 */
	private void writeFlatBase(Type type) throws IOException
	{
		// Un-typedef
		Entity base = type.getBaseType();
		while (base instanceof Alias && !needsEncapsulation((Alias)base)) {
			type = ((Alias)base).getAliasedType();
			base = type.getBaseType();
		}
		// express basename
		m_output.write(Utils.cleanFullName(base));
		// express template
		TemplateArgument[] templateArgs = type.getTemplateArguments();
		if (templateArgs != null) {
			m_output.write("< ");
			for (int i = 0; i < templateArgs.length; i++) {
				TemplateArgument templateArgument = templateArgs[i];
				if (i > 0) m_output.write(",");					
				if (templateArgument == null) m_output.write("?");	
				else if (templateArgument instanceof TypenameTemplateArgument)
					m_output.write (Utils.cleanFormatCpp(
						((TypenameTemplateArgument)templateArgument)
						.getValue(),""));
				else m_output.write(templateArgument.toString());
			}
			m_output.write(" >");
		}
	}
	
	/**
	 * Writes the name of the base type from a <b>flat</b> type given using
	 * writeFlatBase(), and decorates the name with a prefix which is useful
	 * when trying to identify the type.
	 * <p>Currently, enumerated types are prefixed with '#', and other
	 * types are written as are.</p>
	 * @param type flat type to be written out
	 * @throws IOException if errors occur during output operation
	 */
	private void writeDecoratedFlatBase(Type type) throws IOException
	{
		if (type.getBaseType() instanceof sourceanalysis.Enum) {
			m_output.write("#");
		}
		writeFlatBase(type);
	}

	private boolean isPrimitive(Entity base)
	{
		return (base instanceof Primitive);
	}
	
	private boolean isSmallPrimitive(Entity base)
	{
		return (base instanceof Primitive) &&
		       (!base.getName().equals("double")) &&
		       (!base.getName().equals("long long")) &&
			   (!base.getName().equals("unsigned long long"));
	}

	/**
	 * An aggregate (non-primitive) type needs at least one level of pointer
	 * reference.
	 * @param type argument or return type
	 * @return boolean
	 */
	private boolean needsExtraReferencing(Type type)
	{
		int pointers = type.getPointerDegree();
		boolean reference = type.isReference();
		Entity base = type.getBaseType();
		if ((!reference && pointers == 0) && 
			!(isSmallPrimitive(base) || base instanceof sourceanalysis.Enum)) return true;
		else
			return false;
	}

	/**
	 * Determines whether a simple typedef requires encapsulation - such
	 * typedefs are "hidden" using a proxy wrapper class, which has a
	 * constructor from the aliased type.
	 * @param alias
	 * @return <b>true</b> if encapsulation is required - <b>false</b> if
	 * not.
	 */
	private boolean needsEncapsulation(Alias alias)
	{
		if (alias.getAliasedType().isFlat() && 
			alias.getAliasedType().getBaseType() instanceof Primitive)
			return true;
		else
			return false;
	}
	
	/**
	 * Generates call code for a method or function. 
	 * @param routine entity for method or function to be wrapped in code
	 * @param nArguments number of arguments from the parameter list to consider
	 * @param with if <b>true</b> - the generated code will provide an
	 * instance invocation (self-&gt;) when applicable. If <b>false</b> - the
	 * routine will be treated as a global function or a public static method.
	 * @throws IOException if output fails
	 * @throws MissingInformationException if type elements are missing from
	 * prototype
	 */
	private void generateFlatWrapper(Routine routine, int nArguments, boolean with)
		throws IOException, MissingInformationException
	{
		m_output.write("/*\n * "); m_output.write(routine.getFullName());
		m_output.write("\n * returns "); 
		m_output.write(routine.getReturnType().toString());
		m_output.write("\n */\n");
	
		ContainedConnection container = routine.getContainer();
		Entity thisArg = null;
		if (container != null && with) {
			thisArg = container.getContainer();
			if (! (thisArg instanceof Aggregate)) thisArg = null;
		}
		
		Type returnType = routine.getReturnType();
		Type touchupReturnType = null;
		String wrapperName = "routine_" + uid(routine) 
			+ (with ? "r" : "s") + nArguments;
		if (routine.isConstructor()) {
			// This is a constructor
			m_output.write(thisArg.getFullName());
			m_output.write(" *" + wrapperName);
		}
		else {
			String declarator = wrapperName;
			if (isPrimitive(returnType.getBaseType())) {
				returnType = dereference(returnType);
			}
			touchupReturnType = Filters.getTouchup(returnType);
			if (touchupReturnType != null) {
				returnType = touchupReturnType;
			}
			else if (needsExtraReferencing(returnType)) {
				declarator = "*" + declarator;
			}
			m_output.write(Utils.cleanFormatCpp(returnType,declarator));
		}
		
		// Construct parameters fitting for the flat purpose
		int paramCount = 0;
		boolean first = true;
		m_output.write("(");
		if (thisArg != null && !routine.isConstructor()) {
			// - generate a *this
			if (routine.isConst()) m_output.write("const ");
			m_output.write(thisArg.getFullName());
			m_output.write(" *self");
			first = false;
		}
		for (Iterator pi = routine.parameterIterator(); 
				pi.hasNext() && paramCount < nArguments; paramCount++) {
			Parameter param = (Parameter)pi.next();
			if (!first) m_output.write(" , ");
			// Get type of parameter, remove pointer/reference
			Type type = flatUnalias(param.getType());
			
			if (type.isFlat() && type.getBaseType() != null) {
				Entity base = type.getBaseType();
				if (type.isConst()) m_output.write("const ");
				writeFlatBase(type);
				m_output.write(" ");
				// express pointers
				int pointers = type.getPointerDegree();
				boolean reference = type.isReference();
				for (int ptri = 0; ptri < pointers; ++ptri) m_output.write("*");
				if (needsExtraReferencing(type)) m_output.write("*");
				if (reference && !isSmallPrimitive(base)) m_output.write("&");
				// write a fictive argument name
				m_output.write("arg" + paramCount);
			}
			else {
				m_output.write("<error>");
			}
			first = false;
		}
		m_output.write(")\n");
		m_output.flush();
		
		// Generate the body
		int parenNest = 0;
		m_output.write("{\n\t");
		if (routine.isConstructor()) {
			m_output.write("return new ");
			m_output.write(thisArg.getFullName());
		}
		else {
			// - write 'return' where needed
			if (needsExtraReferencing(returnType)) {
				m_output.write("return new ");
				m_output.write(Utils.cleanFormatCpp(returnType,""));
				m_output.write("("); parenNest++;
			}
			else if (Filters.needsReturnStatement(returnType)) {
				m_output.write("return ");
				// - add touchup if necessary
				if (touchupReturnType != null) {
					m_output.write("touchup("); parenNest++;
				}
			}
			// - write function name to call
			if (thisArg != null)
				if (routine.isConversionOperator())
					m_output.write(conversionOperatorSyntax(routine, "self"));
				else if (routine.getName().equals("operator==") || 
						  routine.getName().equals("operator!="))
					m_output.write("*self " +    // @@@ STL issue workaround
							routine.getName().substring("operator".length()));
				else
					m_output.write("self->" + routine.getName());
			else
				m_output.write(routine.getFullName());
		}
		// - generate call parameters
		if (!routine.isConversionOperator()) {
			paramCount = 0;
			first = true;
			
			m_output.write("("); parenNest++;
			
			for (Iterator pi = routine.parameterIterator(); 
					pi.hasNext() && paramCount < nArguments; paramCount++) {
				Parameter param = (Parameter)pi.next();
				// Get parameter attributes
				Type type = flatUnalias(param.getType());
				// Print name of argument
				if (!first) m_output.write(", ");
				// If this is one of the "special" primitives, touchdown
				if (needsExtraReferencing(type) &&
				    isPrimitive(type.getBaseType()) &&
				    !isSmallPrimitive(type.getBaseType())) {
					m_output.write("touchdown(arg" + paramCount + ")");
				}
				// If it is another type that needs dereferencing
				else if (needsExtraReferencing(type)) {
					m_output.write("*arg" + paramCount);
				}
				// If this is just a normal type
				else {
					m_output.write("arg" + paramCount);
				}
				first = false;
			}
		}
		while (parenNest > 0) {
			m_output.write(")");
			parenNest--;
		}
		m_output.write(";\n}\n");
		m_output.flush();
	}
	
	/**
	 * Generates constant integers which contain the values for each of the
	 * enumerated constants.
	 * @param enume enumerated type for which to generate wrapper
	 * @throws IOException if an output error occurs.
	 */
	private void generateFlatWrapper(sourceanalysis.Enum enume) throws IOException
	{
		ContainedConnection uplink = enume.getContainer();
		m_output.write("/*\n * enum "); m_output.write(enume.getFullName());
		m_output.write("\n */\n");
		// Write the enumerated constants
		for (Iterator ci = enume.constantIterator(); ci.hasNext(); ) {
			sourceanalysis.Enum.Constant constant = (sourceanalysis.Enum.Constant)ci.next();
			m_output.write("int const_" + constant.hashCode());
			m_output.write(" = (int)");
			if (uplink != null) {
				m_output.write(uplink.getContainer().getFullName());
				m_output.write("::");
			}
			m_output.write(constant.getLiteral());
			m_output.write(";\n");
		}
		m_output.flush();
	}
	
	/**
	 * Generate routines for typedef - aliasing/unaliasing of types. 
	 * @param alias
	 * @throws IOException
	 */
	private void generateFlatWrapper(Alias alias) throws IOException
	{
		m_output.write("/*\n * typedef ");
		m_output.write(alias.getFullName());
		m_output.write("\n */\n");
		// Create a constructor from aliased type
		m_output.write(alias.getFullName());
		m_output.write(" *routine_alias_" + uid(alias));
		m_output.write("(");
		m_output.write(alias.getAliasedType().formatCpp("arg0"));
		m_output.write(")\n{\n\treturn new " + alias.getFullName());
		m_output.write("(arg0);\n}\n");
		// Create an access method to retrieve stored type
		Type realType = alias.getAliasedType();
		Type touchupType = Filters.getTouchup(realType);
		m_output.write((touchupType == null ? realType : touchupType)
			.formatCpp("routine_unalias_" + uid(alias)));
		m_output.write("(");
		m_output.write(alias.getFullName());
		if (touchupType == null)
			m_output.write(" *self) { return *self; }\n");
		else
			m_output.write(" *self) { return touchup(*self); }\n");
		// Create a destructor
		m_output.write("void dtor_alias_" + uid(alias));
		m_output.write("(");
		m_output.write(alias.getFullName());
		m_output.write(" *self) { delete self; }\n");
	}
	
	/**
	 * Generates an accessor function for a program variable.
	 * @param field
	 * @param with if <b>true</b> - the generated code will provide an
	 * instance invocation (self-&gt;) when applicable. If <b>false</b> - the
	 * field will be treated as a global variable or a public static member.
	 * @throws IOException
	 * @throws MissingInformationException
	 */
	private void generateFlatWrapper(Field field, boolean with)
		throws IOException, MissingInformationException
	{
		String fors = with ? "f" : "s";
		m_output.write("/*\n * var ");
		m_output.write(field.getFullName());
		if (field.getDeclaration() != null)
			m_output.write(" from " + field.getDeclaration().getSourceFilename());
		m_output.write("\n */\n");
		// Get some information about the type
		Type type = removeUnneededConstness(field.getType());
		Entity base = type.getBaseType();
		Type touchupType = type.getRootNode().getKind() == Type.TypeNode.NODE_LEAF 
			? Filters.getTouchup(new Type(new Type.TypeNode(base))) : null;
		type = touchupType == null ? type : touchupType;
		// Create a get accessor
		Entity container = field.getContainer().getContainer();
		String accessorName = "data_get_" + uid(field) + fors;
		String thisArg = container instanceof Aggregate ? 
			container.getFullName() + " *self" : "";
		// - generate accessor function header
		String header = accessorName + "(" + thisArg + ")";
		m_output.write(type.formatCpp(
			needsExtraReferencing(type) 
				? "(&" + header + ")" : header 
			));
		// - generate accessor function body
		m_output.write(" { return ");
		if (touchupType != null) { m_output.write("touchup("); }
		if (container instanceof Aggregate)
			m_output.write("self->" + field.getName());
		else
			m_output.write(Utils.cleanFullName(field));
		if (touchupType != null) m_output.write(")");
		m_output.write("; }\n");
		// Create a set accessor
		if (Filters.hasSetter(field)) {
			accessorName = "data_set_" + uid(field) + fors;
			// - generate accessor function header
			header = accessorName + "(" + thisArg + ", " +
				base.getFullName() + " newval)";
			m_output.write("void " + header);
			// - generate accessor function body
			m_output.write("{ ");
			if (container instanceof Aggregate)
				m_output.write("self->" + field.getName());
			else
				m_output.write(Utils.cleanFullName(field));
			m_output.write(" = newval; }\n");
			// - generate regdata
			if (base instanceof Alias)
				base = ((Alias)base).getAliasedType().getBaseType();
			m_output.write("RegData sink_" + uid(field) + fors + "_proto[] = {\n");
			m_output.write("{\"newval\", \"" + base.getFullName() + "\" ,0,0},\n");
			m_output.write(END_OF_LIST);
		}
	}
	
	/**
	 * Generates an array of pointers which describes the function.
	 * @param routine routine being described for registration
	 * @param nArguments see generateFlatWrapper(Routine,int,boolean)
	 */
	private void generateRegistrationPrototype(Routine routine, int nArguments, 
			boolean with) throws IOException, MissingInformationException
	{
		m_output.write("RegData routine_" + uid(routine) 
			+ (with ? "r": "s") + nArguments + "_proto[] = {\n");
		// Go through arguments but not more than nArguments entries
		int argCount = 0;
		for (Iterator pi = routine.parameterIterator(); 
				argCount < nArguments && pi.hasNext(); ++argCount) {
			Parameter parameter = (Parameter)pi.next();
			// Write name
			m_output.write("\t{\"");
			m_output.write(parameter.getName());
			m_output.write("\", ");
			// Write type
			// - get attributes
			Type type = parameter.getType();
			Entity base = type.getBaseType();
			int pointers = type.getPointerDegree();
			boolean reference = type.isReference();
			boolean output = Filters.isOutputParameter(parameter);
			// - begin name
			m_output.write("\"");
			if (!(base instanceof Primitive)) {
				// - write references
				if (output) m_output.write(">");
				else if (reference) m_output.write("&");
				// - write pointers
				for (int ptr = 0; ptr < pointers; ++ptr) m_output.write("*");
				// - express extra referencing
				if (needsExtraReferencing(type)) m_output.write("&");
			}
			// - special care for char *
			if (base instanceof Primitive && base.getName().equals("char")) {
				if (pointers > 0) m_output.write("*");
			}
			// - write base type name
			writeDecoratedFlatBase(type);
			m_output.write("\", 0},\n");
		}
		m_output.write(END_OF_LIST);
		m_output.flush();
	}

	/**
	 * Generates an array of integral values which describes the enumerated
	 * type.
	 * @param enume enumerated type to describe
	 */
	private void generateRegistrationPrototype(sourceanalysis.Enum enume)
		throws IOException
	{
		m_output.write("RegData enumerated_" + enume.hashCode() + "[] = {\n");
		// Write the enumerated constants
		for (Iterator ci = enume.constantIterator(); ci.hasNext(); ) {
			sourceanalysis.Enum.Constant constant = (sourceanalysis.Enum.Constant)ci.next();
			m_output.write("\t{ \"");
			m_output.write(constant.getLiteral());
			m_output.write("\", 0, 0, (void*)&const_"
				+ constant.hashCode());			
			m_output.write(" },\n");
		}
		m_output.write(END_OF_LIST);
		m_output.flush();
	}
	
	/**
	 * Generates a proxy class registration prototype for an encapsulated
	 * typedef.
	 * @param alias typedef element to describe
	 * @throws IOException if an output error occurs
	 * @see needsEncapsulation()
	 */
	private void generateRegistrationPrototype(Alias alias)
		throws IOException
	{
		// Register creator
		m_output.write("RegData routine_alias_" + uid(alias) + "_proto[] = {\n");
		m_output.write("\t{\"value\", \"" 
			+ Utils.cleanFullName(alias.getAliasedType().getBaseType()));
		m_output.write("\", 0},\n");
		m_output.write(END_OF_LIST);
		// Register accessor
		m_output.write("RegData routine_unalias_" + uid(alias) 
			+ "_proto[] = {\n");
		m_output.write(END_OF_LIST);
		// Register proxy class
		m_output.write("RegData alias_" + uid(alias) + "[] = {\n");
		// - ctor
		m_output.write("\t{ \"^\", \"constructor\", routine_alias_" 
			+ uid(alias) + "_proto, (void*)&routine_alias_"
			+ uid(alias) + "},\n");
		// - accessor
		m_output.write("\t{ \"as\", \""
			+ Utils.cleanFullName(alias.getAliasedType().getBaseType()));
		m_output.write("\", routine_unalias_" + uid(alias)
			 + "_proto, (void*)&routine_unalias_" + uid(alias) + "},\n");
		// - dtor
		m_output.write("\t{ \".\", \"destructor\", 0, (void*)&dtor_alias_" + uid(alias));
		m_output.write(" },\n");
		m_output.write(END_OF_LIST);
	}
	
	/**
	 * Generates a compiler-generated default constructor wrapper for a class.
	 * @param subject
	 * @throws IOException
	 */
	private void 
		generateSpecialDefaultConstructorWrapperAndPrototype(Aggregate subject)
		throws IOException
	{
		m_output.write(Utils.cleanFullName(subject));
		m_output.write(" *ctor_" + uid(subject.getScope()));
		m_output.write("() { return new ");
		m_output.write(Utils.cleanFullName(subject));
		m_output.write("; }\n");
	}

	private void generateSpecialAssignmentOperator(Aggregate subject)
		throws IOException
	{
		String classname = Utils.cleanFullName(subject);
		
		m_output.write("void assign_" + uid(subject.getScope()));
		m_output.write("(");
		m_output.write(classname + " *self, ");
		m_output.write(classname + " *other)");
		m_output.write(" { *self = *other; }\n");
		m_output.write("RegData assign_" + uid(subject.getScope())
			+ "_proto[] = {\n");
		m_output.write("\t{\"other\", \"*" + classname + "\", 0, 0},\n");
		m_output.write(END_OF_LIST);
	}
	
	private void generateSpecialCloneMethod(Aggregate subject)
		throws IOException
	{
		String classname = Utils.cleanFullName(subject);
		
		m_output.write(classname + " *clone_" + uid(subject.getScope()));
		m_output.write("(" + classname + " *self) { return ");
		m_output.write(" new " + classname + "(*self); }\n");
	}
	
	/**
	 * Generates a destructor wrapper for a class.
	 * @param subject
	 * @throws IOException
	 */
	private void generateSpecialDestructor(Aggregate subject)
		throws IOException
	{
		m_output.write("void dtor_" + uid(subject.getScope()));
		m_output.write("(" + Utils.cleanFullName(subject) + " *self)");
		m_output.write(" { delete self; }\n");
	}
	
	/**
	 * Writes an output operator wrapper - currently prints to std::cerr.
	 * @param subject class for which to generate output wrapper.
	 * @throws IOException
	 */
	private void generateSpecialOutputOperator(Aggregate subject)
		throws IOException
	{
		m_output.write("#ifndef IOSTREAM_INCLUDED\n");
		m_output.write("#define IOSTREAM_INCLUDED\n");
		m_output.write("#include <iostream>\n");
		m_output.write("#endif\n");
		m_output.write("void output_" + uid(subject.getScope()));
		m_output.write("(");
		m_output.write(Utils.cleanFullName(subject));
		m_output.write(" *self) { std::cerr << *self; }\n\n");
	}
	
	/**
	 * Writes a suggested conversion to string - by outputting object to
	 * a std::stringstream and acquiring the resulting string.
	 * @param subject class for which to generate string conversion
	 * @throws IOException
	 */
	private void generateSpecialStringConverter(Aggregate subject)
		throws IOException
	{
		// Include STL's stringstream
		m_output.write("#ifndef SSTREAM_INCLUDED\n");
		m_output.write("#define SSTREAM_INCLUDED\n");
		m_output.write("#include <sstream>\n");
		m_output.write("#endif\n");
		// Define conversion to Pascal string
		m_output.write("#ifndef PASCALSTRING_CTOR_INCLUDED\n");
		m_output.write("#define PASCALSTRING_CTOR_INCLUDED\n");
		m_output.write("inline struct PascalString *toPascal(const std::string& cpp)\n");
		m_output.write("{ unsigned long size = (unsigned long)cpp.size();\n");
		m_output.write("  PascalString *pascal = (PascalString*)\n");
		m_output.write("    malloc(sizeof(PascalString) + size);\n");
		m_output.write("  pascal->size = size; pascal->chars = pascal->buffer;\n");
		m_output.write("  memcpy(pascal->buffer, cpp.c_str(), size);\n");
		m_output.write("  return pascal;\n}\n");
		m_output.write("#endif\n");
		// Write operator
		m_output.write("struct PascalString *toString_" 
						+ uid(subject.getScope()));
		m_output.write("(");
		m_output.write(Utils.cleanFullName(subject));
		m_output.write(" *self) { std::stringstream ss; ss << *self;\n");
		m_output.write(" return toPascal(ss.str()); }\n\n");
	}
	
	/**
	 * Generates up-cast and down-cast wrappers for a class.
	 * 
	 * @param subject class
	 * @throws IOException if output operation fails
	 * @throws MissingInformationException 
	 */
	private void generateUpDownCastFlatWrappers(Aggregate subject)
		throws IOException, MissingInformationException
	{
		for (Iterator baseIter = subject.baseIterator(); baseIter.hasNext();) {
			InheritanceConnection connection =
				(InheritanceConnection)baseIter.next();
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC) {
				 Aggregate base = (Aggregate)connection.getBase();
				 String basename = "";
					 
				 if (base.isTemplated()) {
				 	basename = Utils.templateCppExpression(base, 
				 		connection.getBaseTemplateArguments());
				 }
				 else {
				 	basename = Utils.cleanFullName(base);
				 }
				 
				 String derivedname = Utils.cleanFullName(subject);
				 String derived2base = uid(subject.getScope()) + "_to_" + uid(base.getScope());
				 String base2derived = uid(base.getScope()) + "_to_" + uid(subject.getScope());
				 
				 m_output.write(
						 basename + " *upcast_" + derived2base
						 + "(" + derivedname + " *self) { return self; }\n");
				 
				 if (Utils.isPolymorphic(base)) {
					 m_output.write(
							 derivedname + " *downcast_" + base2derived
							 + "(" + basename + " *self)");
					 m_output.write(" { return dynamic_cast<" + derivedname
							 + "*>(self); }\n");
					 m_output.write(
							 "RegData downcast_" + base2derived + "_proto[] = {\n" 
							 + "\t{\"arg0\", \"*" + basename + "\", 0, 0},\n"
							 + END_OF_LIST);
					 m_downCasters.add("\"dynamic_cast< " + derivedname 
							 + " >\", "
							 + "\"&" + derivedname + "\", "
							 + "downcast_" + base2derived + "_proto, "
							 + "(void*)&downcast_" + base2derived);
				 }
			}
		}
	}
	
	/**
	 * Writes a RegData entry which can be included in a scope.
	 * @param routine routine for which to create line
	 * @param skip indicates how many parameters to ignore
	 * @param with indicates whether this method is with an instance, or without an instance (static)
	 * @throws IOException if output operation fails
	 * @throws MissingInformationException if the program database is 
	 * incomplete.
	 */
	private void generateRegistrationLine(Routine routine, int nSkip, boolean with) 
		throws IOException, MissingInformationException
	{
		int minArgs = Utils.minimalArgumentCount(routine),
			maxArgs = Utils.countParameters(routine);
		for (int nArguments = minArgs; nArguments <= maxArgs; 
				++nArguments) {
			if (routine.isConstructor()) {
				char symbol = routine.isExplicitConstructor() ? '%' : '*';
				m_output.write("{ \"" + symbol + "\" , \"constructor\", ");
			}
			else {
				// Write name
				Entity container = routine.getContainer().getContainer();
				m_output.write("{\"");
				m_output.write((with && container instanceof Aggregate) 
								? routine.getName() : Utils.cleanFullName(routine));
				m_output.write("\", ");
				// Write return type
				char ref = '&';
				char ptr = Filters.isForceBorrowed(routine) ? '&' : '*';
				writeTypeSimply(routine.getReturnType(), ref, ptr, '*');
				m_output.write(", ");
			}
			// Write pointer to prototype
			String wrapperName = "routine_" + uid(routine) 
				+ (with ? "r" : "s") + nArguments;
			m_output.write(wrapperName + "_proto+" + nSkip + ", ");
			// Write pointer to implementation, unless this is a pure virtual method
			if (m_interceptorMethods.contains(routine)) {
				m_output.write("0},\n");
			}
			else {
				m_output.write("(void*)&" + wrapperName);
				m_output.write("},\n");
			}
		}
	}
	
	private void generateRegistrationLine(Field field, boolean with)
		throws IOException, MissingInformationException
	{
		String fors = with ? "f" : "s";
		String identifier =
			(field.getContainer().getContainer() instanceof Aggregate && with) ?
			field.getName() : Utils.cleanFullName(field);
		// - data_get
		m_output.write("{ \".data_" + identifier);
		m_output.write("\", ");
		writeTypeSimply(field.getType(), '&', '&', '&');
		m_output.write(", 0, (void*)&data_get_" + uid(field) + fors);
		m_output.write(" },\n");
		// - data_set
		if (Filters.hasSetter(field)) {
			m_output.write("{ \".sink_" + identifier);
			m_output.write("\", \"void\", sink_" + uid(field) + fors + "_proto");
			m_output.write(", (void*)&data_set_" + uid(field) + fors);
			m_output.write(" },\n");
		}
	}
	
	/**
	 * Generates an array of pointers describing the routines contained in
	 * a scope.
	 * @param scope scope containing routines
	 * @throws IOException if output operation fails 
	 */
	public void generateRegistrationPrototype(Scope scope, String classname,
		Iterator basesIterator, boolean isAbstractClass,
		boolean mustHaveCtor, boolean hasAssign, boolean hasClone,
		boolean hasOutput, boolean hasDtor, boolean with, List additional)
		throws IOException, MissingInformationException
	{
		m_output.write("RegData scope_" + scope.hashCode() + "[] = {\n");
		// Go through bases
		if (basesIterator != null) {
			for (; basesIterator.hasNext(); ) {
				InheritanceConnection connection =
					(InheritanceConnection)basesIterator.next();
				if (connection.getVisibility() == Specifiers.Visibility.PUBLIC) {
					Aggregate base = connection.getBase();
					m_output.write("{\"");
					m_output.write(Utils.actualBaseName(connection));
					m_output.write("\", \"extends\", 0, (void*)&upcast_"
						+ uid(scope) + "_to_" + uid(base.getScope()));
					m_output.write("},\n");
				}
			}
		}
		// Go through routines in this scope
		boolean ctors = false;
		for (Iterator routineIter = scope.routineIterator(); 
				routineIter.hasNext(); ) {
			ContainedConnection connection =
				(ContainedConnection)routineIter.next();
			// Create an entry for each public routine
			Routine routine = (Routine)connection.getContained();
			if (routine.isConstructor() && isAbstractClass) continue;
			if (connection.getVisibility() == Specifiers.Visibility.PUBLIC
				&& !routine.isDestructor() 
				&& Filters.isAvailable(routine)) {
				generateRegistrationLine(routine, 0, with);
				ctors |= routine.isConstructor();
			}
		}
		// Go through data members in this scope
		for (Iterator fieldIter = scope.fieldIterator();
				fieldIter.hasNext(); ) {
			ContainedConnection connection =
				(ContainedConnection)fieldIter.next();
			// Create an entry for public data members
			Field field = (Field)connection.getContained();
			if (Filters.isAvailable(field, connection)) 
				generateRegistrationLine(field, true);
		}
		// More special stuff
		if (!ctors && mustHaveCtor && !isAbstractClass) {
			m_output.write("{ \"*\", \"constructor\", 0, (void*)&ctor_"
				+ uid(scope));
			m_output.write("},\n");
		}
		if (hasAssign) {
			m_output.write("{ \"operator=\", \"void\", assign_" 
					+ uid(scope) + "_proto, (void*)&assign_" + uid(scope)
					+ "},\n");
		}
		if (hasClone) {
			m_output.write("{ \"clone\", \"*" + classname + "\", 0, "
					+ "(void*)&clone_" + uid(scope));
			m_output.write("},\n");
		}
		if (hasOutput) {
			m_output.write("{ \".print\", \"void\", 0, (void*)&output_"
				+ uid(scope));
			m_output.write("},\n");
			m_output.write("{ \".string\", \"@string\", 0, (void*)&toString_"
				+ uid(scope));
			m_output.write("},\n");
		}
		if (hasDtor) {
			m_output.write("{ \".\", \"destructor\", 0, (void*)&dtor_"
				+ uid(scope));
			m_output.write("},\n");
		}
		for (Iterator addi = additional.iterator(); addi.hasNext(); ) {
			Routine routine = (Routine)addi.next();
			if (Filters.isAvailable(routine))
				generateRegistrationLine(routine, 1, false);
		}
		m_output.write(END_OF_LIST);
	}

	/**
	 * (internal) Extracts the declaration position of an entity.
	 * If the declaration cannot be found, a warning is printed
	 * to m_output (commented out, of course).
	 * This is used as an auxiliary function by generateIncludeDirectives.
	 * @throws IOException
	 */
	private SourceFile.DeclDefConnection safeGetDeclaration(Entity entity)
		throws IOException
	{
		SourceFile.DeclDefConnection decl = entity.getDeclaration();
		if (decl == null) {
			m_output.write("// " + entity.getFullName() + 
				": location specification may be missing?\n");
		}
		return decl;
	}
	
	/**
	 * Writes a type in as a simple string for a registration item.
	 * @param type
	 * @param refOperator may be either '*' or '&amp;', and is prepended
	 * to the type in case it is a reference
	 * @param ptrOperator may be either '*' or '&amp;', and is prepended 
	 * to the type in case it is a pointer
	 * @param extraOperator may be either '*' or '&amp;', and is prepended
	 * to the type in case it needs extra referencing (see the
	 * method needsExtraReferencing()).
	 */
	private void writeTypeSimply(Type type, 
		char refOperator, char ptrOperator, char extraOperator) 
		throws IOException
	{
		m_output.write("\"");
		if (type.isReference() && !isSmallPrimitive(type.getBaseType()))
			m_output.write(refOperator);
		else if (type.getPointerDegree() > 0)
			m_output.write(ptrOperator);
		else if (needsExtraReferencing(type)
			&& !(type.getBaseType() instanceof Primitive))
			m_output.write(extraOperator);
		if (type.getRoot() != null)
			writeDecoratedFlatBase(type);
		m_output.write("\"");
	}
	
	/**
	 * Removes any reference notations from a type expression.
	 * e.g. "const int&" converts to "const int". 
	 * @param type original type expression
	 * @return a new type expression without a reference
	 */
	/* package */ Type dereference(Type type)
	{
		Type.TypeNode root = type.getRootNode();
		// Descend until node is not a reference
		while (root.getKind() == Type.TypeNode.NODE_REFERENCE) {
			root = (Type.TypeNode)root.getFirstChild();
		}
		return new Type(root);
	}
	
	/**
	 * Resolves typedefs for flat types in a way similar to Utils.flatUnalias,
	 * with one difference that encapsulated aliases are not resolved.
	 * @param type original type expression
	 * @return resolved type expression (may be the original type if no
	 * resolution takes place).
	 */
	/* package */ Type flatUnalias(Type type)
	{
		Entity base = type.getBaseType();
		if (!type.isReference() &&type.getPointerDegree() == 0 
				&& base instanceof Alias && !needsEncapsulation((Alias)base))
			return ((Alias)base).getAliasedType();
		else
			return type;
	}
	
	/**
	 * Generates invocation of a conversion operator.
	 * @param op conversion operator routine
	 * @param self object being converted
	 * @return a C++ expression which can be used to invoke the conversion
	 * @throws MissingInformationException if the conversion operator is 
	 * somehow incomplete.
	 */
	/* package */ String conversionOperatorSyntax(Routine op, String self)
		throws MissingInformationException
	{
		return "CONVOP("+op.getReturnType().formatCpp()+"," + self + ")"; 
	}

	/**
	 * Removes alleged redundant 'const' notations that occur when the
	 * type is merely a primitive with no pointer/reference - thus constness
	 * makes no difference.
	 *  
	 * @param type given type
	 * @return a new modified type - or the original type if no changes
	 * were carried out
	 */
	/* package */ Type removeUnneededConstness(Type type) {
		Type.TypeNode root = type.getRootNode();
	
		if (root.getKind() == Type.TypeNode.NODE_LEAF) {
			try {
				Entity base = root.getBase();
				if (base instanceof Primitive 
					&& (root.getCV() | Specifiers.CVQualifiers.CONST) != 0) {
					type = new Type(new Type.TypeNode(base));
				}
			}
			catch (InappropriateKindException e) {
				// .. ignore
			}
		}
		return type;
	}
	
	/**
	 * @name Reporting
	 */
	//@{
	
	/**
	 * Reports the names of the classes which were successfully wrapped and
	 * registered.
	 */
	public void report() {
		// Print header
		System.out.println("=================================");
		System.out.println("| Registered classes:");
		// Print subjects
		for (Iterator subjectIter = m_subjects.iterator(); 
			subjectIter.hasNext(); ) {
			// - print name
			Entity subject = (Entity)subjectIter.next();
			System.out.println("|   " + subject.getFullName());
		}
		// Print footer
		System.out.println("=================================");
	}

	// Private members
	private Map m_uidMap;
	private int m_uidNext;
	private List m_globalDataMembers;
	private List m_downCasters;

	private List m_interceptors;
	private Set m_interceptorMethods;
	
	// Code skeletons
	private static final String END_OF_LIST = "\t{ 0 }\n};\n\n";
}
