#ifndef UTILS_H
#define UTILS_H

#include "Common.h"

// Convert image in RGB colorspace to grayscale
template<class T>
void convertRGB2GrayImage(const CImageOf<T> &rgb, CImageOf<T> &gray);

#endif // UTILS_H