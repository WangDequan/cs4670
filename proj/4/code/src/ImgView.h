#ifndef IMGVIEW_H
#define IMGVIEW_H

#include "svm.h"
#include "vec.h"

#include "ImageLib/Transform.h"

#pragma warning(push)
#pragma warning(disable : 4786)
#pragma warning(disable : 4996)

#include <vector> // EUGENE: for point stack
#include <set>

const double BK_COLOR[4] = {0.8314,0.8157,0.7843,0.0};

const int EDIT_POINT = 1;
const int EDIT_XLINE = 2;
const int EDIT_YLINE = 3;
const int EDIT_ZLINE = 4;
const int EDIT_OLINE = 5;
const int EDIT_POLYGON = 6;
const int EDIT_XZRECT = 7;
const int EDIT_YZRECT = 8;
const int EDIT_XYRECT = 9;
const int EDIT_SWEEP = 10;

const int DRAW_POINT = 1;
const int DRAW_LINE  = 1<<1;
const int DRAW_POLYGON = 1<<2;
const int DRAW_3D = 1<<3;

const double POINT_COLOR[3] = {1,1,0};
const double XLINE_COLOR[3] = {1,0,0};
const double YLINE_COLOR[3] = {0,1,0};
const double ZLINE_COLOR[3] = {0,0,1};
const double OLINE_COLOR[3] = {1,0,1};
const double POLYGON_COLOR[3] = {0,1,1};
const double HIGHLIGHT_COLOR[3] = {1,1,0.7};
const double GUIDE_COLOR[3] = {0,0.5,0.72};

const double ZImage2D = 0.0;
const double ZPoint2D = 1.0;
const double ZLine2D = 2.0;
const double ZPolygon2D = 3.0;

typedef struct
{
	float dx, dy;
	float tx1, ty1;
	float tx2, ty2;
} GLFONTCHAR;

//glFont structure
typedef struct
{
	int Tex;
	int TexWidth, TexHeight;
	int IntStart, IntEnd;
	GLFONTCHAR *Char;
} GLFONT;

class ImgView : public Fl_Gl_Window 
{
protected:	

	//loaded image;
	unsigned char *imgBuf;
	int imgWidth, imgHeight;

	//texture map of imgBuf, used by opengl to texture map;
	GLuint texName;


	//view parameter in "draw 2D" view mode, when "draw 3D" in draw submenu is not checked. 
	//the position of the bottom left corner in the window
	//the origin is the left bottom corner in this code, NOT left top;
	double imgLeft,imgBottom;	
	double scale2D;	

	//viewing parameters in "draw 3D" view mode;
	double cntX3D,cntY3D,cntZ3D;
	double rotX3D,rotY3D,rotZ3D;
	double traX3D,traY3D,traZ3D;
	double scale3D;

	//the mouse position in the current in the window, in screen coordinates;
	int mouseX,mouseY;		

	// the current line being edited;
	SVMLine curLine; 
    
    // the current rect being edited
    SVMRect curRect;

	// the current polygon being edited;
	SVMPolygon curPly;

    // the current polygon being dragged
    SVMPolygon curDragPly;

    SVMSweep curSweep;

	//all the points 
	PointList pntList;
	LineList lineList;
	PolygonList plyList;
    BoxList boxList;

	//seleted element for deletion, high lighted using HIGHLIGHT_COLOR;
	SVMPoint *selPnt;  
	SVMLine *selLine;
	SVMPolygon *selPly; 

	// selected for reference point 3D coordinate entry
	SVMPoint *curRefPnt;

	//current edit mode;
	int editMode;

	//current draw mode;
	int drawMode;

	bool showGuideLines;
	bool snapToGuideLines;

public:
	GLFONT *glFont;
	GLuint texFont;
	int glFontCreate (GLFONT *Font, char *FileName, int Tex);
	void glFontTextOut (char *String, float x, float y, 
	float z);
	// pointer to mainUI window for information exchange;
	svmUI *svmui;	

	// pointer to helpUI window to show the information;
	HelpPageUI *helpPageUI;						

	// dialogue window for setting 3D coordinates of reference points
	Fl_Window *setRefPnt_Window;
	Fl_Input *setRefPnt_X, *setRefPnt_Y, *setRefPnt_Z;
	Fl_Button *setRefPnt_Ok, *setRefPnt_Cancel;
	Fl_Check_Button *setRefPnt_UseHeight;

public:
	ImgView(int x, int y, int w, int h, const char *label=0);
	virtual ~ImgView();

	//the following two functions are called when menuitems in File menu are selected
	void OpenImage(void);	
	void OpenModel(void);
	void SaveModel(void);	
	void SaveVRML(void);

	//response to edit menu;

	void SetEditMode(int mode) 
	{ 
		editMode = mode; 
	}

	//response to draw menu;

	void SetDrawMode(int mask, int b)
	{ 
		if (b) drawMode |= mask; 
		else drawMode &= ~mask;		
	}
	int GetDrawMode(int mask) const
	{ 
		return drawMode&mask;
	}

	void setGuideLines(int b)
	{
		if( b ) showGuideLines = true;
		else showGuideLines = false;
	}

	void setSnapToGuideLines(int b)
	{
		if( b ) snapToGuideLines = true;
		else snapToGuideLines = false;
	}

	//draw displays proper image as desired.
	void draw(void);

