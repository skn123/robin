package sourceanalysis;

import java.util.Collection;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Vector;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreeNode;

/**
 * Represents a type expression.
 * <p>Type expressions essentially have a tree structure which is
 * recursive in the manner that every subtree is a valid type-expression
 * on its own (with some exceptions).</p>
 * <p>It is therefore the natural way to refer to type-expressions as trees
 * because that makes representing any type-expression possible. However
 * most types seen in C/C++ programs are quite flat, and for that reason
 * there is also a "flat API" which is less powerful but far more straight-
 * forward.</p>
 */
public class Type extends DefaultTreeModel {
	
	public static interface BaseTypeFormatter
	{
		String formatBase(Entity entity);
	}

	/**
	 * Uses the base formatter to format a single entity's name.
	 * Note that this is not as easy as it might sound, due to the
	 * existance of specialization, which might require the application
	 * of the formatter to each of the concrete template arguments.
	 * @param base
	 * @param baseFormatter
	 * @return
	 */
	public static String formatBaseUsing(Entity base, 
		BaseTypeFormatter baseFormatter)
	{
		StringBuffer sb = new StringBuffer();
			
		// Special care is given to specializations
		if (base instanceof TemplateEnabledEntity) {
			TemplateEnabledEntity enabled = (TemplateEnabledEntity)base;
			if (enabled.isSpecialized()) {
				// - access the 'insides' of the specialization connection
				SpecializationConnection specz = 
					enabled.getGeneralTemplateForSpecialization();
				sb.append(baseFormatter.formatBase(specz.getGeneral()));
				Vector args = specz.getSpecificArguments();
				formatTemplateArguments(args, sb, baseFormatter);
			}
			else
				sb.append(baseFormatter.formatBase(base));
		}
		else
			sb.append(baseFormatter.formatBase(base));
		return sb.toString();
	}

	/**
	 * Provides formatting of a template argument list using a 
	 * BaseTypeFormatter.
	 * @param templateArgs a collection of ordered template arguments
	 * @param destination a StringBuffer into which formatted list is inserted
	 * @param baseFormatter formatter to apply to each template argument
	 * (it is only applied to TypeNameTemplateArgument-s, of course).
	 */
	private static void formatTemplateArguments(Collection templateArgs,
		StringBuffer destination, BaseTypeFormatter baseFormatter)
	{
		// Open angle brackets
		destination.append("< ");
		// Add arguments
		boolean first = true;
		for (Iterator iter = templateArgs.iterator(); iter.hasNext(); ) {
			Object object = iter.next();
			// add separating comma
			if (!first) destination.append(",");
			first = false;
			// format base types using the base formatter; other objects
			// are merely converted to text
			if (object instanceof TypenameTemplateArgument) 
				destination.append(((TypenameTemplateArgument)object)
					.getValue().formatCpp("", baseFormatter));
			else
				destination.append(object.toString());
		}
		// Close angle brackets
		destination.append(" >");
	}
		
	/**
	 * Provides formatting of a template argument list using a 
	 * BaseTypeFormatter.
	 * @param templateArgs a collection of ordered template arguments
	 * @param baseFormatter formatter to apply to each template argument
	 * (it is only applied to TypeNameTemplateArgument-s, of course).
	 */
	public static String formatTemplateArguments(Collection templateArgs,
		BaseTypeFormatter baseFormatter)
	{
		// Create a StringBuffer and use formatTemplateUsingArguments to
		// produce formatting onto it
		StringBuffer sb = new StringBuffer();
		formatTemplateArguments(templateArgs, sb, baseFormatter);
		return sb.toString();
	}
	
	/**
	 * A node in the type-expression tree.
	 * <p>Subnodes of a TypeNode may be:</p>
	 * <ul>
	 *  <li>Other TypeNode-s</li>
	 *  <li>DefaultMutableTreeNode if kind==NODE_ARRAY (these specify
	 *    dimensions as java.lang.Integer)</li>
	 *  <li>DefaultMutableTreeNode with TemplateArgument-s if
	 *   kind==NODE_TEMPLATE_INSTANTIATION</li>
	 * </ul>
	 */
	public static class TypeNode extends DefaultMutableTreeNode
	{
		private static final long serialVersionUID = -2800000812822467645L;

		public static final int NODE_LEAF = 0;
		public static final int NODE_POINTER = 1;
		public static final int NODE_REFERENCE = 2;
		public static final int NODE_ARRAY = 3;
		public static final int NODE_FUNCTION = 4;
		public static final int NODE_TEMPLATE_INSTANTIATION = 5;
		public static final int NODE_ELLIPSIS = 6;
		
