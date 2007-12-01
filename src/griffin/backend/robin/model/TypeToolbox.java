package backend.robin.model;

import javax.swing.tree.MutableTreeNode;

import backend.robin.Filters;

import sourceanalysis.Alias;
import sourceanalysis.Entity;
import sourceanalysis.InappropriateKindException;
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
			root = (Type.TypeNode) root.getFirstChild();
		}
		return new Type(root);
	}

	static public Type dereferencePtrOne(Type type) {
		Type.TypeNode root = type.getRootNode();
		// Root should be a pointer node. Skip it.
		assert root.getKind() == Type.TypeNode.NODE_REFERENCE;
		return new Type((Type.TypeNode)root.getFirstChild());
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
		root.add((MutableTreeNode)refof.getRootNode());
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
		root.add((MutableTreeNode)ptrto.getRootNode());
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
				type = ((Alias)base).getAliasedType();
			}
			return type;
		}
		catch (InappropriateKindException e) {
			assert false;
			return type;
		}
	}
}
