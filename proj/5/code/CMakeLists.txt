PROJECT(objdet) 

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Options for Mac OS X
# FIXME: Should do this only if generating for XCode
#SET(CMAKE_OSX_SYSROOT "") # Set to "Current OS X" 

# Where to search for cmake scripts
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Check for dependencies (Ref: http://www.cmake.org/Wiki/CMakeForFLTK)
# http://stackoverflow.com/questions/9779901/setting-up-fltk-on-windows-with-cmake

IF(WIN32)
	# FILTK
	# Find path sets FLTK_INCLUDE_DIR which in turn is used by FIND_PACKAGE when
	# searching for FLTK.
	FIND_PATH(FLTK_INCLUDE_DIR
  		NAMES FL/Fl.H
		PATHS "winlibs\\FLTK\\"
	)

	# JPEG
	FIND_PATH(JPEG_INCLUDE_DIR
  		NAMES jpeglib.h
		PATHS "winlibs\\libjpeg-turbo\\include\\"
	)

	FIND_LIBRARY(JPEG_LIBRARY
		NAMES jpeg-static.lib
		PATHS "winlibs\\libjpeg-turbo\\lib\\"
	)
ELSE(WIN32)
	FIND_PACKAGE(JPEG REQUIRED)
	LIST(APPEND CMAKE_CXX_FLAGS "-Wall")
ENDIF(WIN32)

# FLTK
SET(FLTK_SKIP_FLUID true)
FIND_PACKAGE(FLTK REQUIRED)	
INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIR})
IF(WIN32)
	SET(FLTK_LIB_PATH "winlibs\\FLTK\\lib\\")

	# For some reason find_package(fltk) wont include these
	# libraries in FLTK_LIBRARIES so I add them here.
	FIND_LIBRARY(FLTK_Z_LIB
	  NAMES fltkzlib fltkzlibd
	  PATHS ${FLTK_LIB_PATH}
	)

	FIND_LIBRARY(FLTK_JPEG_LIB
	  NAMES fltkjpeg fltkjpegd
	  PATHS ${FLTK_LIB_PATH}
	)

	FIND_LIBRARY(FLTK_PNG_LIB
	  NAMES fltkpng fltkpngd
	  PATHS ${FLTK_LIB_PATH}
	)

    LIST(APPEND FLTK_LIBRARIES ${FLTK_PNG_LIB} ${FLTK_JPEG_LIB} ${FLTK_Z_LIB})
ELSE(WIN32)
	FIND_PACKAGE(PNG REQUIRED)
    LIST(APPEND FLTK_LIBRARIES ${PNG_LIBRARIES})
ENDIF(WIN32)

FIND_PACKAGE(OpenGL REQUIRED)

IF(CMAKE_SYSTEM_NAME STREQUAL Linux)
	# Not sure why we need this one
	FIND_LIBRARY(LD_LIB NAMES dl)
    LIST(APPEND FLTK_LIBRARIES ${LD_LIB})
ENDIF()

# # Build subdirectories
ADD_SUBDIRECTORY(thirdparty/ImageLib)
INCLUDE_DIRECTORIES(thirdparty/)

ADD_SUBDIRECTORY(thirdparty/libsvm-3.14)

ADD_LIBRARY(od
    Utils.h                                             Utils.cpp
	Feature.h                                           Feature.cpp 
	SupportVectorMachine.h                              SupportVectorMachine.cpp 
	CroppedImageDatabase.h                              CroppedImageDatabase.cpp 
	ImageDatabase.h                                     ImageDatabase.cpp 
	PrecisionRecall.h                                   PrecisionRecall.cpp
	ObjectDetector.h                                    ObjectDetector.cpp
	Detection.h                                         Detection.cpp
	SubBandImagePyramid.h    SubBandImagePyramid.inl    
	FileIO.h                                            FileIO.cpp
	ParametersMap.h                                     ParametersMap.cpp
	Common.h    
)

TARGET_LINK_LIBRARIES(od image svm jpegrw ${JPEG_LIBRARY})

SET(GUI_SRC 
    GUIController.h                                     GUIController.cpp 
    GUIModel.h                                          GUIModel.cpp 
    ImageView.h                                         ImageView.cpp
    ImageViewGL.h                                       ImageViewGL.cpp
    ControlsBox.h                                       ControlsBox.cpp)

# # Building the project 
ADD_EXECUTABLE(objdet main.cpp ${GUI_SRC})
TARGET_LINK_LIBRARIES(objdet od ${FLTK_LIBRARIES} ${JPEG_LIBRARY} ${OPENGL_LIBRARIES})
