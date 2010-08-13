# This file is a shortcut for the user.
# It is easier to do 'import robin' and not 'import robin_v1_1'
# But still the libraries have to be wrapped specifically for a specific robin
# and we will not allow to do 'robin.loadLibrary'.

ver = "v1_1"
from robin_v1_1 import *

def loadLibrary(modulename, path):
        raise RuntimeError("""
              ROBIN COMPATIBILITY EXCEPTION
              ==============================
        
              Libraries have to be wrapped for the robin version they
              are loaded with. From now on, to assure that the proper version 
              of robin is loaded you have to choose it explicitly.
              That means that loading a library requires the user to 
              call robin_v1_1.loadLibrary instead of robin.loadLibrary

              This exception can happen if you are mixing libraries wrapped with
              two different robin versions, which is not permitted. 
              In that case you would need to recompile and rewrap to make 
              your program work. 
              """)