#ifndef IMGVIEW_H
#define IMGVIEW_H

#include "imgflt.h"

const unsigned char BK_COLOR[3] = {212,208,200};
const unsigned char PATH_COLOR[3] = {0,255,0};
const unsigned char SELECTED_PATH_COLOR[3] = {255,0,0};
const unsigned char TREE_COLOR[3] = {255,255,0};
const unsigned char EXPANDED_COLOR[3] = {0,255,255};
const unsigned char ACTIVE_COLOR[3] = {0,0,255};

const int FLT_WIDTH = 5;
const int FLT_HEIGHT = 5;

const int ROUND_BRUSH = 1;
const int SQUARE_BRUSH = 2;

const int IMAGE_ONLY = 1;
const int IMAGE_WITH_CONTOUR = 2;

const int GRAPH_WITH_COLOR = 3;
const int GRAPH_WITH_COST = 4;
const int GRAPH_WITH_TREE = 5;
const int GRAPH_WITH_PATH = 6;

#include "PriorityQueue.h"

// Fixed by Loren. Contour conflicted with ImgView::Contour()
typedef CTypedPtrDblList <Seed> SeedList;

class ImgView : public Fl_Double_Window
{
protected:	
	
	//origImg:				The original image
	//imgBuf:				the image to draw in work mode, it is the same as origImg if brush is not used
	//nodeBuf:				a sequence of node from left to right and from top to bottom, 
	//						which has one to one correspondence with the image pixel
	//imgWidth, imgHeight:	dimension of the image.
	unsigned char *origImg;
	unsigned char *imgBuf;	
	double *curImg;
	unsigned char *curImgChar;
	Node *nodeBuf;
	int imgWidth, imgHeight;

	//pixelNodes:			an image of 3*imgWidth by 3*imgHeight,
	//						represents only pixel colors at the center of each 3 by 3 unit.
	//costGraph:			an image of 3*imgWidth by 3*imgHeight,
	//						represents both pixel colors and inter-pixel link costs in each 3 by 3 unit
	//graphWidth, graphHeight:	dimension of the pixelNodes and costGraph, which are 3*imgWidth by 3*imgHeight;
	unsigned char *pixelNodes;
	unsigned char *costGraph;
	int graphWidth, graphHeight;

	int imgLeft,imgTop;		//the position of the topleft pixel in the window
	int mouseX,mouseY;		//the mouse position in the current in the window	

	double fltKernel[FLT_WIDTH*FLT_HEIGHT];		//the kernel of the image filter
	double scale,offset;						//the scale and offset parameter, 
												//applied after convolving to transform 
												//the filtered image into the range [0,255]

	unsigned char *brushSelection;				//a binary array, indicating which area is selected by brush
	unsigned char *brushSelPtr;
	int brushType;								//brushType is either ROUND or SQUARE
	int brushSize;								//for a round brush, brushSize is the radius of the cicle
												//for a square brush, brushSize is half of the side length
	int brushRadius2;							//brushRadius2 = brushSize * brushSize
												//the default brush is a round brush of size 10

	double brushOpacity;						//brush selected area can be rendered with opacity, ranging in [0,1]
												//the default opacity if 1.0, which is transparent. 
												//the opacity can be adusted in brush config pannel.

	// Fixed by Loren.
	// CTypedPtrDblList <Seed> currentCntr;
	// CTypedPtrArray <CTypedPtrDblList <Seed> > contours;
	SeedList currentCntr;
	CTypedPtrArray <SeedList> contours;
	int freePtX, freePtY;	
	int selectedCntr;

	//the following variables take care of zooming in, you don't need to know them. 
	
	unsigned char *viewBuf;	// is of the same size as viewWidth*viewHeight;
	int viewWidth, viewHeight;
	int zoomFactor; //zooming in between 1,2,..16
	int zoomPort[4]; //the bounding box of zooming port in view port;
	int targetPort[4];//the bounding box of zoomed port in imgBuf or costGraph;

	//what to draw in the view port, depending on what button you press in scissor panel. 
	int drawMode;	

public:

