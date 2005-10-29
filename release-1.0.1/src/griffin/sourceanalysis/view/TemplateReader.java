package sourceanalysis.view;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Reads templates from a textual source.
 * <p>Arbitrary text yields TextTemplateElement;
 * Text in double brackets ([[...]]) yields JythonTemplateElement, with
 * the text in the brackets serving as the Jython script within.
 * </p>
 * <p>Structure of the input file:</p>
 * <p><tt>{{{--template-name---<br />
 * Template text [[Jython expression]] Template Text ...<br />
 * }}}<br />
 * {{{---another-template-name<br />
 * ...</tt>
 * </p>
 */
public class TemplateReader {

	private class StateMachine
	{
		static private final int STATE_VOID = 0;
		static private final int STATE_TEXTUAL = 2;
		static private final int STATE_SCRIPTING_ELEMENT = 3;

		/**
		 * StateMachine constructor - initializes state machine
		 */		
		public StateMachine()
		{
			state = STATE_VOID;
			m_templateBank = new TemplateBank();
			m_template = null;
		}

		/**
		 * Respond to the encounter of a separator from input. 
		 * @param token separator token
		 */
		public void separator(String token)
		{
			Matcher m;
			
			if (OPEN_DOUBLE_BRACKETS.matcher(token).matches()) {
				state = STATE_SCRIPTING_ELEMENT;
			}
			else if (CLOSE_DOUBLE_BRACKETS.matcher(token).matches()) {
				state = STATE_TEXTUAL;
			}
			else if ((m = OPEN_TRIPLE_CURL.matcher(token)).matches()) {
				// Begin template definition
				m_templateName = m.group(1);
				m_template = new Template();
				state = STATE_TEXTUAL;
			}
			else if (CLOSE_TRIPLE_CURL.matcher(token).matches()) {
				if (state == STATE_TEXTUAL && m_template != null
					&& m_templateName != null) {
					// Insert newly constructed template to bank
					m_templateBank.registerTemplate(m_templateName,
						m_template);
				}
				state = STATE_VOID;
			}
		}
		
		/**
		 * Respond to the encounter of a text fragment which is bounded
		 * by two separators.
		 * @param text
		 */
		public void statement(String text)
		{
			if (state == STATE_TEXTUAL)
				m_template.addElement(new TextElement(text));
			else if (state == STATE_SCRIPTING_ELEMENT)
				m_template.addElement(new JythonTemplateElement(text));
		}
		
		/**
		 * Retrieves the template-bank that has been built so far.
		 * @return TemplateBank a bank containing templates with elements
		 * corresponding to previous calls to 'text'.
		 */
		public TemplateBank getConstructedTemplates()
		{
			return m_templateBank;
		}

		private int state;
		private Template m_template;
		private String m_templateName;
		private TemplateBank m_templateBank;
	}

	/**
	 * TemplateReader constructor.
	 */
	public TemplateReader() {
		super();
		OPEN_DOUBLE_BRACKETS = Pattern.compile("\\[\\[");
		CLOSE_DOUBLE_BRACKETS = Pattern.compile("\\]\\]");
		OPEN_TRIPLE_CURL = Pattern.compile("\\{\\{\\{([^\n]*)\n");
		CLOSE_TRIPLE_CURL = Pattern.compile("\\}\\}\\}");
		ANY_SEPARATOR = Pattern.compile("\\[\\[|\\]\\]|\\{\\{\\{([^\n]*)\n|\\}\\}\\}");
	}


	/**
	 * @param templateText template in textual representation (as
	 * describe in detailed description of TemplateReader).
	 * @return TemplateBank translated templates with appropriate elements
	 */
	public TemplateBank read(String templateText)
	{
		Matcher matchSeparator = ANY_SEPARATOR.matcher(templateText);
		StateMachine stateMachine = new StateMachine();
		
		int separatorStart = 0, separatorEnd = 0;
		
		// Separate text according to the separators.
		// The text from the previously detected separator up to the
		// currently revealed one is sent to the state machine, as well
		// as the separator tokens themselves.
		while (matchSeparator.find(separatorEnd)) {
			// Get start index of the separator token
			separatorStart = matchSeparator.start();
			// Send text fragment starting from the previous token's end
			// to state machine
			stateMachine.statement(
				templateText.substring(separatorEnd, separatorStart)
			);
			// Get end index of the separator token and send separator
			// token text to state machine
			separatorEnd = separatorStart + matchSeparator.group().length();
			stateMachine.separator(
				templateText.substring(separatorStart, separatorEnd)
			);			
		}
		
		// Send the rest of the text to the state machine
		stateMachine.statement(
			templateText.substring(separatorEnd, templateText.length())
		);
		
		// Return the fruit of this effort
		return stateMachine.getConstructedTemplates();
	}
	
	/**
	 * Service Routine - reads an entire template bank from an input
	 * text file.
	 * @param templateFilename name of text file in the format described
	 * in the detailed description of TemplateReader
	 * @return TemplateBank a bank containing templates as they are defined
	 * in the input file
	 */
	static public TemplateBank readTemplatesFromFile(String templateFilename)
		throws IOException
	{
		// Read entire file into memory
		File templateFile = new File(templateFilename);
		char buffer[] = new char[(int)templateFile.length()];
		Reader textReader = new FileReader(templateFile);
		textReader.read(buffer);
		// Parse string using the template reader
		String text = new String(buffer);
		TemplateReader templateReader = new TemplateReader();
		return templateReader.read(text);
	}
	
	// Matching patterns
	private Pattern OPEN_DOUBLE_BRACKETS;    // [[
	private Pattern CLOSE_DOUBLE_BRACKETS;   // ]]
	private Pattern OPEN_TRIPLE_CURL;        // {{{...\n
	private Pattern CLOSE_TRIPLE_CURL;       // }}}
	private Pattern ANY_SEPARATOR;           // all of the above
}
