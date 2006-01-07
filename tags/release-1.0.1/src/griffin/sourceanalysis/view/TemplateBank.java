package sourceanalysis.view;

import java.util.Map;
import java.util.HashMap;

import sourceanalysis.ElementNotFoundException;

/**
 * Stores loaded templates by name. By supplying a singleton-like access to
 * this object to all template elements participating in a certain view
 * process, these objects gain a common perspective concerning that view and
 * its elements.
 */
public class TemplateBank {

	/**
	 * Constructor for TemplateBank.
	 */
	public TemplateBank() {
		super();
		m_held_templates = new HashMap();
	}
	
	/**
	 * Method registerTemplate. Stores a template in the bank for future use. Templates must be registered
	 * before fillTemplate() can be used on this TemplateBank.
	 * @param keyname - name of template to register
	 * @param template - a Template object which corresponds to the name
	 */
	public void registerTemplate(String keyname, Template template)
	{
		m_held_templates.put(keyname, template);
	}
	
	/**
	 * Method fillTemplate. Grabs a template from the bank and fills it using the provided context scope.
	 * @param keyname name of template to fill
	 * @param context scope to use while filling the template (this scope is passed to all of the elements)
	 * @param perspective view parameters containing global objects
	 * @return String filled template text
	 * @throws ElementNotFoundException, if the template referenced by 'keyname' is not registered.
	 */
	public String fillTemplate(String keyname, AbstractScope context,
		Perspective perspective) throws ElementNotFoundException
	{
		Template template = (Template) m_held_templates.get(keyname);
		if (template == null) throw new ElementNotFoundException("template", keyname);
		return template.fill(context, perspective);
	}
	
	/**
	 * Method getInstance. Returns a singleton instance which is used as the template registry for
	 * all template-related activies.
	 * @return TemplateBank
	 */
	public static TemplateBank getInstance()
	{
		if (m_singleton == null) m_singleton = new TemplateBank();
		return m_singleton;
	}
	
	/**
	 * Method setInstance. Sets the singleton instance for TemplateBank. Successive calls to getInstance
	 * will provide that instance rather than creating a default instance. The set of instances used by all of
	 * the components can thus be controlled.
	 * @param instance
	 */
	public static void setInstance(TemplateBank instance)
	{
		m_singleton = instance;	
	}
	
	// Bank private members
	private Map m_held_templates;
	private static TemplateBank m_singleton = null;
}
