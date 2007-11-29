package backend.robin.model;

import javax.swing.tree.MutableTreeNode;

import sourceanalysis.Entity;
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
	
	static public Type makeReference(Type refof) {
		Type.TypeNode root = new Type.TypeNode(Type.TypeNode.NODE_REFERENCE);
		root.add((MutableTreeNode)refof.getRootNode());
		return new Type(root);
	}
	
	static public Type makeReference(Entity base) {
		Type.TypeNode root = new Type.TypeNode(Type.TypeNode.NODE_REFERENCE);
		root.add(new Type.TypeNode(base));
		return new Type(root);
	}
}
