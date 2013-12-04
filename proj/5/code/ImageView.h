#ifndef __IMAGEVIEW_H__
#define __IMAGEVIEW_H__

#include "Common.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>

//class Fl_Image;

// The ImageView class is responsible for drawing an image and its
// associated features.
class ImageView : public Fl_Double_Window
{
public:
    // Create an ImageView object
    ImageView(int x, int y, int w, int h, const char *l = NULL);

    // Set the pointer to the image
    void setImage(const CByteImage &image);
    void setImage(const CFloatImage &image);

    void setImages(const std::vector<CFloatImage> &images);
    void setImages(const std::vector<CByteImage> &images);

    void setCurrentImageIndex(int idx);

    void draw();

private:
    std::vector<CByteImage> _imgs;
    int _currImg;
};

#endif // __IMAGEVIEW_H__