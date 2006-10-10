// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/instanceelement.h
 *
 * @par TITLE
 * Simple Instance Object Element
 *
 * @par PACKAGE
 * Robin
 *
 * An extension to the simple interpreter which connects it
 * with Robin's reflection. Any frontend written will essentially have
 * to extend its underlying interpreter's object collection in order to
 * support instance objects returned from functions.<br />
 * Because this extension is not a part of the Simple Interpreter's native
 * implementation, it is declared in namespace Robin rather than in
 * namespace Simple.
 */

#ifndef ROBIN_SIMPLE_FRONTEND_INSTANCE_ELEMENT_H
#define ROBIN_SIMPLE_FRONTEND_INSTANCE_ELEMENT_H

#include "elements.h"
#include <robin/reflection/address.h>
#include <robin/reflection/instance.h>
#include <robin/reflection/enumeratedtype.h>
#include <robin/reflection/pascalstring.h>

namespace Robin {

/**
 * @class SimpleInstanceObjectElement
 * @nosubgrouping
 *
 * Extends Simple::Element and, using a contained Robin::Instance field member,
 * encapsulates a C++ instance object. This object serves as an extension
 * to the Simple Interpreter's object space.
 */
class SimpleInstanceObjectElement : public Simple::Element
{
public:
	SimpleInstanceObjectElement();
	SimpleInstanceObjectElement(Handle<Instance> instance);

	virtual void dbgout() const;

	Handle<Instance> value;
};

/**
 * Extends Simple::Element and, using a contained Robin::Address field member,
 * encapsulates a C++ address (pointer). This object serves as an extension to 
 * the Simple Interpreter's object space.
 */
class SimpleAddressElement : public Simple::Element
{
public:
	SimpleAddressElement(Handle<Address> address);

	virtual void dbgout() const;

	Handle<Address> value;
};

/**
 * @class SimpleEnumeratedConstantElement
 * @nosubgrouping
 *
 * Extends <classref>Simple::Element</classref> and,
 * using a contained <classref>EnumeratedConstant</classref> field
 * member, represents an enumerated value. It extends both the simple
 * frontend and the Simple Interpreter's object space.
 */
class SimpleEnumeratedConstantElement : public Simple::Element
{
public:
	SimpleEnumeratedConstantElement(Handle<EnumeratedType> domain,
									int value);
	
	EnumeratedConstant value;
};

/**
 * @class FakeDoubleElement
 * @nosubgrouping
 */
class FakeDoubleElement : public Simple::Element
{
public:
	double value;
};

/**
 * @class PascalStringElement
 * @nosubgrouping
 */
class PascalStringElement : public Simple::Element
{
public:
	PascalString value;
};

} // end of namespace Robin

#endif
