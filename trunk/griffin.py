"""Basic platform-dependant configuration"""

import os, os.path
import string

# Detect architecture
try:
	uname = os.uname()[0]
except AttributeError:
	uname = os.getenv("OS")
arch = uname.translate(string.maketrans("-_","  ")).split()[0].lower()
soext = { 'windows': ".dll", 'cygwin': ".dll", 'hp': ".sl" }.get(arch, ".so")
sopre = { 'windows': "" }.get(arch, "lib")

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
