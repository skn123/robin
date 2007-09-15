import unittest, sys, os

try:
    __file__
except NameError:
    __file__ = sys.argv[0]

def normjoin(*args):
    return os.path.normpath(os.path.join(*args))

# Need to 'import unittest' before adding Griffin.jar to the path, because it
# contains a 'unittest' Java package
sys.path.append(normjoin(os.path.dirname(__file__), "..", "Griffin.jar"))
#sys.path.append("../Griffin.jar")

def suite():
    import griffin
    return griffin.suite()

if __name__ == "__main__": unittest.main(defaultTest="suite")
