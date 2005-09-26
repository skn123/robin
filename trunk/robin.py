import imp, string, os.path, __builtin__

libdir = os.path.dirname(__file__)
model = ["RELEASE", "DEBUG"][os.environ.has_key("ROBIN_DEBUG")]

# Detect architecture
try:
	uname = os.uname()[0]
except AttributeError:
	uname = os.getenv("OS")
arch = uname.translate(string.maketrans("-_","  ")).split()[0].lower()
soext = { 'windows': ".dll", 'cygwin': ".dll", 'hp': ".sl" }.get(arch, ".so")

target = "librobin_pyfe" + soext

imp.load_dynamic("robin", os.path.join(libdir, target))
__builtin__.double = double
__builtin__.char = char
__builtin__.ulong = ulong
__builtin__.uint = uint
__builtin__.uchar = uchar

ldinfo = { 'm': arch, 'so': soext, \
           'suffix': "", 'confdir': "." }

def here(file):
	import os
	if file.endswith(".pyc") and os.path.isfile(file[:-1]):
		file = file[:-1]
	return os.path.dirname(os.path.realpath(file)) 


# Cleanup
del imp, os, libdir, target, __builtin__
