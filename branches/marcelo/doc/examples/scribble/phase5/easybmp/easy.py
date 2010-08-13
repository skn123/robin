import robin_v1_1
import os
robin_v1_1.loadLibrary("easy", os.path.join(os.path.dirname(__file__), "./libeasy.so"))

BYTE.__to__ = lambda x: ord(x.as())