		public static final int NODE_X_COMPOSITION = 7;
		public static final int NODE_X_BLANK = 8;
		
		/**
		 * TreeNode constructor - creates a node with a kind and no
		 * other information.
		 * @param kind node kind, which should be one of the NODE_* constants
		 *  defined in this class
		 */
		public TypeNode(int kind)
		{
			m_kind = kind;
			m_cvQualifiers = Specifiers.CVQualifiers.NONE;
		}

		/**
		 * Creates a base-type leaf node.
		 * @param base the base type, which may be an Aggregate, an Enum,
		 * or an Alias
		 */
		public TypeNode(Entity base)
		{
			m_kind = NODE_LEAF;
			m_base = base;
			m_cvQualifiers = Specifiers.CVQualifiers.NONE;
		}

		/**
		 * Sets the C-V qualifiers of the type denoted by this node.
		 * @param cv qualifier flags taken from Specifiers.CVQualifiers
		 * (value is or-ed with previously set CV flags).
		 */
		public void setCV(int cv)
		{
			m_cvQualifiers |= cv;
		}

		/**
		 * Returns the kind of this node.
		 * @return int one of the NODE_* constants defined in this class
		 */
		public int getKind()
		{
			return m_kind;
		}
		
		/**
		 * Returns the base class or type, in leaf nodes (kind==NODE_LEAF).
		 * @return Entity base type, which may be an instance of Aggregate,
		 * Enum, or Alias
		 * @throws InappropriateKindException if this is not a leaf node
		 */
		public Entity getBase() throws InappropriateKindException
		{
			if (m_kind == NODE_LEAF)
				return m_base;
			else
				throw new InappropriateKindException("not a leaf type-node");
		}

		/**
		 * Returns the C-V qualifiers associated with the node. This tells if
		 * the node denotes a const or a volatile type.
		 * @return int cv value taken from Specifiers.CVQualifiers.
		 */
		public int getCV()
		{
			return m_cvQualifiers;
		}

		/**
		 * Checks whether or not the node posesses the given CV qualifier.
		 * @param qualifier a CV value taken from Specifiers.CVQualifiers.
		 * Only one flag is allowed - don't <i>or</i> flags.
		 * @return <b>true</b> if this qualifier is set (qualifier flags can
		 * be set using setCV). <b>false</b> if not.
		 */
		public boolean hasCV(int qualifier)
		{
			return (getCV() & qualifier) != 0;
		}

		/**
		 * Returns <b>true</b> if the kind of this node is not NODE_LEAF.
		 * @see javax.swing.tree.TreeNode#getAllowsChildren()
		 */
		public boolean getAllowsChildren() {
			return (m_kind != NODE_LEAF);
		}
		
		/**
		 * Represents this type using an algebraic notation:
		 * <tt>kind(...children...)</tt>
		 * @return String textual representation
		 */
		public String toString()
		{
			StringBuffer sb = new StringBuffer();
			if (hasCV(Specifiers.CVQualifiers.CONST)) sb.append("const ");
			if (hasCV(Specifiers.CVQualifiers.VOLATILE)) sb.append("volatile ");
			
			if (getKind() == NODE_LEAF) {
				sb.append(m_base.getFullName());
			}
			else if (getKind() == NODE_X_BLANK) {
				sb.append("_");
			}
			else {
				// Prepare a string with the children's representations,
				// comma-delimited
				StringBuffer ab = new StringBuffer();
				for (Enumeration cen = children(); cen.hasMoreElements(); ) {
					if (ab.length() > 0) ab.append(',');
					ab.append(cen.nextElement().toString());
				}
				// Get literal for kind and add it before arguments
				String[] literal = { "leaf", "pointer", "reference", "array", "function",
					"template-instantiation", "elipsis", "composition" };
				sb.append(literal[getKind()] + "(" + ab.toString() + ")");
			}
			return sb.toString();
		}
		
