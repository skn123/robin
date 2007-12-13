package backend.robin.model;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import sourceanalysis.Aggregate;
import sourceanalysis.ContainedConnection;
import sourceanalysis.Entity;
import sourceanalysis.MissingInformationException;
import sourceanalysis.Parameter;
import sourceanalysis.Primitive;
import sourceanalysis.Routine;
import sourceanalysis.Type;
import backend.robin.Filters;

public class RoutineDeduction {

	/**
	 * An argument should be managed in three locations: in the flat wrapper's prototype,
	 * in the flat wrapper's body, and in the RegData struct.
	 * @author corwin
	 *
	 */
	public static class ParameterTransformer {
		private Type prototypeType;
		private SimpleType regDataType;
		private CppExpression bodyExpr;

		public ParameterTransformer(Type prototypeType, CppExpression bodyExpr,
				SimpleType regDataType) {
			this.prototypeType = prototypeType;
			this.bodyExpr = bodyExpr;
			this.regDataType = regDataType;
		}

		public Type getPrototypeType() {
			return prototypeType;
		}

		public SimpleType getRegDataType() {
			return regDataType;
		}

		public CppExpression getBodyExpr() {
			return bodyExpr;
		}
	}
	
	
	/**
	 * Produces a transformer which is suitable for a given parameter.
	 * 
	 * @param formal parameter declaration
	 * @return
	 * @throws MissingInformationException
	 */
	public static ParameterTransformer deduceParameterTransformer(Parameter formal) throws MissingInformationException
	{
		return deduceParameterTransformer(formal.getType());
	}

	/**
	 * Produces a transformer which is suitable for a given parameter.
	 * 
	 * @param paramType type of the parameter
	 * @return
	 * @throws MissingInformationException
	 */
	public static ParameterTransformer deduceParameterTransformer(Type paramType)
			throws MissingInformationException 
	{
		Type realType = TypeToolbox.getOriginalTypeShallow(paramType);
		assert paramType.isFlat();
		Entity base = realType.getBaseType();
		int pointers = realType.getPointerDegree();
		boolean reference = realType.isReference();
		
		if (base instanceof Primitive || base instanceof sourceanalysis.Enum) {
			if (Filters.isSmallPrimitive(base) || base instanceof sourceanalysis.Enum) {
				if (pointers > 0 && base != Primitive.CHAR && base != Primitive.VOID) {
					assert pointers == 1 && !paramType.isReference();
					return new ParameterTransformer(TypeToolbox.dereferencePtrOne(paramType),
							new AddressOf(), new SimpleType(base));
				} else {
					return new ParameterTransformer(TypeToolbox.dereference(paramType),
							new NopExpression(), new SimpleType(base));
				}				
			} else {
				if (pointers > 0) {
					assert pointers == 1 && !paramType.isReference();
					return new ParameterTransformer(
							TypeToolbox.makeReference(TypeToolbox.dereferencePtrOne(paramType)),
							new AddressOf(), new SimpleType(base));
				} else if (reference) {
					return new ParameterTransformer(paramType, new NopExpression(), new SimpleType(base));
				} else {
					return new ParameterTransformer(TypeToolbox.makeReference(paramType), 
							new NopExpression(), new SimpleType(base));
				}
			}
		} else {
			if (pointers > 0) {
				assert pointers == 1 && !paramType.isReference();
				return new ParameterTransformer(paramType, new NopExpression(), new SimpleType(base, null, "*"));
			} else if (reference) {
				return new ParameterTransformer(paramType, new NopExpression(), new SimpleType(base, null, "&"));
			} else {
				return new ParameterTransformer(TypeToolbox.makeReference(paramType), new NopExpression(), new SimpleType(base, null, "&"));
			}
		}
	}

	/**
	 * Produces a sequence transformer which is suitable for the given parameters.
	 * 
	 * @param formal parameter declarations
	 * @return
	 * @throws MissingInformationException
	 */
	public static ParameterTransformer[] deduceParametefrTransformers(Parameter[] formal) throws MissingInformationException
	{
		ParameterTransformer[] qual = new ParameterTransformer[formal.length];
		for (int i = 0; i < formal.length; i++) {
			qual[i] = deduceParameterTransformer(formal[i]);
		}
		return qual;
	}

