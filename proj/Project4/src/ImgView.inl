/***************************************************************
 * CS4670/5670, Fall 2012 Project 4
 * File to be modified #2:
 * ImgView.inl (included from ImgView.cpp)
 *		contains routines for computing the 3D position of points
 ***************************************************************/

/**
 * dehomogenize():
 *     INPUT:
 *          v: 3d vector of doubles
 *     
 *     Helper function to dehomogenize Vec3d's by scaling v so that w = 1
 */
void dehomogenize(Vec3d& v){
    v[0] = v[0] / v[2];
    v[1] = v[1] / v[2];
    v[2] = 1;
}

/**
 * sameXY():
 *     Computes the 3D position of newPoint using knownPoint that has the same X and Y
 *     coordinate, i.e. is directly below or above newPoint.
 *     
 *     o newPoint<unknown>      z
 *     |                        *   x
 *     | /                      |  *
 *     |/knownPoint<known>      | /
 *     o------                  |/____*y   
 *
 *     see http://courses.cs.washington.edu/courses/cse455/12wi/lectures/projective.pdf 
 *     (slide 34)
 */	
void ImgView::sameXY()
{   
    // Check that there are enough points on stack for calculation
    if (pntSelStack.size() < 2)
    {
        fl_alert("Not enough points on the stack.");
        return;
    }

    // newPoint (unknown point) should be last on stack
    SVMPoint &newPoint = *pntSelStack[pntSelStack.size() - 1];
    
    // knownPoint should be second to last on stack
    SVMPoint &knownPoint = *pntSelStack[pntSelStack.size() - 2];

    // Check that knownPoint is actually known
    if( !knownPoint.known() )
    {
        fl_alert("Can't compute relative values for unknown point.");
        return;
    }

    // Check that a reference height has been specified
    if( refPointOffPlane == NULL )
    {
        fl_alert("Need to specify the reference height first.");
        return;
    }

    // Project knownPoint onto image plane to get point b0
    double ub0=0., vb0=0.;
    ApplyHomography(ub0, vb0, H, knownPoint.X, knownPoint.Y, 1.);
    Vec3d vec3b0Img = Vec3d(ub0, vb0, 1);

    // Project refPoint onto image plane to get point b (bottom)
    double ub=0., vb=0.;
    ApplyHomography(ub,vb,H,refPointOffPlane->X, refPointOffPlane->Y, 1.);
    Vec3d vec3bImg = Vec3d(ub,vb,1);

    // Get line from b through b0 to vanishing line
    Vec3d b0_b_line = cross(vec3bImg, vec3b0Img);

    // Get vanishing line
    Vec3d vanishLine = cross(Vec3d(xVanish.u, xVanish.v, xVanish.w), Vec3d(yVanish.u, yVanish.v, yVanish.w));
	
    // Find point of intersection of vanishing line and b0_b_line (vanishing point)
    Vec3d vanishPt = cross(b0_b_line, vanishLine);

    // Get line though newPoint and vanishing point
    Vec3d v_new_line = cross(Vec3d(newPoint.u, newPoint.v, 1), vanishPt);
	
    // Get refPoint as Vec3d
    Vec3d r = Vec3d(refPointOffPlane->u, refPointOffPlane->v, 1);

    // Get line through b and r
    Vec3d b_ref_line = cross(vec3bImg, r);

    // Get t point (top)
    Vec3d tPt = cross(v_new_line, b_ref_line);

    // Get Z vanishing point as Vec3d
    Vec3d vZ = Vec3d(zVanish.u, zVanish.v, zVanish.w);

    // Dehomogenize appropriate vectors
    dehomogenize(tPt);
    dehomogenize(vec3bImg);
    dehomogenize(vZ);
    dehomogenize(r);
	
    // Calculate height of newPoint
    double height = 0.;
    if((Vec3d(knownPoint.X, knownPoint.Y, 1) - Vec3d(refPointOffPlane->X, refPointOffPlane->Y, 1)).length() < .05)
    {
        // Handle degenerate case where newPoint, knownPoint and refPointOffPlane are in line 
        
        // Get newPoint as Vec3d
        Vec3d npt = Vec3d(newPoint.u, newPoint.v, 1);
        
        // Get height of newPoint using cross ratio
        height = referenceHeight * (npt-vec3bImg).length() * (vZ - r).length() / ((r - vec3bImg).length() * (vZ-npt).length());
        printf("degenerate case\n");
    }
    else
    {
        // Get height of newPoint using cross ratio
        height = referenceHeight * (tPt-vec3bImg).length() * (vZ - r).length()/((r-vec3bImg).length() * (vZ-tPt).length());
    }

    // Fill in 3D coordinates of newPoint
    newPoint.X = knownPoint.X;
    newPoint.Y = knownPoint.Y;
    newPoint.Z = height;
    newPoint.W = knownPoint.W;

    // Set newPoint as known
    newPoint.known(true);

    printf( "Calculated new coordinates for point: (%e, %e, %e)\n", newPoint.X, newPoint.Y, newPoint.Z );

    redraw();
}

