package backend.robin;

import java.util.List;

import backend.Utils;
import backend.robin.model.CppExpression;
import backend.robin.model.ElementKind;
import backend.robin.model.RegData;
import backend.robin.model.RoutineDeduction;
import backend.robin.model.SimpleType;
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
			ParameterTransformer ret,
			List<ParameterTransformer> params, CppExpression semantic)
	{
		String doc = (routine == null) ? "" : formatDocBlock(routine);
		String header = ret.getPrototypeType().formatCpp(name) + "(" + formatParameters(params) + ")"; 
		String body = ret.getBodyExpr().evaluate(
				semantic.evaluate(formatArguments(params)));
		String proto = formatRegData(name, params);
		return doc + header + "\n{\n\t" + body + ";\n}\n" + proto + "\n";
	}
	
	static String formatSimpleType(SimpleType stype)
	{
		String typespec = (stype.base instanceof sourceanalysis.Enum) ? "#" : "";
		return stype.redir + typespec + Utils.cleanFullName(stype.base)
				+ formatTemplateArguments(stype.templateArgs);
	}
	
	static String formatRegData(String name, List<ParameterTransformer> params)
	{
		StringBuffer buf = new StringBuffer();
		buf.append("RegData " + name + "_proto[] = {\n");
		for (int i = 0; i < params.size(); ++i) {
			String pname = "arg" + i;
			String ptype = formatSimpleType(params.get(i).getRegDataType());
			buf.append("\t{ \"" + pname + "\", \"" + ptype + "\", 0, 0 },\n");
		}
		buf.append("\t{ 0 }\n};\n");
		return buf.toString();
	}
	
	static String formatRegData(RegData regData)
	{
		String type = formatSimpleType(regData.type); 
		String symbol = (regData.symbol == null) ? "0" : "(void*)&" + regData.symbol;
		return "\t{ \"" + regData.name + "\", \"" + type + "\", "
				+ regData.proto + ", " + symbol + "},";
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
	
	static String formatDeclaration(Type type, String name, ElementKind kind, boolean isForFunction)
		throws MissingInformationException
	{
		return formatDeclaration(type, name, kind, false, isForFunction);
	}

	static String formatDeclaration(Type type, String name, ElementKind kind, boolean wrappingInterceptor, boolean isForFunction)
			throws MissingInformationException
	{
		
		if(!wrappingInterceptor) {
			// Handle redundant referencing
			if (TypeToolbox.getOriginalType(type).isReference()
						&& Filters.isPrimitive(type.getBaseType())) {
				type = TypeToolbox.dereference(TypeToolbox.getOriginalType(type));
			}
		}

		RoutineDeduction.ParameterTransformer retf =
			RoutineDeduction.deduceReturnTransformer(type, kind);
		return retf.getPrototypeType().formatCpp(name);
	}

	/**
	 * Formats a list of template arguments in the form "< A,B,C >".
	 *  
	 * @param templateArgs an array of template arguments
	 * @return a formatted string
	 */
	static String formatTemplateArguments(TemplateArgument[] templateArgs)
	{
		StringBuffer fmtd = new StringBuffer();
		
		if (templateArgs != null) {
			fmtd.append("< ");
			for (int i = 0; i < templateArgs.length; i++) {
				TemplateArgument templateArgument = templateArgs[i];
				if (i > 0) fmtd.append(",");					
				if (templateArgument == null) fmtd.append("?");	
				else if (templateArgument instanceof TypenameTemplateArgument)
					fmtd.append(Utils.cleanFormatCpp(
						((TypenameTemplateArgument)templateArgument)
						.getValue(),""));
				else fmtd.append(templateArgument.toString());
			}
			fmtd.append(" >");
		}
		
		return fmtd.toString();
	}
	
}
