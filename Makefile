.DEFAULT_GOAL := example

objects = example.o ptplayer.o

example.o: ptplayer.h
ptplayer.o: ptplayer.h

example: $(objects)
	cc $(objects) -lportaudio -lrt -lm -lasound -pthread -o $@

