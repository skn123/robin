from scrib import *
import easy, robin

def go():
	b = easy.BMP()
	b.ReadFromFile("up.bmp")

	for y in xrange(b.TellHeight()):
		for x in xrange(b.TellWidth()):
			if robin.disown(b(x,y)).Red == 0:
				s.paint(x*5, y*5)