		/**
		 * Represents the type using the normal C++ form;
		 * e.g "<tt>const Plug &".
		 * @param declname name of variable to append to type.
		 * The normal usage is for this to be "".
		 * @param baseFormatter a formatter used to form the names of base
		 * class-names involved in the type expression
		 * @return String textual representation.
		 */
		public String formatCpp(String declname, BaseTypeFormatter baseFormatter)
		{
			StringBuffer sb = new StringBuffer();
			StringBuffer cv = new StringBuffer();
			if (hasCV(Specifiers.CVQualifiers.CONST)) cv.append("const ");
			if (hasCV(Specifiers.CVQualifiers.VOLATILE)) cv.append("volatile ");
			
			if (getKind() == NODE_LEAF) {
				sb.append(cv.toString());
				sb.append(formatBaseUsing(m_base, baseFormatter));
				if (declname.length() > 0) {
					sb.append(" ");
					sb.append(declname);
				}
			}
			else if (getKind() == NODE_POINTER || getKind() == NODE_REFERENCE) {
				char op = (getKind() == NODE_POINTER) ? '*' : '&';
				String op_cv_declname = op + cv.toString() + declname;
				TypeNode referencee = (TypeNode)getFirstChild();
				if (referencee.getKind() == NODE_ARRAY)
					sb.append(referencee.formatCpp("(" + op_cv_declname + ")",
						baseFormatter));
				else
					sb.append(referencee.formatCpp(op_cv_declname,
						baseFormatter));
			}
			else if (getKind() == NODE_ARRAY) {
				String dim = "[" + getChildAt(1) + "]";
				sb.append(((TypeNode)getChildAt(0)).formatCpp(declname+ dim,
					baseFormatter));
			}
			else if (getKind() == NODE_TEMPLATE_INSTANTIATION) {
				sb.append(cv.toString());
				// Show base
				Enumeration cen = children();
				TypeNode basenode = (TypeNode)cen.nextElement();
				sb.append(basenode.formatCpp("", baseFormatter));
				// Prepare a string with the children's representations,
				// comma-delimited
				List children = new LinkedList();
				while (cen.hasMoreElements()) {
					// Get user object of child and add it to list
					DefaultMutableTreeNode element = 
						(DefaultMutableTreeNode)cen.nextElement();
					children.add(element.getUserObject());
				}
				formatTemplateArguments(children, sb, baseFormatter);
				if (declname.length() > 0) sb.append(" " + declname);
			}
			else if (getKind() == NODE_ELLIPSIS) {
				sb.append("...");
			}
			
			return sb.toString();
		}

		/**
		 * Runs formatCpp(String,BaseNameFormatter) with a simple formatter
		 * which invokes Entity.getFullName() to format base class names.
		 * @param declname declarator 
		 * @return formatted name
		 */
		public String formatCpp(String declname)
		{
			return formatCpp(declname, SIMPLE_TYPE_FORMATTER);
		}

		/**
		 * Equivalent to formatCpp("").
		 * @return String
		 */
		public String formatCpp() { return formatCpp(""); } 

		/**
		 * Strips off composition nodes.
		 * A composition is a node of the form:
		 * <p><tt>composition(</tt><i>expr</i>,<i>filltype</i><tt>)</tt></p>
		 * Where <i>expr</i> is a type expression which contains a "blank"
		 * node (kind==NODE_X_BLANK). This has the same meaning as
		 * placing the <i>filltype</i> instead of the blank node and omitting
		 * the composition root.
		 * @return TypeNode a normalized version of the type expression, if
		 * this node is a composition. Otherwise, <b>this</b> is returned
		 * unchanged.
		 */
		public TypeNode normalize()
		{
			if (getKind() == NODE_X_COMPOSITION) {
				TypeNode filltype = (TypeNode)getChildAt(1);
				TypeNode expr = (TypeNode)getChildAt(0);
				return expr.normalize().normalize(filltype.normalize());
			}
			else
				return this;
		}
		
		/**
		 * Assigns "filltype" to any blank location.
		 * @param filltype value to fill in blanks
		 * @return TypeNode a normal form, may be <b>this</b>.
		 */
		private TypeNode normalize(TypeNode filltype)
		{
			if (getKind() == NODE_X_BLANK) {
				return filltype;
			}
			else {
				// Normalize all children
				for (int childi = 0; childi < children.size(); ++childi) {
					Object child = children.get(childi);
					// Replace child with normalized child
					if (child instanceof TypeNode) {
						TypeNode normal = ((TypeNode)child).normalize(filltype);
						if (normal != child)
							children.set(childi, normal);
					}
				}
				return this;
			}
		}
		
