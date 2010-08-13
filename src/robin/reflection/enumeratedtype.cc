// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * reflection/enumeratedtype.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "enumeratedtype.h"

#include <robin/debug/assert.h>
#include "namespace.h"

namespace Robin {





/**
 * Creates an enumerated type. The enum is assigned a
 * name (practically the one following the 'enum' keywords in C
 * declarations), and is initially empty.
 * <p>Note that the name *should* be unique, so prepend any containing
 * class names with :: to 'name'.</p>
 * Define constants using the Declaration API.
 */
EnumeratedType::EnumeratedType(std::string name)
	: m_fullname(name)
{
}

/**
 * In order for this object to be able to generate
 * argument types of itself, is must have a reference to 'this' via
 * a handle. This is both an unfortunate design flaw and an incapability
 * of C++.
 */
void EnumeratedType::activate(Handle<EnumeratedType> self)
{
	// Create the argument object
	m_type = RobinType::create_new(self,RobinType::constReferenceKind);
}



/**
 * Declares a constant in the enumerated set. The integer
 * value assigned to this literal is the maximal value assigned yet + 1.
 */
void EnumeratedType::addConstant(std::string literal)
{
	m_literals[int(m_literals.size())] = literal;
}

/**
 * Declares a constant to the enumerated set, explicity 
 * specifying the value associated with it. Several literals may, this
 * way, be assigned the same value; in this case the EnumeratedType
 * can still be used, but the behavior of <methodref>deduceName</methodref>
 * will be inconsistent.
 */
void EnumeratedType::addConstant(std::string literal, int value)
{
	m_literals[value] = literal;
}


/**
 * Returns the name of this enumerated type (fully qualified).
 */
std::string EnumeratedType::name() const
{
	return m_fullname;
}

/**
 * Returns the literal constant name which is associated
 * with a given integer value, using preceding
 * <methodref>addConstant</methodref> calls. If this integer is beyond
 * the range declared in this EnumeratedType, the return string is of
 * the form: "title #i", where 'title' is the name of the enumerated
 * type, and 'i' is the integer.
 */
std::string EnumeratedType::deduceName(int value) const
{
	literalmap::const_iterator li = m_literals.find(value);
	
	if (li != m_literals.end())
		return li->second;
	else
		return "<unnamed enum>";
}

/**
 * In principle, does the inverse of "deduceName". Given
 * a string literal which is the name of a constant, returns the
 * approperiate value with which it is associated.
 */
int EnumeratedType::lookup(std::string literal) const
{
	int litindex = -1;
	// Have to scan the entire map - ouch!
	// (However, in most cases the map is not THAT big.)
	for (literalmap::const_iterator li = m_literals.begin();
		 li != m_literals.end(); ++li) {
		// look at literal field
		if (li->second == literal) {
			litindex = li->first;
		}
	}

	if (litindex < 0)  /* not found! */
		throw LookupFailureException();

	return litindex;
}

/**
 * Creates a vector containing all the possible values of this enumerated
 * type as integral values.
 */
std::vector<int> EnumeratedType::listOfValues() const
{
	std::vector<int> values(m_literals.size());
	int litindex = 0;
	// Fill vector with keys from the literal map
	for (literalmap::const_iterator li = m_literals.begin();
		 li != m_literals.end(); ++li) {
		values[litindex++] = li->first;
	}

	return values;
}

/**
 * Creates a vector containing all the possible values of an enumerated
 * type as EnumeratedConstant objects.
 */
std::vector<Handle<EnumeratedConstant> > listOfConstants(Handle<EnumeratedType>
														 enumtype)
{
	std::vector<int> values = enumtype->listOfValues();
	std::vector<Handle<EnumeratedConstant> > constants(values.size());
	// Create constants for the values
	for (size_t litindex = 0; litindex < values.size(); ++litindex) {
		Handle<EnumeratedConstant> 
			constobj(new EnumeratedConstant(enumtype, values[litindex]));
		constants[litindex] = constobj;
	}

	return constants;
}


/**
 * Returns the <classref>RobinType</classref>
 * applicable for arguments of this enumerated type (without any
 * indirection).
 */
Handle<RobinType> EnumeratedType::getType() const
{
	assert_true(bool(m_type));
	return m_type;
}






/**
 * Creates a constant which belongs to an enumerated
 * type domain set, and holding a value.
 */
EnumeratedConstant::EnumeratedConstant(Handle<EnumeratedType> enumtype,
									   int value)
	: m_enumtype(enumtype), m_value(value)
{
}



/**
 * Returns the <classref>EnumeratedType</classref> which
 * is the type of this constant.
 */
Handle<EnumeratedType> EnumeratedConstant::getType() const
{
	return m_enumtype;
}

/**
 * Returns this constant's value as an integer.
 */
int EnumeratedConstant::getValue() const
{
	return m_value;
}

/**
 * Returns the string literal representing the value of
 * this constant in the enumerated set.
 */
std::string EnumeratedConstant::getLiteral() const
{
	return m_enumtype->deduceName(m_value);
}

/**
 * For easier syntax, the EnumeratedConstant can be
 * assigned directly to an int.
 */
EnumeratedConstant::operator int() const
{
    return getValue();
}

} // end of namespace Robin
