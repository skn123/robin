package sourceanalysis.view;

/**
 * A textual element in a template.
 */
public interface TemplateElement {

	public String extractText(AbstractScope context, Perspective perspective);

}
