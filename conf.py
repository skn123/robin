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
else:
	java_pathsep = os.path.pathsep
