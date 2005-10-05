import robin, stl
robin.loadLibrary(__name__, "./libprotocols.so")
Times.__getitem__ = "mul"
Times.__setitem__ = "triangle"
Times.__delitem__ = "omit"