		/* (non-Javadoc)
		 * @see java.lang.Object#equals(Object)
		 */
		public boolean equals(Object other)
		{
			if (!(other instanceof TypeNode)) {
				return false;
			}
			
			TypeNode othernode = (TypeNode)other;
			return (othernode.m_base == m_base) &&
			        (othernode.m_kind == m_kind) &&
					(othernode.m_cvQualifiers == m_cvQualifiers);
		}
		
		/* (non-Javadoc)
		 * @see java.lang.Object#hashCode()
		 */
		public int hashCode() {
			int hash = Integer.toString(m_kind).hashCode();
			hash += Integer.toString(m_cvQualifiers).hashCode(); 
			if (m_base != null) hash += m_base.hashCode();
			return hash;
		}

		@Override
		public TypeNode clone() {
			TypeNode replica = new TypeNode(m_kind);
			replica.m_base = m_base;
			replica.m_cvQualifiers = m_cvQualifiers;
			Enumeration cen = children();
			while (cen.hasMoreElements()) {
				replica.add((MutableTreeNode)
						((DefaultMutableTreeNode)cen.nextElement()).clone());
			}
			return replica;
		}
		
		// Private members
		int m_kind;
		Entity m_base;   // base type, can be Aggregate, Enum or Alias
		int m_cvQualifiers;

		
	}


	/**
	 * Type constructor.
	 * @param root root node of new type tree. May be <b>null</b> to indicate
	 * the special type "nothing" (represented by an empty tree).
	 */
	public Type(TypeNode root)
	{
		super((root != null) ? root : new DefaultMutableTreeNode());
		m_isEmpty = (root == null);
	}
	
	/**
	 * Returns the root node of the tree just like DefaultMutableTree, with
	 * one difference - the tree may be empty, in which case the return value
	 * is <b>null</b>.
	 * @return Object a node object which functions as the root, or
	 * <b>null</b> if the type expression tree is empty.
	 * @note some JDKs do not allow DefaultTreeModel to be empty and
	 * throw an error when it's created with a null root. This is the main
	 * reason for the existance of this extension. 
	 */
	public Object getRoot()
	{
		return m_isEmpty ? null : super.getRoot();
	}
	
	/**
	 * Extracts the root of the tree as a TypeNode.
	 * @return Type.TypeNode root
	 */
	public TypeNode getRootNode()
	{
		return (TypeNode)getRoot();
	}
	
	/**
	 * Create a textual representation of the type. If this type is empty, the 
	 * value is "". Otherwise, the representation of the root is taken.
	 */
	public String toString()
	{
		TypeNode root = getRootNode();
		if (root == null)
			return "";
		else
			return root.toString();
	}
	
	/**
	 * Represents the type using C++ form with a formatter for base class
	 * names.
	 * @param declname name of variable to append to type.
	 * The normal usage is for this to be "".
	 * @param baseFormatter a formatter which tells this class how to
	 * represent leaves in the type-expression by providing a method for
	 * formatting names of Entity objects (see BaseTypeFormatter).
	 * @return String textual representation
	 */
	public String formatCpp(String declname, BaseTypeFormatter baseFormatter)
	{
		TypeNode root = (TypeNode)getRoot();
		if (root == null)
			return "";
		else
			return root.formatCpp(declname, baseFormatter);
	}
	
	/**
	 * Represents the type using the normal C++ form;
	 * e.g "<tt>const Plug &</tt>".
	 * @param declname name of variable to append to type.
	 * The normal usage is for this to be "".
	 * @return String textual representation.
	 */
	public String formatCpp(String declname)
	{
		TypeNode root = (TypeNode)getRoot();
		if (root == null)
			return "";
		else
			return root.formatCpp(declname);
	}

	/**
	 * Equivalent to formatCpp("").
	 * @return String textual representation.
	 */
	public String formatCpp() { return formatCpp(""); } 

	/**
	 * Strips off composition nodes from the type expression.
	 */
	public void normalize()
	{
		this.setRoot(((TypeNode)getRoot()).normalize());
	}
	
	/**
	 * @name Flat Access API
	 * Methods for accessing data of the type in a non-recursive way.
	 */
	/*@{*/
	