/**
 * sameZPlane():
 *     Computes the 3D position of newPoint using knownPoint that lies on the same plane 
 *     and whose 3D position is known. If newPoint is on the reference plane (Z==0), use 
 *     homography (this->H, or simply H) directly.
 *                                      
 *     |                                z
 *     | ------o newPoint<unknown>      *   x
 *     |/     /                         |  *
 *     o------                          | /
 *   knownPoint<known>                  |/____*y   
 *
 *     see http://courses.cs.washington.edu/courses/cse455/12wi/lectures/projective.pdf 
 *     (slide 35) 
 */	
void ImgView::sameZPlane()
{
    // Check that there are enough points on stack for calculation
    if (pntSelStack.size() < 2)
    {
        fl_alert("Not enough points on the stack.");
        return;
    }

    // newPoint (unknown point) should be last on stack
    SVMPoint &newPoint = *pntSelStack[pntSelStack.size() - 1];

    // knownPoint should be second to last on stack
    SVMPoint &knownPoint = *pntSelStack[pntSelStack.size() - 2];

    // Check that knownPoint is actually known
    if( !knownPoint.known() )
    {
        fl_alert("Can't compute relative values for unknown point.");
        return;
    }
    
    // Project knownPoint onto image plane to get point b1
    double ub1=0., vb1=0.;
    ApplyHomography(ub1, vb1, H, knownPoint.X, knownPoint.Y, 1);
    Vec3d b1Img = Vec3d(ub1, vb1, 1);
	
    // Find point b0
    Vec3d kpt = Vec3d(knownPoint.u, knownPoint.v, 1);
    Vec3d npt = Vec3d(newPoint.u, newPoint.v, 1);
    Vec3d known_new_line = cross(kpt, npt);
    Vec3d vanishLine = cross(Vec3d(xVanish.u, xVanish.v, xVanish.w), Vec3d(yVanish.u, yVanish.v, yVanish.w));
    Vec3d vanishPt = cross(known_new_line, vanishLine);
    Vec3d b1_vanishPt_line = cross(vanishPt, b1Img);
    Vec3d vz_new_line = cross(npt, Vec3d(zVanish.u, zVanish.v, zVanish.w));
    Vec3d b0 = cross(vz_new_line, b1_vanishPt_line);
	
    if(knownPoint.Z == 0){
        // Handle degenerate case where newPoint and knownPoint are on the ground plane
        double kx=0., ky=0., kz=0.;
        ApplyHomography(kx, ky, Hinv, newPoint.u, newPoint.v, 1);
        newPoint.X = kx;
        newPoint.Y = ky;
        newPoint.Z = 0;
        newPoint.W = 1;
    }
    else{
        // Find 3D coordinates of b0
        double xb0=0., yb0=0.;
        ApplyHomography(xb0, yb0, Hinv, b0[0], b0[1], b0[2]);
        // Fill in 3D coordates of newPoint
        newPoint.X = xb0;
        newPoint.Y = yb0;
        newPoint.Z = knownPoint.Z;
        newPoint.W = 1;
    }
    
    // Set newPoint as known
    newPoint.known(true);

    printf( "Calculated new coordinates for point: (%e, %e, %e)\n", newPoint.X, newPoint.Y, newPoint.Z );

    redraw();
}


