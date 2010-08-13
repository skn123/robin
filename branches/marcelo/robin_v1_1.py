
ver = "v1_1"
import sys
if sys.modules.has_key("robin"):
	import robin
	if robin.ver != ver:
		raise RuntimeError("Mixing two robin versions.\n" +
		      "Robin version " + robin.ver + "loaded from " + robin.__file__ +"\n" +
		      "Robin version " + ver + "loaded from " + __file__ +
		      "Please resolve this conflict which might be caused by a bad PYTHONPATH or\n" +
		      "by using old libraries without re-wrapping them with the new robin.")


import  string,  os.path, __builtin__


def here(file):
	import os
	if file.endswith(".pyc") and os.path.isfile(file[:-1]):
		file = file[:-1]
	return os.path.dirname(os.path.realpath(file)) 

sys.path.insert(0, here(__file__))

from robinlib.platform import uname, arch, soext, sopre, platspec, pyspec, vpath
libdir = None

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

target = "%(sopre)srobin_pyfe%(platspec)s_%(ver)s%(pyspec)s" % vars()

def _stringImport(name):
	'''
	It does exactly the same as 'from packagename import *'
	but the name of the package is passed as a string parameter
	'''
	globals().update(__import__(name).__dict__)
_stringImport(target) # Importing the shared object file
	

__builtin__.double = double
__builtin__.char = char
__builtin__.longlong = longlong
__builtin__.ulong = ulong
__builtin__.uint = uint
__builtin__.uchar = uchar
__builtin__.ulonglong = ulonglong
__builtin__.schar = schar
__builtin__.short = short
__builtin__.ushort = ushort
__builtin__.c_int = c_int
__builtin__.c_long = c_long 
__builtin__.c_float = c_float

ldinfo = { 'm': arch, 'so': soext, \
           'suffix': "", 'confdir': "." }


def implement(tp):
	import warnings
	warnings.warn("robin.implement is deprecated")
	return tp


def getRealElementTypeFromDictionarykey(key):
    '''
    The objective of this function is to translate from the key used in template
    dictionaries (like std.vector)  to the type of the element.
    
    This function is needed because robin tries to make types more intuitive
    to the user even at the cost of wrong logic.
    
    The type: stl.vector[int] represents a std::vector<int> instead of a
    std::vector<long> in spite of int being exactly the same of a c long.
    
    There are currently three of those keys: long, int and float
    The rest of the keys are returned as-is (without any translation)
    '''
    dictionaryKeyTranslator = { 
             float: c_float, 
             long: c_long, 
             int : c_int
    }
    return dictionaryKeyTranslator.get(key,key)
    

def robinTypeFromTempArgument(type):
    '''
    It receives a type which template argument and converts it to 
    a RobinType, which also represents a const value. 
    '''
    robinType = RobinType(type)
    
    #elements need to be const
    if robinType.is_const:
        return robinType
    else:
        return searchOrCreateConstType(robinType)

# Cleanup
del os, target, __builtin__
