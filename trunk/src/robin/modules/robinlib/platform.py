"""Basic platform-dependant configuration"""

import sys, os, os.path
import string
import struct

# Detect architecture
try:
	uname = os.uname()[0]
except AttributeError:
	uname = os.getenv("OS")
arch = uname.translate(string.maketrans("-_","  ")).split()[0].lower()
soext = { 'windows': ".dll", 'cygwin': ".dll", 'darwin': ".dylib", 'hp': ".sl" }.get(arch, ".so")
sopre = { 'windows': "" }.get(arch, "lib")
wordsize = struct.calcsize("l") * 8

# Load configuration
class DefaultConfig:
	prefix = '/usr/local'
	exec_prefix = '/usr/local'
	python = 'python'
	site_packages = \
	  ([""] + [x for x in sys.path if x.endswith("site-packages")])[-1]
	java = 'java'
	has_liberty = False
	
try:
	import config
except ImportError:
	config = DefaultConfig

if hasattr(config, 'multi_platform'):
	platspec = ["_%i" % wordsize, ""][wordsize == 32]
	pyspec = "-py" + sys.version[:3]
else:
	platspec = ""
	pyspec = ""

isCygwin = uname.startswith("CYGWIN")

if isCygwin:
	java_pathsep = ';' # Cygwin is a strange hybrid
	def classpath(elements):
		import commands
		return commands.getoutput("cygpath -w -p '%s'" % \
			":".join(elements))
else:
	try:
		java_pathsep = os.path.pathsep
	except AttributeError:
		java_pathsep = os.pathsep # Python 2.2 and older
	def classpath(elements):
		return ":".join(elements)

# Detect gcj
def is_gcj(env):
	"detects gcj for cases where javac is a symlink to wrapper"
	path = env.WhereIs('javac')
	if path is None: return False
	progname = os.path.split(os.path.realpath(path))[1]
	if progname.startswith("gcj"): return True
