import os


##################################################
#
# Create a debian package for Robin.
#
##################################################

# Here's the core info for the package

DEBNAME = "robin"
DEBMAINT = "Misha Seltzer [mishas@users.sourceforge.net]"
DEBARCH = "i386"
DEBDEPENDS = "doxygen (>= 1.3.8), java-runtime, antlr (>= 2.7.5), libxerces2-java"
DEBDESC = """Robin is a framework that automatically generates Python bindings to C++ libraries.
  Robin is an Open-Source project providing quick, hands-free automated bindings
  of C++ libraries for Python, and possibly some other scripting languages (such
  as Java, Groovy, and Ruby) in the future. This is achieved by creating a
  minimal amount of "Glue Code" above the existing routines, along with some
  "Meta-Data" which, when combined with a thin run-time mechanism exposes the C++
  types as native types in the target language.
  Robin's main ideal is to make bindings as tight as possible, such that
  usability will resemble that which the user can produce in the original C++
  code; Some examples of such "sharp edges" are:
  * Imitating C++-style implicit conversions
  * Coupling STL primitives with the primitives of the target language
  * Automatic template instantiation and exposure of template
  * instances
  * Allowing user Python code to derive from a C++ interface
  * class and implement virtual methods
"""

libdir = "usr/lib/"
bindir = "usr/bin/"
jardir = "usr/share/java/"
pycdir = "usr/lib/python2.4/site-packages/"
robindir = "usr/share/robin/"

def getDebFilesListForDir(srcDir, destDir):
  l = []
  if not hasattr(os, 'walk'): 
    import warnings
    warnings.warn("no debian package for python < 2.3")
    return l
  for (dirpath, dirnames, filenames) in os.walk(srcDir):
    delta_dir = dirpath.split(srcDir)[1]
    if ".svn" in dirpath:
      continue
    for fn in filenames:
      l.append(("%s/%s/%s" % (destDir, delta_dir, fn), "%s/%s" % (dirpath, fn)))
  return l


def debian_package(env, robin, stl, pyfe, jar, stl_dox, fullver, Copy):
    DEBFILES = [
        (libdir + robin[0].name,        robin[0].path),
        (libdir + stl[0].name,          stl[0].path),
        (pycdir + pyfe[0].name,         pyfe[0].path),
        (jardir + jar[0].name,          jar[0].path),
        (robindir + stl_dox[0].name,      stl_dox[0].path),
        (robindir + "stl.st.xml",         "src/griffin/modules/stl/stl.st.xml"),
        (bindir + "griffin",            "griffin"),
        (pycdir + "robin.py",           "robin.py"),
        (pycdir + "griffin.py",         "griffin.py"),
        (pycdir + "stl.py",             "stl.py"),
    ] + (
      getDebFilesListForDir("build/dox-xml", robindir + "dox-xml/") +
      getDebFilesListForDir("robinlib", pycdir + "robinlib/"))

    # This is the debian package we're going to create
    debpkg = '#%s_%s.deb' % (DEBNAME, fullver)

    # and we want it to be built when we build 'debian'
    env.Alias("debian", debpkg)

    DEBCONTROLFILE = os.path.join(DEBNAME, "DEBIAN/control")

    # This copies the necessary files into place into place.
    # Fortunately, SCons creates the necessary directories for us.
    for f in DEBFILES:
        # We put things in a directory named after the package
        dest = os.path.join(DEBNAME, f[0])
        # The .deb package will depend on this file
        env.Depends(debpkg, dest)
        # Copy from the the source tree.
        env.Command(dest, f[1], Copy('$TARGET','$SOURCE'))
        # The control file also depends on each source because we'd like
        # to know the total installed size of the package
        env.Depends(DEBCONTROLFILE, dest)

    # Now to create the control file:
    CONTROL_TEMPLATE = """
    Package: %s
    Priority: optional
    Section: devel
    Installed-Size: %s
    Maintainer: %s
    Architecture: %s
    Version: %s
    Depends: %s
    Description: %s
    """
    env.Depends(debpkg, DEBCONTROLFILE)

    # This function creates the control file from the template and info
    # specified above, and works out the final size of the package.
    def make_control(target=None, source=None, env=None):
        installed_size = 0
        for i in DEBFILES:
            installed_size += os.stat(str(env.File(i[1])))[6]
        control_info = CONTROL_TEMPLATE % (
            DEBNAME, installed_size, DEBMAINT, DEBARCH,
            fullver, DEBDEPENDS, DEBDESC)
        f = open(str(target[0]), 'w')
        f.write(control_info)
        f.close()
        os.chmod(os.path.dirname(str(target[0])), 0755)

    # We can generate the control file by calling make_control
    env.Command(DEBCONTROLFILE, None, make_control)

    # And we can generate the .deb file by calling dpkg-deb
    env.Command(debpkg, DEBCONTROLFILE,
                "dpkg-deb -b %s %s" % ("%s" % DEBNAME, "$TARGET"))

