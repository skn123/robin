package sourceanalysis;

/**
 * Occurs when trying to access an Entity in a way suitable for a certain
 * kind - whereas the Entity belongs to a different kind than is expected.
 */
public class InappropriateKindException extends Exception {

	/**
	 * Constructor for InappropriateKindException.
	 */
	public InappropriateKindException() {
		super();
	}

	/**
	 * Constructor for InappropriateKindException.
	 * @param s
	 */
	public InappropriateKindException(String s) {
		super(s);
	}

}
