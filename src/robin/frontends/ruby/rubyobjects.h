// -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/ruby/rubyobjects.h
 *
 * @par TITLE
 * Ruby Frontend Objects
 *
 * @par PACKAGE
 * Robin
 */

#ifndef ROBIN_RUBY_OBJECTS_H
#define ROBIN_RUBY_OBJECTS_H


// Ruby includes
#include <ruby.h>

// Package includes
#include <robin/reflection/callable.h>


namespace Robin {

namespace Ruby {


class FunctionObject
{
public:
    static VALUE rb_class;

    static VALUE allocate(Callable *underlying);
    static VALUE allocate(VALUE klass);

    static VALUE rb_call(int argc, VALUE *argv, VALUE self);

private:
    static void mark(FunctionObject *c);
    static void free(FunctionObject *c);
    static VALUE allocate(VALUE klass, Callable *underlying);

    Callable *c; 
};




} // end of namespace Robin::Ruby

} // end of namespace Robin


#endif
