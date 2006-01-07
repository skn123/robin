package sourceanalysis;

/**
 * Implies that some required information is missing.
 */
public class MissingInformationException extends Exception {

	/**
	 * Constructor for MissingInformationException.
	 */
	public MissingInformationException() {
		super();
	}

	/**
	 * Constructor for MissingInformationException.
	 * @param s error string
	 */
	public MissingInformationException(String s) {
		super(s);
	}

}
