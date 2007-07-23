
##################################################
#
# Robin C++ Library Targets
#
##################################################

ver = "1.0"
fullver = "1.0.3"

BuildDir('build', 'src')

FOUNDATION_SRC = """build/robin/debug/trace.cc
"""

REFLECTION_SRC = """build/robin/reflection/argumentsbuffer.cc
build/robin/reflection/backtrace.cc
build/robin/reflection/boundmethod.cc
build/robin/reflection/cfunction.cc
build/robin/reflection/class.cc
build/robin/reflection/conversion.cc
build/robin/reflection/conversiontable.cc
build/robin/reflection/enumeratedtype.cc
build/robin/reflection/fundamental_conversions.cc
build/robin/reflection/instance.cc
build/robin/reflection/interface.cc
build/robin/reflection/intrinsic_type_arguments.cc
build/robin/reflection/library.cc
build/robin/reflection/low_level.cc
build/robin/reflection/memorymanager.cc
build/robin/reflection/method.cc
build/robin/reflection/namespace.cc
build/robin/reflection/overloadedset.cc
build/robin/reflection/typeofargument.cc
build/robin/reflection/address.cc
"""

REGISTRATION_SRC = """build/robin/registration/mechanism.cc
build/robin/registration/regdata.cc
"""

FRONTEND_FRAMEWORK_SRC = """build/robin/frontends/framework.cc
"""

SIMPLE_FRONTEND_SRC = """build/robin/frontends/simple/elements.cc
build/robin/frontends/simple/instanceelement.cc
build/robin/frontends/simple/simpleadapters.cc
build/robin/frontends/simple/simplefrontend.cc"""

PYTHON_FRONTEND_SRC = """build/robin/frontends/python/enhancements.cc
build/robin/frontends/python/facade.cc
build/robin/frontends/python/inheritance.cc
build/robin/frontends/python/module.cc
build/robin/frontends/python/pythonadapters.cc
build/robin/frontends/python/pythonconversions.cc
build/robin/frontends/python/pythonerrorhandler.cc
build/robin/frontends/python/pythonfrontend.cc
build/robin/frontends/python/pythoninterceptor.cc
build/robin/frontends/python/pythonlowlevel.cc
build/robin/frontends/python/pythonobjects.cc
"""

LIBPREFIX = "lib"

import os

env = Environment(ENV = {'PATH': os.environ['PATH'], 'SystemRoot': r"C:\Windows"})

# Debug mode (for developers)
robin_opts = Options()
robin_opts.Add(BoolOption('debug', 
                          'Set a value to compile a debug version', 0))
robin_opts.Add(BoolOption('clsux', 
                          'Set this to 1 if cl.exe fails to initialize', 0))

if ARGUMENTS.get('clsux', 0):
	env = Environment()
if ARGUMENTS.get('debug', 0):
	env.Append(CXXFLAGS = "-g")

env.Append(CPPPATH = ["src"])

# Configure library prefix and auto-import flag for Cygwin
import os.path, griffin as conf
if conf.isCygwin:
	env["SHLIBPREFIX"] = "lib"
	env.Append(LINKFLAGS = "-Wl,--enable-auto-import")

# Configure C++ compiler
if hasattr(conf.config, 'cxx'):
	env["CXX"] = conf.config.cxx

# Configure Python include and library
import sys, distutils.sysconfig

def CheckTemplate(context, templatename, pre=""):
	context.Message("Checking for %s..." % templatename)
	result = context.TryCompile(pre + "namespace __s { using %s; }\n" \
					% templatename, ".cxx")
	context.Result(result)
	return result

def CheckLibPIC(context, libname, text_code):
	context.Message("Checking for %s (PIC)..." % libname)
	context.AppendLIBS([libname])
	result = context.TryBuild(context.env.SharedLibrary, text_code, ".cc")
	context.Result(result)
	return result

libiberty_use = """
#include "libiberty.h"
extern "C" char *cplus_demangle(const char *mangled, int options);
void f() { cplus_demangle("robin4", 0); }
"""

INCLUDEPY = distutils.sysconfig.get_config_var("INCLUDEPY")
CONFINCLUDEPY = distutils.sysconfig.get_config_var("CONFINCLUDEPY")
LIBP = distutils.sysconfig.get_config_var("LIBP")
EXEC_PREFIX = distutils.sysconfig.get_config_var("exec_prefix")
if LIBP:
	LIBPYCFG = os.path.join(LIBP, "config")