	//handle processes all the mouse, keyboard messages to support user interface.
	int handle(int c);

	//hide all the sub window open in imgView;
	void HideAll(void);

	//called if help menu item is selected.
	void AboutMe();

protected:

	//the following functions help to implement file I/O, 
	//routines required in the public interface function;

	void OpenImage(const char *filename);
	void OpenModel(const char *filename);
	void SaveModel(const char *filename);
	void SaveVRML(const char *filename);
	
	void writeTexture(SVMPolygon& ply, const char* texname );
	void populateHomography(SVMPolygon &ply, std::vector<Vec3d> *basisPts);

	// handle UI message;
	int handle2D(int c);
	int handle3D(int c);

	//drawing stuff;
	void draw2D(void);
	void drawCurrentElements(void);
	void drawImg2D(void);
	void drawPoints2D(void);
	void drawLines2D(void);
	void drawPolygons2D(void);
	
	void draw3D(void);
	void drawPoints3D(void);
	void drawLines3D(void);
	void drawPolygons3D(void);

	//compute center of the bounding box of points in the list;
	static void ComputePointCenter3D(double &X, double &Y, double &Z, CTypedPtrDblList <SVMPoint> *pList);

	//compute the mean of all points in the list;
	static void ComputePointAverage2D(double &x, double &y, CTypedPtrDblList <SVMPoint> *pList);

	//search existing point that is close to (imgX,imgY);
	SVMPoint *SearchNearbyPoint(double imgX, double imgY);

	//search existing line who passes (imgX,imgY);
	SVMLine *SearchNearbyLine(double imgX, double imgY);

	//search existing polygon whose mean is close to (imgX,imgY);
	SVMPolygon *SearchNearbyPolygon(double imgX, double imgY);

	//search existing polygon whose origin is close to (imgX,imgY) (and returns the origin p)
	SVMPolygon *SearchNearbyPolygonOrigin(double imgX, double imgY, SVMPoint **p);

	//check whether goal has been in pntList or not. 
	//return the index of goal in the list, the index starts from 1 instead of 0;
	int CheckExistingPoint(SVMPoint *goal);

	//search whether goal is used in other lines;
	SVMLine *CheckExistingLine(SVMPoint *goal);

	//search whether goal is used in other polygons;
	SVMPolygon *CheckExistingPolygon(SVMPoint *goal);

    // find a polygon and return its index (starting from 1)
    int CheckExistingPolygonIndex(SVMPolygon *goal);

	//free all related memory allocation;
	void FreeBuffer(void);
	void FreeImgBuf(void);
	void FreeModelBuf(void);

	// EUGENE's
public:
	// compute vanishing points
	// calls BestFitIntersect in svmmath.cpp to compute line intersection
	void computeVanishingPoints();

	// compute 3D position of a point using another known point on the same Z plane
	void sameZPlane();

	// compute 3D positoin of a ponit using another known point with same X, Y
	void sameXY();

    // find the 3D positions of the 8 corners of the box.  The 3D position of points[0] is known.
    void find3DPositionsBox(SVMPoint *points[8]);

    // compute the corners of a rect being drawn normal to one of the axes
    void solveForOppositeCorners(double u1, double v1, double u3, double v3,
                                 double &u2, double &v2, double &u4, double &v4);

    // compute the 3D corners of a rect being drawn normal to one of the axes
    void solveForOppositeFace(SVMSweep *sweep, double imgX, double imgY,
                              Vec3d &p4_out, Vec3d &p5_out, Vec3d &p6_out, Vec3d &p7_out);

    // unwrap boxes
    void unwrapBoxes();

    // unwrap the boxes to form a "normal" cube map
    void unwrapBoxesNormal();
    
    // unwrap the inverted boxes
    void unwrapBoxesInverted();

    // invert the scene
    void invertScene(double zScale);

	// compute the homography for reference plane
	void computeRefHomography();

	// set coordinates of a reference point
	void setReferencePoint();

    // compute the camera position and the projection matrix
    void computeCameraParameters();

    void decomposePMatrix();

	// and associated callback functions
private:
	static void cb_setRefPnt_Ok(Fl_Button*, void*);
	static void cb_setRefPnt_Cancel(Fl_Button*, void*);
	static void cb_setRefPnt_UseHeight(Fl_Check_Button*, void*);

protected:

	// the point stack for user interface
	std::vector<SVMPoint*> pntSelStack;

	// for guidelines option
	std::vector<SVMPoint*> intersectionPoints;
	std::vector< std::pair< SVMPoint*, SVMPoint* > > lines;

	// vanishing points
	SVMPoint xVanish, yVanish, zVanish;

	SVMPoint xyzOrigin, xUnit, yUnit, zUnit; // unused

	// homography from the reference plane to the image plane, and its inverse
	// double H[3][3], Hinv[3][3];
    CTransform3x3 H, Hinv;
	bool homographyComputed;

	// for extra credit
	Vec3d camPos;
    Mat4d camP;  // projection matrix
    Mat3d camK;
    Mat4d camR;
    bool camComputed;

    // has the scene been inverted?
    bool sceneInverted;

	// reference point off of the plane for reference height
	SVMPoint *refPointOffPlane;

	// reference height specified by refPointOffPlane
	double referenceHeight;
};

#pragma warning(pop)

#endif