	/**
	 * Produces a sequence transformer which is suitable for the given parameters.
	 * 
	 * @param formal an iterator over parameter declarations (Parameter instances)
	 * @param count number of parameters to consider; will extract at most 'count'
	 *   values out of 'formal'.
	 * @return
	 * @throws MissingInformationException
	 */
	public static List<ParameterTransformer> deduceParameterTransformers(Iterator formal, int count) throws MissingInformationException
	{
		ArrayList<ParameterTransformer> qual = new ArrayList<ParameterTransformer>();
		for (int n = 0; formal.hasNext() && n < count; ++n) {
			qual.add(deduceParameterTransformer((Parameter)formal.next()));
		}
		return qual;
	}
	
	/**
	 * Produces an expression suitable for the body of the flat wrapper for
	 * this routine.
	 * 
	 * @param routine the routine to match expression for
	 * @param statically for a method, 'true' indicates that the method is being
	 *   wrapped as a class method even if it's an instance method
	 * @return a C++ expression which, given the arguments, produces the
	 *   invocation.
	 */
	public static CppExpression deduceCallExpression(Routine routine, boolean statically)
	{
		ContainedConnection uplink = routine.getContainerConnection();
		Entity container = (uplink == null) ? null : uplink.getContainer();
		boolean isMethod = container != null && container instanceof Aggregate;
		
		if (isMethod && !statically) {
			return new MethodCall(routine);
		}
		else {
			return new FunctionCall(routine);
		}
	}
	
	/**
	 * Produces a transformer which is suitable for the return value of a 
	 * function.
	 * @throws MissingInformationException 
	 */
	public static ParameterTransformer deduceReturnTransformer(Routine routine)
			throws MissingInformationException
	{
		return deduceReturnTransformer(routine.getReturnType());
	}
	
	/**
	 * Produces a transformer which is suitable for the return value of a 
	 * function.
	 * @throws MissingInformationException 
	 */
	public static ParameterTransformer deduceReturnTransformer(Type returnType)
			throws MissingInformationException
	{
		Type realType = TypeToolbox.getOriginalTypeShallow(returnType);
		assert returnType.isFlat();
		Entity base = realType.getBaseType();
		int pointers = realType.getPointerDegree();
		boolean reference = realType.isReference();
		
		if (!reference && pointers == 0 && base == Primitive.VOID) { /* void */
			return new ParameterTransformer(returnType, new NopExpression(), new SimpleType(base));
		}
		else if (base instanceof Primitive || base instanceof sourceanalysis.Enum) {
			if (Filters.isSmallPrimitive(base) || base instanceof sourceanalysis.Enum) {
				if (pointers > 0 && base != Primitive.CHAR && base != Primitive.VOID) {
					assert pointers == 1 && !returnType.isReference();
					return new ParameterTransformer(TypeToolbox.dereferencePtrOne(returnType),
							ret(new Dereference()), new SimpleType(base));
				} else {
					return new ParameterTransformer(TypeToolbox.dereference(returnType),
							ret(new NopExpression()), new SimpleType(base));
				}				
			} else {
				if (pointers > 0) {
					assert pointers == 1 && !returnType.isReference();
					return new ParameterTransformer(
							returnType,
							ret(new NopExpression()), new SimpleType(base));
				} else if (reference) {
					return new ParameterTransformer(returnType, ret(new NopExpression()), new SimpleType(base, null, "&"));
				} else {
					return new ParameterTransformer(TypeToolbox.makePointer(returnType), 
							ret(new ConstructCopy(base)), new SimpleType(base));
				}
			}
		} else {
			if (pointers > 0) {
				assert pointers == 1 && !returnType.isReference();
				return new ParameterTransformer(returnType, ret(new NopExpression()), new SimpleType(base, null, "*"));
			} else if (reference) {
				return new ParameterTransformer(returnType, ret(new NopExpression()), new SimpleType(base, null, "&"));
			} else {
				return new ParameterTransformer(TypeToolbox.makePointer(returnType), ret(new ConstructCopy(base)), 
						new SimpleType(base, null, "*"));
			}
		}
	}
	
	private static CppExpression ret(CppExpression expr)
	{
		return new ReturnDecorator(expr);
	}
}
