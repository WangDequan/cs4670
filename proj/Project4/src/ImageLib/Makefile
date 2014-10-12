# Makefile for ImageLib

IMAGELIB=libImage.a
IMAGELIB_OBJS=Transform.o WarpImage.o FileIO.o Image.o RefCntMem.o

CC=g++
CPPFLAGS=-Wall -O0

all: $(IMAGELIB)

$(IMAGELIB): $(IMAGELIB_OBJS)
	ar r $(IMAGELIB) $(IMAGELIB_OBJS)

clean:
	rm -f *.o *~ $(IMAGELIB)
