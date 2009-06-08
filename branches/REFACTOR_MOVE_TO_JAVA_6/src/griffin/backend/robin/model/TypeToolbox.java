package backend.robin.model;

import backend.robin.Filters;

import sourceanalysis.Alias;
import sourceanalysis.ContainedConnection;
import sourceanalysis.Entity;
import sourceanalysis.InappropriateKindException;
import sourceanalysis.Namespace;
import sourceanalysis.Specifiers;
import sourceanalysis.Type;

public class TypeToolbox {

	/**
	 * Removes any reference notations from a type expression. e.g. "const int&"
	 * converts to "const int".
	 * 
	 * @param type
	 *            original type expression
	 * @return a new type expression without a reference
	 */
	static public Type dereference(Type type) {
		Type.TypeNode root = type.getRootNode();
		// Descend until node is not a reference
		while (root.getKind() == Type.TypeNode.NODE_REFERENCE) {
			root = (Type.TypeNode)root.getFirstChild();
		}
		return new Type(root.clone());
	}

	static public Type dereferencePtrOne(Type type) {
		Type.TypeNode root = type.getRootNode();
		// Root should be a pointer node. Skip it.
		assert root.getKind() == Type.TypeNode.NODE_REFERENCE;
		return new Type(((Type.TypeNode)root.getFirstChild()).clone());
	}
	
	/**
	 * Creates a reference type by adding one redirection degree to an
	 * existing type.
	 * e.g. int* will be transformed to int*&.
	 * 
	 * @param refof type to reference
	 * @return a Type representing a reference to 'refof'.
	 */
	static public Type makeReference(Type refof) {
		Type.TypeNode root = new Type.TypeNode(Type.TypeNode.NODE_REFERENCE);
		root.add(refof.getRootNode().clone());
		return new Type(root);
	}
	
	/**
	 * Creates a reference-to-object type, e.g. MyClass&.
	 * 
	 * @param base the basic type entity (usually Aggregate or Primitive)
	 * @return a Type representing a reference to 'base'.
	 */
	static public Type makeReference(Entity base) {
		Type.TypeNode root = new Type.TypeNode(Type.TypeNode.NODE_REFERENCE);
		root.add(new Type.TypeNode(base));
		return new Type(root);
	}

	/**
	 * Creates a pointer type by adding one redirection degree to an
	 * existing type.
	 * e.g. int* will be transformed to int*&.
	 * 
	 * @param ptrto type to point to
	 * @return a Type representing a pointer to 'ptrof'.
	 */
	static public Type makePointer(Type ptrto) {
		Type.TypeNode root = new Type.TypeNode(Type.TypeNode.NODE_POINTER);
		root.add(ptrto.getRootNode().clone());
		return new Type(root);
	}
	
	/**
	 * Creates a pointer-to-object type, e.g. MyClass*.
	 * 
	 * @param base the basic type entity (usually Aggregate or Primitive)
	 * @return a Type representing a pointer to 'base'.
	 */
	static public Type makePointer(Entity base) {
		Type.TypeNode root = new Type.TypeNode(Type.TypeNode.NODE_POINTER);
		root.add(new Type.TypeNode(base));
		return new Type(root);
	}
	
	/**
	 * Create a constant type, e.g. const MyClass.
	 * @param base the basic type entity (usually Aggregate or Primitive)
	 * @return a Type representing const 'base'.
	 */
	static public Type makeConst(Entity base) { 
		Type.TypeNode root = new Type.TypeNode(base);
		root.setCV(Specifiers.CVQualifiers.CONST);
		return new Type(root);
	}

	/**
	 * Given a type alias, returns the type it was 
	 * originally meant to alias
	 * 
	 * @param type type to be stripped from typedefs
	 * @return Base of the typedef
	 */
	public static Type getOriginalType(Type type) {
		// Un-typedef
		Type passedType = type; // to get the modifiers
		Entity base = type.getBaseType();
		Entity prev = null;
		while (base instanceof Alias && base != prev && !Filters.needsEncapsulation((Alias)base, true)) {
			type = ((Alias)base).getAliasedType();
			prev = base; // - avoid singular loops "typedef struct A A;"
			base = type.getBaseType();
		}
		// add the const / volatile modifiers back
		Type.TypeNode rootNode = type.getRootNode();
		rootNode.setCV(rootNode.getCV() | passedType.getRootNode().getCV());
		
		return type;
	}
	
	/**
	 * 
	 */
	public static Type getOriginalTypeShallow(Type type) {
		Entity base;
		try {
			while (type.getRootNode().getKind() == Type.TypeNode.NODE_LEAF
					&& (base = type.getRootNode().getBase()) instanceof Alias) {
				Type aliased = ((Alias)base).getAliasedType();
				if (!isVisible(aliased)) break;
				type = aliased;
			}
			return type;
		}
		catch (InappropriateKindException e) {
			assert false;
			return type;
		}
	}
	
	public static Type getOriginalTypeDeep(Type type) {
		return getOriginalTypeDeep(type.getRootNode());
	}
	
	private static Type getOriginalTypeDeep(Type.TypeNode root)
	{
		Type original = null;
		
		switch (root.getKind()) {
		case Type.TypeNode.NODE_POINTER:
			original = makePointer(
					getOriginalTypeDeep((Type.TypeNode)root.getFirstChild()));
			break;
		case Type.TypeNode.NODE_REFERENCE:
			original = makeReference(
					getOriginalTypeDeep((Type.TypeNode)root.getFirstChild()));
			break;
		default:
			return getOriginalTypeShallow(new Type(root));
		}
		original.getRootNode().setCV(root.getCV());
		return original;
	}
	
	/**
	 * Verifies that a type can be accessed from any C++ scope.
	 * This is done by checking the visibility of every entity that is
	 * referenced by the type expression.
	 * @param type a type to be checked
	 * @return 'true' if type is visible. 'false' if there is at least
	 *   one entity that is not declared public in the scope where it
	 *   is declared.
	 */
	public static boolean isVisible(Type type)
	{
		class VisibilityCheck implements Type.Transformation {
			public boolean visible;
			public VisibilityCheck() { visible = true; }
			public Type.TypeNode transform(Type.TypeNode original) 
					throws InappropriateKindException 
			{
				if (original.getKind() == Type.TypeNode.NODE_LEAF) {
					if (!isVisible(original.getBase()))
						visible = false;
				}
				return null;
			}
		}
		
		VisibilityCheck checkVisible = new VisibilityCheck();
		try {
			Type.transformType(type, checkVisible);
			return checkVisible.visible;
		} catch (InappropriateKindException e) {
			assert false; return false;
		}
	}
	
	/**
	 * Verifies that an entity can be accessed from any C++ scope.
	 * @param entity an entity to be checked
	 * @return 'true' iff the entity and all its containers are public
	 *   within the scope in which they are declared.
	 */
	public static boolean isVisible(Entity entity)
	{
		ContainedConnection uplink = entity.getContainerConnection();
		return uplink == null
				|| uplink.getContainer() instanceof Namespace
				|| uplink.getVisibility() == Specifiers.Visibility.PUBLIC;
	}
}
