package sourceanalysis.view;

/**
 * A textual element in a template.
 */
public interface TemplateElement {

	String extractText(AbstractScope context, Perspective perspective);

}
