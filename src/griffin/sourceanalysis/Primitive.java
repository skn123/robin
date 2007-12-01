package sourceanalysis;

/**
 * A special type of aggregate which represents a primitive type (such as an
 * integer or a real number).
 * This class does not add any functionality over Entity, and is introduced
 * only so back-end writers can check the condition:
 * <ul><li><tt>(base instanceof Primitive)</tt></li></ul>
 * 
 * TODO: Primitive should not be derived from Aggregate.
 */
public class Primitive extends Aggregate {

	/**
	 * Constructor for Primitive.
	 */
	public Primitive() {
		super();
	}

	/**
	 * Direct construction with the name of the primitive.
	 * @param name
	 */
	public Primitive(String name) {
		super();
		setName(name);
	}
	
	public static Primitive byName(String name)
		throws ElementNotFoundException
	{
		Primitive[] prims = { 
			VOID, BOOL, INT, UINT, LONG, LONGLONG, ULONG, ULONGLONG, 
			CHAR, SCHAR, UCHAR, SHORT, USHORT, FLOAT, DOUBLE
		};
		// Look in array of primitive types
		for (int i = 0; i < prims.length; ++i) {
			if (prims[i].getName().equals(name)) return prims[i];
		}
		// - not found
		throw new ElementNotFoundException("primitive", name);
	}
	
	public static Primitive VOID = new Primitive("void");
	public static Primitive BOOL = new Primitive("bool");
	public static Primitive INT = new Primitive("int");
	public static Primitive UINT = new Primitive("unsigned int");
	public static Primitive LONG = new Primitive("long");
	public static Primitive LONGLONG = new Primitive("long long");
	public static Primitive ULONG = new Primitive("unsigned long");
	public static Primitive ULONGLONG = new Primitive("unsigned long long");
	public static Primitive CHAR = new Primitive("char");
	public static Primitive SCHAR = new Primitive("signed char");
	public static Primitive UCHAR = new Primitive("unsigned char");
	public static Primitive SHORT = new Primitive("short");
	public static Primitive USHORT = new Primitive("unsigned short");
	public static Primitive FLOAT = new Primitive("float");
	public static Primitive DOUBLE = new Primitive("double");
}
