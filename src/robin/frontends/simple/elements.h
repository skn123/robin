// -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/elements.h
 *
 * @par TITLE
 * Simple Interpreter
 *
 * Emulates a very simple model of an object-oriented
 * interpreter. All the elements are subclasses of Simple::Element. There
 * is not an actual interactive user interface, and the interaction with
 * the interpreter is done from a C++ program.<br />
 * There is only one purpose for the existance of this interpreter, and
 * that is, to unit-test Robin by writing a frontend for this interpreter.
 */


#ifndef SIMPLE_INTERPRETER_ELEMENTS_H
#define SIMPLE_INTERPRETER_ELEMENTS_H

#include <string>
#include <ostream>

typedef unsigned long ulong;


/**
 * @brief Robin's built-in implementation of a primitive interpreter.
 *
 * Emulates a very simple model of an object-oriented
 * interpreter. All the elements are subclasses of Simple::Element. There
 * is not an actual interactive user interface, and the interaction with
 * the interpreter is done from a C++ program.<br />
 * There is only one purpose for the existance of this interpreter, and
 * that is, to unit-test Robin by writing a frontend for this interpreter.
 */
namespace Simple {


    /**
     * @class Element
     * @nosubgrouping
     *
     * Base class for all of the simple interpreter's
     * elements.
     */
    class Element 
    {
    public:
		virtual ~Element();

		virtual void dbgout(std::ostream &out) const;
    };

    std::ostream& operator<<(std::ostream& os, const Simple::Element& e);





    /**
     * @class Integer
     * @nosubgrouping
     *
     * Extends Element and holds a C int.
     */
    class Integer : public Element
    {
    public:
		int value;

		virtual void dbgout(std::ostream &out) const;
    };

    /**
     * @class Long
     * @nosubgrouping
     *
     * Extends Element and holds a C long.
     */
    class Long : public Element
    {
    public:
		long value;

		virtual void dbgout(std::ostream &out) const;
    };

    /**
     * @class Float
     * @nosubgrouping
     *
     * Extens Elements and holds a C float.
     */
    class Float : public Element
    {
    public:
		float value;

		virtual void dbgout(std::ostream &out) const;
    };

    /**
     * @class Char
     * @nosubgrouping
     *
     * Extends Element and holds a C character.
     */
    class Char : public Element
    {
    public:
		char value;

		virtual void dbgout(std::ostream &out) const;
    };

    /**
     * @class Object
     * @nosubgrouping
     *
     * Extends Element and holds a pointer to an opaque
     * C/C++ object.
     */
    class Object : public Element
    {
    public:
		~Object() { }
		void *value;

		virtual void dbgout(std::ostream &out) const;
    };

    /**
     * @class String
     * @nosubgrouping
     *
     * Extends Element, holding a string in STL standard.
     */
    class String : public Element
    {
    public:
		std::string value;

		virtual void dbgout(std::ostream &out) const;
    };

    /**
     * Creates an Element from the given subtype, and
     * fills a value in it.
     */
    template < class ElementType, class Value >
    ElementType *buildElement(Value value)
    {
		ElementType *element = new ElementType;
		element->value = value;
		return element;
    }

    inline Integer *build(int value)  { return buildElement<Integer>(value); }
    inline Long    *build(long value) { return buildElement<Long>(value); }
    inline Long    *build(ulong valu) { return buildElement<Long>(valu ); }
    inline Float   *build(float valu) { return buildElement<Float>(valu); }
    inline Float   *build(double val) { return buildElement<Float>(val); }
    inline Char    *build(char value) { return buildElement<Char>(value); }
    inline String  *build(char *value){ return buildElement<String>(value); }
    inline Object  *build(void *valu) { return buildElement<Object>(valu); }

} // end of namespace Simple


#endif