else:
	LIBPYCFG = os.path.join(EXEC_PREFIX, "libs")
LIBPY1 = "python%i.%i" % sys.version_info[:2]
LIBPY2 = "python%i%i" % sys.version_info[:2]
AUXLIBS = []

# - Start configuring
pyenv = env.Copy()
configure = Configure(pyenv, custom_tests = {'CheckTemplate': CheckTemplate,
                                             'CheckLibPIC': CheckLibPIC})

pyenv.Append(CPPPATH = [INCLUDEPY, CONFINCLUDEPY])
pyenv.Append(LIBPATH = [".", LIBPYCFG])
pyenv.Append(CXXFLAGS = "-D_VERSION=" + fullver)

if conf.arch == "windows":
	env.Append(CXXFLAGS = "/EHsc")
	pyenv.Append(CXXFLAGS = "/EHsc /Imsvc")
if not configure.CheckCXXHeader("Python.h"):
	print "Missing Python.h !"
	Exit(1)
if configure.CheckTemplate("__gnu_cxx::hash_map", "#include <ext/hash_map>\n"):
	env.Append(CXXFLAGS = '-DWITH_EXT_HASHMAP')
elif configure.CheckTemplate("std::hash_map", "#include <ext/hash_map>\n"):
	env.Append(CXXFLAGS = '-DWITH_STD_HASHMAP')
if conf.config.has_liberty and \
   configure.CheckCXXHeader("libiberty.h") and configure.CheckLibPIC("iberty", libiberty_use):
	env.Append(CXXFLAGS = '-DWITH_LIBERTY')
	AUXLIBS.append("iberty")
if configure.CheckLib("dl"):
	AUXLIBS.append("dl")
if configure.CheckLib(LIBPY1):
	LIBPY = LIBPY1
else:
	if configure.CheckLib(LIBPY2):
		LIBPY = LIBPY2
	else:
		print "Missing library for -l%s or -l%s !" % (LIBPY1, LIBPY2)
		Exit(1)

# Add additional flags
#env.Append(LINKFLAGS = "-Wl,-z,defs")
spec = conf.platspec + "-" + ver + conf.pyspec

# Set up targets
if conf.arch == "windows":
	robin = env.StaticLibrary("robin"+spec, Split(FOUNDATION_SRC) + \
	                                        Split(REFLECTION_SRC) + \
	                                        Split(REGISTRATION_SRC) + \
	                                        Split(FRONTEND_FRAMEWORK_SRC))
else:
	robin = env.SharedLibrary("robin"+spec, Split(FOUNDATION_SRC) + \
	                                        Split(REFLECTION_SRC) + \
	                                        Split(REGISTRATION_SRC) + \
	                                        Split(FRONTEND_FRAMEWORK_SRC),
	                          LIBS = AUXLIBS)

pyfe = pyenv.SharedLibrary("robin_pyfe"+spec, Split(PYTHON_FRONTEND_SRC), 
                           LIBS=["robin"+spec, LIBPY])

stl = env.SharedLibrary("robin_stl"+spec,
			["build/robin/modules/stl/stl_robin.cc"])

Default(robin, pyfe, stl)


##################################################
#
# Tests in C++
#
##################################################

ENTERPRISE_SRC = """build/robin/extreme/enterprise.cc
build/robin/extreme/test_registration_arena.cc"""

TEST_REFLECTION_SRC = """build/robin/extreme/test_reflection_main.cc
build/robin/extreme/test_reflection_arena.cc
build/robin/extreme/test_reflection_lowmed.cc
build/robin/extreme/test_reflection_oop.cc
build/robin/extreme/test_reflection_conversions.cc"""

TEST_REGISTRATION_SRC = """build/robin/extreme/test_registration_main.cc
build/robin/extreme/test_registration_scenario.cc"""

INTERACTIVE_SRC = """build/robin/extreme/interactive/launcher.cc"""

LIBINTERACTIVE_SRC = """build/robin/extreme/interactive/syntax.cc
build/robin/extreme/interactive/simple.cc
build/robin/extreme/interactive/inclusion.cc"""

testenv = env.Copy()
testenv.Append(LIBPATH=["."])

