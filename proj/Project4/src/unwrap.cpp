/* unwrap.cpp */
#pragma warning(disable : 4996)

#include "ImgView.h"
#include "svm.h"
#include "svmmath.h"
#include "ImageLib/Transform.h"
#include "ImageLib/WarpImage.h"

#include <vector>
using namespace std;

#ifndef M_PI 
#define M_PI    3.1415926536
#endif // M_PI

// unwrap boxes
void ImgView::unwrapBoxes()
{
    if (sceneInverted)
        unwrapBoxesInverted();
    else
        unwrapBoxesNormal();
}


void ImgView::unwrapBoxesNormal()
{
    if (boxList.GetCount() == 0) {
        fl_message("No boxes have been created\n");
        return;
    }

    CTypedPtrDblElement <SVMBox> *boxNode = boxList.GetHeadPtr();    

    while (!boxList.IsSentinel(boxNode)) {
        SVMBox *box = boxNode->Data();

        // lay out the box like this:
        //       5
        //    2  0  3  1
        //       4
        // 

        // measure the length in pixels of the first side of the first polygon
        CTypedPtrDblElement <SVMPoint> *head = box->polys[0]->pntList.GetHeadPtr();
        CTypedPtrDblElement <SVMPoint> *next = head->Next();

        SVMPoint *p0 = head->Data();
        SVMPoint *p1 = next->Data();

        double du = p1->u - p0->u;
        double dv = p1->v - p0->v;

        double dx = p1->X - p0->X;
        double dy = p1->Y - p0->Y;
        double dz = p1->Z - p0->Z;

        double dist2D = sqrt(du * du + dv * dv);
        double dist3D = sqrt(dx * dx + dy * dy + dz * dz);

        // defines how to convert a 3D distance to a 2D distance in the unwrapped box
        double ratio = dist2D / dist3D;

        // Create Rick's image
        CByteImage rick(imgWidth, imgHeight, 3);

	    //Rick's .tga file format is in G,B,R order from bottom to top and from left to right;
	    //our buffer is in R, G, B order from top to bottom and from left to right;
	    for (int j=0;j<imgHeight;j++)
	    {
		    for (int i=0;i<imgWidth;i++)
		    {
			    int index = 3*(j*imgWidth+i);
			    int x = i;
			    int y = j;
			    rick.Pixel(x,y,2) = imgBuf[index+0];
			    rick.Pixel(x,y,1) = imgBuf[index+1];
			    rick.Pixel(x,y,0) = imgBuf[index+2];
		    }
	    }

        // compute the size of each face in the output image
        int w_out[6], h_out[6];

        for (int i = 0; i < 6; i++) {
            SVMPolygon *ply = box->polys[i];
            SVMPoint *p0, *p1, *p2, *p3;
            ply->getFourPoints(&p0, &p1, &p2, &p3);
            
            double dx1 = p1->X - p0->X;
            double dy1 = p1->Y - p0->Y;
            double dz1 = p1->Z - p0->Z;
            double dist1 = sqrt(dx1 * dx1 + dy1 * dy1 + dz1 * dz1);

            double dx2 = p3->X - p0->X;
            double dy2 = p3->Y - p0->Y;
            double dz2 = p3->Z - p0->Z;
            double dist2 = sqrt(dx2 * dx2 + dy2 * dy2 + dz2 * dz2);
        
            w_out[i] = (int) ceil(dist1 * ratio);
            h_out[i] = (int) ceil(dist2 * ratio);
        }

        int w = w_out[2] + w_out[0] + w_out[3] + w_out[1];
        int h = h_out[5] + h_out[0] + h_out[4];

        // compute the start position of each face
        int x_start[6];
        int y_start[6];

        x_start[0] = w_out[2];
        x_start[1] = w_out[2] + w_out[0] + w_out[3];
        x_start[2] = 0;
        x_start[3] = w_out[2] + w_out[0];
        x_start[4] = w_out[2];
        x_start[5] = w_out[2];

        y_start[0] = h_out[4];
        y_start[1] = h_out[4];
        y_start[2] = h_out[4];
        y_start[3] = h_out[4];
        y_start[4] = 0;
        y_start[5] = h_out[4] + h_out[0];

        CByteImage outputImg(w, h, 3);

        // compute the transform for each face
        for (int i = 0; i < 6; i++) {
            SVMPolygon *ply = box->polys[i];
            
            CTypedPtrDblElement<SVMPoint> *selNode = ply->pntList.GetHeadPtr();
		    vector<SVMPoint> points;

		    while (!ply->pntList.IsSentinel(selNode))
		    {
			    points.push_back(*selNode->Data());
			    selNode = selNode->Next();
		    }
    		
            vector<Vec3d> basisPts;
		    ComputeHomography(ply->invH, ply->H, points, basisPts, false);
        
	        CTransform3x3 tx;
	        tx[0][0] = ply->invH[0][0]; tx[0][1] = ply->invH[0][1]; tx[0][2] = ply->invH[0][2];
	        tx[1][0] = ply->invH[1][0]; tx[1][1] = ply->invH[1][1]; tx[1][2] = ply->invH[1][2];
	        tx[2][0] = ply->invH[2][0]; tx[2][1] = ply->invH[2][1]; tx[2][2] = ply->invH[2][2];        

            CTransform3x3 scale;
            scale[0][0] = 1.0 / w_out[i];
            scale[1][1] = 1.0 / h_out[i];

            tx = tx * scale;

            // get the subimage for this face
            CByteImage sub = outputImg.SubImage(x_start[i], y_start[i], w_out[i], h_out[i]);
    	    WarpGlobal(rick, sub, tx, eWarpInterpLinear);    
        }

        char texname[256];
        char *dot = strrchr((char *) box->polys[0]->name, '.');
        *dot = 0;
        sprintf(texname, "%s.tga", box->polys[0]->name);
        *dot = '.';

        printf("Writing %s...\n", texname);
        WriteFile(outputImg, texname);
    
        boxNode = boxNode->Next();
    }
}

