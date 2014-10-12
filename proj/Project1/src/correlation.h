#ifndef CONVOLVE_H
#define CONVOLVE_H

#include "imgflt.h"

void image_filter(double *rsltImg, const unsigned char *origImg, const unsigned char *selection, 
			int imgWidth, int imgHeight, 
			const double *kernel, int knlWidth, int knlHeight,
			double scale, double offset); 

void pixel_filter(double rsltPixel[3], int x, int y, const unsigned char *origImg, int imgWidth, int imgHeight, 
			const double *kernel, int knlWidth, int knlHeight,
			double scale, double offset);

double CrossCorrelate(int boxLen, const double* filterKernel, int knlWidth, int knlHeight, const unsigned char* img, int imgWidth, int imgHeight, int x, int y, int c);

#endif