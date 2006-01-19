
##################################################
#
# Robin C++ Library Targets
#
##################################################

ver = "1.0"
fullver = "1.0.2"

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
"""

REGISTRATION_SRC = """build/robin/registration/mechanism.cc
build/robin/registration/regdata.cc
"""

FRONTEND_FRAMEWORK_SRC = """build/robin/frontends/framework.cc
"""

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

env = Environment()
env.Append(CPPPATH = ["src"])

# Debug mode (for developers)
robin_opts = Options()
robin_opts.Add(BoolOption('debug', 
                          'Set a value to compile a debug version', 0))
robin_opts.Add(BoolOption('clsux', 
                          'Set this to 1 if cl.exe fails to initialize', 0))

if ARGUMENTS.get('debug', 0):
	env.Append(CXXFLAGS = "-g")
if not ARGUMENTS.get('clsux', 0):
	env['ENV']['PATH'] = os.environ['PATH']

# Configure library prefix and auto-import flag for Cygwin
import os.path, griffin as conf
if conf.isCygwin:
	env["SHLIBPREFIX"] = "lib"
	env.Append(LINKFLAGS = "-Wl,--enable-auto-import")

# Configure Python include and library
import sys, distutils.sysconfig

def CheckTemplate(context, templatename, pre=""):
	context.Message("Checking for %s..." % templatename)
	result = context.TryCompile(pre + "namespace __s { using %s; }\n" \
					% templatename, ".cxx")
	context.Result(result)
	return result

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
configure = Configure(pyenv, custom_tests = {'CheckTemplate': CheckTemplate})

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
if configure.CheckCXXHeader("libiberty.h") and configure.CheckLib("iberty"):
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

# Set up targets
if conf.arch == "windows":
	robin = env.StaticLibrary("robin-"+ver, Split(FOUNDATION_SRC) + \
	                                        Split(REFLECTION_SRC) + \
	                                        Split(REGISTRATION_SRC) + \
	                                        Split(FRONTEND_FRAMEWORK_SRC))
else:
	robin = env.SharedLibrary("robin-"+ver, Split(FOUNDATION_SRC) + \
	                                        Split(REFLECTION_SRC) + \
	                                        Split(REGISTRATION_SRC) + \
	                                        Split(FRONTEND_FRAMEWORK_SRC),
	                          LIBS = AUXLIBS)

pyfe = pyenv.SharedLibrary("robin_pyfe-"+ver, Split(PYTHON_FRONTEND_SRC), 
                           LIBS=["robin-"+ver, LIBPY])

stl = env.SharedLibrary("robin_stl", ["build/robin/modules/stl/stl_robin.cc"])

Default(robin, pyfe, stl)


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
env.Append(JAVACFLAGS = '-classpath "%s"' % conf.java_pathsep.join(classpath))


def jarme(source, target, env):
    os.system("jar cf %s -C build/griffin ." % target[0])


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