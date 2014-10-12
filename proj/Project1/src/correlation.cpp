#include "correlation.h"

/*
 *	INPUT:
 *		origImg:		the original image,
 *		imgWidth:		the width of the image
 *		imgHeight:		the height of the image
 *						the image is arranged such that
 *						origImg[3*(row*imgWidth+column)+0],
 *						origImg[3*(row*imgWidth+column)+1],
 *						origImg[3*(row*imgWidth+column)+2]
 *						are R, G, B values for pixel at (column, row).
 *
 *      kernel:			the 2D filter kernel,
 *		knlWidth:		the width of the kernel
 *		knlHeight:		the height of the kernel
 *
 *		scale, offset:  after correlating the kernel with the origImg,
 *						each pixel should be divided by scale and then added by offset
 *
 *		selection:      a byte array of the same size as the image,
 *						indicating where in the original image should be filtered, e.g.,
 *						selection[k] == 1 ==> pixel k should be filtered
 *                      selection[k] == 0 ==> pixel k should NOT be filtered
 *                      a special case is selection is a NULL pointer, which means all the pixels should be filtered.
 *
 *  OUTPUT:
 *		rsltImg:		the filtered image of the same size as original image.
 *						it is a valid pointer ( allocated already ).
 */

void image_filter(double* rsltImg, const unsigned char* origImg, const unsigned char* selection,
                  int imgWidth, int imgHeight,
                  const double* kernel, int knlWidth, int knlHeight,
                  double scale, double offset)
{
	int selIndex, pxlIndex;	
	double* resultPixel = new double[3];

	// apply kernel, scale, and offset for each pixel in image 
	int i, j;
    for (j = 0; j < imgHeight; j++) {
        for (i = 0; i < imgWidth; i++) {
			selIndex = (j * imgWidth + i);
			pxlIndex = selIndex * 3;

			if (selection == NULL || selection[selIndex] != 0) {
				pixel_filter(resultPixel, i, j, origImg, imgWidth, imgHeight, kernel, knlWidth, knlHeight, scale, offset);
			
				// save filtered pixel R,G,B values into result image
				*(rsltImg + pxlIndex) = resultPixel[0];
				*(rsltImg + pxlIndex+1) = resultPixel[1];
				*(rsltImg + pxlIndex+2) = resultPixel[2];
			}
		}
	}
}

/*
 *	INPUT:
 *      x:				a column index,
 *      y:				a row index,
 *		origImg:		the original image,
 *		imgWidth:		the width of the image
 *		imgHeight:		the height of the image
 *						the image is arranged such that
 *						origImg[3*(row*imgWidth+column)+0],
 *						origImg[3*(row*imgWidth+column)+1],
 *						origImg[3*(row*imgWidth+column)+2]
 *						are R, G, B values for pixel at (column, row).
 *
 *      kernel:			the 2D filter kernel,
 *		knlWidth:		the width of the kernel
 *		knlHeight:		the height of the kernel
 *
 *		scale, offset:  after correlating the kernel with the origImg,
 *						the result pixel should be divided by scale and then added by offset
 *
 *  OUTPUT:
 *		rsltPixel[0], rsltPixel[1], rsltPixel[2]:
 *						the filtered pixel R, G, B values at row y , column x;
 */

void pixel_filter(double rsltPixel[3], int x, int y, const unsigned char* origImg, int imgWidth, int imgHeight,
                  const double* kernel, int knlWidth, int knlHeight,
                  double scale, double offset)
{
	// calculate the length of the fully expanded kernel (kernel should be an NxN square matrix where N is odd)
	int boxLen;
	if (knlWidth >= knlHeight) {
		// if kernel width is greater or equal to kernel height..
		if (knlWidth % 2 != 0) {
			// if kernel width is odd, expand kernel to be a square
			//
			// 1 1 1    1 1 1
			//       => 0 0 0
			//          0 0 0
			//
			boxLen = knlWidth*knlWidth;
		} else {
			// if kernel width is even, expand kernel to be a square of odd dimensions
			//
			// 1 1    1 1 0
			//     => 0 0 0
			//        0 0 0
			//
			boxLen = (knlWidth+1)*(knlWidth+1);
		}
	} else {
		// if kernel heights is greater than kernel width..
		if (knlHeight % 2 != 0) {
			// if kernel height is odd, expand kernel to be a square
			//
			// 1    1 0 0
			// 1 => 1 0 0
			// 1    1 0 0
			//
			boxLen = knlHeight*knlHeight;
		} else {
			// if kernel heights is even, expand kernel to be a square of odd dimensions
			//
			// 1    1 0 0
			// 1 => 1 0 0
			//      0 0 0
			//
			boxLen = (knlHeight+1)*(knlHeight+1);
		}
	}

	// call CrossCorrelate() and save the resulting value for each R,G,B value
	rsltPixel[0] = (CrossCorrelate(boxLen, kernel, knlWidth, knlHeight, origImg, imgWidth, imgHeight, x, y, 0))/scale + offset;
	rsltPixel[1] = (CrossCorrelate(boxLen, kernel, knlWidth, knlHeight, origImg, imgWidth, imgHeight, x, y, 1))/scale + offset;
	rsltPixel[2] = (CrossCorrelate(boxLen, kernel, knlWidth, knlHeight, origImg, imgWidth, imgHeight, x, y, 2))/scale + offset;
}

/*
 *	INPUT:
 *		boxLen:			the length of the fully expanded kernel,
 *      filterKernel:	the 2D filter kernel,
 *		knlWidth:		the width of the kernel,
 *		knlHeight:		the height of the kernel,
 *		img:			the original image,
 *		imgWidth:		the width of the image
 *		imgHeight:		the height of the image
 *						the image is arranged such that
 *						origImg[3*(row*imgWidth+column)+0],
 *						origImg[3*(row*imgWidth+column)+1],
 *						origImg[3*(row*imgWidth+column)+2]
 *						are R, G, B values for pixel at (column, row).
 *      x:				a column index,
 *      y:				a row index,
 *		c:				offset for R, G, or B value
 *
 *  OUTPUT:
 *		Double vaue resulting from cross-correlating filter kernel to specified image area
 */

double CrossCorrelate(int boxLen, const double* filterKernel, int knlWidth, int knlHeight, const unsigned char* img, int imgWidth, int imgHeight, int x, int y, int c)
{
	double result = 0;

	double sideLen = sqrt((double)boxLen);		  
	int offset = floor(sqrt((double)boxLen)/2);  

	int i, j, xOff, yOff;						  
	double knlVal, imgVal;						   

	int dX = 3;									   
    int dY = 3 * imgWidth;						   
	int boxCenter = 3 * (y * (imgWidth) + x) + c; 

	// perform cross-correlation
	yOff = -offset;
	for (j = 0; j < sideLen; j++) {
		xOff = -offset;
		for (i = 0; i < sideLen; i++) {
			// get corresponding kernel value
			if (i >= knlWidth || j >= knlHeight) {
				knlVal = 0;							// return 0 if index is outside bounds of original kernel (zero-padding)
			} else {
				knlVal = filterKernel[j * (knlWidth) + i];
			}

			// get corresponding image value
			if (x+xOff < 0 || x+xOff >= imgWidth || y+yOff < 0 || y+yOff >= imgHeight) {
				imgVal = 0;							// return 0 if index is outside bounds of original image (zero-padding)
			} else {
				imgVal = img[boxCenter + xOff*dX + yOff*dY];
			}
			result+= knlVal*imgVal;
			xOff++;
		}
		yOff++;
	}
	return result;
}