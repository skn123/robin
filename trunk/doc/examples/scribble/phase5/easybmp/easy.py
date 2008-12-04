import robin
import os
robin.loadLibrary("easy", os.path.join(os.path.dirname(__file__), "./libeasy.so"))

BYTE.__to__ = lambda x: ord(x.as())
