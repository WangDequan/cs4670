/***************************************************************
 * CS4670/5670, Fall 2012 Project 4
 * File to be modified #3:
 * ImgViewBox.cpp
 *		routines for finding the corners of an axis-aligned
 *      box (in 2D and in 3D)
 **************************************************************/
#pragma warning(disable : 4996)

#include "ImgView.h"

/**
 * solveForOppositeCorners():
 *     INPUT:
 *          u0: u coordinate of a point p0 defining an XZ rectangle
 *          v0: v coordinate of a point p0 defining an XZ rectangle
 *          u2: u coordinate of a point p2 defining an XZ rectangle
 *          v2: v coordinate of a point p2 defining an XZ rectangle
 *     OUTPUT:
 *          u1: u coordinate of a point p1 corresponding to XZ rectangle defined by p0 and p2
 *          v1: v coordinate of a point p1 corresponding to XZ rectangle defined by p0 and p2
 *          u3: u coordinate of a point p3 corresponding to XZ rectangle defined by p0 and p2
 *          v3: v coordinate of a point p3 corresponding to XZ rectangle defined by p0 and p2
 *
 *        p2<known>
 *       / |          z   
 *      /  |          *   x
 *     p3  p1         |  *
 *     |  /           | /
 *     | /            |/
 *     p0<known>
 */	
void ImgView::solveForOppositeCorners(double u0, double v0, double u2, double v2,
                                      double &u1, double &v1, double &u3, double &v3)
{
    /* Vanishing points must be known */
    assert(xVanish.known() && yVanish.known() && zVanish.known());    

    // Store endpoint homogeneous coordinates, where w is taken as 1, into Vec3d's
    Vec3d xV = Vec3d(xVanish.u, xVanish.v, 1);  // x vanishing point
    Vec3d zV = Vec3d(zVanish.u, zVanish.v, 1);  // z vanishing point
    Vec3d p0 = Vec3d(u0, v0, 1);                // point p0
    Vec3d p2 = Vec3d(u2, v2, 1);                // point p2

    // Compute homogeneous coordinate vectors representing lines as the cross product 
    // of two endpoints
    Vec3d x0 = cross(xV,p0);    // x vanishing point to point p0
    Vec3d x2 = cross(xV,p2);    // x vanishing point to point p2
    Vec3d z0 = cross(zV,p0);    // z vanishing point to point p0
    Vec3d z2 = cross(zV,p2);    // z vanishing point to point p2

    // Compute intersects of the lines
    Vec3d p1 = cross(x0,z2);    // point p1
    p1 /= p1[2];                // scale p1 so that w = 1
    Vec3d p3 = cross(x2,z0);    // point p3
    p3 /= p3[2];                // scale p3 so that w = 1

    // Store u, v coordinates of p1 and p3 into their corresponding variables
    u1 = p1[0];             
    v1 = p1[1];
    u3 = p3[0];
    v3 = p3[1];
}

/**
 * solveForOppositeFace():
 *     INPUT:
 *          sweep: variable containing SVMPolygon containing 2D positions of one 
 *                 rectangular face parallel to the XZ plane
 *           imgX: x position of mouse in image plane (u coordinate)
 *           imgY: y position of mouse in image plane (v coordinate)
 *     OUTPUT:
 *          p4_out: 2D position of a point p4 on a parallel face being swept out from sweep
 *          p5_out: 2D position of a point p5 on a parallel face being swept out from sweep  
 *          p6_out: 2D position of a point p6 on a parallel face being swept out from sweep
 *          p7_out: 2D position of a point p7 on a parallel face being swept out from sweep
 *
 *     NOTE: The line through points p4 and p5 will go through the mouse position
 *
 *        p6----------p2
 *       / |         / |                z
 *      /  |p5      /  |                *   x
 *     p7----------p3  p1 <known>       |  *
 *     |  /        |  /                 | /
 *     | /         | /                  |/____*y
 *     p4----------p0
 */	
