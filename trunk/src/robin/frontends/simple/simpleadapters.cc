// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/simpleadapters.cc
 *
 * @par PACKAGE
 * Robin
 */

#include "simpleadapters.h"

#include <assert.h>
#include "instanceelement.h"

#include <robin/reflection/instance.h>


namespace Robin {



/**
 * Assumes that 'value' is a Simple::String, and
 * puts a pointer to the underlying string.
 */
void SimpleCStringAdapter::put(ArgumentsBuffer& argsbuf, 
							  scripting_element value)
{
	Simple::Element *e = reinterpret_cast<Simple::Element *>(value);
	Simple::String *ste = dynamic_cast<Simple::String *>(e);
	assert(ste);
	argsbuf.push(ste->value.c_str());
}

/**
 * Interprets the return value as a char*, and builds
 * a string based on it.
 */
scripting_element SimpleCStringAdapter::get(basic_block data)
{
	return (scripting_element)(Simple::build(reinterpret_cast<char*>(data)));
}


/**
 * Assumes that 'value' is a Simple::PascalStringElement, and
 * puts a pointer to the underlying string.
 */
void SimplePascalStringAdapter::put(ArgumentsBuffer& argsbuf, 
									scripting_element value)
{
	Simple::Element *e = reinterpret_cast<Simple::Element *>(value);
	PascalStringElement *ste = static_cast<PascalStringElement *>(e);
	argsbuf.push(&(ste->value));
}

/**
 * Interprets the return value as a PascalString*, and builds
 * a string based on it.
 */
scripting_element SimplePascalStringAdapter::get(basic_block data)
{   
	PascalString *pascal_str = reinterpret_cast<PascalString*>(data);
	scripting_element el = (scripting_element)(Simple::build(pascal_str->chars));
	free(pascal_str);
	return el;
}


/**
 * Builds an instance adapter, which handles instances
 * of the class 'domain'.
 */
SimpleInstanceAdapter::SimpleInstanceAdapter(Handle<Class> domain)
  : m_domain(domain)
{
}

/**
 * Interprets the 'element' as a SimpleInstanceObjectElement
 * of the type specified in this adapter's construction, and
 * puts the pointer on the arguments buffer.
 */
void SimpleInstanceAdapter::put(ArgumentsBuffer& argsbuf, 
			       scripting_element value)
{
	Simple::Element *element = (Simple::Element *)value;
	SimpleInstanceObjectElement *instobj = 
		dynamic_cast<SimpleInstanceObjectElement *>(element);
	argsbuf.push(instobj->value->getObject());
}

/**
 * Interprets the returned value as a pointer to an
 * object of the domain class, and builds a SimpleInstanceObjectElement.
 */
scripting_element SimpleInstanceAdapter::get(basic_block data)
{
	void *ptr = (void *)data;
	SimpleInstanceObjectElement *instobj = new SimpleInstanceObjectElement;
	instobj->value = Handle<Instance>(new Instance(m_domain, ptr));
	return (scripting_element)((Simple::Element *)instobj);
}



/**
 * Builds an instance adapter, which handles instances
 * of the class 'domain'.
 */
SimpleAddressAdapter::SimpleAddressAdapter(Handle<TypeOfArgument> domain)
  : m_domain(domain)
{
}

/**
 * Interprets the 'element' as a SimpleAddressElement
 * of the type specified in this adapter's construction, and
 * puts the pointer on the arguments buffer.
 */
void SimpleAddressAdapter::put(ArgumentsBuffer& argsbuf, 
								scripting_element value)
{
	Simple::Element *element = (Simple::Element *)value;	
	SimpleAddressElement *addr_element = 
		dynamic_cast<SimpleAddressElement *>(element);
	argsbuf.push(addr_element->value->asPointer());
}

/**
 * Interprets the returned value as a pointer to a datum
 * of the domain type, and builds a SimpleAddressElement
 */
scripting_element SimpleAddressAdapter::get(basic_block data)
{
	void *ptr = (void *)data;
	Handle<Address> address(new Address(m_domain, ptr));;
	SimpleAddressElement *addr_element = new SimpleAddressElement(address);
	return (scripting_element)((Simple::Element *)addr_element);
}




/**
 * Builds an enumerated adapter, which handles values
 * of the enumerated type 'domain'.
 */
SimpleEnumeratedAdapter::SimpleEnumeratedAdapter(Handle<EnumeratedType> domain)
  : m_domain(domain)
{
}

/**
 * Interprets the 'element' as a 
 * <classref>SimpleEnumeratedConstantElement</classref>, and passes
 * its value as an int.
 */
void SimpleEnumeratedAdapter::put(ArgumentsBuffer& argsbuf, 
								  scripting_element value)
{
	Simple::Element *element = (Simple::Element *)value;
	SimpleEnumeratedConstantElement *enumobj = 
		dynamic_cast<SimpleEnumeratedConstantElement *>(element);
	assert(enumobj);
	argsbuf.push(enumobj->value.getValue());
}

/**
 * Interprets the returned value as an integer, which
 * is the serial value of an enumerated variable - then it builds a
 * <classref>SimpleEnumeratedConstantElement</classref> object.
 */
scripting_element SimpleEnumeratedAdapter::get(basic_block data)
{
	int value = *reinterpret_cast<int*>(&data);
	Simple::Element *enumobj = 
		new SimpleEnumeratedConstantElement(m_domain, value);
	return (scripting_element)enumobj;
}


} // end of namespace Robin
