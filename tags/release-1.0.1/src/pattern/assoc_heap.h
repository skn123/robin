// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par PACKAGE
 * Pattern
 *
 * @par SOURCE
 * assoc_heap.h
 *
 * @par TITLE
 * Associative Heap
 *
 * A heap with a search capability, acheived by maintaining
 * a map of value->index alongside the array, updating it as elements are
 * moved.
 */

#ifndef PATTERN_ASSOCIATIVE_HEAP__
#define PATTERN_ASSOCIATIVE_HEAP__

#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>

/**
 * @class AssocHeap
 * @nosubgrouping
 *
 * A simple implementation of a binary heap, which allows
 * quick searching of elements by maintaining a map pointing into the
 * underlying structure. This can be used when keys are associated with
 * objects of the representation, such as done in Dijkstra shortest-path
 * algorithm (where d[v] is associated with every vertex v).
 */
template < class KeyType, class ElementType >
class AssocHeap
{
public:
	AssocHeap();

	/**
	 * @name Basic Heap Management
	 */

	//@{
	void insert(KeyType key, ElementType e);
	void decreaseKey(ElementType e, KeyType newkey);
	int size() const;

	//@}
	/**
	 * @name More Associativity
	 */

	//@{
	KeyType getAssociatedKey(ElementType e) const;
	bool    containsElement(ElementType e) const;

	//@}
	/**
	 * @name Minimum
	 */

	//@{
	KeyType     minimum() const;
	ElementType minimal() const;
	void        minimum(KeyType& okey, ElementType& oe) const;
	void        extractMinimum(KeyType& okey, ElementType& oe);
	//@}

private:
	/**
	 * @name Maintenance
	 */
	//@{
	void swap(int index1, int index2);
	int parent(int node) const { return node >> 1; }
	int left(int node)   const { return 2 * node; }
	int right(int node)  const { return 2 * node + 1; }
	void heapify(int node);
	//@}

	typedef std::pair<KeyType, ElementType> node;
	typedef std::vector<node> heaparray;
	typedef std::map<ElementType, int> elementassoc;

	heaparray m_heap;
	elementassoc m_assoc;
	int m_size;
};


/**
 * Builds an empty heap with no keys nor elements.
 */
template < class KeyType, class ElementType >
AssocHeap<KeyType, ElementType>::AssocHeap()
	: m_size(0)
{ }

/**
 * Returns the number of elements in the associative
 * heap.
 */
template < class KeyType, class ElementType >
int AssocHeap<KeyType, ElementType>::size() const
{
	return m_size;
}

/**
 * Puts a key in the key and associates it with an
 * element.
 */
template < class KeyType, class ElementType >
void AssocHeap<KeyType, ElementType>::insert(KeyType key, ElementType e)
{
	/* If array capacity exceeded, enlarge m_heap */
	if (m_size == m_heap.size())
		m_heap.resize(m_heap.size() * 2 + 1);
	/* Add new element as last element */
	m_heap[m_size].first = key;
	m_heap[m_size].second = e;
	m_assoc[e] = m_size;
	++m_size;
	/* Keep heapifying until the heap is consistent */
	for (int node = m_size-1; node > 0; node = parent(node))
		heapify(node);
	heapify(0);
}

/**
 * Modifies a key in the heap to a smaller value,
 * correcting the heap's structure to maintain consistency.
 */
template < class KeyType, class ElementType >
void AssocHeap<KeyType, ElementType>::decreaseKey(ElementType e,
												  KeyType newkey)
{
	/* Find the node which key should be decreased */
	typename elementassoc::iterator ei = m_assoc.find(e);
	assert(ei != m_assoc.end());
	int node = ei->second;

	/* Set the new key value */
	assert(newkey < m_heap[node].first);  /* make sure key is really smaller */
	m_heap[node].first = newkey;

	/* Heapify up the heap to regain the heaporder */
	while (node != 0) {
		node = parent(node);
		heapify(node);
	}
}

/**
 * Returns the smallest key in the heap.
 */
template < class KeyType, class ElementType >
KeyType AssocHeap<KeyType, ElementType>::minimum() const
{
	assert(size() > 0);
	return m_heap[0].first;
}

/**
 * Returns the element associated with the smallest key
 * in the heap.
 */
template < class KeyType, class ElementType >
ElementType AssocHeap<KeyType, ElementType>::minimal() const
{
	assert(size() > 0);
	return m_heap[0].second;
}

/**
 * Fetches both the smallest key in the heap and the
 * element associated with it. They are assigned to 'okey' and 'oe',
 * respectively.
 */
template < class KeyType, class ElementType >
void AssocHeap<KeyType, ElementType>::minimum(KeyType& okey, ElementType& oe)
	const
{
	assert(size() > 0);
	okey = m_heap[0].first;
	oe = m_heap[0].second;
}

/**
 * Removes the smallest key from the heap, returning it
 * and the element associated with it to the caller. They are assigned
 * to 'okey' and 'oe', respectively.
 */
template < class KeyType, class ElementType >
void AssocHeap<KeyType, ElementType>::extractMinimum(KeyType& okey,
													 ElementType& oe)
{
	minimum(okey, oe);
	/* Replace the first element by the last (thus discarding the first) */
	m_assoc.erase(m_assoc.find(oe));
	m_assoc[m_heap[size()-1].second] = 0;
	m_heap[0] = m_heap[size()-1];
	m_size--;
	/* Now correct the heap */
	heapify(0);
}

/**
 * Exchanges two nodes in the heap, keeping the associative
 * map up-to-date.
 */
template < class KeyType, class ElementType >
void AssocHeap<KeyType, ElementType>::swap(int index1, int index2)
{
	std::swap(m_assoc[m_heap[index1].second], m_assoc[m_heap[index2].second]);
	std::swap(m_heap[index1], m_heap[index2]);
}

/**
 * Assuming the sub-trees of 'node' are legcal heaps
 * (i.e., maintain the heap-order), makes the tree rooted at 'node'
 * a legal heap by redistributing the keys among node, right(node),
 * and left(node).
 */
template < class KeyType, class ElementType >
void AssocHeap<KeyType, ElementType>::heapify(int node)
{
	/* Two comparisons are sufficient to move the minimum of three
	 * elements a, b, and c to element a:
	 * 1. Order [a, b]
	 * 2. Order [a, c]
	 */
	int cright = right(node);
	int cleft = left(node);

#define adjust(C) \
	if (C < size() && m_heap[C].first < m_heap[node].first) { \
		swap(node, C); \
		heapify(C);    \
	}

	adjust(cleft);
	adjust(cright);
#undef adjust
}

/**
 * Returns the key associated with a given element in
 * the heap.
 */
template < class KeyType, class ElementType >
KeyType AssocHeap<KeyType, ElementType>::getAssociatedKey(ElementType e) const
{
	/* Find the node index containing that element */
	typename elementassoc::const_iterator fi = m_assoc.find(e);
	assert(fi != m_assoc.end());
	/* Fetch the key from the heap */
	return m_heap[fi->second].first;
}

/**
 * Returns the key associated with a given element in
 * the heap.
 */
template < class KeyType, class ElementType >
bool AssocHeap<KeyType, ElementType>::containsElement(ElementType e) const
{
	/* Find the node index containing that element */
	typename elementassoc::const_iterator fi = m_assoc.find(e);
	return (fi != m_assoc.end());
}

#endif