	/**
	 * Returns <b>true</b> if the type has the form:
	 * <p><tt>const</tt>? <i>base-type</i> (<tt>**</tt>...)? <tt>&</tt>? [dimensions]?</p>
	 * If the answer to this question is "no", calling the other methods in
	 * this group is meaningless.
	 * @return boolean "flatness" flag
	 */
	public boolean isFlat()
	{
		TypeNode ptr = getRootNode();
		if (ptr == null) return false;
		// Arrays must come first
		while (ptr.getKind() == TypeNode.NODE_ARRAY) {
			ptr = (TypeNode)ptr.getFirstChild();
		}
		// References (actually, exactly one) must follow here and not later
		if (ptr.getKind() == TypeNode.NODE_REFERENCE) {
			ptr = (TypeNode)ptr.getFirstChild();
		}
		// Any number of pointer dereferencing levels are allowed here
		while (ptr.getKind() == TypeNode.NODE_POINTER) {
			ptr = (TypeNode)ptr.getFirstChild();
		}
		// Now all that remains must be "base-type"
		return (ptr.getKind() == TypeNode.NODE_LEAF ||
			ptr.getKind() == TypeNode.NODE_TEMPLATE_INSTANTIATION);
	}
	
	/**
	 * Computes the number of pointer dereferencing levels in this type.
	 * @return int count of "*" operators before declarator
	 */
	public int getPointerDegree()
	{
		int pointerCount = 0;
		// Walk the chain down the tree
		for (TypeNode ptr = getRootNode();
			!(ptr.getKind() == TypeNode.NODE_LEAF ||
			ptr.getKind() == TypeNode.NODE_TEMPLATE_INSTANTIATION);
			ptr = (TypeNode)ptr.getFirstChild()) {
			// Count pointers
			if (ptr.getKind() == TypeNode.NODE_POINTER)
				++pointerCount;
		}
		
		return pointerCount;
	}
	
	/**
	 * Checks whether this (flat) type is a reference.
	 * @return boolean reference flag
	 */
	public boolean isReference()
	{
		TypeNode ptr = getRootNode();
		// Arrays may come first
		while (ptr.getKind() == TypeNode.NODE_ARRAY) {
			ptr = (TypeNode)ptr.getFirstChild();
		}
		// A reference has to come now, if ever
		return (ptr.getKind() == TypeNode.NODE_REFERENCE);
	}
	
	/**
	 * Checks whether the actual "base-type" pointed or referred to by this
	 * flat type is <tt>const</tt>.
	 * @return boolean constness flag
	 */
	public boolean isConst()
	{
		return ((getFlatBaseCV() & Specifiers.CVQualifiers.CONST) != 0);
	}
	
	/**
	 * Checks whether the actual "base-type" pointed or referred to by this
	 * flat type is <tt>volatile</tt>.
	 * @return boolean volatile flag
	 */
	public boolean isVolatile()
	{
		return ((getFlatBaseCV() & Specifiers.CVQualifiers.VOLATILE) != 0);
	}
	
	/**
	 * Checks whether this flat type is an array.
	 * @return boolean <b>true</b> if there is at least one array dimension.
	 */
	public boolean isArray()
	{
		// Array spec. in a flat type must be at the root
		return (getRootNode().getKind() == TypeNode.NODE_ARRAY);
	}
	
	
	
	/**
	 * Extracts the array dimensions of a flat type.
	 * @return int[] array of array dimensions; 0's stand for unknown
	 * dimensions.
	 */
	public int[] getArrayDimensions()
	{
		TypeNode ptr = getRootNode();
		int degree = 0;
		// Arrays always come first
		while (ptr.getKind() == TypeNode.NODE_ARRAY) {
			ptr = (TypeNode)ptr.getFirstChild();
			degree++;
		}
		
		// Now get the dimension sizes
		ptr = getRootNode();
		int depth = 0;
		int dimensions[] = new int[degree];
		
		while (ptr.getKind() == TypeNode.NODE_ARRAY) {
			// Extract the dimension from second child of array node
			DefaultMutableTreeNode dimNode =
				(DefaultMutableTreeNode)ptr.getChildAt(1);
			dimensions[depth] = ((Integer)dimNode.getUserObject()).intValue();
			// Go deeper
			ptr = (TypeNode)ptr.getFirstChild();
			depth++;
		}
		return dimensions;
	}
	
	/**
	 * Returns the base type as an Entity (for a flat type). 
	 * This can be any of:
	 * <ul><li>Aggregate</li>
	 *       <li>Enum</li>
	 *       <li>Alias</li>
	 * </ul>
	 * If the returned entity is templated, use getTemplateArguments to get
	 * the arguments.
	 * @return Entity the base type
	 */
	public Entity getBaseType()
	{
		TypeNode node = getBaseTypeNode();
		// - bypass template instantiation directions
		if (node.getKind() == TypeNode.NODE_TEMPLATE_INSTANTIATION)
			node = (TypeNode) node.getFirstChild();
		// - fetch user object from leaf
		if (node.getKind() == TypeNode.NODE_LEAF) {
			try {
				return node.getBase();
			}
			catch (InappropriateKindException e) { return null; }
		}
		else return null; /* unexpected */
	}
	