void ImgView::solveForOppositeFace(SVMSweep *sweep, double imgX, double imgY,
                                   Vec3d &p4_out, Vec3d &p5_out, Vec3d &p6_out, Vec3d &p7_out)
{
    // Get corresponding polygon from sweep, make sure that it is not NULL
    SVMPolygon *poly = sweep->poly;
    if (poly == NULL)
        return;

    // Get the four existing points
    SVMPoint *n0, *n1, *n2, *n3;
    poly->getFourPoints(&n0, &n1, &n2, &n3);

    // Convert existing points into Vec3d's
    Vec3d p0(n0->u, n0->v, n0->w);
    Vec3d p1(n1->u, n1->v, n1->w);
    Vec3d p2(n2->u, n2->v, n2->w);
    Vec3d p3(n3->u, n3->v, n3->w);

    // Store mouse u and v coordinates into a Vec3d
    Vec3d pMouse(imgX, imgY, 1.0);

    // Initialize Vec3d's for box corners p4, p5, p6, p7
	Vec3d p4, p5, p6, p7;

    // Store endpoint homogeneous coordinates, where w is taken as 1, into Vec3d's
    Vec3d xV = Vec3d(xVanish.u, xVanish.v, 1);  // x vanishing point
    Vec3d yV = Vec3d(yVanish.u, yVanish.v, 1);  // y vanishing point
    Vec3d zV = Vec3d(zVanish.u, zVanish.v, 1);  // z vanishing point

    // Compute homogeneous coordinate vectors representing lines as the cross product 
    // of two endpoints, used to find p4 and p5
    Vec3d xM = cross(xV, pMouse);   // x vanishing point to mouse position
    Vec3d y0 = cross(yV, p0);       // y vanishing point to point p0
    Vec3d y1 = cross(yV, p1);       // y vanishing point to point p1

    // Compute intersects of the lines to find points p4 and p5
    p4 = cross(xM, y0);             // point p4
    p4 /= p4[2];                    // scale p4 so that w = 1
    p5 = cross(xM, y1);             // point p5
    p5 /= p5[2];                    // scale p5 so that w = 1
    
    // Compute additional homogeneous coordinate vectors representing lines as the cross 
    // product of two endpoints, used to find p6 and p7
    Vec3d z4 = cross(zV, p4);       // z vanishing point to point p4
    Vec3d z5 = cross(zV, p5);       // z vanishing point to point p5
    Vec3d y2 = cross(yV, p2);       // y vanishing point to point p2
    Vec3d y3 = cross(yV, p3);       // y vanishing point to point p3

    // Compute intersects of the lines to find points p6 and p7
    p6 = cross(z5, y2);             // point p6
    p6 /= p6[2];                    // scale p6 so that w = 1
    p7 = cross(z4, y3);             // point p7
    p7 /= p7[2];                    // scale p7 so that w = 1
    
    // Store point p4, p5, p6, p7 2D coordinates into corresponding output variables
    p4_out = p4;
    p5_out = p5;
    p6_out = p6;
    p7_out = p7;
}

/**
 * find3DPositionsBox():
 *     INPUT:
 *          points: vector of 8 SVMPoints representing the corners of a given box, only the 
 *                  3D position of points[0] is known, 2D position is known for all points
 *     
 *     Function to compute the 3D positions of the 8 corners of the box using their 2D 
 *     positions and the known 3D position of points[0]. The corresponding 3D positions 
 *     will be stored in the appropriate points[1] through points[7]
 */
void ImgView::find3DPositionsBox(SVMPoint *points[8]) 
{	
    // Find 3D coordinates for points[3]
	pntSelStack.push_back(points[0]);
	pntSelStack.push_back(points[3]);   
    sameXY();			                // points[3] is directly above points[0]

    // Find 3D coordinates for points[2]
	pntSelStack.push_back(points[2]);
	sameZPlane();		                // points[2] is in same Z plane as points[3]
	pntSelStack.pop_back();             // pop points[2]
	                                    
    // Find 3D coordinates for points[6]
    pntSelStack.push_back(points[6]);
	sameZPlane();		                // points[6] is in same Z plane as points[3]
	pntSelStack.pop_back();             // pop points[6]

    // Find 3D coordinates for points[7]
	pntSelStack.push_back(points[7]); 
	sameZPlane();		                // points[7] is in same Z plane as points[3]
	pntSelStack.pop_back();             // pop points[7]
	pntSelStack.pop_back();		        // pop points[3]

    // Find 3D coordinates for points[1]
	pntSelStack.push_back(points[1]);   
	sameZPlane();                       // points[1] is in same Z plane as points[0]
	pntSelStack.pop_back();             // pop points[1]

    // Find 3D coordinates for points[4]
	pntSelStack.push_back(points[4]);   
	sameZPlane();                       // points[4] is in same Z plane as points[0]
	pntSelStack.pop_back();             // pop points[4]

    // Find 3D coordinates for points[5]
	pntSelStack.push_back(points[5]);
	sameZPlane();                       // points[5] is in same Z plane as points[0]
    pntSelStack.pop_back();             // pop points[5]
	pntSelStack.pop_back();		        // pop points[0]
}


