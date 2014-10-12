# Makefile for project 2

PROJ=svm
PROJ_OBJS=$(shell ls *.cpp | awk '{sub(".cpp", ".o"); print}') svmCMath.o
IMAGELIB=ImageLib/libImage.a

CC=g++

UNAME := $(shell uname)
# Flags for OSX
ifeq ($(UNAME), Darwin)
#-O3
CPPFLAGS= -g --no-warnings `fltk-config --cflags` -I ../
LIB_PATH=-L/usr/X11R6/lib -L/usr/lib `fltk-config --ldflags`
LIBS=-lfltk -lfltk_images -lpng -ljpeg -lX11 \
     `fltk-config --libs --use-gl --use-glut` \
     -framework OpenGL -framework GLUT -framework AGL \
     -DYA_BLAS -DYA_LAPACK -DYA_BLASMULT  -lblas -llapack -framework vecLib
else
# Flags for Linux / Cygwin
CPPFLAGS=-Wall -O3 `fltk-config --cflags` -I ../ -I /usr/include
LIB_PATH=-L/usr/X11R6/lib -L/usr/lib
LIBS= -lpng -ljpeg `fltk-config --libs --use-gl --use-glut --use-images` -lGL -lglut -lGLU -lX11 -lXft -lXinerama -lXext -ldl -lfontconfig
endif

all: $(PROJ)

$(IMAGELIB): 
	make -C ImageLib

$(PROJ): $(PROJ_OBJS) $(IMAGELIB)
	$(CC) -o $@ $(LIB_PATH) $(PROJ_OBJS) $(IMAGELIB) $(LIBS) 

clean:
	make -C ImageLib clean
	rm -f *.o *~ $(PROJ)
