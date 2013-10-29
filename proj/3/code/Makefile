HEADERS = BrushConfigUI.h correlation.h FltDesignUI.h HelpPageUI.h ImageLib/FileIO.h ImageLib/Image.h ImageLib/RefCntMem.h ImgFilterUI.h ImgFltAux.h imgflt.h ImgView.h iScissor.h PriorityQueue.h ScissorPanelUI.h
CPPS = BrushConfigUI.cpp correlation.cpp FltDesignUI.cpp HelpPageUI.cpp ImageLib/FileIO.cpp ImageLib/Image.cpp ImageLib/RefCntMem.cpp ImgFilterUI.cpp ImgFltAux.cpp ImgFltMain.cpp ImgView.cpp iScissor.cpp ScissorPanelUI.cpp
OBJS = PanoramaMain.o BlendImages.o FeatureAlign.o FeatureSet.o WarpSpherical.o SVD.o
IMAGELIB=ImageLib/libImage.a


UNAME := $(shell uname)

# Flags for OSX
ifeq ($(UNAME), Darwin)
# Set this variable to the directory where libpng is installed. The
# one bellow is where MacPorts usually puts things, if you used some other
# package manager (e.g. brew) or installed libpng from source you will
# have to modify it.
PNGLIB_PATH=/opt/local/lib 
LIBS =  -L$(PNGLIB_PATH) `fltk-config --libs --use-images` -framework Cocoa -lpng -ljpeg -lz
CFLAGS = -g `fltk-config --cxxflags`

else
# Flags for Linux / Cygwin
LIBS = -L$(HOME)/local/lib `fltk-config --libs --use-images` -lpng -ljpeg -lfltk -L/usr/X11R6/lib -lXext -lXft -lXinerama -lfontconfig -lX11 -ldl
CFLAGS = -g
endif

all: Panorama
%.o: %.cpp
	g++ -c -o $@ $(CFLAGS) $<

Panorama: $(OBJS) $(IMAGELIB)
	g++ -o $@ $(CFLAGS) $(OBJS) $(IMAGELIB) $(LIBS)

$(IMAGELIB): 
	make -C ImageLib

clean:
	rm -rf Panorama *.o */*.o *~

.PHONY: clean