	/**
	 * Extracts the template arguments in the case where the "base-type" is
	 * templated.
	 * @return TemplateArgument[] array of arguments to template. If there is
	 * no template usage in this flat type, the return value is <b>null</b>.
	 */
	public TemplateArgument[] getTemplateArguments()
	{
		TypeNode node = getBaseTypeNode();
		// - get children of template instantiation node
		if (node.getKind() == TypeNode.NODE_TEMPLATE_INSTANTIATION) {
			// the first child of 'node' is the template being instantiated,
			// the rest of the children are the arguments - therefore, the size
			// of the arguments' array is count-1.
			TemplateArgument[] arguments =
				new TemplateArgument[node.getChildCount()-1];
			for (int child = 1; child < node.getChildCount(); ++child) {
				TemplateArgument argument = (TemplateArgument)
					((DefaultMutableTreeNode)node.getChildAt(child))
					.getUserObject();
				arguments[child-1] = argument;
			}
			return arguments;
		}
		else return null;  /* not a template */
	}
	
	/**
	 * Extracts the subtree which forms the bottom level in this flat type -
	 * the "base-type" node.
	 * @return TypeNode a type-node representing base-type
	 */
	private TypeNode getBaseTypeNode()
	{
		TypeNode ptr;
		// Walk the chain down the tree
		for (ptr = getRootNode();
			!(ptr.getKind() == TypeNode.NODE_LEAF ||
			ptr.getKind() == TypeNode.NODE_TEMPLATE_INSTANTIATION);
			ptr = (TypeNode)ptr.getFirstChild()) ;
		return ptr;
	}
	
	/**
	 * Extracts the CV qualifers from the bottom level of the type - that is,
	 * the flags of the "base-type".
	 * @return int CV flags from Specifiers.CVQualifiers
	 */
	private int getFlatBaseCV()
	{
		// The const/volatile specifier must come at the bottom level
		return getBaseTypeNode().getCV();
	}
	
	/*@}*/

	/**
	 * @name Type Transformations
	 */
	/*@{*/
	
	public static interface Transformation
	{
		TypeNode transform(TypeNode original)
			throws InappropriateKindException;
	}
	
	public static interface ExtendedTransformation extends Transformation
	{
		TemplateArgument transform(TemplateArgument original);
	}
	
	/**
	 * Builds a clone of a type expression, where any part of the expression
	 * from certain nodes down may be replaced according to a criteria defined
	 * by the caller. As long as the type nodes do not fall in the criteria,
	 * traversal continues and the tree is just copied; when the criteria is
	 * fulfilled, and entire sub-tree is pruned and replaced with a transformed
	 * node which is calculated from the original.   
	 * @param typeRoot root node of type tree or sub-tree
	 * @param transformation the replacement to be done in the type expression
	 * @return transformed type expression
	 * @throws InappropriateKindException if the type expression is internally
	 * broken
	 */
	public static Type.TypeNode transformType(Type.TypeNode typeRoot,
		Transformation transformation) throws InappropriateKindException
	{
		final Transformation basic = transformation;
		// Create an ExtendedTransformation that forwards the call to the original
		// Transformation object
		ExtendedTransformation extended = new ExtendedTransformation() {
			public TypeNode transform(TypeNode original) 
				throws InappropriateKindException 
			{
				return basic.transform(original);
			}
			public TemplateArgument transform(TemplateArgument original)
			{
				if (original instanceof TypenameTemplateArgument) {
					Type argType = ((TypenameTemplateArgument)original).getValue();
					try {
						Type xformed = transformType(argType, this);
						if (xformed != argType)
							return new TypenameTemplateArgument(xformed);
					}
					catch (InappropriateKindException e) {
					}
				}
				return original;
			}
		};
		// Call the extended version of transformType
		return transformType(typeRoot, extended);
	}

