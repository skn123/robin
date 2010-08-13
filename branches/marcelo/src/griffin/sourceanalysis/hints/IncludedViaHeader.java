package sourceanalysis.hints;

import sourceanalysis.Hint;
import sourceanalysis.SourceFile;

/**
 * Indicates that an entity (usually a SourceFile) should not be included
 * directly but rather through another header file.
 */
public class IncludedViaHeader implements Hint {

	private SourceFile includingHeader;
	
	public IncludedViaHeader(SourceFile header)
	{
		includingHeader = header;
	}
	
	public SourceFile getIncludingHeader()
	{
		return includingHeader;
	}
	
}
