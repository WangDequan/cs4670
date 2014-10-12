# Makefile for project 2

PROJ1=Features
PROJ1_OBJS=FeaturesDoc.o FeaturesUI.o FeatureSet.o ImageDatabase.o \
	ImageView.o FeaturesMain.o features.o

IMAGELIB=ImageLib/libImage.a

CC=g++

UNAME := $(shell uname)
# Flags for Linux
ifeq ($(UNAME), Linux)
CPPFLAGS=-Wall -O3
LIB_PATH=-L/usr/X11R6/lib -L/usr/lib
LIBS=-lfltk -lfltk_images -lpng -ljpeg -lX11
endif
# Flags for OSX
ifeq ($(UNAME), Darwin)
CPPFLAGS=--no-warnings -O3 `fltk-config --cflags`
LIB_PATH=-L/usr/X11R6/lib -L/usr/lib `fltk-config --ldflags`
LIBS=-lfltk -lfltk_images -lpng -ljpeg -lX11 `fltk-config --libs`
endif

all: $(PROJ1)

$(IMAGELIB): 
	make -C ImageLib

$(PROJ1): $(PROJ1_OBJS) $(IMAGELIB)
	$(CC) -o $@ $(PROJ1_OBJS) $(LIB_PATH) $(LIBS) $(IMAGELIB)

clean:
	make -C ImageLib clean
	rm -f *.o *~ $(PROJ1)
