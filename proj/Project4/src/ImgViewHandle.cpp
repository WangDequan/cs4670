/* ImgViewHandle.cpp */
#pragma warning(disable : 4996)

#include "ImgView.h"

#include <algorithm> 
#include <vector>
using namespace std;

double distance( double x, double y, std::pair<SVMPoint*, SVMPoint*> line )
{
	// Calculate distance from the line connection
	// the two points.
	const double x1 = (line.first)->u/(line.first)->w;
	const double x2 = (line.second)->u/(line.second)->w;
	const double y1 = (line.first)->v/(line.first)->w;
	const double y2 = (line.second)->v/(line.second)->w;

	return fabs( (y2-y1)*(x-x1) - (x2-x1)*(y-y1) ) /
		sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}


class LineComparator : public std::binary_function< std::pair< SVMPoint*, SVMPoint* >,
	std::pair< SVMPoint*, SVMPoint* >, bool >
{
	double _x, _y;

public:
	LineComparator( double x, double y )
		: _x(x), _y(y)
	{
	}

	bool operator()(const std::pair< SVMPoint*, SVMPoint* >& x, 
		const std::pair< SVMPoint*, SVMPoint* >& y)
	{
		return distance(_x, _y, x) < distance(_x, _y, y);
	}
};

int ImgView::handle2D(int c)
{	
	if (c==FL_PUSH)
	{
        // printf("push...\n");
        // fflush(stdout);
        
        int x = Fl::event_x();
		int y = Fl::event_y();

		if (imgBuf)
		{
			if (Fl::event_button()==FL_LEFT_MOUSE)
			{
				if (editMode==EDIT_POINT)
				{
					double imgX=(x+0.5-w()/2.0)/scale2D-imgLeft;
					double imgY=(h()-(y+0.5)-h()/2.0)/scale2D-imgBottom;

					if( true == showGuideLines && (Fl::event_state(FL_CTRL)) )
					{
						if( lines.size() == 1 )
						{
							const std::pair<SVMPoint*, SVMPoint*>& line = lines[0];
							const SVMPoint& point1 = *line.first;
							const SVMPoint& point2 = *line.second;

							// Need to project onto the line.
							const double x1 = point1.u / point1.w;
							const double y1 = point1.v / point1.w;

							const double x2 = point2.u / point2.w;
							const double y2 = point2.v / point2.w;
							
							const double x3 = imgX;
							const double y3 = imgY;

							const double u = ((x3 - x1)*(x2 - x1) + (y3 - y1)*(y2 - y1))
								/ ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
							imgX = x1 + u * (x2 - x1);
							imgY = y1 + u * (y2 - y1);
						}
						else if( lines.size() >= 2 )
						{
							// Need to find the intersection of the 
							// first two lines
							const std::pair<SVMPoint*, SVMPoint*>& line1 = lines[0];
							const std::pair<SVMPoint*, SVMPoint*>& line2 = lines[1];
							const SVMPoint& point1 = *line1.first;
							const SVMPoint& point2 = *line1.second;
							const SVMPoint& point3 = *line2.first;
							const SVMPoint& point4 = *line2.second;

							// Need to project onto the line.
							const double x1 = point1.u / point1.w;
							const double y1 = point1.v / point1.w;

							const double x2 = point2.u / point2.w;
							const double y2 = point2.v / point2.w;

							const double x3 = point3.u / point3.w;
							const double y3 = point3.v / point3.w;

							const double x4 = point4.u / point4.w;
							const double y4 = point4.v / point4.w;

							const double u_a = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3))/
								((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
							imgX = x1 + u_a*(x2-x1);
							imgY = y1 + u_a*(y2-y1);

						}
					}

					selPnt = new SVMPoint(imgX,imgY);
					pntList.AddTail(selPnt);

					redraw();
				}
				else if (editMode==EDIT_XLINE||editMode==EDIT_YLINE||editMode==EDIT_ZLINE||editMode==EDIT_OLINE)
				{
					double imgX=(x+0.5-w()/2.0)/scale2D-imgLeft;
					double imgY=(h()-(y+0.5)-h()/2.0)/scale2D-imgBottom;

					if (curLine.pnt1==NULL)
					// create the first point;
					{
						if (Fl::event_state(FL_CTRL))
						// if control is pressed, use the existing point nearby as one end point of this line.
						{
							curLine.pnt1 = SearchNearbyPoint(imgX,imgY);
						}
						
						if (curLine.pnt1==NULL)
						{
							curLine.pnt1=new SVMPoint(imgX,imgY);
						}

						if (editMode == EDIT_XLINE)
						{
							curLine.orientation = PARA_X;
						}
						else if (editMode == EDIT_YLINE)
						{
							curLine.orientation = PARA_Y;
						}
						else if (editMode == EDIT_ZLINE)
						{
							curLine.orientation = PARA_Z;
						}
						else 
						{
							curLine.orientation = OTHER_ORIENT;
						}
					}
					else 
					// create the second point;
					{
						if (Fl::event_state(FL_CTRL))
						// if control is pressed, use the existing point nearby as one end point of this line.
						{
							curLine.pnt2 = SearchNearbyPoint(imgX,imgY);
						}
						
						if (curLine.pnt2==NULL)
						{
							curLine.pnt2=new SVMPoint(imgX,imgY);
						}				
						
						if (editMode == EDIT_XLINE)
						{
							curLine.orientation = PARA_X;
						}
						else if (editMode == EDIT_YLINE)
						{
							curLine.orientation = PARA_Y;
						}
						else if (editMode == EDIT_ZLINE)
						{
							curLine.orientation = PARA_Z;
						}
						else 
						{
							curLine.orientation = OTHER_ORIENT;
						}

						if (!CheckExistingPoint(curLine.pnt1))
						{
							pntList.AddTail(curLine.pnt1);
						}
						if (!CheckExistingPoint(curLine.pnt2))
						{
							pntList.AddTail(curLine.pnt2);
						}

						selLine = new SVMLine(curLine);
						lineList.AddTail(selLine);

						curLine.pnt1 = curLine.pnt2 = NULL;
						
						redraw();
					}
				}
				else if (editMode==EDIT_POLYGON)
				{
					if (Fl::event_state(FL_SHIFT))
					{
						if (selPly)
						{
							const char *name = fl_input("change the name of the polygon",selPly->name);
							if (name)
								strcpy(selPly->name,name);
						}
					}
					else
					{
						double imgX=(x+0.5-w()/2.0)/scale2D-imgLeft;
						double imgY=(h()-(y+0.5)-h()/2.0)/scale2D-imgBottom;

						SVMPoint *pnt = NULL;
						if (Fl::event_state(FL_CTRL))
						// if control is pressed, use the existing point nearby as one end point of this line.
						{
							pnt = SearchNearbyPoint(imgX,imgY);
						}

						if (pnt==NULL)
						{
							pnt=new SVMPoint(imgX,imgY);
						}

						if( curPly.pntList.IsSentinel(curPly.pntList.Find(pnt)) )
							curPly.pntList.AddTail(pnt);
					}
				}
                else if (editMode==EDIT_YZRECT||editMode==EDIT_XZRECT||editMode==EDIT_XYRECT)
                {
                    printf("Creating a rectangle...\n");

                    if (referenceHeight == 0.0) {
                        fl_message("Please first specify a reference height\n");
                        return 1;
                    }

                    if (!homographyComputed) {
                        fl_message("Please first specify a reference homography\n");
                        return 1;
                    }

					double imgX=(x+0.5-w()/2.0)/scale2D-imgLeft;
					double imgY=(h()-(y+0.5)-h()/2.0)/scale2D-imgBottom;

					if (curRect.pnt1==NULL)
					// create the first point;
					{
                        // existing point must be used as the first point
						curRect.pnt1 = SearchNearbyPoint(imgX,imgY);
						
						if (curRect.pnt1==NULL || !curRect.pnt1->known())
						{
                            fl_message("Must use known 3D point as the initial point for a rectangle!");
                            return 1;
                        }

                    } else {
                        /* -TODO: This could be relaxed ... */
						curRect.pnt2 = SearchNearbyPoint(imgX,imgY);

						if (curRect.pnt2==NULL /*|| !curRect.pnt2->known()*/)
						{
							curRect.pnt2=new SVMPoint(imgX,imgY);
                            pntList.AddTail(curRect.pnt2);
                        }

                        /* Create the new polygon */
                     	selPly = new SVMPolygon;

						if (editMode == EDIT_YZRECT)
						{
							selPly->orientation = PARA_YZ;
						}
						else if (editMode == EDIT_XZRECT)
						{
							selPly->orientation = PARA_XZ;
						}
						else if (editMode == EDIT_XYRECT)
						{
							selPly->orientation = PARA_XY;
						}

    					SVMPoint *p1 = curRect.pnt1; // existing point
                        SVMPoint *p3 = curRect.pnt2; // existing point

                        SVMPoint *p2 = new SVMPoint;
                        SVMPoint *p4 = new SVMPoint;

                        solveForOppositeCorners(p1->u, p1->v, p3->u, p3->v, p2->u, p2->v, p4->u, p4->v);

					    pntList.AddTail(p2);
                        pntList.AddTail(p4);

						selPly->pntList.AddTail(p1);
						selPly->pntList.AddTail(p2);
						selPly->pntList.AddTail(p3);
						selPly->pntList.AddTail(p4);

					    ComputePointAverage2D(selPly->cntx,selPly->cnty,&(selPly->pntList));

					    //the value here is for initialization, not the final answer;
					    //the final answer is dependent on the final reconstruction. 

					    CTypedPtrDblElement<SVMPoint> *selNode = selPly->pntList.GetHeadPtr();
					    vector<SVMPoint> points;
					    for( int i=0; i < 4; i++ )
					    {
						    points.push_back(*selNode->Data());
/*							printf("p%d = [ %f %f %f ] -> [ %f %f %f ]\n", 
							    *selNode->Data()->u, *selNode->Data()->v, *selNode->Data()->w,  
							    *selNode->Data()->X, *selNode->Data()->Y, */
						    selNode = selNode->Next();
					    }

//						MakeHomography(selPly->invH, selPly->H, points);

					    const char *name = fl_input("please give a name to the polygon, which will be used as texture filename later.");

					    if (name)
					    {
						    strcpy(selPly->name,name);
					    }

					    plyList.AddTail(selPly);

					    curRect.pnt1 = NULL;
					    curRect.pnt2 = NULL;

					    redraw();
                    }
                } 
                else if (editMode==EDIT_SWEEP)
                {
				    double imgX=(x+0.5-w()/2.0)/scale2D-imgLeft;
				    double imgY=(h()-(y+0.5)-h()/2.0)/scale2D-imgBottom;

                    if (curSweep.poly == NULL) {
                        // select a nearby line on a polygon

                        SVMPoint *pt;
						printf("t- %f %f\n", (float)imgX, (float)imgY);
                        SVMPolygon *poly = SearchNearbyPolygonOrigin(imgX, imgY, &pt);

                        if (!poly) {
                            fl_message("Must click on a polygon!");
                            return 1;
                        }

                        if (poly->orientation == OTHER_ORIENT) {
                            fl_message("Must sweep a polygon with known orientation!");
                            return 1;
                        }

                        if (pt != poly->pntList.GetHeadPtr()->Data()) {
                            fl_message("Must click on the first point of a polygon\n");
                            return 1;
                        }

                        curSweep.poly = poly;
                        curSweep.pStart = pt;

                        redraw();
                    } else {
                        // clicked on a final point, compute the six faces of the box
                        SVMPoint *points[8];

                        curSweep.poly->getFourPoints(points+0, points+1, points+2, points+3);

                        // get the point on the opposite face
                        Vec3d p1, p2, p3, p4;
                        solveForOppositeFace(&curSweep, imgX, imgY, p1, p2, p3, p4);

                        // create new points for the 'back' face
                        points[4] = new SVMPoint(p1[0] / p1[2], p1[1] / p1[2]);
                        points[5] = new SVMPoint(p2[0] / p2[2], p2[1] / p2[2]);
                        points[6] = new SVMPoint(p3[0] / p3[2], p3[1] / p3[2]);
                        points[7] = new SVMPoint(p4[0] / p4[2], p4[1] / p4[2]);


                        // solve for the 3D positions of each point
                        find3DPositionsBox(points);

					    pntList.AddTail(points[4]);
					    pntList.AddTail(points[5]);
					    pntList.AddTail(points[6]);
					    pntList.AddTail(points[7]);

                        // create a set of new polygons and add to a new box
                        SVMPolygon *polys[6];
                        polys[0] = curSweep.poly;
                        polys[1] = new SVMPolygon();
                        polys[2] = new SVMPolygon();
                        polys[3] = new SVMPolygon();
                        polys[4] = new SVMPolygon();
                        polys[5] = new SVMPolygon();
                    
                        // 0 -- 1      4 -- 5
                        // |    |      |    |
                        // 3 -- 2      7 -- 6

                        // front is already done

                        // back
                        polys[1]->pntList.AddTail(points[5]);
                        polys[1]->pntList.AddTail(points[4]);
                        polys[1]->pntList.AddTail(points[7]);
                        polys[1]->pntList.AddTail(points[6]);
                        sprintf(polys[1]->name, "%s.1", polys[0]->name);

                        // left
                        polys[2]->pntList.AddTail(points[4]);
                        polys[2]->pntList.AddTail(points[0]);
                        polys[2]->pntList.AddTail(points[3]);
                        polys[2]->pntList.AddTail(points[7]);
                        sprintf(polys[2]->name, "%s.2", polys[0]->name);
                        
                        // right
                        polys[3]->pntList.AddTail(points[1]);
                        polys[3]->pntList.AddTail(points[5]);
                        polys[3]->pntList.AddTail(points[6]);
                        polys[3]->pntList.AddTail(points[2]);
                        sprintf(polys[3]->name, "%s.3", polys[0]->name);

                        // bottom
                        polys[4]->pntList.AddTail(points[4]);
                        polys[4]->pntList.AddTail(points[5]);
                        polys[4]->pntList.AddTail(points[1]);
                        polys[4]->pntList.AddTail(points[0]);
                        sprintf(polys[4]->name, "%s.4", polys[0]->name);

                        // top
                        polys[5]->pntList.AddTail(points[3]);
                        polys[5]->pntList.AddTail(points[2]);   
                        polys[5]->pntList.AddTail(points[6]);
                        polys[5]->pntList.AddTail(points[7]);
                        sprintf(polys[5]->name, "%s.5", polys[0]->name);

                        char tmp[256];
                        sprintf(tmp, "%s.0", polys[0]->name);
                        strcpy(polys[0]->name, tmp);

                        plyList.AddTail(polys[1]);
                        plyList.AddTail(polys[2]);
                        plyList.AddTail(polys[3]);
                        plyList.AddTail(polys[4]);
                        plyList.AddTail(polys[5]);

                        SVMBox *box = new SVMBox();
                        box->polys[0] = polys[0];
                        box->polys[1] = polys[1];
                        box->polys[2] = polys[2];
                        box->polys[3] = polys[3];
                        box->polys[4] = polys[4];
                        box->polys[5] = polys[5];

                        boxList.AddTail(box);

                        curSweep.poly = NULL;
                        curSweep.pStart = NULL;
                    
                        redraw();
                    }
                }
			}
		}

		mouseX = x;
		mouseY = y;

		return 1;
	}
	else if (c==FL_MOVE)
	{
        // printf("move...\n");
        // fflush(stdout);

        int x = Fl::event_x();
		int y = Fl::event_y();

		const int oldLinesNumber = lines.size();
		lines.clear();


		if (imgBuf)
		{
			unsigned char pixel[3] = {212,208,200};
			
			double imgX=(x+0.5-w()/2.0)/scale2D-imgLeft;
			double imgY=(h()-(y+0.5)-h()/2.0)/scale2D-imgBottom;
			int iX=(int)imgX;
			int iY=(int)imgY;

			if (0<=iX && iX<imgWidth && 0<=iY && iY<imgHeight)
			{
				memcpy(pixel,imgBuf+3*(iY*imgWidth+iX),3*sizeof(unsigned char));
			}		

			char info[256];
			sprintf(info,"img coord: x = %lf y = %lf, img color: r = %d g = %d b = %d",
				imgX,imgY,(int)pixel[0],(int)pixel[1],(int)pixel[2]);
			svmui->mouseInfo->value(info);		

			if (editMode==EDIT_POINT)
			{
				SVMPoint *tmpSel = SearchNearbyPoint(imgX,imgY);
				if (tmpSel&&selPnt!=tmpSel)
				{
					selPnt = tmpSel;
					redraw();
				}
				else if( showGuideLines )
				{
					const static double minDist = 10;
					// See if current position lies near any line.
					for( std::vector<SVMPoint*>::iterator firstIt = intersectionPoints.begin();
						firstIt != intersectionPoints.end();
						++firstIt )
					{
						for( std::vector<SVMPoint*>::iterator secondIt = intersectionPoints.begin();
							secondIt != intersectionPoints.end();
							++secondIt )
						{
							if( firstIt != secondIt )
							{
								std::pair< SVMPoint*, SVMPoint* > line = 
									(*firstIt < *secondIt) ? std::make_pair( *firstIt, *secondIt ) :
										std::make_pair( *secondIt, *firstIt );

								const double d = distance( imgX, imgY, line );

								if( d >= 0 && d < minDist && lines.end() == std::find( lines.begin(), lines.end(), line ) )
									lines.push_back( line );
								//printf( "distance: %f\n", d );
							}
						}
					}

					// Want them sorted in order of increasing distance.
					std::sort( lines.begin(), lines.end(), LineComparator(imgX, imgY) );

					if( oldLinesNumber != lines.size() || lines.size() > 0 )
					{
						//printf( "lines count: %d, intersection points: %d\n", lines.size(), intersectionPoints.size() );
						redraw();
					}
				}

			}
			else if (editMode==EDIT_XLINE||editMode==EDIT_YLINE||editMode==EDIT_ZLINE||editMode==EDIT_OLINE)				
			{
				if (curLine.pnt1)
				{
					redraw();
				}
				else
				{
					SVMLine *tmpSel = SearchNearbyLine(imgX,imgY);
					if (tmpSel&&selLine!=tmpSel)
					{
						selLine = tmpSel;
						redraw();
					}
				}
			}
			else if (editMode==EDIT_POLYGON)
			{
				if (curPly.pntList.GetCount())
				{
					redraw();
				}
				else
				{
					SVMPolygon *tmpSel = SearchNearbyPolygon(imgX,imgY);
					if (tmpSel&&selPly!=tmpSel)
					{
						selPly = tmpSel;
						redraw();
					}
				}
            } else if (editMode==EDIT_YZRECT||editMode==EDIT_XZRECT||editMode==EDIT_XYRECT) {
				if (curRect.pnt1)
				{
					redraw();
				}
            } else if (editMode==EDIT_SWEEP) {
                if (curSweep.poly)
                {
                    redraw();
                }
            }
		}

		mouseX = x;
		mouseY = y;

		return 1;
	}	
	else if (c==FL_DRAG)
	{
        // printf("drag...\n");
        // fflush(stdout);

        int x = Fl::event_x();
		int y = Fl::event_y();

		if (imgBuf)
		{
			if (Fl::event_state(FL_BUTTON3))
			{
				imgLeft += (x - mouseX)/scale2D;
				imgBottom += (mouseY - y)/scale2D;			
				
				redraw();
			}
		}

		mouseX = x;
		mouseY = y;

		return 1;
	}
	else if (c==FL_RELEASE)
	{
        // printf("release...\n");
        // fflush(stdout);

        int x = Fl::event_x();
		int y = Fl::event_y();

		mouseX = x;
		mouseY = y;

		return 1;
	}
	else if (c==FL_SHORTCUT || c==FL_KEYUP)	
	{
        // printf("shortcut or keyup...\n");
        // printf(" key: %s\n", Fl::event_text());
        // printf(" key: %d\n", Fl::event_key());
        // fflush(stdout);

        int x = Fl::event_x();
		int y = Fl::event_y();

        // printf("key pressed...\n");

		const double s = 1.5;
		if (Fl::event_key() == '=' && Fl::event_state(FL_CTRL))
		{
			if (scale2D < 16.0)
			{
				scale2D *= s;				
				redraw();
			}
			return 1;
		}
		else if (Fl::event_key() == '-' && Fl::event_state(FL_CTRL))
		{
			if (scale2D > 1.0/16.0)
			{
				scale2D /= s;
				redraw();
			}
			return 1;
		}
		else if (Fl::event_key() == FL_Enter)
		// to finish creating a polygon;
		{
			if (imgBuf)
			{
				if (editMode == EDIT_POLYGON)
				{
					selPly = new SVMPolygon;

					CTypedPtrDblElement <SVMPoint> *node = curPly.pntList.GetHeadPtr();
					
					while (!curPly.pntList.IsSentinel(node))
					{
						SVMPoint *pnt = node->Data();
						if (!CheckExistingPoint(pnt))
						{
							pntList.AddTail(pnt);
						}
						selPly->pntList.AddTail(pnt);

						node=node->Next();
					}

					ComputePointAverage2D(selPly->cntx,selPly->cnty,&(selPly->pntList));

					//the value here is for initialization, not the final answer;
					//the final answer is dependent on the final reconstruction. 

					if( selPly->pntList.GetCount() != 4 )
					{
						selPly->H[0][0] = imgWidth;
						selPly->H[0][1] = 0;
						selPly->H[0][2] = 0;
						selPly->H[1][0] = 0;
						selPly->H[1][1] = imgHeight;
						selPly->H[1][2] = 0;
						selPly->H[2][0] = 0;
						selPly->H[2][1] = 0;
						selPly->H[2][2] = 1;

						selPly->invH = selPly->H.Inverse();
					}
					else
					{
						CTypedPtrDblElement<SVMPoint> *selNode = selPly->pntList.GetHeadPtr();
						vector<SVMPoint> points;
						for( int i=0; i < 4; i++ )
						{
							points.push_back(*selNode->Data());
/*							printf("p%d = [ %f %f %f ] -> [ %f %f %f ]\n", 
								*selNode->Data()->u, *selNode->Data()->v, *selNode->Data()->w,  
								*selNode->Data()->X, *selNode->Data()->Y, */
							selNode = selNode->Next();
						}

//						MakeHomography(selPly->invH, selPly->H, points);
					}

					const char *name = fl_input("please give a name to the polygon, which will be used as texture filename later.");

					if (name)
					{
						strcpy(selPly->name,name);
					}

					plyList.AddTail(selPly);

					curPly.pntList.RemoveAll();

					redraw();
				}
				// added for winter 2003
				// enter 3D coordinate for reference point
				else if (editMode == EDIT_POINT)
				{
					if (selPnt)
					{
						curRefPnt = selPnt;

							// pop up dialogue window
							char float2str[40];
						sprintf(float2str, "%f", curRefPnt->X);
						setRefPnt_X->value(float2str);
						sprintf(float2str, "%f", curRefPnt->Y);
						setRefPnt_Y->value(float2str);
						sprintf(float2str, "%f", curRefPnt->Z);
						setRefPnt_Z->value(float2str);

						if (refPointOffPlane == curRefPnt)
							setRefPnt_UseHeight->value(1);
						else
							setRefPnt_UseHeight->value(0);

						int xx, yy;
						xx = (int)(svmui->mainWindow->x() + this->x() + w()/2 + (curRefPnt->u/curRefPnt->w - imgWidth/2)*scale2D);
						yy = (int)(svmui->mainWindow->y() + this->y() + h()/2 - (curRefPnt->v/curRefPnt->w - imgHeight/2)*scale2D);
						setRefPnt_Window->resize(xx+40, yy+40, 300, 160);

						setRefPnt_Window->show();
					}
				}
			}
			return 1;
		}
		else if (Fl::event_key() == FL_BackSpace)
		{

            // printf("backspace hit\n");
            // fflush(stdout);

			if (imgBuf)
			{
				/*
				pntSelStack.clear(); // EUGENE_ADD -- must clear out stack in case we delete a point on it

				intersectionPoints.clear();
				if( xVanish.known ) intersectionPoints.push_back( &xVanish );
				if( yVanish.known ) intersectionPoints.push_back( &yVanish );
				if( zVanish.known ) intersectionPoints.push_back( &zVanish );
				*/

				if (editMode==EDIT_POINT)
				{
					if (selPnt)
					{
						if (CheckExistingLine(selPnt)||CheckExistingPolygon(selPnt))
						{
							fl_message("Can not delete point being used by other lines or polygons!");
						}
						else
						{
							if (refPointOffPlane == selPnt)
							{
								refPointOffPlane = NULL;
								referenceHeight = 0;
							}

							// delete selected point from points list
							pntList.Remove(selPnt);

							// delete selected point from stack and guideline points list
							vector<SVMPoint*>::iterator iter;

							if (intersectionPoints.size() > 0)
							{
								for (iter = intersectionPoints.begin(); iter != intersectionPoints.end() && *iter != selPnt; iter++);
								if (iter != intersectionPoints.end())
									intersectionPoints.erase(iter);
							}

							if (pntSelStack.size() > 0)
							{
								for (iter = pntSelStack.begin(); iter != pntSelStack.end() && *iter != selPnt; iter++);
								if (iter != pntSelStack.end())
									pntSelStack.erase(iter);
							}

							redraw();
						}
					}
				}
				else if (editMode==EDIT_XLINE||editMode==EDIT_YLINE||editMode==EDIT_ZLINE||editMode==EDIT_OLINE)
				{
					if (curLine.pnt1)
					{
						if (!CheckExistingPoint(curLine.pnt1))
						{
							delete curLine.pnt1;						
						}
						curLine.pnt1 = NULL;
						redraw();
					}
					else if (selLine)
					{
						lineList.Remove(selLine);
						selLine = NULL;
						redraw();
					}					
				}
				else if (editMode==EDIT_POLYGON)
				{
					if (curPly.pntList.GetCount())
					{
						SVMPoint *pnt = curPly.pntList.GetTailPtr()->Data();
						if (!CheckExistingPoint(pnt))
						{
							delete pnt;
						}
						curPly.pntList.RemoveTail();
						redraw();
					}
					else if (selPly)
					{
						plyList.Remove(selPly);
						selPly = NULL;
						redraw();
					}
				}
			}
			return 1;
		}
		// EUGENE ADD
		else if (Fl::event_key() == ' ')
		{
			int x = Fl::event_x();
			int y = Fl::event_y();

			double imgX=(x+0.5-w()/2.0)/scale2D-imgLeft;
			double imgY=(h()-(y+0.5)-h()/2.0)/scale2D-imgBottom;

			SVMPoint *tmpSel = SearchNearbyPoint(imgX,imgY);


			if (tmpSel)
			{
//				selPnt->known = !selPnt->known;
				if (pntSelStack.end() == std::find(pntSelStack.begin(), pntSelStack.end(), tmpSel))
				{
					pntSelStack.push_back(tmpSel);
					intersectionPoints.push_back(tmpSel);

					printf("pushed point %x onto stack, current size %d\n", selPnt, pntSelStack.size());
					printf("Point information: \n    (u, v, w) = (%f, %f, %f);\n    (X, Y, Z, W) = (%f, %f, %f, %f);\n\n",
						tmpSel->u, tmpSel->v, tmpSel->w, tmpSel->X, tmpSel->Y, tmpSel->Z, tmpSel->W);
				}
				else
				{
					printf("already on stack!\n");
				}
			}
			else
			{
				if (!pntSelStack.empty())
				{
					printf("popping!\n");
					pntSelStack.pop_back();
					intersectionPoints.pop_back();
				}
			}
			redraw();
			return 1;

		}
		else
			return Fl_Gl_Window::handle(c);
	}
	else
	{
        // printf("c = %d...\n", c);
        // fflush(stdout);

        return Fl_Gl_Window::handle(c);
	}
}

