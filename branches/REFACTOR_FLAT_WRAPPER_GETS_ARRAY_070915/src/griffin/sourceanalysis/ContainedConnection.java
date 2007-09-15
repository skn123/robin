package sourceanalysis;

/**
 * Connects an enclosed entity with its container.
 * Almost every entity in the Program Database is conceptually contained in
 * a larger entity surrounding it: routines in their classes, classes in other
 * classes and in namespaces, and so forth. This composition can have some
 * attributes:
 * <ul>
 * <li>the visibility of the member</li>
 * <li>virtuality, for member functions of classes</li>
 * <li>storage specification</li>
 * </ul>
 */
public class ContainedConnection {

	/**
	 * Constructor for ContainedConnection. The connection must be
	 * initialized with all the information at the time it is built. Some of the
	 * members may be inapplicable in some occasions, and should be assigned
	 * the value of Specifiers.DONT_CARE.
	 * @param container the container (construct) Entity
	 * @param visibility access rights, taken from Specifiers.Visibility
	 * @param virtuality type of virtual, taken from Specifiers.Virtuality
	 * @param storage storage specifier, taken from Specifiers.Storage
	 * @param contained the contained (member) Entity
	 */
	public ContainedConnection(Entity container, int visibility, int virtuality, int storage, Entity contained) {
		super();
		m_container = container;
		m_visibility = visibility;
		m_virtuality = virtuality;
		m_storage = storage;
		m_contained = contained;
	}

	/**
	 * Returns the contained.
	 * @return Entity
	 */
	public Entity getContained() {
		return m_contained;
	}

	/**
	 * Returns the container.
	 * @return Entity
	 */
	public Entity getContainer() {
		return m_container;
	}

	/**
	 * Returns the storage.
	 * <p>Values are taken from Specifiers.Storage:</p>
	 * <ul>
	 * <li>STATIC / CLASS_WIDE - if the <b>static</b> specifier prefixes
	 *  the declaration.</li>
	 * <li>EXTERN / INSTANCE_OWN - if the <b>static</b> specifier is
	 *  missing or <b>extern</b> is explicitly specified.</li>
	 * </ul>
	 * @return int
	 */
	public int getStorage() {
		return m_storage;
	}

	/**
	 * Returns the virtuality.
	 * <p>Values are taken from Specifiers.Virtuality:</p>
	 * <ul>
	 * <li>NON_VIRTUAL - for regular methods.</li>
	 * <li>VIRTUAL - if the method has a <b>virtual</b> specifier and an
	 *   implementation in the base class.</li>
	 * <li>PURE_VIRTUAL - if the method has a <b>virtual</b> specifier
	 *   and <b>= 0</b> instead of the implementation.</li>
	 * </ul>
	 * @return int
	 */
	public int getVirtuality() {
		return m_virtuality;
	}

	/**
	 * Returns the visibility.
	 * <p>Values are taken from Specifiers.Visibility:</p>
	 * <ul>
	 * <li>PUBLIC - for public members</li>
	 * <li>PROTECTED - for protected members</li>
	 * <li> PRIVATE - for private members</li>
	 * <li> PACKAGE - for package-wide members, currently reserved for
	 *    Java</li>
	 * </ul>
	 * @return int
	 */
	public int getVisibility() {
		return m_visibility;
	}

	/**
	 * Sets the contained.
	 * @param contained The contained to set
	 */
	public void setContained(Entity contained) {
		m_contained = contained;
	}

	/**
	 * Sets the container.
	 * @param container The container to set
	 */
	public void setContainer(Entity container) {
		m_container = container;
	}

	/**
	 * Sets the storage.
	 * @param storage The storage to set
	 */
	public void setStorage(int storage) {
		m_storage = storage;
	}

	/**
	 * Sets the virtuality.
	 * @param virtuality The virtuality to set
	 */
	public void setVirtuality(int virtuality) {
		m_virtuality = virtuality;
	}

	/**
	 * Sets the visibility.
	 * @param visibility The visibility to set
	 */
	public void setVisibility(int visibility) {
		m_visibility = visibility;
	}

	/* Connection tuple: (container, visibility, virtuality, storage, contained) */
	Entity m_container;
	int m_visibility;
	int m_virtuality;
	int m_storage;
	Entity m_contained;
}
