// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/enumeratedtype.h
 *
 * @par TITLE
 * Enumerated Type Encapsulation
 *
 * @par PACKAGE
 * Robin
 *
 * @par PUBLIC-CLASSES
 *
 * <ul>
 *   <li>EnumeratedType</li>
 *   <li>EnumeratedConstant</li>
 * </ul>
 */

#ifndef ROBIN_REFLECTION_ENUMERATED_TYPES_H
#define ROBIN_REFLECTION_ENUMERATED_TYPES_H

// STL includes
#include <string>
#include <map>

// Pattern includes
#include <pattern/handle.h>

// Package includes
#include <robin/reflection/robintype.h>


namespace Robin {

class EnumeratedConstant;

/**
 * @class EnumeratedType
 * @nosubgrouping
 *
 * Defines an integral enumeration. The names of the
 * constants are saved along with their names, so literals can be
 * deduced from the values at runtime.
 */
class EnumeratedType
{
public:
	/**
	 * @name Constructors
	 */

	//@{
	EnumeratedType(std::string name);

	void activate(Handle<EnumeratedType> self);
	//@}

	/**
	 * @name Declaration API
	 */

	//@{
	void addConstant(std::string literal);
	void addConstant(std::string literal, int value);
	//@}

	/**
	 * @name Access
	 */

	//@{
    std::string name() const;
	std::string deduceName(int value) const;
	int lookup(std::string literal) const;
	std::vector<int> listOfValues() const;
	//@}

	/**
	 * @name Arguments
	 *
	 * In order to use this enumerated type as
	 * an argument type and return type for functions, an
	 * approperiate <classref>RobinType</classref> is supplied.
	 */

	//@{
	Handle<RobinType> getType() const;
	//@}

private:
	std::string m_fullname;

	typedef std::map<int, std::string> literalmap;
	literalmap m_literals;

	Handle<RobinType> m_type;
};


/**
 * @class EnumeratedConstant
 * @nosubgrouping
 *
 * Represents a single value of an enumerated type. For
 * simplicity, this value is considered immutable.
 */
class EnumeratedConstant
{
public:
	/**
	 * @name Constructors
	 */

	//@{
	EnumeratedConstant(Handle<EnumeratedType> enumtype, int value);
	//@}

	/**
	 * @name Access
	 */

	//@{
	Handle<EnumeratedType> getType() const;
	int getValue() const;
	std::string getLiteral() const;

	operator int() const;
	//@}

private:
	Handle<EnumeratedType> m_enumtype;
	int m_value;
};



std::vector<Handle<EnumeratedConstant> > listOfConstants(Handle<EnumeratedType>
														 enumtype);


} // end of namespace Robin

#endif