	/**
	 * Builds a clone of a type expression, where any part of the expression
	 * from certain nodes down may be replaced according to a criteria defined
	 * by the caller. As long as the type nodes do not fall in the criteria,
	 * traversal continues and the tree is just copied; when the criteria is
	 * fulfilled, and entire sub-tree is pruned and replaced with a transformed
	 * node which is calculated from the original.   
	 * @param typeRoot root node of type tree or sub-tree
	 * @param transformation the replacement to be done in the type expression
	 * @return transformed type expression
	 * @throws InappropriateKindException if the type expression is internally
	 * broken
	 */
	public static Type.TypeNode transformType(Type.TypeNode typeRoot,
		ExtendedTransformation transformation) throws InappropriateKindException
	{
		Type.TypeNode transformed = null;
		
		if ((transformed = transformation.transform(typeRoot)) == null) {
			transformed = new Type.TypeNode(typeRoot.getKind());
			transformed.m_base = typeRoot.m_base;  // in case it's a leaf node
			// Repair children of typeNode and put results under repaired
			for (int child = 0; child < typeRoot.getChildCount(); ++child) {
				TreeNode childNode = typeRoot.getChildAt(child);
				// If child is itself a type node, recursively translate
				// otherwise, attempt to translate user object as a type
				// finally, fall back to just copying the node
				if (childNode instanceof Type.TypeNode) {
					transformed.add(transformType((Type.TypeNode)childNode,transformation));
				}
				else {
					Object objectInNode =
						((DefaultMutableTreeNode)childNode).getUserObject();
					if (objectInNode instanceof TemplateArgument) {
						objectInNode = transformation
							.transform((TemplateArgument)objectInNode);
					}
					transformed.add(new DefaultMutableTreeNode(objectInNode));
				}
			}

			// Copy CV qualifiers as-are
			transformed.setCV(typeRoot.getCV());
		}

		return transformed;
	}
	
	/**
	 * Builds a clone of a type expression, where any part of the expression
	 * from certain nodes down may be replaced according to a criteria defined
	 * by the caller. As long as the type nodes do not fall in the criteria,
	 * traversal continues and the tree is just copied; when the criteria is
	 * fulfilled, and entire sub-tree is pruned and replaced with a transformed
	 * node which is calculated from the original.   
	 * @param type type expression to transform
	 * @param transformation the replacement to be done in the type expression
	 * @return transformed type expression
	 * @throws InappropriateKindException if the type expression is internally
	 * broken
	 */
	public static Type transformType(Type type,
		Transformation transformation) throws InappropriateKindException
	{
		if (type.getRoot() == null)
			return type;
		else
			return new Type(transformType(type.getRootNode(), transformation));	
	}
	
	/**
	 * Builds a clone of a type expression, where any part of the expression
	 * from certain nodes down may be replaced according to a criteria defined
	 * by the caller. As long as the type nodes do not fall in the criteria,
	 * traversal continues and the tree is just copied; when the criteria is
	 * fulfilled, and entire sub-tree is pruned and replaced with a transformed
	 * node which is calculated from the original.   
	 * @param type type expression to transform
	 * @param transformation the replacement to be done in the type expression
	 * @return transformed type expression
	 * @throws InappropriateKindException if the type expression is internally
	 * broken
	 */
	public static Type transformType(Type type,
		ExtendedTransformation transformation) throws InappropriateKindException
	{
		if (type.getRoot() == null)
			return type;
		else
			return new Type(transformType(type.getRootNode(), transformation));	
	}

	/*@}*/
	
	
	
	
	/**
	 * @name Comparison Functions
	 */
	
	/*@{*/
	
	/**
	 * Compares TypeNode trees for equality
	 * 
	 * If 'expandTypedefs' is true, then typedefs are expanded into a typenode tree
	 */
	