int ImgView::handle3D(int c)
{
	if (c==FL_PUSH)
	{
		int x = Fl::event_x();
		int y = Fl::event_y();

		mouseX=x;
		mouseY=y;
		
		return 1;
	}
	else if (c==FL_MOVE)
	{
		int x = Fl::event_x();
		int y = Fl::event_y();

		mouseX=x;
		mouseY=y;
		
		return 1;
	}
	else if (c==FL_DRAG)
	{
		int x = Fl::event_x();
		int y = Fl::event_y();

		if (imgBuf)
		{			
			if (Fl::event_state(FL_BUTTON1))
			{				
				double rate = 0.2;
				
				if (Fl::event_state(FL_CTRL))
				//rotate around x,y;
				{
					rotX3D += rate*(y-mouseY);
					rotY3D += rate*(x-mouseX);
				}
				else if (Fl::event_state(FL_SHIFT))
				//rotate around z;
				{
					rotZ3D += rate*(mouseX-x);
				}
				else if (Fl::event_state(FL_ALT))
				//translate along z;
				{
					traZ3D += y-mouseY;
				}
				else
				//translate along x,y;
				{
					traX3D += x-mouseX;
					traY3D += mouseY-y;
				}

				redraw();
			}
		}

		mouseX=x;
		mouseY=y;
		
		return 1;	
	}
	else if (c==FL_RELEASE)
	{
		int x = Fl::event_x();
		int y = Fl::event_y();

		mouseX=x;
		mouseY=y;
		
		return 1;
	}
	else if (c==FL_SHORTCUT)	
	{
		int x = Fl::event_x();
		int y = Fl::event_y();

		const double s = 1.5;

		if (Fl::event_key('=') && Fl::event_state(FL_CTRL))
		{
			scale3D *= s;				
			redraw();

			return 1;
		}
		else if (Fl::event_key('-') && Fl::event_state(FL_CTRL))
		{
			scale3D /= s;
			redraw();

			return 1;
		}
		else
		{
			return Fl_Gl_Window::handle(c);			
		}
	}
	else 
	{
		return Fl_Gl_Window::handle(c);		
	}	
}
