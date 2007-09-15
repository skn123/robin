// -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/ruby/module.cc
 *
 * @par TITLE
 * Ruby Frontend Importable Module
 *
 * @par PACKAGE
 * Robin
 */


#include <ruby.h>

#include <robin/registration/mechanism.h>
#include <robin/reflection/library.h>
#include <robin/frontends/framework.h>
#include "rubyfrontend.h"
#include "rubyobjects.h"


namespace Robin {



VALUE rb_loadLibrary(int argc, VALUE *argv, VALUE self)
{
  char *libname;
  char *libfile;

  if (argc != 2) rb_raise(rb_eRuntimeError, "expected 2 arguments");

  Check_Type(argv[0], T_STRING);  libname = RSTRING(argv[0])->ptr;
  Check_Type(argv[1], T_STRING);  libfile = RSTRING(argv[1])->ptr;

  try {
    RegistrationMechanism *mech = 
      RegistrationMechanismSingleton::getInstance();
    
    Handle<Library> lib = mech->import(libfile, libname);

    FrontendsFramework::activeFrontend()->exposeLibrary(*lib);

    return LONG2NUM(1);
  }
  catch (DynamicLibraryOpenException& e) {
    rb_raise(rb_eRuntimeError, e.dlerror_at.c_str());
  }
  catch (std::exception& e) {
    rb_raise(rb_eRuntimeError, e.what());
  }
}


} // end of namespace Robin


using namespace Robin;
using namespace Robin::Ruby;


extern "C" void Init_robin()
{
    Handle<RubyFrontend> fe;
    VALUE module;

    // Activate frontend
    fe = Handle<RubyFrontend>(new RubyFrontend);
    FrontendsFramework::selectFrontend(static_hcast<Frontend>(fe));

    // Register Ruby functions
    module = rb_define_module("Robin");
    rb_define_module_function(module, "loadLibrary", 
                              RUBY_METHOD_FUNC(rb_loadLibrary), -1);

    FunctionObject::rb_class = rb_define_class_under(module, "Function", 
                                                     rb_cObject);
    rb_define_alloc_func(FunctionObject::rb_class, FunctionObject::allocate);
    rb_define_method(FunctionObject::rb_class, "call",
                     RUBY_METHOD_FUNC(FunctionObject::rb_call), -1);
}
