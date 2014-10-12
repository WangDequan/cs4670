/* ImgViewDraw.cpp */
#pragma warning(disable : 4996)

#include "ImgView.h"

#include <FL/glut.H>


/*
	Part of the code from Brad Fish http://students.cs.byu.edu/~bfish/glfont.php
*/
int ImgView::glFontCreate (GLFONT *Font, char *FileName, int Tex)
{
	FILE *Input;
	char *TexBytes;
	int Num;

	//Open font file
	if ((Input = fopen(FileName, "rb")) == NULL) {
		return FALSE;
	}
	//Read glFont structure
	fread(Font, sizeof(GLFONT), 1, Input);

	//Save texture number
	Font->Tex = Tex;

	//Get number of characters
	Num = Font->IntEnd - Font->IntStart + 1;
	
	//Allocate memory for characters
	if ((Font->Char = (GLFONTCHAR *)malloc(
		sizeof(GLFONTCHAR) * Num)) == NULL)
		return FALSE;

	//Read glFont characters
	fread(Font->Char, sizeof(GLFONTCHAR), Num, Input);

	//Get texture size
	Num = Font->TexWidth * Font->TexHeight * 2;

	//Allocate memory for texture data
	if ((TexBytes = (char *)malloc(Num)) == NULL)
		return FALSE;
	
	//Read texture data
	fread(TexBytes, sizeof(char), Num, Input);

	//Set texture attributes
	glBindTexture(GL_TEXTURE_2D, Font->Tex);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
		GL_MODULATE);  
	
	//Create texture
	glTexImage2D(GL_TEXTURE_2D, 0, 2, Font->TexWidth,
		Font->TexHeight, 0, GL_LUMINANCE_ALPHA, 
		GL_UNSIGNED_BYTE, (void *)TexBytes);

	//Clean up
	free(TexBytes);
	fclose(Input);

	//Return pointer to new font
	return TRUE;
}

//*********************************************************
void ImgView::glFontTextOut (char *String, float x, float y, 
	float z)
{
	int Length, i;
	GLFONTCHAR *Char;

	//Return if we don't have a valid glFont 
	if (glFont == NULL)
		return;
	
	//Get length of string
	Length = strlen(String);
	
	//Begin rendering quads
	glBegin(GL_QUADS);
	int size = 7;
	//Loop through characters
	for (i = 0; i < Length; i++)
	{
		//Get pointer to glFont character
		Char = &glFont->Char[(int)String[i] -
			glFont->IntStart];
		//Specify vertices and texture coordinates
		glTexCoord2f(Char->tx1, Char->ty1);
		glVertex3f(x, y, z);
		glTexCoord2f(Char->tx1, Char->ty2);
		glVertex3f(x, y - Char->dy*size, z);
		glTexCoord2f(Char->tx2, Char->ty2);
		glVertex3f(x + Char->dx*size, y - Char->dy*size, z);
		glTexCoord2f(Char->tx2, Char->ty1);
		glVertex3f(x + Char->dx*size, y, z);
	
		//Move to next character
		x += Char->dx * size;
	}

	//Stop rendering quads
	glEnd();
}

