/*
 * Created on Jun 22, 2003
 */
package sourceanalysis.view;

import java.util.Iterator;

public class JythonIterator {

	/**
	 * 
	 */
	public JythonIterator(Iterator javaIterator) {
		super();
		m_javaIterator = javaIterator;
	}
	
	public Object __getitem__(int n)
	{
		if (m_javaIterator.hasNext())
			return m_javaIterator.next();
		else
			throw org.python.core.Py.IndexError("iteration completed");
	}
	
	// Private attributes
	Iterator m_javaIterator;
}
