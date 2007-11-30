package backend.robin;

import java.util.List;

import backend.Utils;
import backend.robin.model.CppExpression;
import backend.robin.model.RoutineDeduction;
import backend.robin.model.TypeToolbox;
import backend.robin.model.RoutineDeduction.ParameterTransformer;
import sourceanalysis.*;
import sourceanalysis.SourceFile.DeclDefConnection;

public class Formatters {

	static String formatParameter(Parameter param, String name)
			throws MissingInformationException
	{
		return RoutineDeduction.deduceParameterTransformer(param)
				.getPrototypeType().formatCpp(name);
	}
	
	static String formatArgument(Parameter param, String name)
			throws MissingInformationException 
	{
		return RoutineDeduction.deduceParameterTransformer(param).getBodyExpr()
				.evaluate(name);
	}

	static String formatParameters(List<ParameterTransformer> params)
	{
		String[] fmtd = new String[params.size()];
		for (int i = 0; i < fmtd.length; i++) {
			fmtd[i] = params.get(i).getPrototypeType().formatCpp("arg"+i);
		}
		return join(", ", fmtd);
	}
	
	static String formatArguments(List<ParameterTransformer> params)
	{
		String[] fmtd = new String[params.size()];
		for (int i = 0; i < fmtd.length; i++) {
			fmtd[i] = params.get(i).getBodyExpr().evaluate("arg"+i);
		}
		return join(", ", fmtd);
	}
	
	static String formatFunction(Entity routine, String name, 
			List<ParameterTransformer> params, CppExpression semantic)
	{
		String doc = formatDocBlock(routine);
		String header = "void " + name + "(" + formatParameters(params) + ")"; 
		String body = semantic.evaluate(formatArguments(params));
		String proto = formatRegData(name, params);
		return doc + header + "\n{\n\t" + body + ";\n}\n" + proto + "\n";
	}
	
	static String formatRegData(String name, List<ParameterTransformer> params)
	{
		StringBuffer buf = new StringBuffer();
		buf.append("RegData " + name + "_proto[] = {\n");
		for (int i = 0; i < params.size(); ++i) {
			String pname = "arg" + i;
			String ptype = params.get(i).getRegDataType();
			buf.append("\t{ \"" + pname + "\", \"" + ptype + "\", 0, 0 },\n");
		}
		buf.append("\t{ 0 }\n};\n");
		return buf.toString();
	}
	
	static String formatDocBlock(Entity entity)
	{
		return "/*\n" +
		       " * " + entity.getFullName() + "\n" +
		       " * " + formatLocation(entity) + "\n" +
		       " */\n";
	}
	
	private static String join(String sep, String[] items) {
		StringBuffer buf = new StringBuffer();
		for (int i = 0; i < items.length; i++) {
			if (i > 0) buf.append(sep);
			buf.append(items[i]);
		}
		return buf.toString();
	}

	static String formatArgument(Type type, String name) 
	{
		if (Filters.needsExtraReferencing(type))
			return "touchdown(" + name + ")";
		else
			return name;
	}
	
	static String formatLocation(Entity entity)
	{
		DeclDefConnection declaration = entity.getDeclaration();
		if (declaration == null) {
			return "location unknown";
		}
		else {
			return "from " + declaration.getSourceFilename() 
			       + ":" + declaration.where().getStart().line;
		}
	}
	
	static String formatMember(Field field, boolean hasThis)
	{
		if (field.getContainer() instanceof Aggregate) {
			if(hasThis) {
				return "self->" + field.getName();
			} else {
				return field.getName();
			}
		}
		else
			return Utils.cleanFullName(field);
	}
	
	static String formatExpression(Type type, String expression)
	{
		// Handle redundant referencing
		if (type.isReference()
				&& Filters.isPrimitive(type.getBaseType())) {
			type = TypeToolbox.dereference(type);
		}
		Type touchupType = Filters.getTouchup(type);
		if (touchupType == null)
			return expression;
		else
			return "touchup(" + expression + ")";
	}
	
	static String formatDeclaration(Type type, String name, char extraRefScheme, boolean isForFunction) {
		return formatDeclaration(type, name, extraRefScheme, false, isForFunction);
	}
	static String formatDeclaration(Type type, String name, char extraRefScheme, boolean wrappingInterceptor, boolean isForFunction)
	{
		
		if(!wrappingInterceptor) {
			// Handle redundant referencing
			if (Filters.getOriginalType(type).isReference()
						&& Filters.isPrimitive(type.getBaseType())) {
					type = TypeToolbox.dereference(Filters.getOriginalType(type));
			}
		}

		// Handle touch-up
		Type touchupType = Filters.getTouchup(Utils.flatUnalias(type));
		if (touchupType != null) type = touchupType;

        // This needs to be done before extra referencing is handled, because
        // __CDECL must not split the type
        if(isForFunction) {
                name = " __CDECL " + name;
        }
		// Handle extra referencing
		if (Filters.needsExtraReferencing(type) && !wrappingInterceptor) {
			name = extraRefScheme + name;
		}
	    return type.formatCpp(name);
		
	}

}