	private static boolean equalTypenodes(TypeNode first, TypeNode second, boolean expandTypedefs) {
		
		// trivial case, same nodes
		if(first == second) {
			return true;
		}
		
		// base type, kind and and qualifiers are the same
		if(first.equals(second)) {
			if(first.getChildCount() != second.getChildCount()) {
				return false;
			} else {
				
				Enumeration firstChildren = first.children();
				Enumeration secondChildren = second.children();
				for(; firstChildren.hasMoreElements() && secondChildren.hasMoreElements();) {
					
					Object firstChildObject = firstChildren.nextElement();
					Object secondChildObject = secondChildren.nextElement();
					
					
					if(firstChildObject instanceof TypeNode && secondChildObject instanceof TypeNode) {
						if(!equalTypenodes((TypeNode)firstChildObject, (TypeNode)secondChildObject, expandTypedefs)) {
							return false;
						}
					} else if(firstChildObject instanceof DefaultMutableTreeNode && secondChildObject instanceof DefaultMutableTreeNode) {
						// this can happen if our node is NODE_ARRAY and we are looking at its dimensions
						// or if we are NODE_TEMPLATE_INSTANTIATION and we are looking at template arguments
						Object firstContainedUserObject = ((DefaultMutableTreeNode)firstChildObject).getUserObject();
						Object secondContainedUserObject = ((DefaultMutableTreeNode)secondChildObject).getUserObject();
						
						if(!compareTypenodeUserObjects(firstContainedUserObject, secondContainedUserObject, expandTypedefs)) {
							return false;
						}
					}
				}
	
		
				// all children are equal
				return true;
			}
		}
		
		if(expandTypedefs == false) {
			// do not perform any more expansion
			return false;
		}
		
		// not strictly equal, one of them may be an alias that needs expanding
		try {
			Entity firstBase = first.getBase();
			// if firstBase is not Alias, it's not equal at all - basic types
			// were compared before by TypeNode.equals()
			if(!(firstBase instanceof Alias) ) {
				return false;
			}
			
			return equalTypenodes(((Alias)firstBase).getAliasedType().getRootNode(),second, expandTypedefs);
		} catch(InappropriateKindException e) {
			// ignore
		}
		
		try {
			Entity secondBase = second.getBase();
			// if secondBase is not Alias, it's not equal at all - basic types
			// were compared before by TypeNode.equals()
			if(!(secondBase instanceof Alias) ) {
				return false;
			}
			
			return equalTypenodes(first, ((Alias)secondBase).getAliasedType().getRootNode(),expandTypedefs);
		} catch(InappropriateKindException e) {
			// ignore
		}
		
		return false;
		
	}
	
	/**
	 * Compares two objects that were contained in DefaultMutableTreeNode
	 * The objects can be either - TemplateArgument or Integer
	 * In case of TemplateArgument, the case where both are TypenameTemplateArguments is interesting, since
	 * expandTypedefs must be taken into account
	 * @param first first object
	 * @param second second object
	 * @param expandTypedefs whether to expand typedefs during comparison
	 * @return true if the objects are equal w.r.t above
	 */
	private static boolean compareTypenodeUserObjects(Object first, Object second, boolean expandTypedefs) {
		if(first instanceof TemplateArgument && second instanceof TemplateArgument) {
			if(first instanceof DataTemplateArgument && second instanceof DataTemplateArgument) {
				return ((DataTemplateArgument)first).getValueString().equals(((DataTemplateArgument)second).getValueString());

			} else if(first instanceof TypenameTemplateArgument && second instanceof TypenameTemplateArgument) {
				Type firstChildType = ((TypenameTemplateArgument)first).getValue();
				Type secondChildType = ((TypenameTemplateArgument)second).getValue();
				return firstChildType.isCompatible(secondChildType, expandTypedefs);
				
			} else {
				return false;
			}
		} else {
			return first.equals(second);
		}
	}

	/**
	 * Checks whether both types are compatible, i.e they are the same type 
	 * if expandTypedefs is true, then typedefs to compatible types are explored
	 * @param other other type
	 * @param expandTypedefs if <b>true</b>, any Alias nodes will get expanded to trees
	 * @return boolean <b>true</b> if they are compatible as per the above definition
	 */
	public boolean isCompatible(Type other, boolean expandTypedefs) {
				
		try {
			return equalTypenodes(getRootNode(), other.getRootNode(), expandTypedefs);
		}
		catch (NullPointerException e) {
			return false;
		}
		
		
		
	}
	
	/**
	 * @see java.lang.Object#equals(Object)
	 */
	public boolean equals(Object other)
	{
		if (!(other instanceof Type)) {
			return false;
		}
		
		Type othertype = (Type)other;
		try {
			 return isCompatible(othertype, false);
		}
		catch (NullPointerException e) {
			return false;
		}
	}
	
	/**
	 * @see java.lang.Object#hashCode()
	 */
	public int hashCode() {
		int hash = Boolean.toString(m_isEmpty).hashCode();
		if(root != null) hash += root.hashCode();
		return hash;
	}
	
	/*@}*/
	
	/* Members */
	private boolean m_isEmpty;
	
	static BaseTypeFormatter SIMPLE_TYPE_FORMATTER
		= new BaseTypeFormatter() {
					public String formatBase(Entity e) { return e.getFullName(); }
				};

	/**
	 * 
	 */
	private static final long serialVersionUID = 7933970838007178238L;

}