static double getRotation(Vec3d a, Vec3d b, Vec3d c, Vec3d d)
{
    Vec3d ba = b - a;
    Vec3d dc = d - c;

    ba.normalize();
    dc.normalize();

    double dot = ba * dc;
    Vec3d e = cross(ba, dc);

    double angle = 180 * acos(dot) / M_PI;

    if (e[2] < 0.0)
        return angle;
    else
        return -angle;
}

void ConvertToPlaneCoordinate(SVMPolygon *ply, Vec3d &p1, Vec3d &p2, Vec3d &p3, Vec3d &p4) 
{
    SVMPoint *a, *b, *c, *d;
    ply->getFourPoints(&a, &b, &c, &d);

    std::vector<SVMPoint> points;
    points.push_back(*a);
    points.push_back(*b);
    points.push_back(*c);
    points.push_back(*d);

    std::vector<Vec3d> basisPts;
    double uScale, vScale;

    // Compute the basis points for the vertices
    ConvertToPlaneCoordinate(points, basisPts, uScale, vScale);

    p1 = basisPts[0];
    p2 = basisPts[1];
    p3 = basisPts[2];
    p4 = basisPts[3];

    p1[0] *= uScale;
    p1[1] *= vScale;

    p2[0] *= uScale;
    p2[1] *= vScale;

    p3[0] *= uScale;
    p3[1] *= vScale;

    p4[0] *= uScale;
    p4[1] *= vScale;
}

static bool PointInsideQuad(int x, int y, Vec3d quad[4])
{
    // assume quad is given in ccw order and is convex
    int sign[4];

    for (int i = 0; i < 4; i++) {
        int next = (i+1) % 4;
        double dx = x - quad[i][0];
        double dy = y - quad[i][1];

        double dx_n = quad[next][0] - quad[i][0];
        double dy_n = quad[next][1] - quad[i][1];

        double cross = dy * dx_n - dx * dy_n;

        sign[i] = (cross < 0) ? -1 : 1;
    }

    return (sign[0] == sign[1] && sign[0] == sign[2] && sign[0] == sign[3]);
}