	// Those are four UI windows for infomation prompting, and parameter adjusting;
	Fl_Output *mouseInfo;
	Fl_Menu_Bar *mainMenu;
	FltDesignUI *fltDesignUI;	
	BrushConfigUI *brushConfigUI;
	HelpPageUI *helpPageUI;						
	ScissorPanelUI *scissorPanelUI;

public:
	ImgView(int x, int y, int w, int h, const char *label=0);
	virtual ~ImgView();

	//the following three functions are called when menuitems in File menu are selected
	void OpenImage(void);	
	void SaveContour(void);	
	void SaveMask(void);
	// Publicized by Loren.
	void OpenImage(const char *filename);
	void SaveContour(const char *filename);
	void SaveMask(const char *filename);

	//TryFilter is called when Tools-->Filter menu is selected
	void TryFilter(void);
	//the following six functions are called when respective buttons are 
	//pressed in the filter design panel.
	void LoadFilter(void);
	void SaveFilter(void);	
	void PreviewFilter(void);
	void CancelFilter(void);
	void AcceptFilter(void);
	void StopFilter(void);
	//UpdateFilter is called whenever filter property is modified through the filter design panel
	void UpdateFilter(void);


	//TryScissor is called when menu Tool-->Scissor is selected
	void TryScissor(void);

	//the following seven functions are called when buttons/widgets in scissor panel are pressed
	void OrigImage(void);
	void Contour(void);

	void PixelColor(void);
	void CostGraph(void);
	void PathTree(void);
	void MinPath(void);	

	void PartialExpanding(void);
	
	void BrushSelection(int b);

	//TryBrush is called when Tools-->Brush menu is selected
	void TryBrush(void);	
	//UpdateBrushConfig is called whenever brush property is modified through
	//the brush config panel
	void UpdateBrushConfig(void);
	//CleanBrushSelection is called when the "clean" button is pressed in the 
	//the brush config panel, to clean all the bookmarking for previous selected brush area.
	void CleanBrushSelection(void);

	//two utility routines:
	int IsPtInImage(int i, int j) const 
	{ return 0<=i && i<imgWidth && 0<=j && j<imgHeight; }
	int IsPtInRoundBrush(int i, int j) const 
	{ return i*i+j*j<brushRadius2; }
	int IsPtAroundContour(int x, int y, const CTypedPtrDblList <Seed> *cntr) const;

	//HideAll is called before exit, closing filter design panel and brush config panel
	void HideAll(void);

	//draw displays proper image as desired.
	void draw(void);

	//handle processes all the mouse, keyboard messages to support user interface.
	int handle(int c);

	//called when the window is resized
	virtual void resize(int x, int y, int w, int h);

	//called if help menu item is selected.
	void AboutMe();

protected:

	//the following functions help to implement file I/O, memory allocate/copy/free
	//routines required in the public interface function;

	void FreeBuffer(void);

	void LoadFilter(const char *filename);
	void InitFltDesignUI(void);
	void SaveFilter(const char *filename);	

	//the Filter calls the filter function you write to actually *filter* images.
	void Filter(void);


	void UpdateImgBufOpacity(void);

	void AllocateViewBuffer(int w, int h);
	void UpdateViewPort(int bufLeft, int bufTop, int bufWidth, int bufHeight);	
	void UpdateViewBuffer(const unsigned char *origBuf, int bufLeft, int bufTop, int bufWidth, int bufHeight);
	void UpdateViewBuffer(void);
	
	void MarkPath(int col, int row, const unsigned char clr[3]);
	void MarkPath(const CTypedPtrDblList <Seed> *cntr, const unsigned char clr[3]);
	void MarkCurrentContour(void);
	void MarkPreviousContour(void);
	void MarkAllContour(void);
	void MarkAllContour(int col, int row);

	void UnMarkPath(int col, int row);
	void UnMarkPath(const CTypedPtrDblList <Seed> *cntr);
	void UnMarkCurrentContour(void);
	void UnMarkPreviousContour(void);
	void UnMarkAllContour(void);

	void UpdatePathTree(void);

	void MarkPathTree(void);
	void MarkPathOnTree(int col, int row);
	void UnMarkPathOnTree(int col, int row);

	void AppendCurrentContour(int col,int row);
	void FinishCurrentContour(int col,int row);
	void FinishCurrentContour(void);
	void CommitCurrentContour(void);
	void ChopLastSeed(void);
};

#endif
