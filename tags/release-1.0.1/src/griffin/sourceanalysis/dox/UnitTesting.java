package sourceanalysis.dox;

import java.util.Random;

import javax.swing.tree.DefaultMutableTreeNode;

import sourceanalysis.*;

public class UnitTesting {

	/**
	 * UnitTesting constructor.
	 */
	public UnitTesting()
	{
		random = new Random();
		base_type = new Aggregate();
		base_type.setName("LLk");
	}

	/**
	 * Makes a random type of the form:
	 * <tt>const</tt>? <i>base type</i> &lt;<i>template arguments</i>&gt;?
	 * (<tt>**</tt>...)? <tt>&amp;</tt>? [<i>array dimensions</i>]?;
	 * <!--
	 * const? base type <template arguments>? (**)? &? declarator [array dimensions]?;
	 * -->
	 */
	public Type randomFlatType()
	{
		boolean isConst = random.nextBoolean();
		int pointers = random.nextInt() % 3;
		boolean reference = random.nextBoolean();
		
		int ntemplate_args = 0;
		if (random.nextBoolean()) ntemplate_args = random.nextInt()%4;

		Type.TypeNode node;

		// Apply template arguments
		if (ntemplate_args > 0) {
			node = new Type.TypeNode(Type.TypeNode.NODE_TEMPLATE_INSTANTIATION);
			node.add(new Type.TypeNode(base_type));
			for (int tai = 0; tai < ntemplate_args; ++tai) {
				TemplateArgument targ = randomTemplateArgument();
				node.add(new DefaultMutableTreeNode(targ));
			}			
		}
		else {
			node = new Type.TypeNode(base_type);
		}
		
		// Apply constntess
		if (isConst) {
			node.setCV(Specifiers.CVQualifiers.CONST);
		}
		// Apply pointer dereference
		for (int pointeri = 0; pointeri < pointers; ++pointeri) {
			Type.TypeNode p = new Type.TypeNode(Type.TypeNode.NODE_POINTER);
			p.add(node);
			node = p;
		}
		// Apply reference
		if (reference) {
			Type.TypeNode r = new Type.TypeNode(Type.TypeNode.NODE_REFERENCE);
			r.add(node);
			node = r;
		}
		
		return new Type(node);
	}
	
	/**
	 * Generates a random template-argument.
	 */
	public TemplateArgument randomTemplateArgument()
	{
		if (random.nextBoolean())
			return new TypenameTemplateArgument(randomFlatType());
		else
			return new DataTemplateArgument("" + random.nextInt()%10);
	}


	private Random random;
	//
	private Entity base_type;
}
