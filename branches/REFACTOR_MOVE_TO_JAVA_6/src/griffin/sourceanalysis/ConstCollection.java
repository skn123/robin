package sourceanalysis;

import java.util.Collection;
import java.util.Iterator;

/**
 * @author Misha Seltzer
 *
 */
public class ConstCollection<T> implements Iterable<T> {
	public ConstCollection(Collection<T> collection) {
		this.collection = collection;
	}
	
	public Iterator<T> iterator() {
		return this.collection.iterator();
	}
	
	private Collection<T> collection;
}
