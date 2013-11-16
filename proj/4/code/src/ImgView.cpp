#pragma warning(disable : 4786)
#pragma warning(disable : 4996)

#include <assert.h>

#include "ImgView.h"
#include "mat.h"
#include "vec.h"
#include "ImageLib/FileIO.h"
#include "ImageLib/Transform.h"
#include "ImageLib/WarpImage.h"
#include "svmCMath.h"

#include <list>

#include "svmmath.h"
using namespace std;

void ImgView::cb_setRefPnt_Ok(Fl_Button* o, void* v)
{
	ImgView *me = (ImgView *) v;
	me->setReferencePoint();
}

void ImgView::cb_setRefPnt_Cancel(Fl_Button* o, void* v)
{
	ImgView *me = (ImgView *) v;
	me->setRefPnt_Window->hide();
}

void ImgView::cb_setRefPnt_UseHeight(Fl_Check_Button* o, void* v)
{
	ImgView *me = (ImgView *) v;
	if (me->setRefPnt_UseHeight->value() == 1)
	{
		me->referenceHeight = me->selPnt->Z;
		me->refPointOffPlane = me->selPnt;
	}
}

ImgView::ImgView(int x, int y, int w, int h, const char *label) : Fl_Gl_Window(x,y,w,h,label)
{
	
	mode(FL_DOUBLE|FL_RGB8|FL_DEPTH);

	imgBuf = NULL;
	
	imgLeft = 0;
	imgBottom = 0;
	scale2D = 1;

	svmui = NULL;

    homographyComputed = false;
    camComputed = false;
    sceneInverted = false;

    char helpInfo[2048];
	my_svm(helpInfo);

	helpPageUI = new HelpPageUI();
	helpPageUI->helpText->value(helpInfo);

	editMode = EDIT_POINT;
	drawMode = 0;
	SetDrawMode(DRAW_POINT|DRAW_LINE|DRAW_POLYGON,1);
	setGuideLines(0);
	setSnapToGuideLines(1);

	// dialogue window for setting reference points
	setRefPnt_Window = new Fl_Window(0, 0, 300, 160, "Set Reference Point");
	
	setRefPnt_X = new Fl_Input(60, 10, 210, 20, "X");
	setRefPnt_Y = new Fl_Input(60, 40, 210, 20, "Y");
	setRefPnt_Z = new Fl_Input(60, 70, 210, 20, "Z");
	
	setRefPnt_UseHeight = new Fl_Check_Button(60, 100, 180, 20, "Use for reference height");
	setRefPnt_UseHeight->callback((Fl_Callback *)cb_setRefPnt_UseHeight, (void *)this);
	
	setRefPnt_Ok = new Fl_Button(65, 130, 70, 20, "Ok");
	setRefPnt_Ok->callback((Fl_Callback *)cb_setRefPnt_Ok, (void *)this);
	
	setRefPnt_Cancel = new Fl_Button(165, 130, 70, 20, "Cancel");
	setRefPnt_Cancel->callback((Fl_Callback *)cb_setRefPnt_Cancel, (void *)this);

	setRefPnt_Window->end();
	
	setRefPnt_Window->resizable(setRefPnt_Window);

	H[0][0] = H[1][1] = H[2][2] = 1.0;
	H[0][1] = H[0][2] = 0.0;
	H[1][0] = H[1][2] = 0.0;
	H[2][0] = H[2][1] = 0.0;

	Hinv[0][0] = Hinv[1][1] = Hinv[2][2] = 1.0;
	Hinv[0][1] = Hinv[0][2] = 0.0;
	Hinv[1][0] = Hinv[1][2] = 0.0;
	Hinv[2][0] = Hinv[2][1] = 0.0;
	
	texFont = 0;
}
ImgView::~ImgView()
{
	FreeBuffer();
	glDeleteTextures(1, &texName);

	delete helpPageUI;
}

void ImgView::FreeBuffer(void)
{
	FreeImgBuf();
	FreeModelBuf();	
}

void ImgView::FreeImgBuf(void)
{
	if (imgBuf)
	{
		delete[] imgBuf;
		imgBuf = NULL;
	}
}

void ImgView::FreeModelBuf(void)
{
	if (curLine.pnt1 && !CheckExistingPoint(curLine.pnt1))
	{
		delete curLine.pnt1;
	}
	curLine.pnt1 = NULL;

	if (curLine.pnt2 && !CheckExistingPoint(curLine.pnt2))
	{
		delete curLine.pnt2;	
	}	
	curLine.pnt2 = NULL;

	CTypedPtrDblElement <SVMPoint> *node = curPly.pntList.GetHeadPtr();

	while (!curPly.pntList.IsSentinel(node))
	{
		SVMPoint *pnt = node->Data();
		if (!CheckExistingPoint(pnt))
		{
			delete pnt;
		}
		node=node->Next();
	}

	curPly.pntList.RemoveAll();

	pntList.FreePtrs();
	pntList.RemoveAll();

	lineList.FreePtrs();
	lineList.RemoveAll();

	plyList.FreePtrs();
	plyList.RemoveAll();
}

void ImgView::HideAll(void)
{
	helpPageUI->hide();
}

void ImgView::OpenImage(void)
{
	char *filename = fl_file_chooser("choose a tga file", "*.tga", 0);

	if (filename)
	{
		OpenImage(filename);

		editMode = EDIT_POINT;

		svmui->editPoint->setonly();

		drawMode = 0;
		SetDrawMode(DRAW_POINT|DRAW_LINE|DRAW_POLYGON,1);

		svmui->drawPoint->set();
		svmui->drawLine->set();
		svmui->drawPly->set();
		svmui->draw3D->clear();				

		selPnt = NULL;
		selLine = NULL;
		selPly = NULL;

		imgLeft = -imgWidth/2.0;
		imgBottom = -imgHeight/2.0;
		scale2D = 1;

		rotX3D = rotY3D = rotZ3D = 0;
		traX3D = traY3D = traZ3D = 0;
		traZ3D = -400;
		scale3D = 1;

		redraw();
	}
}