void ImgView::draw(void)
{
	glClearColor((GLclampf)BK_COLOR[0],(GLclampf)BK_COLOR[1],(GLclampf)BK_COLOR[2],(GLclampf)BK_COLOR[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	if (GetDrawMode(DRAW_3D))
		draw3D();
	else
		draw2D();
}

void ImgView::draw2D(void)
{
	glViewport(0,0,w(),h());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-0.5*w()/scale2D,0.5*w()/scale2D,-0.5*h()/scale2D,0.5*h()/scale2D,-20000,20000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (glIsTexture(texName))
	{
		drawImg2D();		

        if (GetDrawMode(DRAW_POINT))
		{
			drawPoints2D();
		}
		if (GetDrawMode(DRAW_LINE))
		{
			drawLines2D();
		}
		if (GetDrawMode(DRAW_POLYGON))
		{
			drawPolygons2D();
		}

		drawCurrentElements();
	}	
}

void ImgView::drawCurrentElements(void)
{
	if (editMode==EDIT_POINT)
	{
	}
	else if (editMode==EDIT_XLINE||editMode==EDIT_YLINE||editMode==EDIT_ZLINE||editMode==EDIT_OLINE)
	{
		if (curLine.pnt1)
		{
			glColor3dv(HIGHLIGHT_COLOR);

			glBegin(GL_LINES);

			//current line shouldn't contain points at infinity;
			double u1=curLine.pnt1->u+imgLeft;
			double v1=curLine.pnt1->v+imgBottom;

			double u2=(mouseX+0.5-w()/2.0)/scale2D;
			double v2=(h()-(mouseY+0.5)-h()/2.0)/scale2D;
			
			glVertex3d(u1,v1,ZLine2D);
			glVertex3d(u2,v2,ZLine2D);

			glEnd();
		}
	}
	else if (editMode==EDIT_POLYGON)
	{
		if (curPly.pntList.GetCount())
		{
			glColor3dv(HIGHLIGHT_COLOR);
			
			CTypedPtrDblElement <SVMPoint> *node = curPly.pntList.GetHeadPtr();

			glBegin(GL_LINE_STRIP);

			double u,v;
			
			while (!curPly.pntList.IsSentinel(node))
			{
				SVMPoint *pnt = node->Data();

				//current polygon shouldn't contain points at infinity;
				u = pnt->u+imgLeft;
				v = pnt->v+imgBottom;

				glVertex3d(u,v,ZPolygon2D);

				node=node->Next();
			}

			u=(mouseX+0.5-w()/2.0)/scale2D;
			v=(h()-(mouseY+0.5)-h()/2.0)/scale2D;
			glVertex3d(u,v,ZPolygon2D);		

			glEnd();
		}
    } else if (editMode == EDIT_YZRECT || editMode == EDIT_XZRECT || editMode == EDIT_XYRECT) {
		if (curRect.pnt1) {
			glColor3dv(HIGHLIGHT_COLOR);
            glLineWidth(3.0);
            // glEnable(GL_BLEND);
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            // glEnable(GL_LINE_SMOOTH);

			//current line shouldn't contain points at infinity;
			double u1=curRect.pnt1->u+imgLeft;
			double v1=curRect.pnt1->v+imgBottom;

			double u3=(mouseX+0.5-w()/2.0)/scale2D;
			double v3=(h()-(mouseY+0.5)-h()/2.0)/scale2D;
			
            double u1n = curRect.pnt1->u;
            double v1n = curRect.pnt1->v;

            double u3n = u3 - imgLeft;
            double v3n = v3 - imgBottom;

			// glVertex3d(u1,v1,ZLine2D);
			// glVertex3d(u2,v2,ZLine2D);

            // Infer p2, p4 from p1 and p3
            double u2n, v2n, u4n, v4n;
            solveForOppositeCorners(u1n, v1n, u3n, v3n, u2n, v2n, u4n, v4n);

            double u2 = u2n + imgLeft;
            double v2 = v2n + imgBottom;

            double u4 = u4n + imgLeft;
            double v4 = v4n + imgBottom;

			glBegin(GL_LINE_LOOP);

            glVertex3d(u1, v1, ZLine2D);
            glVertex3d(u2, v2, ZLine2D);
            glVertex3d(u3, v3, ZLine2D);
            glVertex3d(u4, v4, ZLine2D);

			glEnd();
		
            glLineWidth(1.0);
        }
    } else if (editMode == EDIT_SWEEP) {
		double imgX=(mouseX+0.5-w()/2.0)/scale2D-imgLeft;
		double imgY=(h()-(mouseY+0.5)-h()/2.0)/scale2D-imgBottom;

        if (curSweep.poly == NULL)
            return;

        SVMPoint *p1, *p2, *p3, *p4;
        curSweep.poly->getFourPoints(&p1, &p2, &p3, &p4);
        Vec3d pt1(p1->u, p1->v, p1->w);
        Vec3d pt2(p2->u, p2->v, p2->w);
        Vec3d pt3(p3->u, p3->v, p3->w);
        Vec3d pt4(p4->u, p4->v, p4->w);

        Vec3d pc1, pc2, pc3, pc4;
        solveForOppositeFace(&curSweep, imgX, imgY, pc1, pc2, pc3, pc4);

        // now we can draw the points
        double u1, v1, u2, v2, u3, v3, u4, v4;
        u1 = pc1[0] / pc1[2] + imgLeft;
        v1 = pc1[1] / pc1[2] + imgBottom;

        u2 = pc2[0] / pc2[2] + imgLeft;
        v2 = pc2[1] / pc2[2] + imgBottom;

        u3 = pc3[0] / pc3[2] + imgLeft;
        v3 = pc3[1] / pc3[2] + imgBottom;

        u4 = pc4[0] / pc4[2] + imgLeft;
        v4 = pc4[1] / pc4[2] + imgBottom;

        double u1a, v1a, u2a, v2a, u3a, v3a, u4a, v4a;
        u1a = pt1[0] / pt1[2] + imgLeft;
        v1a = pt1[1] / pt1[2] + imgBottom;

        u2a = pt2[0] / pt2[2] + imgLeft;
        v2a = pt2[1] / pt2[2] + imgBottom;

        u3a = pt3[0] / pt3[2] + imgLeft;
        v3a = pt3[1] / pt3[2] + imgBottom;

        u4a = pt4[0] / pt4[2] + imgLeft;
        v4a = pt4[1] / pt4[2] + imgBottom;

		glColor3dv(HIGHLIGHT_COLOR);
        glLineWidth(3.0);
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINE_LOOP);
        glVertex3d(u1, v1, ZLine2D);
        glVertex3d(u2, v2, ZLine2D);
        glVertex3d(u3, v3, ZLine2D);
        glVertex3d(u4, v4, ZLine2D);
        glEnd();

        glBegin(GL_LINES);
        glVertex3d(u1a, v1a, ZLine2D);
        glVertex3d(u1, v1, ZLine2D);
        glVertex3d(u2a, v2a, ZLine2D);
        glVertex3d(u2, v2, ZLine2D);
        glVertex3d(u3a, v3a, ZLine2D);
        glVertex3d(u3, v3, ZLine2D);
        glVertex3d(u4a, v4a, ZLine2D);
        glVertex3d(u4, v4, ZLine2D);
        glEnd();

        glLineWidth(1.0);
    }
}

void ImgView::drawImg2D(void)
{
	glEnable( GL_TEXTURE_2D );
	
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glBindTexture( GL_TEXTURE_2D, texName );

	glColor3d( 1.0, 1.0, 1.0 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0.0, 0.0 );
	glVertex3d( imgLeft, imgBottom, ZImage2D );
	glTexCoord2d( 1.0, 0.0 );
	glVertex3d( imgLeft + imgWidth, imgBottom, ZImage2D );
	glTexCoord2d( 1.0, 1.0 );
	glVertex3d( imgLeft + imgWidth, imgBottom + imgHeight, ZImage2D );
	glTexCoord2d( 0.0, 1.0 );
	glVertex3d( imgLeft, imgBottom + imgHeight, ZImage2D );
	glEnd();

	glDisable( GL_TEXTURE_2D );
}

static void GlutPrint(const char str[])
{


	// const char *ptr;
	//glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'c');
	//glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "yesy");
	//glutStrokeString(
	
	//glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'c');
	/*
	for (ptr = str; *ptr != '\0'; ++ptr)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *ptr);*/
}

void ImgView::drawPoints2D(void)
{
	CTypedPtrDblElement <SVMPoint> *node=pntList.GetHeadPtr();

	glBegin(GL_LINES);		

	while (!pntList.IsSentinel(node))
	{
		SVMPoint *pnt = node->Data();

		if (pnt->w>EPS)
		//if the point is not at the infinity;
		{
			double u=pnt->u/pnt->w+imgLeft;
			double v=pnt->v/pnt->w+imgBottom;		

			if (editMode==EDIT_POINT 
				&& pnt==selPnt)
			{
				glColor3dv(HIGHLIGHT_COLOR);
			}
			else
			{
				glColor3dv(POINT_COLOR);
			}

			//draw a cross to represent a point on z=1 plane;
			glVertex3d(u-5,v,ZPoint2D);
			glVertex3d(u+5,v,ZPoint2D);
			glVertex3d(u,v-5,ZPoint2D);
			glVertex3d(u,v+5,ZPoint2D);

			// EUGENE_ADD
			// if the point is known, then draw a box too
			if (pnt->known())
			{
				glVertex3d(u-3,v-3,ZPoint2D);
				glVertex3d(u+3,v-3,ZPoint2D); // bot
				glVertex3d(u+3,v-3,ZPoint2D);
				glVertex3d(u+3,v+3,ZPoint2D); // right
				glVertex3d(u+3,v+3,ZPoint2D);
				glVertex3d(u-3,v+3,ZPoint2D); // top
				glVertex3d(u-3,v+3,ZPoint2D);
				glVertex3d(u-3,v-3,ZPoint2D); // left
			}
		}
		node=node->Next();
	}		

	glEnd();


	if( showGuideLines )
	{
		glEnable( GL_LINE_STIPPLE );
		// glEnable( GL_LINE_SMOOTH );
		glLineStipple( 2, 0x5555 );
		glBegin(GL_LINES);		

			for( int i=0; i < (int)lines.size() && i < 2; i++ )
			{
				const std::pair<SVMPoint*, SVMPoint*>& line = lines[i];
				glColor3dv(GUIDE_COLOR);
				const double u1 = line.first->u/line.first->w+imgLeft;
				const double v1 = line.first->v/line.first->w+imgBottom;
				const double u2 = line.second->u/line.second->w+imgLeft;
				const double v2 = line.second->v/line.second->w+imgBottom;

				const double uDir = u2 - u1;
				const double vDir = v2 - v1;

				glVertex3d(u1 + 100*uDir, v1 + 100*vDir, ZPoint2D);
				glVertex3d(u2 - 100*uDir, v2 - 100*vDir, ZPoint2D);
			}

		glEnd();
		// glDisable( GL_LINE_SMOOTH );
		glDisable( GL_LINE_STIPPLE );
	}

	// EUGENE_ADD

	char buf[16];
	glEnable(GL_TEXTURE_2D);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, texFont);
	for (int p=0; p<(int)pntSelStack.size(); p++)
	{
		glColor3dv(POINT_COLOR);

		SVMPoint *pnt = pntSelStack[p];

		double u=pnt->u/pnt->w+imgLeft;
		double v=pnt->v/pnt->w+imgBottom;

		
		//glRasterPos2d(100, 100);
		
		sprintf(buf, "%d", p);
		
		glFontTextOut(buf, (float) u+2, (float) v-2, (float) ZPoint2D);
		
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void ImgView::drawLines2D(void)
{
	CTypedPtrDblElement <SVMLine> *node=lineList.GetHeadPtr();	

	glBegin(GL_LINES);

	while (!lineList.IsSentinel(node))
	{
		SVMLine *line = node->Data();
		SVMPoint *pnt1 = line->pnt1;
		SVMPoint *pnt2 = line->pnt2;

		if (pnt1->w>EPS && pnt2->w>EPS)
		//if two end point are not at the infinity;
		{
			double u1=pnt1->u/pnt1->w+imgLeft;
			double v1=pnt1->v/pnt1->w+imgBottom;
			double u2=pnt2->u/pnt2->w+imgLeft;
			double v2=pnt2->v/pnt2->w+imgBottom;

			if ((editMode==EDIT_XLINE||editMode==EDIT_YLINE||editMode==EDIT_ZLINE||editMode==EDIT_OLINE)
				&& curLine.pnt1==NULL && line==selLine)
				glColor3dv(HIGHLIGHT_COLOR);
			else if (line->orientation==PARA_X)
				glColor3dv(XLINE_COLOR);
			else if (line->orientation==PARA_Y)
				glColor3dv(YLINE_COLOR);
			else if (line->orientation==PARA_Z)
				glColor3dv(ZLINE_COLOR);
			else 
				glColor3dv(OLINE_COLOR);

			//draw a line on z=2 plane;
			glVertex3d(u1,v1,ZLine2D);
			glVertex3d(u2,v2,ZLine2D);
		}

		node=node->Next();
	}

	glEnd();
}

void ImgView::drawPolygons2D(void)
{

	CTypedPtrDblElement <SVMPolygon> *plyNode = plyList.GetHeadPtr();
		
	while (!plyList.IsSentinel(plyNode))
	{
		CTypedPtrDblElement <SVMPoint> *pntNode = plyNode->Data()->pntList.GetHeadPtr();

		glBegin(GL_LINE_LOOP);

		if (editMode==EDIT_POLYGON 
			&& curPly.pntList.IsEmpty() && plyNode->Data() == selPly)
		{
			glColor3dv(HIGHLIGHT_COLOR);
		}
		else
		{
			glColor3dv(POLYGON_COLOR);
		}

		while (!plyNode->Data()->pntList.IsSentinel(pntNode))
		{
			SVMPoint *pnt = pntNode->Data();

			double u=pnt->u/pnt->w+imgLeft;
			double v=pnt->v/pnt->w+imgBottom;		

			glVertex3d(u,v,ZPolygon2D);

			pntNode = pntNode->Next();
		}				

		glEnd();

		double cntx = plyNode->Data()->cntx+imgLeft;
		double cnty = plyNode->Data()->cnty+imgBottom;

		glBegin(GL_LINE_LOOP);

		glVertex3d(cntx+NBR,cnty+NBR,ZPolygon2D);
		glVertex3d(cntx-NBR,cnty+NBR,ZPolygon2D);
		glVertex3d(cntx-NBR,cnty-NBR,ZPolygon2D);
		glVertex3d(cntx+NBR,cnty-NBR,ZPolygon2D);

		glEnd();
		
		plyNode=plyNode->Next();
	}	
}

void ImgView::draw3D(void)
{
	glViewport(0,0,w(),h());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90,(double)w()/(double)h(),0.01,1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (glIsTexture(texName)&&pntList.GetCount())
	{
		ComputePointCenter3D(cntX3D,cntY3D,cntZ3D,&pntList);

		glTranslated(traX3D,traY3D,traZ3D);
		glTranslated(cntX3D,cntY3D,cntZ3D);
		glScaled(scale3D,scale3D,scale3D);
		glRotated(rotX3D,1,0,0);
		glRotated(rotY3D,0,1,0);
		glRotated(rotZ3D,0,0,1);
		glTranslated(-cntX3D,-cntY3D,-cntZ3D);

		if (GetDrawMode(DRAW_POINT))
		{
			drawPoints3D();
		}
		if (GetDrawMode(DRAW_LINE))
		{
			drawLines3D();
		}
		if (GetDrawMode(DRAW_POLYGON))
		{
			drawPolygons3D();
		}		
	}	
}

void ImgView::drawPoints3D(void)
{
	CTypedPtrDblElement <SVMPoint> *node=pntList.GetHeadPtr();

	glBegin(GL_LINES);

	glColor3dv(POINT_COLOR);

	while (!pntList.IsSentinel(node))
	{
		SVMPoint *pnt = node->Data();

		if (pnt->W>EPS)
		//if the point is not at the infinity;
		{
			double x=pnt->X/pnt->W;
			double y=pnt->Y/pnt->W;
			double z=pnt->Z/pnt->W;

			//draw a cross to represent a point;
			glVertex3d(x-5,y,z);
			glVertex3d(x+5,y,z);
			glVertex3d(x,y-5,z);
			glVertex3d(x,y+5,z);
			glVertex3d(x,y,z-5);
			glVertex3d(x,y,z+5);

		}
		node=node->Next();
	}		
	glEnd();
}

void ImgView::drawLines3D(void)
{
	CTypedPtrDblElement <SVMLine> *node=lineList.GetHeadPtr();	

	glBegin(GL_LINES);

	while (!lineList.IsSentinel(node))
	{
		SVMLine *line = node->Data();
		SVMPoint *pnt1 = line->pnt1;
		SVMPoint *pnt2 = line->pnt2;

		if (pnt1->W>EPS && pnt2->W>EPS)
		//if two end point are not at the infinity;
		{
			double x1=pnt1->X/pnt1->W;
			double y1=pnt1->Y/pnt1->W;
			double z1=pnt1->Z/pnt1->W;

			double x2=pnt2->X/pnt2->W;
			double y2=pnt2->Y/pnt2->W;
			double z2=pnt2->Z/pnt2->W;

			if ((editMode==EDIT_XLINE||editMode==EDIT_YLINE||editMode==EDIT_ZLINE||editMode==EDIT_OLINE)
				&& curLine.pnt1==NULL && line==selLine)
				glColor3dv(HIGHLIGHT_COLOR);
			else if (line->orientation==PARA_X)
				glColor3dv(XLINE_COLOR);
			else if (line->orientation==PARA_Y)
				glColor3dv(YLINE_COLOR);
			else if (line->orientation==PARA_Z)
				glColor3dv(ZLINE_COLOR);
			else 
				glColor3dv(OLINE_COLOR);

			glVertex3d(x1,y1,z1);
			glVertex3d(x2,y2,z2);
		}

		node=node->Next();
	}

	glEnd();
}

void ImgView::drawPolygons3D(void)
{
	glEnable( GL_TEXTURE_2D );

	glBindTexture( GL_TEXTURE_2D, texName );
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glColor3d( 1.0, 1.0, 1.0 );
	
	CTypedPtrDblElement <SVMPolygon> *plyNode = plyList.GetHeadPtr();

	while (!plyList.IsSentinel(plyNode)) {		
		SVMPolygon *ply = plyNode->Data();
		if(!ply->isHomographyPopulated) {
			populateHomography(*ply, NULL);
			ply->isHomographyPopulated = true;
		}
		CTypedPtrDblElement <SVMPoint> *pntNode = ply->pntList.GetHeadPtr();

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScaled(1.0/(double)imgWidth,1.0/(double)imgHeight,1.0);

		double m[4*4];
		m[0]= ply->H[0][0];		m[4] = ply->H[0][1];	m[8] = 0;	m[12] = ply->H[0][2];
		m[1]= ply->H[1][0];		m[5] = ply->H[1][1];	m[9] = 0;	m[13] = ply->H[1][2];
		m[2]= 0;				m[6] = 0;				m[10] = 1;	m[14] = 0;
		m[3]= ply->H[2][0];		m[7] = ply->H[2][1];	m[11] = 0;	m[15] = ply->H[2][2];

		glMultMatrixd(m);		
		glBegin( GL_POLYGON );

		while (!ply->pntList.IsSentinel(pntNode)) {
			SVMPoint *pnt = pntNode->Data();
			
			if (pnt->W>EPS) {	//the point is not at infinity
				double u,v;
				ApplyHomography(u,v,ply->invH,pnt->u,pnt->v,pnt->w);

				double x=pnt->X/pnt->W;
				double y=pnt->Y/pnt->W;
				double z=pnt->Z/pnt->W;

				glTexCoord4d( u, v, 0, 1 );
				glVertex3d(x,y,z);
			}

			pntNode = pntNode->Next();
		}				

		glEnd();	
		
		plyNode=plyNode->Next();
	}	

	glDisable( GL_TEXTURE_2D );
}
