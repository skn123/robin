import robin, stl

robin.loadLibrary(__name__, "./libscribble.so")

s = Scribble()
import thread
thread.start_new(s.run, ())


def nice_sine(i):
    import math
    return int(math.sin(i/20.) * 50 + 120)

def mkpoint(x, y):
    p = Point[int]()
    p.x = x
    p.y = y
    return p

Point[int].__repr__ = lambda p: "%i,%i" % (p.x,p.y)
Point[int].__from__[()] = lambda (x,y): mkpoint(x,y)
