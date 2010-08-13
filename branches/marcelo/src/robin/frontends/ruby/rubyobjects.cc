// -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/ruby/rubyobjects.cc
 *
 * @par TITLE
 * Ruby Frontend Objects
 *
 * @par PACKAGE
 * Robin
 */


#include "rubyobjects.h"


namespace Robin {

namespace Ruby {



VALUE FunctionObject::rb_class;

void FunctionObject::mark(FunctionObject *c) { }
void FunctionObject::free(FunctionObject *c) { delete c; }


VALUE FunctionObject::allocate(Callable *underlying) {
    return allocate(rb_class, underlying);
}

VALUE FunctionObject::allocate(VALUE klass) {
    return allocate(klass, 0);
}

VALUE FunctionObject::allocate(VALUE klass, Callable *underlying) {
    FunctionObject *c = new FunctionObject;
    c->c = underlying;
    return Data_Wrap_Struct(klass, mark, free, c);
}


VALUE FunctionObject::rb_call(int argc, VALUE *argv, VALUE self)
{
	FunctionObject *func;
    Data_Get_Struct(self, FunctionObject, func);

	ActualArgumentList args;
	KeywordArgumentMap nokw;
	for (int i = 0; i < argc; ++i) args.push_back((scripting_element)argv[i]);

	try {
		return (VALUE)func->c->call(args, nokw);
	}
	catch (std::exception& e) {
		rb_raise(rb_eRuntimeError, e.what());
	}
}



} // end of namespace Robin::Ruby

} // end of namespace Robin

