package sourceanalysis.view;

/**
 * An implementation of Perspective is given to Templates and TemplateElements
 * when filling them. The Perspective provides an environment observation,
 * with access to objects which should have otherwise been designed as
 * singletons.
 * <p>A Perspective instance should be transferred to the Template and to
 * each TemplateElement participating in a certain view derivation; if a
 * template element ensues the invocation of another template, them same
 * perspective propagates to that template as well.
 * </p>
 */
public interface Perspective {

	public TemplateBank getTemplates();

}
