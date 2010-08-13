import robin_v1_1, stl

robin_v1_1.loadLibrary(__name__, "./libscribble.so")

s = Scribble()
import thread
thread.start_new(s.run, ())


def nice_sine(i):
    import math
    return int(math.sin(i/20.) * 50 + 120)

def mkpoint(x, y):
    p = Point()
    p.x = x
    p.y = y
    return p
