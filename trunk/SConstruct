FOUNDATION_SRC = """src/robin/debug/trace.cc
"""

REFLECTION_SRC = """src/robin/reflection/argumentsbuffer.cc
src/robin/reflection/boundmethod.cc
src/robin/reflection/cfunction.cc
src/robin/reflection/class.cc
src/robin/reflection/conversion.cc
src/robin/reflection/conversiontable.cc
src/robin/reflection/enumeratedtype.cc
src/robin/reflection/fundamental_conversions.cc
src/robin/reflection/instance.cc
src/robin/reflection/interface.cc
src/robin/reflection/intrinsic_type_arguments.cc
src/robin/reflection/library.cc
src/robin/reflection/low_level.cc
src/robin/reflection/memorymanager.cc
src/robin/reflection/method.cc
src/robin/reflection/namespace.cc
src/robin/reflection/overloadedset.cc
src/robin/reflection/typeofargument.cc
"""

REGISTRATION_SRC = """src/robin/registration/mechanism.cc
src/robin/registration/regdata.cc
"""

FRONTEND_FRAMEWORK_SRC = """src/robin/frontends/framework.cc
"""

PYTHON_FRONTEND_SRC = """src/robin/frontends/python/enhancements.cc
src/robin/frontends/python/facade.cc
src/robin/frontends/python/inheritance.cc
src/robin/frontends/python/module.cc
src/robin/frontends/python/pythonadapters.cc
src/robin/frontends/python/pythonconversions.cc
src/robin/frontends/python/pythonfrontend.cc
src/robin/frontends/python/pythoninterceptor.cc
src/robin/frontends/python/pythonlowlevel.cc
src/robin/frontends/python/pythonobjects.cc
"""

LIBPREFIX = "lib"

env = Environment()
env.Append(CPPPATH = ["src"])

# Configure library prefix and auto-import flag for Cygwin
import os
if os.uname()[0].startswith("CYGWIN"):
	env["SHLIBPREFIX"] = "lib"
	env.Append(LINKFLAGS = "-Wl,--enable-auto-import")

# Configure Python include and library
import sys, distutils.sysconfig

INCLUDEPY = distutils.sysconfig.get_config_var("INCLUDEPY")
LIBPYCFG = os.path.join(distutils.sysconfig.get_config_var("LIBP"), "config")
LIBPY = "python%i.%i" % sys.version_info[:2]

pyenv = env.Copy()
conf = Configure(pyenv)

pyenv.Append(CPPPATH = [INCLUDEPY])
pyenv.Append(LIBPATH = [".", LIBPYCFG])

if not conf.CheckCXXHeader("Python.h"):
	print "Missing Python.h !"
	Exit(1)
if not conf.CheckLib(LIBPY):
	print "Missing library for -l%s !" % LIBPY
	Exit(1)


env.SharedLibrary("robin", Split(FOUNDATION_SRC) + \
                           Split(REFLECTION_SRC) + \
                           Split(REGISTRATION_SRC) + \
                           Split(FRONTEND_FRAMEWORK_SRC))

pyenv.SharedLibrary("robin_pyfe", Split(PYTHON_FRONTEND_SRC), 
                    LIBS=["robin", LIBPY])
