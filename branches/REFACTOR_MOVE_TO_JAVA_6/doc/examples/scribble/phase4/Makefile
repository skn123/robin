
%.o: %.cc
	g++ `pkg-config --cflags gtk+-2.0` -c $< -o $@


libscribble.so: 4robin.o scribble.o
	g++ -shared `pkg-config --libs gtk+-2.0 gthread-2.0` -o $@ $+

clean:
	rm *.o 4robin.cc
