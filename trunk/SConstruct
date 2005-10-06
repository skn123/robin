class IBuilder:
    "Interface for a builder class."
    def __init__(self):
        "Init the builddir and some other variables"
        from os import system
        from os.path import join
        import griffin
        self.system = system
        self.path_join = join
        self.conf = griffin
        self.env = Environment()
        BuildDir('build', 'src')
    
    def build(self):
        "Do the actual build"
        pass

class RobinBuilder(IBuilder):
    "Robin C++ Library targets builder"
    def __init__(self):
        "Init the environment"
        IBuilder.__init__(self)
        self.ver = "1.0"

    def build(self):
        "Build all the targets"
        Default(self.buildRobin(),
                self.buildPyfe(),
                self.buildStlWrap())

    def buildRobin(self):
        "Build the Robin C++ Library."

        FOUNDATION_SRC = """
            build/robin/debug/trace.cc
        """.strip()

        REFLECTION_SRC = """
            build/robin/reflection/argumentsbuffer.cc
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
        """.strip()

        REGISTRATION_SRC = """
            build/robin/registration/mechanism.cc
            build/robin/registration/regdata.cc
        """.strip()

        FRONTEND_FRAMEWORK_SRC = """
            build/robin/frontends/framework.cc
        """.strip()

        LIBPREFIX = "lib"
        self.env.Append(CPPPATH = ["src"])

        # Debug mode (for developers)
        # TODO : Make help for this.
        if ARGUMENTS.get('debug', 0):
            self.env.Append(CXXFLAGS = "-g")

        # Configure library prefix and auto-import flag for Cygwin
        if self.conf.isCygwin:
            self.env["SHLIBPREFIX"] = "lib"
            self.env.Append(LINKFLAGS = "-Wl,--enable-auto-import")

        return self.env.SharedLibrary("robin-"+self.ver, Split(FOUNDATION_SRC) + \
                                                         Split(REFLECTION_SRC) + \
                                                         Split(REGISTRATION_SRC) + \
                                                         Split(FRONTEND_FRAMEWORK_SRC))

    def buildPyfe(self):
        "Build the robin python frontend"

        PYTHON_FRONTEND_SRC = """
            build/robin/frontends/python/enhancements.cc
            build/robin/frontends/python/facade.cc
            build/robin/frontends/python/inheritance.cc
            build/robin/frontends/python/module.cc
            build/robin/frontends/python/pythonadapters.cc
            build/robin/frontends/python/pythonconversions.cc
            build/robin/frontends/python/pythonfrontend.cc
            build/robin/frontends/python/pythoninterceptor.cc
            build/robin/frontends/python/pythonlowlevel.cc
            build/robin/frontends/python/pythonobjects.cc
        """.strip()

        # Configure Python include and library
        import sys, distutils.sysconfig

        INCLUDEPY = distutils.sysconfig.get_config_var("INCLUDEPY")
        LIBP = distutils.sysconfig.get_config_var("LIBP")
        EXEC_PREFIX = distutils.sysconfig.get_config_var("exec_prefix")
        if LIBP:
            LIBPYCFG = self.path_join(LIBP, "config")
        else:
            LIBPYCFG = self.path_join(EXEC_PREFIX, "libs")
        LIBPY1 = "python%i.%i" % sys.version_info[:2]
        LIBPY2 = "python%i%i" % sys.version_info[:2]
        pyenv = self.env.Copy()
        configure = Configure(pyenv)

        pyenv.Append(CPPPATH = [INCLUDEPY])
        pyenv.Append(LIBPATH = [".", LIBPYCFG])

        if not configure.CheckCXXHeader("Python.h"):
            print "Missing 'Python.h'!"
            Exit(1)

        if configure.CheckLib(LIBPY1):
            LIBPY = LIBPY1
        elif configure.CheckLib(LIBPY2):
            LIBPY = LIBPY2
        else:
            print "Missing library for '-l%s' or '-l%s'!" % (LIBPY1, LIBPY2)
            Exit(1)

        pyenv = configure.Finish()

        return pyenv.SharedLibrary("robin_pyfe-"+self.ver, Split(PYTHON_FRONTEND_SRC), 
                                   LIBS=["robin-"+self.ver, LIBPY])

    def buildStlWrap(self):
        "Build the robin wrap for the stl"

        return self.env.SharedLibrary("robin_stl", ["build/robin/modules/stl/stl_robin.cc"])

class GriffinBuilder(IBuilder):
    "Griffin Java Source-Analyzer builder"
    def build(self):
        "Build all griffin targets"
        Default(self.buildGriffinJar(),
                self.buildStlDox())

    def buildGriffinJar(self):
        "Build the griffin jar"

        premisedir = "./premises"
        premises = ["jython.jar", "antlr-2.7.5.jar", "xercesImpl.jar", "junit.jar", 
                    "xmlParserAPIs.jar"]

        classpath = [self.path_join(premisedir, x) for x in premises]
        self.env.Append(JAVACFLAGS = "-classpath '%s'" % self.conf.java_pathsep.join(classpath))

        def jarme(source, target, env):
            self.system("jar cf %s -C build/griffin ." % target[0])

        griffin = self.env.Java("build/griffin", "src/griffin")
        self.env['BUILDERS']['Jar'] = Builder(action = jarme)
        return self.env.Jar("Griffin.jar", griffin)

    def buildStlDox(self):
        return self.env.Command("build/stl.tag", "src/griffin/modules/stl", 
                           "( cd src/griffin/modules/stl; doxygen )")

RobinBuilder().build()
GriffinBuilder().build()
