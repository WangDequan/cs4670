# Makefile for ImageLib

IMAGELIB=libImage.a
IMAGELIB_OBJS=WarpImage.o Convert.o Convolve.o FileIO.o Image.o ImageProc.o Pyramid.o RefCntMem.o Transform.o

CC=g++
CPPFLAGS=-Wall #-O3

all: $(IMAGELIB)

$(IMAGELIB): $(IMAGELIB_OBJS)
	ar r $(IMAGELIB) $(IMAGELIB_OBJS)

clean:
	rm -f *.o *~ $(IMAGELIB)
