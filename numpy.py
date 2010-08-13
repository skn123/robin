import robin, os

if os.path.islink(__file__): __file__ = os.readlink(__file__)
here = os.path.dirname(__file__)
machine = os.getenv("MACHINE")
soext = 'so'
lib = os.path.join(here, "lib/%s/numpy_robin.%s")
robin.loadLibrary(__name__, lib % (machine, soext))

engage()
