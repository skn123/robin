package backend.website;

/**
 * This exception is thrown when the file being processed is corrupt or
 * malformed.
 */
public class FileFormatException extends Exception {
		
	/**
	 * 
	 */
	private static final long serialVersionUID = -6077272850932291780L;

	/**
	 * Constructor for FileFormatException.
	 * @param fileName The file name.
	 */
	public FileFormatException(String msg, String fileName) {
		
		super(msg);
		m_fileName = fileName;
	}

	/**
	 * @see java.lang.Throwable#getMessage()
	 */
	@Override
	public String getMessage() {
		return super.getMessage();
	}
	
	/**
	 * Return the file which the error accoured in.
	 * @return The file name.
	 */
	public String getFile() {
			
		return m_fileName;
	}
		
	/** The file name */
	protected String m_fileName;	
}
	