sfe = testenv.SharedLibrary("robin_sfe"+spec, Split(SIMPLE_FRONTEND_SRC),
			    LIBS=["robin"+spec])
fwtesting = env.SharedLibrary("fwtesting", "build/robin/extreme/fwtesting.cc")

enterprise = testenv.SharedLibrary("enterprise", Split(ENTERPRISE_SRC))

test_reflection = testenv.Program("test_reflection",
				  Split(TEST_REFLECTION_SRC),
				  LIBS=["robin"+spec,
					"robin_sfe"+spec, "fwtesting",
					"enterprise"])

test_registration = testenv.Program("test_registration",
				    Split(TEST_REGISTRATION_SRC),
				    LIBS=["robin"+spec,
					  "robin_sfe"+spec, "fwtesting",
					  "enterprise", "interactive"])

simplecc = "build/robin/extreme/interactive/simple.cc"
simpleyy = "build/robin/extreme/interactive/simple.yy"
simple_flex = testenv.Command(simplecc, simpleyy,
			      "flex -o%s %s" % (simplecc, simpleyy))

libinteractive = testenv.StaticLibrary("interactive",
				       Split(LIBINTERACTIVE_SRC),
				       LIBS=["robin"+spec, "robin_sfc"+spec])

interactive = testenv.Program("interactive",
			      Split(INTERACTIVE_SRC),
			      LIBS=["robin"+spec, "robin_sfe"+spec,
				    "interactive"])

Depends(sfe, robin)
Depends(test_reflection, [robin, sfe, fwtesting])
Depends(test_registration, [robin, sfe, fwtesting, libinteractive])
Depends(libinteractive, [robin, sfe])
Depends(interactive, [robin, sfe, libinteractive])


##################################################
#
# Griffin Java Source-Analyzer
#
##################################################

import os.path

#env = Environment()

premises = ["jython.jar", "antlr-2.7.5.jar", "xercesImpl.jar", "junit.jar", 
            "xmlParserAPIs.jar", "swt.jar", "jface.jar"]
premisedir = os.path.join(".", "premises")

classpath = [os.path.join(premisedir, x) for x in premises]
if conf.is_gcj(env):
	classpath.append("src/griffin")

env.Append(JAVACFLAGS = '-classpath "%s"' % conf.java_pathsep.join(classpath))


def jarme(source, target, env):
    os.system("jar cf %s -C build/griffin ." % target[0])

def touch(filename):
	if not os.path.exists(filename):
		open(filename, "w").close()
	
touch("src/griffin/sourceanalysis/dox/TypeExpressionLexer.java")
touch("src/griffin/sourceanalysis/dox/TypeExpressionParser.java")


griffin = env.Java("build/griffin", "src/griffin")
env['BUILDERS']['Jar'] = Builder(action = jarme)
jar = env.Jar("Griffin.jar", griffin)

typeexpr = env.Command("src/griffin/sourceanalysis/dox/" + 
			"TypeExpressionLexerTokenTypes.txt",
			"src/griffin/sourceanalysis/dox/TypeExpression.g",
			"java -cp premises/antlr-2.7.5.jar antlr.Tool "
                        "-o src/griffin/sourceanalysis/dox "
                        "src/griffin/sourceanalysis/dox/TypeExpression.g")

Depends(griffin, typeexpr)

stl_dox = env.Command("build/stl.tag", "src/griffin/modules/stl", 
                      "( cd src/griffin/modules/stl; doxygen )")

Default(jar, stl_dox)


##################################################
#
# Show help for the different targets.
#
##################################################

helpstring = ""
helpline = "="*45 + "\n"
targetList = COMMAND_LINE_TARGETS
if not targetList:
    # Default target.
    helpstring += "\nMaking the default target (compiling robin and griffin).\n"
    targetList += map(str, DEFAULT_TARGETS)

if "librobin-1.0.so" in targetList or \
   "librobin_pyfe-1.0.so" in targetList or \
   "librobin_stl.so" in targetList:
    helpstring += "\nFlags for compiling the SharedObjects of robin\n"
    helpstring += helpline
    helpstring += robin_opts.GenerateHelpText(env)

if "install" in targetList:
    helpstring += "\nFlags for installing Robin and Griffin\n"
    helpstring += helpline
    helpstring += install_opts.GenerateHelpText(env)

Help(helpstring)
