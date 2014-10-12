#ifndef IMGFLT_H
#define IMGFLT_H

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ImageLib/Transform.h"

//#include <strstrea.h>

// FLTK
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/fl_message.H>
#include <FL/Fl_Check_Button.H>

// OpenGL
#ifndef _WIN32

#ifdef __APPLE__ 
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else // linux
#include <GL/glu.h>
#endif // __APPLE__
//#include <GLUT/glut.h>

#include <limits>

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#else
#include <GL/glu.h>
#endif

#include <FL/gl.h>


#include "ImageLib/Image.h"
#include "ImageLib/FileIO.h"

class svmUI;
class HelpPageUI;

class ImgView;

#include "svmAux.h"

#include "PriorityQueue.h"

#include "vec.h"

/***************************** Important Data Structures, must read! *************/

struct SVMPoint
{
	//2D Homogeneous Coordinates in image plane;
	//if w = 1, u, v, are image coordinates, ranging from 0 to imgWidth and 0 to imgHeight respectively;
	//else if w=0 means the point is at infinity;
	//else u/w, v/w are image coordinates.
	
	double u,v,w;
	
	//3D Homogeneous Coordinates in 3D world;

	double X,Y,Z,W;

	SVMPoint(void) //: known(false)
	{
		u=v=0; w=1;
		//X=Y=Z=0; W=1;
		X=Y=Z=W=0;
	}

	SVMPoint(double u0,double v0) //: known(false)
	{
		u=u0;v=v0;w=1;
		//X=u;Y=v;Z=0;W=1;
		X=Y=Z=W=0;
	}

	// EUGENE_ADD
	inline bool old_known() { return !(u==X && v==Y && Z==0); }
	inline bool known() { return (W!=0); }
	inline void known(bool flag) { W = flag ? 1 : 0; }

	//bool known; // whether the 3d coordinate is known
};

typedef CTypedPtrDblList<SVMPoint> PointList;

const int PARA_X = 1;
const int PARA_Y = 2;
const int PARA_Z = 3;
const int PARA_XZ = 4;
const int PARA_YZ = 5;
const int PARA_XY = 6;
const int OTHER_ORIENT = 0;

struct SVMLine
{	
	//oriention indicates whether the line is supposed to be parallel to X, Y, Z axis or just
	//any possible orienation in 3D. orientation is one of PARA_X, PARA_Y, PARA_Z, and OTHER_ORIENT.
	int orientation;

	SVMPoint *pnt1, *pnt2;

	SVMLine(void)
	{ 
		pnt1=pnt2=NULL; 
	}	
	
	SVMLine(const SVMLine &line)
	{ 
		orientation = line.orientation;
		pnt1=line.pnt1;
		pnt2=line.pnt2;
	}
};

typedef CTypedPtrDblList<SVMLine> LineList;

struct SVMRect
{	
	//oriention indicates whether the rect is supposed to be parallel to X, Y, Z axis or just
	//any possible orienation in 3D. orientation is one of PARA_X, PARA_Y, PARA_Z, and OTHER_ORIENT.
	int orientation;

	SVMPoint *pnt1, *pnt2;

	SVMRect(void)
	{ 
		pnt1=pnt2=NULL; 
	}	
	
	SVMRect(const SVMRect &rect)
	{ 
		orientation = rect.orientation;
		pnt1=rect.pnt1;
		pnt2=rect.pnt2;
	}
};

struct SVMPolygon
{
	bool isHomographyPopulated;

	// each polygon consist of a list of SVMPoint. 
	// the pointers to the SVMPoints are saved in pntList;
	CTypedPtrDblList <SVMPoint> pntList;

	// the mean of all points in the list, used for polygon selection in UI;
	double cntx, cnty;

    // the orientation of the polygon
    int orientation;

	// H is the homography from normalized texture image of this polygon to the original image;
	// that is, if the INVERSE of H is applied to the image coordiates (u,v,w) in the pntList, 
	// the result is the texture coordinates, ranging between [0,1].
	// H is used when generating texture images from original image. 

	// invH is the inverse matrix of H; 
	// whenever you change H, please update invH using Matrix3by3Inv function in svmAux.h.
	// since the texture coordinate for each points are not saved, 
	// invH is used to convert image coordinates in pntList to texture coordinates.
	
	// double H[3][3],invH[3][3];
	CTransform3x3 H, invH;

	// name is the name of the polygon. name.gif willl be used as texture file name for VRML file. 
	// you will generate name.tga as texture file in the program first. 
	// then use your scissor program to mask the "useful" texture out.
	// finally genertate the name.tif in photo shop based on the mask and the name.tga
	// in name.gif, the "useful" texture should be opaque and other region should be transparent. 

	char name[256];

    // boolean for whether this polygon is visible
    bool visible;

	SVMPolygon(void)
	{
        orientation = OTHER_ORIENT;
		// memset(H,0,sizeof(double)*9);
		// memset(invH,0,sizeof(double)*9);
		// H[0][0]=H[1][1]=H[2][2]=invH[0][0]=invH[1][1]=invH[2][2]=1;
		
		name[0]='\0';
        visible = true;
		isHomographyPopulated = false;
	}

    void getFourPoints(SVMPoint **p1, SVMPoint **p2, SVMPoint **p3, SVMPoint **p4)
    {
        assert(pntList.GetCount() == 4);

        CTypedPtrDblElement<SVMPoint> *node = pntList.GetHeadPtr();

        *p1 = node->Data();
        node = node->Next();
        *p2 = node->Data();
        node = node->Next();
        *p3 = node->Data();
        node = node->Next();
        *p4 = node->Data();
    }

    void getFourPoints2D(Vec3d &p1, Vec3d &p2, Vec3d &p3, Vec3d &p4)
    {
        assert(pntList.GetCount() == 4);

        CTypedPtrDblElement<SVMPoint> *node = pntList.GetHeadPtr();

        SVMPoint *p;

        p = node->Data();
        p1 = Vec3d(p->X, p->Y, p->Z);        
        node = node->Next();
        p = node->Data();
        p2 = Vec3d(p->X, p->Y, p->Z);        
        node = node->Next();
        p = node->Data();
        p3 = Vec3d(p->X, p->Y, p->Z);        
        node = node->Next();
        p = node->Data();
        p4 = Vec3d(p->X, p->Y, p->Z);        
    }
};

typedef CTypedPtrDblList<SVMPolygon> PolygonList;

struct SVMSweep
{
    SVMPolygon *poly;
    SVMPoint *pStart;

	SVMSweep(void)
	{ 
		poly = NULL;
        pStart = NULL; 
	}	
};


struct SVMBox
{
    SVMPolygon *polys[6];
};

typedef CTypedPtrDblList<SVMBox> BoxList;

/***************************** End of Important Data Structures *************/

const double EPS = 1.0e-6;
const double NBR = 5.0;

#include "svmUI.h"
#include "HelpPageUI.h"

#include "ImgView.h"

void my_svm(char helpInfo[2048]);

#endif