void ImgView::OpenImage(const char *filename)
{
	int i,j,total;

	FreeBuffer();	

	CByteImage rick;
	ReadFile(rick,filename);

	imgWidth = rick.Shape().width;
	imgHeight = rick.Shape().height;

	total = imgWidth*imgHeight*3;

	imgBuf = new unsigned char [total];

	//Rick's .tga file format is in G,B,R order from bottom to top and from left to right;
	//our buffer is in R, G, B order from top to bottom and from left to right;
	for (j=0;j<imgHeight;j++)
	{
		for (i=0;i<imgWidth;i++)
		{
			int index = 3*(j*imgWidth+i);
			int x = i;
			int y = j;
			imgBuf[index+0] = rick.Pixel(x,y,2);
			imgBuf[index+1] = rick.Pixel(x,y,1);
			imgBuf[index+2] = rick.Pixel(x,y,0);
		}
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	
	glDeleteTextures(1, &texName);
	glGenTextures( 1, &texName );

	glBindTexture( GL_TEXTURE_2D, texName );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, imgWidth, imgHeight, GL_RGB, GL_UNSIGNED_BYTE, imgBuf );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	if(!texFont) {
		glGenTextures( 1, &texFont );
		glFont = new GLFONT;
		if(!glFontCreate(glFont, "src/resources/fonts.glf", texFont)) {
			if(!glFontCreate(glFont, "resources/fonts.glf", texFont)) {
				printf("error: font not found\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

void ImgView::OpenModel(void)
{
	if (imgBuf==NULL)
	{
		fl_message("Please load an image first!");
		return;
	}

	char *filename = fl_file_chooser("choose a model file", "*.txt", 0);

	if (filename)
	{
		OpenModel(filename);
	}
}

void ImgView::OpenModel(const char *filename)
{
	FreeModelBuf();

	FILE *fp = fopen(filename, "r");

	int i;

	int numPoints;

	pntSelStack.clear();
	intersectionPoints.clear();

	fscanf(fp, "vanishing points:\n");
	fscanf(fp,"(%lf,%lf,%lf,%lf,%lf,%lf,%lf)\n",
		&(xVanish.u),&(xVanish.v),&(xVanish.w),&(xVanish.X),&(xVanish.Y),&(xVanish.Z),&(xVanish.W));
	fscanf(fp,"(%lf,%lf,%lf,%lf,%lf,%lf,%lf)\n",
		&(yVanish.u),&(yVanish.v),&(yVanish.w),&(yVanish.X),&(yVanish.Y),&(yVanish.Z),&(yVanish.W));
	fscanf(fp,"(%lf,%lf,%lf,%lf,%lf,%lf,%lf)\n",
		&(zVanish.u),&(zVanish.v),&(zVanish.w),&(zVanish.X),&(zVanish.Y),&(zVanish.Z),&(zVanish.W));

	intersectionPoints.push_back( &xVanish );
	intersectionPoints.push_back( &yVanish );
	intersectionPoints.push_back( &zVanish );

	fscanf(fp, "reference plane homography:\n");
	for (i=0; i<3; i++)
	{
		fscanf(fp, "%lf,%lf,%lf\n", &H[i][0], &H[i][1], &H[i][2]);
	}

	if (H.Determinant() == 0)
		fl_alert("Homography matrix for reference plane is uninvertible\n");
	else
		Hinv = H.Inverse();

    if (H[0][0] == 1.0 && H[0][1] == 0.0 && H[0][2] == 0.0 &&
        H[1][0] == 0.0 && H[1][1] == 1.0 && H[1][2] == 0.0 &&
        H[2][0] == 0.0 && H[2][1] == 1.0 && H[2][2] == 1.0) {

        homographyComputed = false;
    } else {
        homographyComputed = true;
    }

	int heightPntIdx;
	fscanf(fp, "point for reference height: %d\n", &heightPntIdx);

	fscanf(fp,"num of points: %d\n", &numPoints);

	printf("num of points: %d\n", numPoints);

	SVMPoint **indBuf = new SVMPoint *[numPoints];

	refPointOffPlane = NULL;
	referenceHeight = 0;

	for (i=0;i<numPoints;i++)
	{
		SVMPoint *pnt = new SVMPoint;
		fscanf(fp,"(%lf,%lf,%lf,%lf,%lf,%lf,%lf)\n",&(pnt->u),&(pnt->v),&(pnt->w),&(pnt->X),&(pnt->Y),&(pnt->Z),&(pnt->W));

		if (!pnt->old_known()) // interprete old scheme for known/unknown points
		{
			pnt->X=pnt->Y=pnt->Z=0;
			pnt->W = 0;
		}
		
		if (i == heightPntIdx)
		{
			refPointOffPlane = pnt;
			referenceHeight = pnt->Z;
		}

		pntList.AddTail(pnt);

		indBuf[i]=pnt;
	}

	int numLines;

	fscanf(fp,"num of lines: %d\n", &numLines);

	printf("num of lines: %d\n", numLines);

	for (i=0;i<numLines;i++)
	{		
		char o;
		int ind1,ind2;

		fscanf(fp,"%c: %d %d\n",&o,&ind1,&ind2);

		SVMLine *line = new SVMLine;
		
		if (o=='x')
		{
			line->orientation = PARA_X;
		}
		else if (o=='y')
		{
			line->orientation = PARA_Y;
		}
		else if (o=='z')
		{
			line->orientation = PARA_Z;
		}
		else
		{
			line->orientation = OTHER_ORIENT;
		}

		line->pnt1 = indBuf[ind1-1];
		line->pnt2 = indBuf[ind2-1];

		lineList.AddTail(line);
	}

	int numPlys;

	fscanf(fp,"num of polygons: %d\n", &numPlys);

	printf("num of polygons: %d\n", numPlys);
	SVMPolygon **indPlyBuf = new SVMPolygon *[numPlys];

	for (i=0;i<numPlys;i++)
	{
		SVMPolygon *ply=new SVMPolygon;

		fscanf(fp, "%s\n", ply->name);

		int numSides;

		fscanf(fp,"%d:",&numSides);		

		int j;
		for (j=0;j<numSides;j++)
		{
			int ind;
			fscanf(fp," %d", &ind);
			ply->pntList.AddTail(indBuf[ind-1]);
		}
		fscanf(fp,"\n");
		
		for (j=0;j<3;j++)
		{
			int i;
			for (i=0;i<3;i++)
			{
				fscanf(fp,"%lf ",&(ply->H[j][i]));
			}
			fscanf(fp,"\n");
		}
		fscanf(fp,"\n");

		if (ply->H.Determinant() == 0)
			fl_alert("Homography matrix for polygon %d is uninvertible \n", i);
		else
			ply->invH = ply->H.Inverse();

		ComputePointAverage2D(ply->cntx,ply->cnty,&(ply->pntList));
		plyList.AddTail(ply);
        indPlyBuf[i] = ply;
	}

    // read any boxes
    int numBoxes = 0;
	fscanf(fp,"num of boxes: %d\n", &numBoxes);
	printf("num of boxes: %d\n", numBoxes);

    for (int i = 0; i < numBoxes; i++) {
        SVMBox *box = new SVMBox();

        for (int i = 0; i < 6; i++) {
            int plyIdx = 0;
            fscanf(fp, " %d", &plyIdx);
            box->polys[i] = indPlyBuf[plyIdx-1];
        }

        boxList.AddTail(box);
    }

    // read the camera parameters
    fscanf(fp, "%d\n", &camComputed);

    if (camComputed) {
        // read position
        double pos[3];
        fscanf(fp, "%lf,%lf,%lf\n", pos+0, pos+1, pos+2);
        camPos = Vec3d(pos[0], pos[1], pos[2]);

        // P matrix
        double P[16];
        fscanf(fp, "%lf,%lf,%lf,%lf,"
                   "%lf,%lf,%lf,%lf,"
                   "%lf,%lf,%lf,%lf,"
                   "%lf,%lf,%lf,%lf\n", 
                   P+0, P+1, P+2, P+3, 
                   P+4, P+5, P+6, P+7, 
                   P+8, P+9, P+10, P+11, 
                   P+12, P+13, P+14, P+15);

        camP = Mat4d(P[0], P[1], P[2], P[3], 
                     P[4], P[5], P[6], P[7], 
                     P[8], P[9], P[10], P[11], 
                     P[12], P[13], P[14], P[15]);

        // K matrix
        double K[9];
        fscanf(fp, "%lf,%lf,%lf,"
                   "%lf,%lf,%lf,"
                   "%lf,%lf,%lf\n", 
                   K+0, K+1, K+2, K+3, K+4, K+5, K+6, K+7, K+8);

        camK = Mat3d(K[0], K[1], K[2], K[3], K[4], K[5], K[6], K[7], K[8]);

        // R matrix
        double R[16];
        fscanf(fp, "%lf,%lf,%lf,%lf,"
                   "%lf,%lf,%lf,%lf,"
                   "%lf,%lf,%lf,%lf,"
                   "%lf,%lf,%lf,%lf\n", 
                   R+0, R+1, R+2, R+3, 
                   R+4, R+5, R+6, R+7, 
                   R+8, R+9, R+10, R+11, 
                   R+12, R+13, R+14, R+15);

        camR = Mat4d(R[0], R[1], R[2], R[3], 
                     R[4], R[5], R[6], R[7], 
                     R[8], R[9], R[10], R[11], 
                     R[12], R[13], R[14], R[15]);
    }

	delete[] indBuf;
    delete [] indPlyBuf;

	fclose(fp);

    redraw();
}

void ImgView::SaveModel(void)
{
	char *filename = fl_file_chooser("choose a model file", "*.txt", 0);

	if (filename)
	{
		SaveModel(filename);
	}
}

void ImgView::SaveModel(const char *filename)
{
	int i,j;
	FILE *fp = fopen(filename, "w");

	fprintf(fp, "vanishing points:\n");
	fprintf(fp,"(%lf,%lf,%lf,%lf,%lf,%lf,%lf)\n",
		xVanish.u,xVanish.v,xVanish.w,xVanish.X,xVanish.Y,xVanish.Z,xVanish.W);
	fprintf(fp,"(%lf,%lf,%lf,%lf,%lf,%lf,%lf)\n",
		yVanish.u,yVanish.v,yVanish.w,yVanish.X,yVanish.Y,yVanish.Z,yVanish.W);
	fprintf(fp,"(%lf,%lf,%lf,%lf,%lf,%lf,%lf)\n",
		zVanish.u,zVanish.v,zVanish.w,zVanish.X,zVanish.Y,zVanish.Z,zVanish.W);

	fprintf(fp, "reference plane homography:\n");
	for (i=0; i<3; i++)
	{
		fprintf(fp, "%lf,%lf,%lf\n", H[i][0], H[i][1], H[i][2]);
	}

	i = 0;
	CTypedPtrDblElement <SVMPoint> *pntNode = pntList.GetHeadPtr();
	while (!pntList.IsSentinel(pntNode))
	{
		SVMPoint *pnt = pntNode->Data();

		if (pnt == refPointOffPlane) {
			fprintf(fp, "point for reference height: %d\n", i);
			break;
		}

		pntNode = pntNode->Next();
		i++;
	}

	if (i==pntList.GetCount())
		fprintf(fp, "point for reference height: %d\n", -1);

	fprintf(fp,"%s: %d\n","num of points", pntList.GetCount());

	pntNode = pntList.GetHeadPtr();
	while (!pntList.IsSentinel(pntNode))
	{
		SVMPoint *pnt = pntNode->Data();

		fprintf(fp,"(%lf,%lf,%lf,%lf,%lf,%lf,%lf)\n",pnt->u,pnt->v,pnt->w,pnt->X,pnt->Y,pnt->Z,pnt->W);

		pntNode = pntNode->Next();
	}

	fprintf(fp,"%s: %d\n","num of lines", lineList.GetCount());

	CTypedPtrDblElement <SVMLine> *lineNode = lineList.GetHeadPtr();
	while (!lineList.IsSentinel(lineNode))
	{
		SVMPoint *pnt1 = lineNode->Data()->pnt1;
		SVMPoint *pnt2 = lineNode->Data()->pnt2;
		char o='o';
		if (lineNode->Data()->orientation==PARA_X)
		{
			o='x';
		}
		else if (lineNode->Data()->orientation==PARA_Y)
		{
			o='y';
		}
		else if (lineNode->Data()->orientation==PARA_Z)
		{
			o='z';
		}

		fprintf(fp,"%c: %d %d\n",o,CheckExistingPoint(pnt1),CheckExistingPoint(pnt2));

		lineNode = lineNode->Next();
	}

	// remove garbage polygons
	CTypedPtrDblElement <SVMPolygon> *plyNode = plyList.GetHeadPtr();
	while (!plyList.IsSentinel(plyNode))
	{
		SVMPolygon *ply = plyNode->Data();
		if (ply->pntList.GetCount() <= 0 ||
			ply->pntList.GetCount() > 200)
		{
//			printf("garbage polygon with %d points\n", ply->pntList.GetCount());
			CTypedPtrDblElement <SVMPolygon> *tmpNode = plyNode;
			plyNode = plyNode->Next();
			plyList.Remove(tmpNode);
		}
		else
		{
			plyNode = plyNode->Next();
		}
	}

	fprintf(fp,"%s: %d\n","num of polygons", plyList.GetCount());

	//CTypedPtrDblElement <SVMPolygon> *plyNode = plyList.GetHeadPtr();
	plyNode = plyList.GetHeadPtr();
	while (!plyList.IsSentinel(plyNode))
	{		
		SVMPolygon *ply = plyNode->Data();

		fprintf(fp, "%s\n", ply->name);

		fprintf(fp,"%d:",ply->pntList.GetCount());

		CTypedPtrDblElement <SVMPoint> *pntNode = ply->pntList.GetHeadPtr();

		while(!ply->pntList.IsSentinel(pntNode))
		{
			SVMPoint *pnt = pntNode->Data();

			fprintf(fp," %d",CheckExistingPoint(pnt));

			pntNode=pntNode->Next();
		}

		fprintf(fp,"\n");

		for (j=0;j<3;j++)
		{
			for (i=0;i<3;i++)
			{
				fprintf(fp,"%lf ",ply->H[j][i]);
			}
			fprintf(fp,"\n");
		}
		fprintf(fp,"\n");

		plyNode=plyNode->Next();
	}

    // write any boxes
	fprintf(fp,"%s: %d\n","num of boxes", boxList.GetCount());

    CTypedPtrDblElement <SVMBox> *boxNode = boxList.GetHeadPtr();
	boxNode = boxList.GetHeadPtr();
	while (!boxList.IsSentinel(boxNode))
	{		
        SVMBox *box = boxNode->Data();

        for (int i = 0; i < 6; i++) {
            fprintf(fp, " %d", CheckExistingPolygonIndex(box->polys[i]));
        }
		fprintf(fp,"\n");

        boxNode = boxNode->Next();
    }

    // write the camera parameters
    if (camComputed) {
        fprintf(fp, "1\n");
        // position
        fprintf(fp, "%lf,%lf,%lf\n", camPos[0], camPos[1], camPos[2]);
        // P matrix
        fprintf(fp, "%lf,%lf,%lf,%lf,"
                    "%lf,%lf,%lf,%lf,"
                    "%lf,%lf,%lf,%lf,"
                    "%lf,%lf,%lf,%lf\n", 
                    camP[0][0], camP[0][1], camP[0][2], camP[0][3],
                    camP[1][0], camP[1][1], camP[1][2], camP[1][3],
                    camP[2][0], camP[2][1], camP[2][2], camP[2][3],
                    camP[3][0], camP[3][1], camP[3][2], camP[3][3]);

        // K matrix
        fprintf(fp, "%lf,%lf,%lf,"
                    "%lf,%lf,%lf,"
                    "%lf,%lf,%lf\n", 
                    camK[0][0], camK[0][1], camK[0][2],
                    camK[1][0], camK[1][1], camK[1][2],
                    camK[2][0], camK[2][1], camK[2][2]);

        // R matrix
        fprintf(fp, "%lf,%lf,%lf,%lf,"
                    "%lf,%lf,%lf,%lf,"
                    "%lf,%lf,%lf,%lf,"
                    "%lf,%lf,%lf,%lf\n", 
                    camR[0][0], camR[0][1], camR[0][2], camR[0][3],
                    camR[1][0], camR[1][1], camR[1][2], camR[1][3],
                    camR[2][0], camR[2][1], camR[2][2], camR[2][3],
                    camR[3][0], camR[3][1], camR[3][2], camR[3][3]);
    } else {
        fprintf(fp, "0\n"); // unknown camera parameters
    }


	fclose(fp);
}

void ImgView::SaveVRML(void)
{
	char *filename = fl_file_chooser("choose a VRML file", "*.wrl", 0);

	if (filename)
	{
		SaveVRML(filename);
	}
}

/*
# an example of a shape description in VRML 2.0;

Shape { 
	appearance Appearance {
		material Material { } 
		texture ImageTexture { 
			url "brian.gif" 
		}
	} 
	geometry IndexedFaceSet { 
		coord Coordinate { 
			point [
				-1.0 -1.0 0.0,
				1.0 -1.0 0.0,
				1.0 1.0 0.0,
				-1.0 1.0 0.0,
			] 
		} 
		coordIndex [
			0, 1, 2, 3, 
		] 
		texCoord TextureCoordinate {
			point [ 
				0.2 0.2, 
				0.8 0.2,
				0.8 0.8,
				0.2 0.8,
			] 
		} 
		texCoordIndex [
			0, 1, 2, 3,
		] 
		solid FALSE 
	}
} 
*/

static void matrix_to_axis_angle(double *R, double *axis, double *angle) {
    double d1 = R[7] - R[5];
    double d2 = R[2] - R[6];
    double d3 = R[3] - R[1];

    double norm = sqrt(d1 * d1 + d2 * d2 + d3 * d3);
    double x = (R[7] - R[5]) / norm;
    double y = (R[2] - R[6]) / norm;
    double z = (R[3] - R[1]) / norm;

    *angle = acos((R[0] + R[4] + R[8] - 1.0) * 0.5);

    axis[0] = x;
    axis[1] = y;
    axis[2] = z;
}

void ImgView::SaveVRML(const char *filename)
{
	FILE *fp = fopen(filename,"w");

	fprintf(fp,"#VRML V2.0 utf8\n\n");

    // if the camera has been found, create a viewpoint node
    if (camComputed) {
        double axis[3], angle;
        double R[9], RT[9];
        memcpy(R+0, camR[0], 3 * sizeof(double));
        memcpy(R+3, camR[1], 3 * sizeof(double));
        memcpy(R+6, camR[2], 3 * sizeof(double));

        // check if the scene needs to be reflected
    	CTypedPtrDblElement <SVMPoint> *pntNode;
        pntNode = pntList.GetHeadPtr();
        while (!pntList.IsSentinel(pntNode)) {
            SVMPoint *pnt = pntNode->Data();

            if (pnt->known()) {
                Vec4d p(pnt->X, pnt->Y, pnt->Z, pnt->W);
                Vec4d pos(camPos[0], camPos[1], camPos[2], 1.0);
                Vec4d proj = camR * (p - pos);
                Vec3d proj3 = camK * Vec3d(proj[0], proj[1], proj[2]);

#if 0
                printf("P = %0.3f %0.3f %0.3f\n", pnt->X, pnt->Y, pnt->Z);
                printf("proj = %0.3f, %0.3f; %0.3f, %0.3f\n", 
                       proj3[0] / proj3[2], proj3[1] / proj3[2], pnt->u, pnt->v);
#endif

                if (proj[2] > 0.0) {
                    printf("scene is flipped\n");
                    double Y180[9] = { -1.0, 0.0, 0.0,
                                       0.0, 1.0, 0.0,
                                       0.0, 0.0, -1.0 };
                    double tmp[9];

                    matrix_product(3, 3, 3, 3, Y180, R, tmp);
                    memcpy(R, tmp, 9 * sizeof(double));
                
                    break;
                } else {
                    // rotate around the z axis for some bizarre reason
                    double Z180[9] = { -1.0,  0.0, 0.0,
                                        0.0, -1.0, 0.0,
                                        0.0,  0.0, 1.0  };
                    
                    double tmp[9];
                    matrix_product(3, 3, 3, 3, Z180, R, tmp);
                    memcpy(R, tmp, 9 * sizeof(double));

                    break;
                }
            }

            pntNode = pntNode->Next();
        }

        matrix_transpose(3, 3, R, RT);
        matrix_to_axis_angle(RT, axis, &angle);

        fprintf(fp, "Viewpoint {\n"
                    "   fieldOfView 1.0419\n"
                    "   position %0.4f %0.4f %0.4f\n"
                    "   orientation %0.4f %0.4f %0.4f %0.4f\n"
                    "   description \"startview\"\n"
                    "}\n",
                camPos[0], camPos[1], camPos[2],
                axis[0], axis[1], axis[2], angle);
    }

    fprintf(fp, "Collision {\n"
                "    collide FALSE\n"
                "    children [\n");

	CTypedPtrDblElement <SVMPoint> *pntNode;
	CTypedPtrDblElement <SVMPolygon> *plyNode;

	plyNode = plyList.GetHeadPtr();
	while (!plyList.IsSentinel(plyNode))
	{
		SVMPolygon *ply = plyNode->Data();

		if (ply->pntList.GetCount() <= 0 ||
            ply->pntList.GetCount() > 200) {

    		plyNode=plyNode->Next();
			continue;
        }

        // check if the polygon is hidden -- true if it's the second face of a box
    	CTypedPtrDblElement <SVMBox> *boxNode = boxList.GetHeadPtr();
    	bool skip = false;
        while (!boxList.IsSentinel(boxNode))
	    {
            SVMBox *box = boxNode->Data();

            if (box->polys[1] == ply) {
                skip = true;
                break;
            }
            
            boxNode = boxNode->Next();
        }

        if (skip) {
            plyNode = plyNode->Next();
            continue;
        }

		fprintf(fp," Shape {\n");

		fprintf(fp,"\t appearance Appearance {\n");
		// fprintf(fp,"\t\t material Material { }\n");
		fprintf(fp,"\t\t texture ImageTexture {\n");
		
		char texname[256];
		strcpy(texname,ply->name);
		strcat(texname,".tga");

		writeTexture( *ply, texname );

		strcpy(texname,ply->name);
		strcat(texname,".gif");

		fprintf(fp,"\t\t\t url \"%s\"\n",texname);
		fprintf(fp,"\t\t }\n");
		fprintf(fp,"\t }\n");

		fprintf(fp,"\t geometry IndexedFaceSet {\n");
		fprintf(fp,"\t\t coord Coordinate {\n");
		fprintf(fp,"\t\t\t point [\n");

		pntNode = ply->pntList.GetHeadPtr();
		while (!ply->pntList.IsSentinel(pntNode))
		{
			SVMPoint *pnt = pntNode->Data();

			fprintf(fp,"\t\t\t\t %lf %lf %lf,\n",pnt->X/pnt->W,pnt->Y/pnt->W,pnt->Z/pnt->W);
			
			pntNode = pntNode->Next();
		}

		fprintf(fp,"\t\t\t ]\n");
		fprintf(fp,"\t\t }\n");	
		fprintf(fp,"\t\t coordIndex [\n");

		int i;

		fprintf(fp,"\t\t\t ");		
		for (i=0;i<ply->pntList.GetCount();i++)
		{
			fprintf(fp,"%d, ",i);
		}
		fprintf(fp,"-1,\n");
			
		fprintf(fp,"\t\t ]\n");
	
		fprintf(fp,"\t\t texCoord TextureCoordinate {\n");
		fprintf(fp,"\t\t\t point [\n");

		pntNode = ply->pntList.GetHeadPtr();
		while (!ply->pntList.IsSentinel(pntNode))
		{
			SVMPoint *pnt = pntNode->Data();

			double u,v;
			ApplyHomography(u,v,ply->invH,pnt->u,pnt->v,pnt->w);
			
			fprintf(fp,"\t\t\t\t %lf %lf,\n",u,v);
			
			pntNode = pntNode->Next();
		}
		
		fprintf(fp,"\t\t\t ]\n");
		fprintf(fp,"\t\t }\n");
		
		fprintf(fp,"\t\t texCoordIndex [\n");
		
		fprintf(fp,"\t\t\t ");
		for (i=0;i<ply->pntList.GetCount();i++)
		{
			fprintf(fp,"%d, ",i);
		}
		fprintf(fp,"-1,\n");

		fprintf(fp,"\t\t ]\n"); 

		fprintf(fp,"\t\t solid FALSE\n");
		fprintf(fp,"\t }\n");

		fprintf(fp,"}\n");

		plyNode=plyNode->Next();
	}	

    fprintf(fp, "    ]\n");  // end children
    fprintf(fp, "}\n");      // end collision

	fclose(fp);
}

int ImgView::handle(int c)
{
	if (GetDrawMode(DRAW_3D))
	{
		return handle3D(c);
	}
	else 
	{
		return handle2D(c);
	}
}



void ImgView::ComputePointCenter3D(double &X, double &Y, double &Z, CTypedPtrDblList <SVMPoint> *pList)
{
	double Xmax,Ymax,Zmax;
	double Xmin,Ymin,Zmin;

	int first = 1;

	CTypedPtrDblElement <SVMPoint> *node=pList->GetHeadPtr();	

	while (!pList->IsSentinel(node))
	{
		SVMPoint *pnt = node->Data();

		if (pnt->W>EPS)
		//if the point is not at the infinity;
		{
			double x=pnt->X/pnt->W;
			double y=pnt->Y/pnt->W;
			double z=pnt->Z/pnt->W;

			if (first)
			{
				Xmax=Xmin=x;
				Ymax=Ymin=y;
				Zmax=Zmin=z;
				first = 0;
			}
			else 
			{
				Xmax=__max(Xmax,x);
				Ymax=__max(Ymax,y);
				Zmax=__max(Zmax,z);

				Xmin=__min(Xmin,x);
				Ymin=__min(Ymin,y);
				Zmin=__min(Zmin,z);
			}
		}

		node=node->Next();
	}

	X=(Xmin+Xmax)/2;
	Y=(Ymin+Ymax)/2;
	Z=(Zmin+Zmax)/2;
}

void ImgView::ComputePointAverage2D(double &x, double &y, CTypedPtrDblList <SVMPoint> *pList)
{
	x=y=0;

	CTypedPtrDblElement <SVMPoint> *node=pList->GetHeadPtr();

	int count = 0;

	while (!pList->IsSentinel(node))
	{
		SVMPoint *pnt = node->Data();

		if (pnt->w>EPS)
		//if the point is not at the infinity;
		{
			x+=pnt->u/pnt->w;
			y+=pnt->v/pnt->w;
			count++;
		}

		node=node->Next();
	}

	if (count)
	{
		x/=count;
		y/=count;
	}
}

SVMPoint *ImgView::SearchNearbyPoint(double imgX, double imgY)
{
	SVMPoint *result = NULL;

	CTypedPtrDblElement <SVMPoint> *node=pntList.GetHeadPtr();	

	while (!pntList.IsSentinel(node))
	{
		SVMPoint *pnt = node->Data();

		if (pnt->w>EPS)
		//if the point is not at the infinity;
		{
			double u=pnt->u/pnt->w;
			double v=pnt->v/pnt->w;

			if (fabs(imgX-u)+fabs(imgY-v)<2*NBR)
			{
				result = pnt;
				break;
			}
		}

		node=node->Next();
	}

	return result;
}

SVMLine *ImgView::SearchNearbyLine(double imgX, double imgY)
{
	SVMLine *result = NULL;

	CTypedPtrDblElement <SVMLine> *node=lineList.GetHeadPtr();	

	while (!lineList.IsSentinel(node))
	{
		SVMLine *line = node->Data();

		if (line->pnt1->w>EPS && line->pnt2->w>EPS)
		//if the two end points are not at the infinity;
		{
			double u1=line->pnt1->u/line->pnt1->w;
			double u2=line->pnt2->u/line->pnt2->w;
			double v1=line->pnt1->v/line->pnt1->w;
			double v2=line->pnt2->v/line->pnt2->w;

			double du=u2-u1;
			double dv=v2-v1;
			
			double dx=imgX-u1;
			double dy=imgY-v1;

			double para=(dx*du+dy*dv)/(du*du+dv*dv);
			double perpx=dx-para*du;
			double perpy=dy-para*dv;
			double perp=perpx*perpx+perpy*perpy-NBR*NBR;

			if (EPS<para && para+EPS<1 && perp<EPS)
			{
				result = line;
				break;
			}
		}
		
		node=node->Next();
	}

	return result;
}

SVMPolygon *ImgView::SearchNearbyPolygon(double imgX, double imgY)
{
	SVMPolygon *result = NULL;

	CTypedPtrDblElement <SVMPolygon> *node=plyList.GetHeadPtr();	

	while (!plyList.IsSentinel(node))
	{
		SVMPolygon *ply = node->Data();

		if (fabs(imgX-ply->cntx)+fabs(imgY-ply->cnty)<2*NBR)
		{
			result = ply;
			break;
		}	

		node=node->Next();
	}

	return result;
}

SVMPolygon *ImgView::SearchNearbyPolygonOrigin(double imgX, double imgY, SVMPoint **p)
{
	SVMPolygon *result = NULL;

    if (p != NULL)
        *p = NULL;

	CTypedPtrDblElement <SVMPolygon> *node=plyList.GetHeadPtr();	

	while (!plyList.IsSentinel(node))
	{
		SVMPolygon *ply = node->Data();

    	CTypedPtrDblElement <SVMPoint> *ptnode=ply->pntList.GetHeadPtr();	
        SVMPoint *p0 = ptnode->Data();
        
		if (p0->w>EPS)
		//if the point is not at the infinity;
		{
			double u=p0->u/p0->w;
			double v=p0->v/p0->w;

			if (fabs(imgX-u)+fabs(imgY-v)<2*NBR)
			{
				result = ply;
                if (p != NULL) {
                    *p = p0;
                }

				break;
			}
		}

        node = node->Next();
	}

	return result;
}


int ImgView::CheckExistingPoint(SVMPoint *goal)
{
	int index = 0;
	CTypedPtrDblElement <SVMPoint> *node=pntList.GetHeadPtr();	

	while (!pntList.IsSentinel(node))
	{
		index++;

		if (node->Data()==goal) return index;

		node=node->Next();
	}

	return 0;
}

SVMLine *ImgView::CheckExistingLine(SVMPoint *goal)
{
	SVMLine *result = NULL;

	CTypedPtrDblElement <SVMLine> *node=lineList.GetHeadPtr();	

	while (!lineList.IsSentinel(node))
	{
		SVMLine *line = node->Data();

		if (line->pnt1==goal || line->pnt2==goal)
		{
			result = line;
			break;
		}

		node=node->Next();
	}

	return result;
}


SVMPolygon *ImgView::CheckExistingPolygon(SVMPoint *goal)
{
	SVMPolygon *result = NULL;

	CTypedPtrDblElement <SVMPolygon> *node=plyList.GetHeadPtr();	

	while (!plyList.IsSentinel(node))
	{
		SVMPolygon *ply = node->Data();

		if (!ply->pntList.IsSentinel(ply->pntList.Find(goal)))
		{
			result = ply;
			break;
		}	

		node=node->Next();
	}

	return result;
}

int ImgView::CheckExistingPolygonIndex(SVMPolygon *goal)
{
	int index = 0;

    SVMPolygon *result = NULL;

	CTypedPtrDblElement <SVMPolygon> *node=plyList.GetHeadPtr();	

	while (!plyList.IsSentinel(node))
	{
		index++;

        SVMPolygon *ply = node->Data();

        if (ply == goal)
            return index;

		node=node->Next();
	}

	return 0;
}

void ImgView::AboutMe(void)
{
	helpPageUI->show();
}

void ImgView::computeVanishingPoints()
{

	CTypedPtrDblElement <SVMLine> *node=lineList.GetHeadPtr();	

	list<SVMLine> xLines, yLines, zLines;

	// Builds lists of x,y,z lines for VP processing
	while (!lineList.IsSentinel(node))
	{
		SVMLine *line = node->Data();
		switch (line->orientation)
		{
		case PARA_X:
			xLines.push_back(*line); break;
		case PARA_Y:
			yLines.push_back(*line); break;
		case PARA_Z:
			zLines.push_back(*line); break;
		}
		node=node->Next();
	}

	// Are there enough lines?
	if (xLines.size() < 2 || yLines.size() < 2 || zLines.size() < 2)
	{
		fl_alert("Not enough lines to compute vanishing points!");
		return;
	}

	xVanish = BestFitIntersect(xLines, imgWidth, imgHeight);
	yVanish = BestFitIntersect(yLines, imgWidth, imgHeight);
	zVanish = BestFitIntersect(zLines, imgWidth, imgHeight);

	xVanish.X = 1000; xVanish.Y = 0; xVanish.Z = 0; xVanish.W = 1;
	yVanish.X = 0; yVanish.Y = 1000; yVanish.Z = 0; yVanish.W = 1;
	zVanish.X = 0; zVanish.Y = 0; zVanish.Z = 1000; zVanish.W = 1;

	char TEMP[100];
	sprintf(TEMP, "xvp=[%f %f] yvp=[%f %f] zvp=[%f %f]", 
		xVanish.u, xVanish.v, yVanish.u, yVanish.v, zVanish.u, zVanish.v);

	printf("\nVANISHING POINTS\n");
	printf(TEMP);
	printf("\n");
    fflush(stdout);

	intersectionPoints.clear();
	intersectionPoints.push_back( &xVanish );
	intersectionPoints.push_back( &yVanish );
	intersectionPoints.push_back( &zVanish );
	intersectionPoints.insert( intersectionPoints.end(), pntSelStack.begin(), pntSelStack.end() );

}

void ImgView::computeRefHomography()
{
	vector<SVMPoint> points;
	
	// only 2 reference points on the stack: use x & y vanishing points
	if (pntSelStack.size() == 2 && xVanish.known() && yVanish.known())
	{
		points.push_back(xVanish);
		points.push_back(yVanish);
	}
	// only 3 reference points on the stack:  use x vanishing point
	else if (pntSelStack.size() == 3 && xVanish.known() && yVanish.known())
	{
		points.push_back(xVanish);
	}
	// not enough reference points
	else if (pntSelStack.size() < 4)
	{
		fl_alert( "Unable to calculate homography from fewer than 4 reference points." );
		return;
	}

	// push reference points on the stack
	for (int i = pntSelStack.size(); i > 0; i--)
	{
		SVMPoint &p = *pntSelStack[pntSelStack.size() - i];
		if ( !p.known() )
		{
			fl_alert( "Unable to use unknown point as reference point." );
			return;
		}
		if ( p.Z != 0 )
		{
			fl_alert( "Unable to use off-plane reference point to compute homography." );
			return;
		}
		points.push_back(p);
	}

    vector<Vec3d> basisPts;
	ComputeHomography(H, Hinv, points, basisPts, true);
    homographyComputed = true;
}

void ImgView::setReferencePoint()
{
	if (curRefPnt)
	{
		curRefPnt->X = atof(setRefPnt_X->value());
		curRefPnt->Y = atof(setRefPnt_Y->value());
		curRefPnt->Z = atof(setRefPnt_Z->value());

		// this point should be known by now and marked with a box
		curRefPnt->known(true);

		// set reference height if desired
		if (setRefPnt_UseHeight->value() == 1) // selPnt->Z != 0)
		{
			referenceHeight = curRefPnt->Z;
			refPointOffPlane = curRefPnt;
		}
		else if (setRefPnt_UseHeight->value() == 0 && refPointOffPlane == curRefPnt)
		{
			referenceHeight = 0;
			refPointOffPlane = NULL;
		}

		setRefPnt_Window->hide();

		redraw();
	}
}
void ImgView::populateHomography(SVMPolygon &ply, vector<Vec3d> *basisPts) {
	CTypedPtrDblElement<SVMPoint> *selNode = ply.pntList.GetHeadPtr();
	vector<SVMPoint> points;

	while (!ply.pntList.IsSentinel(selNode)) {
		points.push_back(*selNode->Data());
		selNode = selNode->Next();
	}
	if(basisPts == NULL) {
		vector<Vec3d> tmp;
		ComputeHomography(ply.H, ply.invH, points, tmp, false);
	} else {
		ComputeHomography(ply.H, ply.invH, points, *basisPts, false);
	}
}
void ImgView::writeTexture( SVMPolygon& ply, const char* texname )
{
	
	int i,j;
    printf( "Writing texture %s...\n", texname );
    vector<Vec3d> basisPts;
        
    if( ply.pntList.GetCount() < 4 ) {
		printf( "Cannot write texture %a; polygon smaller not a quadrilateral.", 
			texname );
		return;
	}
	else {
		populateHomography(ply, &basisPts);
		CTypedPtrDblElement<SVMPoint> *selNode = ply.pntList.GetHeadPtr();
	}

	CByteImage rick(imgWidth, imgHeight, 3);
	
	//Rick's .tga file format is in G,B,R order from bottom to top and from left to right;
	//our buffer is in R, G, B order from top to bottom and from left to right;
	for (j=0;j<imgHeight;j++)
	{
		for (i=0;i<imgWidth;i++)
		{
			int index = 3*(j*imgWidth+i);
			int x = i;
			int y = j;
			rick.Pixel(x,y,2) = imgBuf[index+0];
			rick.Pixel(x,y,1) = imgBuf[index+1];
			rick.Pixel(x,y,0) = imgBuf[index+2];
		}
	}

	//orig.Pixel(10,10,0)=0; orig.Pixel(10,10,1)=0; orig.Pixel(10,10,2)=1;
	//WriteFile(rick, "test.tga");

	CTypedPtrDblElement <SVMPoint> *pntNode = ply.pntList.GetHeadPtr();
	double umin=1e99, umax=-1e99, vmin=1e99, vmax=-1e99;

#if 1
    while (!ply.pntList.IsSentinel(pntNode))
	{
		SVMPoint *pnt = pntNode->Data();
		if (pnt->u < umin) umin=pnt->u; if (pnt->u > umax) umax=pnt->u;
		if (pnt->v < vmin) vmin=pnt->v; if (pnt->v > vmax) vmax=pnt->v;
		pntNode = pntNode->Next();
	}
#else
    for (int i = 0; i < (int) basisPts.size(); i++) {
		if (basisPts[i][0] < umin) umin=basisPts[i][0]; if (basisPts[i][0] > umax) umax=basisPts[i][0];
		if (basisPts[i][1] < vmin) vmin=basisPts[i][1]; if (basisPts[i][1] > vmax) vmax=basisPts[i][1];
    }
#endif

	int texHeight, texWidth;
	texHeight = texWidth = (int) __max(umax-umin, vmax-vmin);
    // texWidth = (int) (scale * (umax - umin));
    // texHeight = (int) (scale * (vmax - vmin));

    printf(" texWidth, texHeight: %d, %d\n", texWidth, texHeight);

	CByteImage tex(texWidth, texHeight, 3);
	
	CTransform3x3 tx;

	tx[0][0] = ply.H[0][0]; tx[0][1] = ply.H[0][1]; tx[0][2] = ply.H[0][2];
	tx[1][0] = ply.H[1][0]; tx[1][1] = ply.H[1][1]; tx[1][2] = ply.H[1][2];
	tx[2][0] = ply.H[2][0]; tx[2][1] = ply.H[2][1]; tx[2][2] = ply.H[2][2];

	CTransform3x3 s;
	s = CTransform3x3::Translation(0,0);
	s[0][0] = 1.0/texWidth; ///tx[2][2];
	s[1][1] = 1.0/texHeight; // /tx[2][2];

	tx = tx*s;

	printf("tx = [%f %f %f;\n"
           "      %f %f %f;\n"
           "      %f %f %f ]\n",
		tx[0][0], tx[0][1], tx[0][2],
		tx[1][0], tx[1][1], tx[1][2],
		tx[2][0], tx[2][1], tx[2][2]);

    Mat3d M(tx[0][0], tx[0][1], tx[0][2],
		tx[1][0], tx[1][1], tx[1][2],
		tx[2][0], tx[2][1], tx[2][2]);
		
/*
    Mat3d M(texWidth*tx[0][0], texHeight*tx[0][1], tx[0][2],
		texWidth*tx[1][0], texHeight*tx[1][1], tx[1][2],
		texWidth*tx[2][0], texHeight*tx[2][1], tx[2][2]);
*/
/*				


	for (int x=0; x<texWidth; x++)
	{
		for (int y=0; y<texHeight; y++)
		{
			Vec3d texCoord(x,y,1);
			texCoord[0]/=texWidth;
			texCoord[1]/=texHeight;

			Vec3d imgCoord = M*texCoord;
			imgCoord /= imgCoord[2];
			printf("xy=%d %d -- texCoord=%f %f -- imgCoord=%f %f\n", x,y, texCoord[0], texCoord[1],
				imgCoord[0], imgCoord[1]);

			
			for (int c=0; c<3; c++)
			{
				tex.Pixel(x,y,c) = rick.Pixel(imgCoord[0], imgCoord[1], c);
			}

		}

	}*/
	
	// tx specifies orig->tex
	// 
	//tx = tx.Inverse();
	WarpGlobal(rick, tex, tx, eWarpInterpLinear);

	printf("Writing %s...\n", texname);
	WriteFile(tex, texname);
}

#include "ImgView.inl"
