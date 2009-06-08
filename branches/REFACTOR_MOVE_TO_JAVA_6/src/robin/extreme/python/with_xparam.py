import imp, os.path 
here = os.path.dirname(__file__) 
lib = os.path.join(here, "./lib/%(m)s/with_xparammodule%(so)s") 
imp.load_dynamic(__name__, lib % {"m": os.getenv("MACHINE"), \
								  "so": ".so"})
