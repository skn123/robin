
##################################################
#
# Robin C++ Library Targets
#
##################################################

ver = "1.0"
fullver = "1.0.1"

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

env = Environment()
env.Append(CPPPATH = ["src"])

# Debug mode (for developers)
robin_opts = Options()
robin_opts.Add(BoolOption('debug', 'Set a value to compile a debug version', 0))
if ARGUMENTS.get('debug', 0):
	env.Append(CXXFLAGS = "-g")

# Configure library prefix and auto-import flag for Cygwin
import os.path, griffin as conf
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
pyenv.Append(CXXFLAGS = "-D_VERSION=" + fullver)

if conf.arch == "windows":
	env.Append(CXXFLAGS = "/EHsc")
	pyenv.Append(CXXFLAGS = "/EHsc")
if not configure.CheckCXXHeader("Python.h"):
	print "Missing Python.h !"
	Exit(1)
if configure.CheckCXXHeader("ext/hash_map"):
	env.Append(CXXFLAGS = '-DWITH_EXT_HASHMAP')
if configure.CheckLib(LIBPY1):
	LIBPY = LIBPY1
else:
	if configure.CheckLib(LIBPY2):
		LIBPY = LIBPY2
	else:
		print "Missing library for -l%s or -l%s !" % (LIBPY1, LIBPY2)
		Exit(1)

# Add additional flags
env.Append(LINKFLAGS = "-Wl,-z,defs")

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
	                          LIBS=["dl","iberty"])

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

env = Environment()

premises = ["jython.jar", "antlr-2.7.5.jar", "xercesImpl.jar", "junit.jar", 
            "xmlParserAPIs.jar", "swt.jar", "jface.jar"]
premisedir = "./premises"

classpath = [os.path.join(premisedir, x) for x in premises]
env.Append(JAVACFLAGS = "-classpath '%s'" % conf.java_pathsep.join(classpath))


def jarme(source, target, env):
    os.system("jar cf %s -C build/griffin ." % target[0])


griffin = env.Java("build/griffin", "src/griffin")
env['BUILDERS']['Jar'] = Builder(action = jarme)
jar = env.Jar("Griffin.jar", griffin)

stl_dox = env.Command("build/stl.tag", "src/griffin/modules/stl", 
                      "( cd src/griffin/modules/stl; doxygen )")

Default(jar, stl_dox)

##################################################
#
# Install all the packages.
#
##################################################
def get_site_packages_dir():
    import sys
    for element in sys.path:
        if element.endswith("/site-packages") or element.endswith("\\site-packages"):
            return element
    # Failed to find site-packages directory
    print "** Error: couldn't locate 'site-packages' in your Python " \
          "installation"
    sys.exit(1)

env = Environment()
install_opts = Options()
install_opts.AddOptions(
                PathOption("prefix", "Installation prefix directory", "/usr"),
                PathOption("exec_prefix",
                    "Installation prefix directory for platform-dependant files", "/usr"),
                ("with_python", "Determines which Python interpreter to use", "python"),
             )

prefix = ARGUMENTS.get('prefix', "/usr")
exec_prefix = ARGUMENTS.get('exec_prefix', "/usr")
python = ARGUMENTS.get('woth_python', "python")
site_packages = get_site_packages_dir()

### Install python packages.
pydir = site_packages + "/robin"
env.Install(pydir, ["robin.py", "griffin.py"])
env.Install(pydir, ["src/robin/modules/" + file for file in [
                       "stl.py", "robinhelp.py", "document.py", "pickle_weakref.py",]])
env.Alias('install', pydir)
env.Install(pydir + "/html", ["src/robin/modules/" + file for file in [
                                 "html/__init__.py", "html/textformat.py"]])
env.Alias('install', pydir + "/html")

### Install Robin Libraries.
ver = "1.0"
soext = ".so"
libdir = exec_prefix + "/lib"
robin = "librobin-%s%s" % (ver, soext)
robin_pyfe = "librobin_pyfe-%s%s" % (ver, soext)
robin_stl = "librobin_stl%s" % (soext)
env.Install(libdir, [robin, robin_pyfe, robin_stl])
env.Alias('install', libdir)

### Install Griffin jars.
jardir = libdir + "/griffin"
env.Install(jardir, ["Griffin.jar", "src/griffin/modules/stl/stl.st.xml", "build/stl.tag"])
env.Alias('install', jardir)
        
### Install launch Scripts.
scriptdir = prefix + "/bin"
env.Install(scriptdir, ["griffin"])
env.Alias('install', scriptdir)
        
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
