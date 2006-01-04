package sourceanalysis.dox;

import sourceanalysis.Entity;
import sourceanalysis.Primitive;

/**
 * Used to get the entity which is referenced by some name string in
 * a type-expression.
 */
public interface EntityNameResolving {

	/**
	 * Finds an entity to go with a C++ base-name.
	 * @param name C++ name string from type-expression; may be
	 * nested such as std::vector but never templated
	 * @return Entity an entity with a proper name, which must never
	 * be <b>null</b>
	 */
	public Entity resolve(String name);

	/**
	 * Finds a primitive entity by name.
	 * @param name C++ name for primitive
	 * @return Primitive an entity with a proper name
	 */
	public Primitive resolvePrimitive(String name);
}
