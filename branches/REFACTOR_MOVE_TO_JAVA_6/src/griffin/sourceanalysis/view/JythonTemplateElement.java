package sourceanalysis.view;

import java.util.Iterator;

import org.python.core.PyObject;
import org.python.core.PySequence;
import org.python.util.PythonInterpreter;

import sourceanalysis.ContainedConnection;
import sourceanalysis.ElementNotFoundException;
import sourceanalysis.Entity;

/**
 * enclosing_type - undocumented.
 */
public class JythonTemplateElement extends ScriptingTemplateElement {

	/**
	 * Combines the capabilities of the context and the perspective to
	 * provide the JythonElement with maximal accessibility.
	 */
	public class ActiveContext {

		/**
		 * Constructor for ActiveScope.
		 */
		public ActiveContext(AbstractScope context, Perspective perspective) {
			super();
			m_context = context;
			m_perspective = perspective;
		}
		
		public String refill(String templatekey, String addition, Entity addition_element) throws ElementNotFoundException
		{
			// Create a nested scope
			AbstractScope nested_scope = m_context.cloneScope();
			nested_scope.declareMember(addition, addition_element, true);
			// Fill using the template bank
			return TemplateBank.getInstance()
				.fillTemplate(templatekey, nested_scope, m_perspective);
		}
		
		public String refill(String templatekey, String addition, Iterator iterator)
			throws ElementNotFoundException
		{
			StringBuffer buffer = new StringBuffer();
			while (iterator.hasNext()) {
				Object element =  iterator.next();
				if (element instanceof ContainedConnection) {
					element = ((ContainedConnection)element).getContained();				
				}
				buffer.append(refill(templatekey, addition, (Entity)element));
			}
			return buffer.toString();
		}
		
		public String property(Entity entity, String propName)
		{
			try {
				return entity.findProperty(propName).getValue();
			}
			catch (ElementNotFoundException e) {
				return "(missing " + propName + ")";
			}
		}

		private AbstractScope m_context;
		private Perspective m_perspective;
	}
	
	/**
	 * Constructor for JythonTemplateElement.
	 * @param script
	 */
	public JythonTemplateElement(String script) {
		super(script);
		
		if (m_interp == null) {
			m_interp = new PythonInterpreter();
			m_interp.exec("from sourceanalysis.view import JythonIterator");
		}
	}

	/**
	 * @see sourceanalysis.view.TemplateElement#extractText(AbstractScope)
	 */
	public String extractText(AbstractScope context, Perspective perspective) {
		// Add members of the scope as variables to the interpreter environment
		java.util.Iterator memberi = context.declIterator();
		while (memberi.hasNext()) {
			try {
				java.util.Map.Entry entry = (java.util.Map.Entry)(memberi.next());
				String varname = (String)entry.getKey();
				Entity varobj = (Entity)entry.getValue();
				// Set variable
				m_interp.set(varname, varobj);
			}
			catch (ClassCastException e) {
				System.err.println("*** Warning: invalid member encountered in scope: " + e.getMessage());
			}
		}
		// Add the active context too
		m_interp.set("context", new ActiveContext(context, perspective));
		m_interp.set("perspective", perspective);

		// Run instruction code in Jython
		PyObject formatted = null;
		
		try {
			formatted = m_interp.eval(getInstructions());
		}
		catch (org.python.core.PyException e) {
			System.err.println("*** Jython error: in '" + getInstructions() + "'" + e);
			return "" + e;
		}
		
		if (formatted instanceof PySequence) {
			PySequence formatted_list = (PySequence)formatted;
			StringBuffer formatted_text = new StringBuffer();
			for (int i =0; i < formatted_list.__len__(); i++) {
				PyObject formatted_item = formatted_list.__getitem__(i);
				formatted_text.append(formatted_item.toString());
			}
			return formatted_text.substring(0);
		}
		else {
			return formatted.toString();
		}
	}


	// Jython interpreter environment
	private PythonInterpreter m_interp = null;
}
