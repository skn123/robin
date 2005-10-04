
##################################################
#
# Robin C++ Library Targets
#
##################################################

BuildDir('build', 'src')

FOUNDATION_SRC = """build/robin/debug/trace.cc
"""

REFLECTION_SRC = """build/robin/reflection/argumentsbuffer.cc
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
build/robin/frontends/python/pythonfrontend.cc
build/robin/frontends/python/pythoninterceptor.cc
build/robin/frontends/python/pythonlowlevel.cc
build/robin/frontends/python/pythonobjects.cc
"""

LIBPREFIX = "lib"

env = Environment()
env.Append(CPPPATH = ["src"])

# Configure library prefix and auto-import flag for Cygwin
import os.path, conf
if conf.isCygwin:
	env["SHLIBPREFIX"] = "lib"
	env.Append(LINKFLAGS = "-Wl,--enable-auto-import")

# Configure Python include and library
import sys, distutils.sysconfig

INCLUDEPY = distutils.sysconfig.get_config_var("INCLUDEPY")
LIBP = distutils.sysconfig.get_config_var("LIBP")
EXEC_PREFIX = distutils.sysconfig.get_config_var("exec_prefix")
if LIBP:
	LIBPYCFG = os.path.join(LIBP, "config")
else:
	LIBPYCFG = os.path.join(EXEC_PREFIX, "libs")
LIBPY1 = "python%i.%i" % sys.version_info[:2]
LIBPY2 = "python%i%i" % sys.version_info[:2]

pyenv = env.Copy()
configure = Configure(pyenv)

pyenv.Append(CPPPATH = [INCLUDEPY])
pyenv.Append(LIBPATH = [".", LIBPYCFG])

if not configure.CheckCXXHeader("Python.h"):
	print "Missing Python.h !"
	Exit(1)
if configure.CheckLib(LIBPY1):
	LIBPY = LIBPY1
else:
	if configure.CheckLib(LIBPY2):
		LIBPY = LIBPY2
	else:
		print "Missing library for -l%s or -l%s !" % (LIBPY1, LIBPY2)
		Exit(1)


env.SharedLibrary("robin", Split(FOUNDATION_SRC) + \
                           Split(REFLECTION_SRC) + \
                           Split(REGISTRATION_SRC) + \
                           Split(FRONTEND_FRAMEWORK_SRC))

pyenv.SharedLibrary("robin_pyfe", Split(PYTHON_FRONTEND_SRC), 
                    LIBS=["robin", LIBPY])

env.SharedLibrary("robin_stl", ["build/robin/modules/stl/stl_robin.cc"])


##################################################
#
# Griffin Java Source-Analyzer
#
##################################################

import os.path

env = Environment()

premises = ["jython.jar", "antlr-2.7.5.jar", "xercesImpl.jar", "junit.jar", 
            "xmlParserAPIs.jar"]
premisedir = "./premises"

classpath = [os.path.join(premisedir, x) for x in premises]
env.Append(JAVACFLAGS = "-classpath '%s'" % conf.java_pathsep.join(classpath))


def jarme(source, target, env):
    os.system("jar cf %s -C build/griffin ." % target[0])


griffin = env.Java("build/griffin", "src/griffin")
env['BUILDERS']['Jar'] = Builder(action = jarme)
jar = env.Jar("Griffin.jar", griffin)
