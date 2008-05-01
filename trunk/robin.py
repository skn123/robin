import imp, string, sys, os.path, __builtin__
from griffin import uname, arch, soext, sopre, platspec, pyspec, vpath


def here(file):
	import os
	if file.endswith(".pyc") and os.path.isfile(file[:-1]):
		file = file[:-1]
	return os.path.dirname(os.path.realpath(file)) 

def locate(thisfile, target):
	import os
	libpath = [here(thisfile)] + sys.path
	if libdir: libpath[:0] = [os.path.join(libdir, vpath)]
	for ldir in libpath:
		libfile = os.path.join(ldir, target)
		if os.path.isfile(libfile):
			return libfile
	else:
		raise ImportError, "%s could not be found" % target

libdir = None
ver = "1.0"

target = "%(sopre)srobin_pyfe%(platspec)s-%(ver)s%(pyspec)s%(soext)s" % vars()
libfile = locate(__file__, target)

__pyfile__ = __file__
imp.load_dynamic(__name__, libfile)
__file__ = __pyfile__
__builtin__.double = double
__builtin__.char = char
__builtin__.longlong = longlong
__builtin__.ulong = ulong
__builtin__.uint = uint
__builtin__.uchar = uchar
__builtin__.ulonglong = ulonglong
__builtin__.schar = schar

ldinfo = { 'm': arch, 'so': soext, \
           'suffix': "", 'confdir': "." }


def implement(tp):
	import warnings
	warnings.warn("robin.implement is deprecated")
	return tp

# Cleanup
del imp, os, target, __builtin__
