// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * iterator.h
 *
 * @par TITLE
 * Abstract iterator interface.
 *
 * @par PACKAGE
 * Pattern
 */

#ifndef PATTERN_ITERATOR_INTERFACE_H
#define PATTERN_ITERATOR_INTERFACE_H


namespace Pattern {

/**
 * An abstract interface for a generic iterator.
 */
template < class ELEMENT >
class AbstractIterator
{
public:
	virtual void next() = 0;
	virtual bool done() const = 0;
	virtual ELEMENT value() const = 0;
	virtual ~AbstractIterator()
	{

	}
};

/**
 * Abstract class; iterates over a map.
 */
template < class ELEMENT, class MAP >
class MapIterator : public AbstractIterator<ELEMENT> 
{
public:
	MapIterator(const MAP& usermap) 
		: m_map(usermap), m_iter(usermap.begin()) { }


	virtual ~MapIterator()
	{

	}

	virtual void next() {
		++m_iter;
	}

	virtual bool done() const {
		return (m_iter == m_map.end());
	}

protected:
	const MAP& m_map;
	typename MAP::const_iterator m_iter;
};

/**
 * Iterates over keys in a standard map.
 */
template < class ELEMENT, class MAP >
class MapKeyIterator: public MapIterator<ELEMENT,MAP>
{
public:
	MapKeyIterator(const MAP& usermap) : MapIterator<ELEMENT,MAP>(usermap) { }

	virtual ~MapKeyIterator()
	{

	}

	virtual ELEMENT value() const {
		return this->m_iter->first;
	}
};


} // end of namespace Pattern

#endif
