package backend.robin;

import backend.Utils;
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

	static String formatParameters(ParameterTransformer[] params) throws MissingInformationException
	{
		String[] fmtd = new String[params.length];
		for (int i = 0; i < fmtd.length; i++) {
			fmtd[i] = params[i].getPrototypeType().formatCpp("arg"+i);
		}
		return join(", ", fmtd);
	}
	
	static String formatArguments(ParameterTransformer[] params)
	{
		String[] fmtd = new String[params.length];
		for (int i = 0; i < fmtd.length; i++) {
			fmtd[i] = params[i].getBodyExpr().evaluate("arg"+i);
		}
		return join(", ", fmtd);
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
		// Handle extra referencing
		if (Filters.needsExtraReferencing(type) && !wrappingInterceptor) {
			name = extraRefScheme + name;
		}
		if(isForFunction) {
			// function-level formatting needs the appropriate cdecl
			return type.formatCpp("__CDECL " + name);
		} else {
			return type.formatCpp(name);
		}
		
	}

}
