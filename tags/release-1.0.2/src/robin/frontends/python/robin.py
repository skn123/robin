import imp, sys, os.path, __builtin__

if (os.path.islink(__file__)): __file__ = os.readlink(__file__)
libdir = os.path.dirname(__file__)
pyspec = "py%i.%i" % sys.version_info[:2]
machine = os.environ["MACHINE"]
soext = "TODO: autoconf"

if machine == "win32":
	target = "lib/%s/%s/robin_pyfe%s"
else:
	target = "lib/%s/%s/librobin_pyfe%s"

target = target % (pyspec, machine, soext)
imp.load_dynamic("robin", os.path.join(libdir, target))
__builtin__.double = double
__builtin__.char = char
__builtin__.ulong = ulong
__builtin__.uint = uint
__builtin__.uchar = uchar

ldinfo = { 'm': machine, 'so': soext, 'suffix': "", 'confdir': "." }

def here(file):
	import os
	if file.endswith(".pyc") and os.path.isfile(file[:-1]):
		file = file[:-1]
	return os.path.dirname(os.path.realpath(file)) 


# Cleanup
del imp, sys, os, libdir, target, __builtin__
