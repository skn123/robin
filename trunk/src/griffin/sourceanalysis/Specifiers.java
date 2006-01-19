package sourceanalysis;

/**
 * Collects some constants which are used by some connections to
 * express Entity/Relation attributes.
 */
public class Specifiers {

	static public final int DONT_CARE = 0;

	/**
	 * Constants indicating the visibility (access) of elements.
	 */
	public class Visibility
	{
		static public final int PUBLIC = 4;
		static public final int PRIVATE = 1;
		static public final int PROTECTED = 2;
		static public final int PACKAGE = 3;
	};
	
	/**
	 * Constants indicating virtuality levels of methods.
	 */
	public class Virtuality
	{
		static public final int NON_VIRTUAL = 1;
		static public final int VIRTUAL = 2;
		static public final int PURE_VIRTUAL = 3;
	};
	
	/**
	 * Constants indicating storage specifiers.
	 */
	public class Storage
	{
		static public final int STATIC = 1;
		static public final int EXTERN = 2;
		
		static public final int CLASS_WIDE = 1;
		static public final int INSTANCE_OWN = 2;
	};

	/**
	 * Constants referring to variable access. Use these as bit-mask flags.
	 */
	public class CVQualifiers
	{
		static public final int NONE = 0x0;
		static public final int CONST = 0x1;
		static public final int VOLATILE = 0x2;
	}

}
