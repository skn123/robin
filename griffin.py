"""Basic platform-dependant configuration"""

import os, os.path

# Detect architecture
try:
	uname = os.uname()[0]
except AttributeError:
	uname = os.getenv("OS")

isCygwin = uname.startswith("CYGWIN")

if isCygwin:
	java_pathsep = ';' # Cygwin is a strange hybrid
	def classpath(elements):
		import commands
		return commands.getoutput("cygpath -w -p '%s'" % \
			":".join(elements))
else:
	java_pathsep = os.path.pathsep
	def classpath(elements):
		return ":".join(elements)