CTransform3x3 genRotation(double degrees)
{
    CTransform3x3 M;

    double rad    = degrees * M_PI / 180.0;
    double c      = (float) cos(rad);
    double s      = (float) sin(rad);

    M[0][0] = c, M[0][1] = -s;
    M[1][0] = s, M[1][1] = c;

    return M;
}

// unwrap an inverted model
void ImgView::unwrapBoxesInverted()
{
    if (boxList.GetCount() == 0) {
        fl_message("No boxes have been created\n");
        return;
    }

    if (!camComputed) {
        fl_message("Please find the camera position first\n");
        return;
    }

    CTypedPtrDblElement <SVMBox> *boxNode = boxList.GetHeadPtr();    

    while (!boxList.IsSentinel(boxNode)) {
        SVMBox *box = boxNode->Data();

        // lay out the box like this:
        //       5
        //    2  0  3  1
        //       4
        // 

        SVMPoint *p0, *p1, *p2, *p3;
        box->polys[0]->getFourPoints(&p0, &p1, &p2, &p3);

        // measure the length in pixels of the first side of the first polygon
        double du = p1->u - p0->u;
        double dv = p1->v - p0->v;

        double dx = p1->X - p0->X;
        double dy = p1->Y - p0->Y;
        double dz = p1->Z - p0->Z;

        double dist2D = sqrt(du * du + dv * dv);
        double dist3D = sqrt(dx * dx + dy * dy + dz * dz);

        // defines how to convert a 3D distance to a 2D distance in the unwrapped box
        double ratio = dist2D / dist3D;

        // Create Rick's image
        CByteImage rick(imgWidth, imgHeight, 4);

	    //Rick's .tga file format is in G,B,R order from bottom to top and from left to right;
	    //our buffer is in R, G, B order from top to bottom and from left to right;
	    for (int j=0;j<imgHeight;j++)
	    {
		    for (int i=0;i<imgWidth;i++)
		    {
			    int index = 3*(j*imgWidth+i);
			    int x = i;
			    int y = j;
			    rick.Pixel(x,y,2) = imgBuf[index+0];
			    rick.Pixel(x,y,1) = imgBuf[index+1];
			    rick.Pixel(x,y,0) = imgBuf[index+2];
		        rick.Pixel(x,y,3) = 255;
            }
	    }

        // lay out the box like this:
        //       5
        //    2  0  3  1
        //       4
        // 
        // compute the positions of the four corners of each face in the output image (14 corners in total)
        //
        // 0 -> 0
        // 1 -> 1
        // 2 -> 2
        // 3 -> 3
        // 4 -> 4
        // 5 -> 7
        // 6 -> 7
        // 7 -> 6
        // 8 -> 4
        // 9 -> 5
        // 10 -> 5
        // 11 -> 6
        // 12 -> 7
        // 13 -> 4

        Vec3d pnts[14];
        CTransform3x3 X[6];

        Vec3d c0, c1, c2, c3;
        ConvertToPlaneCoordinate(box->polys[0], c0, c1, c2, c3);

        // place the first four points 
        pnts[0] = ratio * c0;
        pnts[1] = ratio * c1;
        pnts[2] = ratio * c2;
        pnts[3] = ratio * c3;

        // get the four points for polygon 2
        ConvertToPlaneCoordinate(box->polys[2], c0, c1, c2, c3);

        // c1 -> pnt[0], c2 -> pnt[3]
        double angle = getRotation(pnts[0], pnts[3], c1, c2);
        CTransform3x3 T1, R, T2, S;
        S[0][0] = ratio;  S[1][1] = ratio;
        T2 = CTransform3x3::Translation((float) -c1[0], (float) -c1[1]);
        R = genRotation((float) angle);
        T1 = CTransform3x3::Translation((float) pnts[0][0], (float) pnts[0][1]);

        CTransform3x3 M = T1 * R * S * T2;
        X[2] = M;
        Mat3d M2 = Mat3d(M[0][0], M[0][1], M[0][2], 
                         M[1][0], M[1][1], M[1][2],
                         M[2][0], M[2][1], M[2][2]);

        Vec3d test1 = M2 * c1;
        Vec3d test2 = M2 * c2;

        printf("c1 = %0.3f, %0.3f;  pnts[0] = %0.3f, %0.3f\n", test1[0], test1[1], pnts[0][0], pnts[0][1]);
        printf("c2 = %0.3f, %0.3f;  pnts[3] = %0.3f, %0.3f\n", test2[0], test2[1], pnts[3][0], pnts[3][1]);

        pnts[4] = M2 * c0;
        pnts[5] = M2 * c3;


        // get the four points for polygon 5
        ConvertToPlaneCoordinate(box->polys[5], c0, c1, c2, c3);

        // c0 -> pnt[3], c1->pnt[2]
        angle = getRotation(pnts[3], pnts[2], c0, c1);
        T2 = CTransform3x3::Translation((float) -c0[0], (float) -c0[1]);
        R = genRotation((float) angle);
        T1 = CTransform3x3::Translation((float) pnts[3][0], (float) pnts[3][1]);

        M = T1 * S * R * T2;
        X[5] = M;
        M2 = Mat3d(M[0][0], M[0][1], M[0][2], 
                   M[1][0], M[1][1], M[1][2],
                   M[2][0], M[2][1], M[2][2]);
        
        pnts[6] = M2 * c3;
        pnts[7] = M2 * c2;


        // get the four points for polygon 4
        ConvertToPlaneCoordinate(box->polys[4], c0, c1, c2, c3);

        // c3 -> pnt[0], c2->pnt[1]
        angle = getRotation(pnts[0], pnts[1], c3, c2);
        T2 = CTransform3x3::Translation((float) -c3[0], (float) -c3[1]);
        R = genRotation((float) angle);
        T1 = CTransform3x3::Translation((float) pnts[0][0], (float) pnts[0][1]);

        M = T1 * S * R * T2;
        X[4] = M;
        M2 = Mat3d(M[0][0], M[0][1], M[0][2], 
                   M[1][0], M[1][1], M[1][2],
                   M[2][0], M[2][1], M[2][2]);

        pnts[8] = M2 * c0;
        pnts[9] = M2 * c1;


        // get the four points for polygon 3
        ConvertToPlaneCoordinate(box->polys[3], c0, c1, c2, c3);

        // c0 -> pnt[1], c3->pnt[2]
        angle = getRotation(pnts[1], pnts[2], c0, c3);
        T2 = CTransform3x3::Translation((float) -c0[0], (float) -c0[1]);
        R = genRotation((float) angle);
        T1 = CTransform3x3::Translation((float) pnts[1][0], (float) pnts[1][1]);

        M = T1 * S * R * T2;
        X[3] = M;
        M2 = Mat3d(M[0][0], M[0][1], M[0][2], 
                   M[1][0], M[1][1], M[1][2],
                   M[2][0], M[2][1], M[2][2]);

        pnts[10] = M2 * c1;
        pnts[11] = M2 * c2;


        // get the four points for polygon 1
        ConvertToPlaneCoordinate(box->polys[1], c0, c1, c2, c3);

        // c0 -> pnt[10], c3->pnt[11]
        angle = getRotation(pnts[10], pnts[11], c0, c3);
        T2 = CTransform3x3::Translation((float) -c0[0], (float) -c0[1]);
        R = genRotation((float) angle);
        T1 = CTransform3x3::Translation((float) pnts[10][0], (float) pnts[10][1]);

        M = T1 * S * R * T2;
        X[1] = M;
        M2 = Mat3d(M[0][0], M[0][1], M[0][2], 
                   M[1][0], M[1][1], M[1][2],
                   M[2][0], M[2][1], M[2][2]);

        pnts[12] = M2 * c2;
        pnts[13] = M2 * c1;


        // Compute the size of the output image
        double xmin = pnts[0][0], ymin = pnts[0][1], xmax = pnts[0][0], ymax = pnts[0][1];
        for (int i = 1; i < 14; i++) {
            if (i == 12 || i == 13)
                continue; // skip the back face

            xmin = min(xmin, pnts[i][0]);
            ymin = min(ymin, pnts[i][1]);
            xmax = max(xmax, pnts[i][0]);
            ymax = max(ymax, pnts[i][1]);
        }

        int w_out = (int) ceil(xmax - xmin);
        int h_out = (int) ceil(ymax - ymin);

        // translate the entire image by (-xmin, -ymin)
        CTransform3x3 Tglobal = CTransform3x3::Translation((float) -xmin, (float) -ymin);
        for (int i = 0; i < 14; i++) {
            pnts[i][0] -= xmin;
            pnts[i][1] -= ymin;
        }

        // compute homographies for each image
        int indices[6][4] = 
        { 
            { 0, 1, 2, 3 },
            { 10, 13, 12, 11 },
            { 4, 0, 3, 5 },
            { 1, 10, 11, 2 },
            { 8, 9, 1, 0 },
            { 3, 2, 7, 6 }
        };

        CByteImage outputImg(w_out, h_out, 4);

        for (int i = 0; i < 6; i++) {
            if (i == 1)
                continue; // skip the back face

            CTransform3x3 Htmp, Hinvtmp;
            SVMPoint *a, *b, *c, *d;
            box->polys[i]->getFourPoints(&a, &b, &c, &d);
            std::vector<SVMPoint> points;
            points.push_back(*a);
            points.push_back(*b);
            points.push_back(*c);
            points.push_back(*d);

            std::vector<Vec3d> basisPts;
            ComputeHomography(Htmp, Hinvtmp, points, basisPts, false);
            double uScale, vScale;
            ConvertToPlaneCoordinate(points, basisPts, uScale, vScale);

            // Hp2i is the transformation from the normalized plane to the input image
            CTransform3x3 Hp2i;
            memcpy(Hp2i[0], Htmp[0], 3 * sizeof(double));
            memcpy(Hp2i[1], Htmp[1], 3 * sizeof(double));
            memcpy(Hp2i[2], Htmp[2], 3 * sizeof(double));

            // compute homgraphy to output image
            Vec3d p[4];
            for (int j = 0; j < 4; j++) {
                p[j][0] = points[j].u = pnts[indices[i][j]][0];
                p[j][1] = points[j].v = pnts[indices[i][j]][1];
                p[j][2] = points[j].w = 1.0;
            }
            ComputeHomography(Htmp, Hinvtmp, points, basisPts, false);
        
            CTransform3x3 Ho2p;
            // memcpy(H[i][0], Htmp[0], 3 * sizeof(double));
            // memcpy(H[i][1], Htmp[1], 3 * sizeof(double));
            // memcpy(H[i][2], Htmp[2], 3 * sizeof(double));

            // Ho2p is a homography from the output image to the normalized plane
            memcpy(Ho2p[0], Hinvtmp[0], 3 * sizeof(double));
            memcpy(Ho2p[1], Hinvtmp[1], 3 * sizeof(double));
            memcpy(Ho2p[2], Hinvtmp[2], 3 * sizeof(double));

            CTransform3x3 S;
            S[0][0] = 1.0 / uScale;
            S[1][1] = 1.0 / vScale;

            // compose Ho2p and Hp2i
            CTransform3x3 H = Hp2i * Ho2p; // * X[i].Inverse(); // * Tglobal; // .Inverse();

            // set the alpha of the output image
            for (int y = 0; y < h_out; y++) {
                for (int x = 0; x < w_out; x++) {
                    // test if this pixel is inside the polygon, and set alpha accordingly
                    if (PointInsideQuad(x, y, p)) {
                        outputImg.Pixel(x, y, 3) = 255;
                    } else {
                        // outputImg.Pixel(x, y, 2) = 255;
                        outputImg.Pixel(x, y, 3) = 0;
                    }
                }
            }

    	    WarpGlobal(rick, outputImg, H, eWarpInterpLinear);
        }

        char texname[256];
        char *dot = strrchr((char *) box->polys[0]->name, '.');
        *dot = 0;
        sprintf(texname, "%s.inv.tga", box->polys[0]->name);
        *dot = '.';

        printf("Writing %s...\n", texname);
        WriteFile(outputImg, texname);

        boxNode = boxNode->Next();
    }
}
