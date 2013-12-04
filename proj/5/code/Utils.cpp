#include "Utils.h"

template <typename T>
void
convertRGB2GrayImage(const CImageOf<T>& rgb, CImageOf<T>& gray)
{
	CShape shape = rgb.Shape();
	shape.nBands = 1;
	gray.ReAllocate(shape);

	for (int i = 0; i < shape.height; i++) {
		for (int j = 0; j < shape.width; j++) {

			gray.Pixel(j, i, 0) = 0;
			double val = 0.0;
			for (int c = 0; c < rgb.Shape().nBands; c++) {
				val +=  rgb.Pixel(j, i, c) / 3.0;
			}
			gray.Pixel(j, i, 0) = min((double)gray.MaxVal(), (double)val);
		}
	}	
}

// Instantiate the templates
template void convertRGB2GrayImage<>(const CImageOf<unsigned char>& rgb, CImageOf<unsigned char>& gray);
template void convertRGB2GrayImage<>(const CImageOf<float>& rgb, CImageOf<float>& gray);
