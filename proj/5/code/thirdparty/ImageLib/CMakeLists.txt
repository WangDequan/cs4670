CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_SUBDIRECTORY(thirdparty/JPEG)

ADD_LIBRARY(image STATIC
	Convert.h         Convert.inl
	Convolve.h                          Convolve.cpp
	FileIO.h                            FileIO.cpp  
	Image.h           Image.inl         Image.cpp
	ImageProc.h                         ImageProc.cpp
	Pyramid.h         Pyramid.inl     
	RefCntMem.h                         RefCntMem.cpp
	Transform.h                         Transform.cpp
	WarpImage.h       WarpImage.inl
    IPLInterface.h 
    ImageLib.h)	

INCLUDE_DIRECTORIES(thirdparty ${JPEG_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(image jpegrw ${JPEG_LIBRARIES})
