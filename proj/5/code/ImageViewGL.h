#ifndef __IMAGEVIEW_GL_H__
#define __IMAGEVIEW_GL_H__

#include "Common.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>

#include <FL/Fl_Gl_Window.H>
#include <FL/glut.H>
#include <FL/glu.h>

class Point
{
public:
    Point(): x(0), y(0) {};
    Point(double x, double y): x(x), y(y) {};

    double x, y;
    /* data */
};

class TextureGL
{
private:
    GLuint _textureID;
    int _imW, _imH;
    double _aspect;

public:
    TextureGL();
    ~TextureGL();

    void setImage(const CByteImage &img);
    void bind();

    int width() { return _imW; }
    int height() { return _imH; }
    double aspect() { return _aspect; }
};

class ImageViewGL;
typedef void (ImageViewGLClickCallback)(ImageViewGL *w, int imgIdx, double x, double y, void *data);

class ImageViewGL : public Fl_Gl_Window
{
private:
    Point _mousePos, _mousePrevPos, _mouseFirstPos;
    enum { TRANSLATE_IMAGE, SCALE_IMAGE, IDLE } _mode;

    Point _viewPos;
    double _viewScale;
    TextureGL *_texture;

    std::vector<CByteImage> _imgs;
    int _currImg;

    void _createTexture();

    ImageViewGLClickCallback *_clickCallback;
    void *_callbackData;

    void _toImageCoords(const Point &view, Point &img );

public:
    ImageViewGL(int X, int Y, int W, int H, const char *L = NULL);

    ~ImageViewGL();

    void setImage(const CByteImage &img);
    void setImage(const CFloatImage &image);

    void setImages(const std::vector<CFloatImage> &images);
    void setImages(const std::vector<CByteImage> &images);

    void setCurrentImageIndex(int idx);

    void draw();
    void resize(int X, int Y, int W, int H);
    int handle(int e);

    double getViewScale() const { return _viewScale; }
    void setViewScale(double s) { _viewScale = s; redraw(); }

    Point getViewTranslation() const { return _viewPos; }
    void setViewTranslation(Point t) { _viewPos = t; redraw(); }

    void resetScale();

    void setClickCallback(ImageViewGLClickCallback *cb, void *data = NULL);
};

template<class T>
void
showImage(const CImageOf<T> &img, bool normalizeRange, bool hold, const char *winName = NULL)
{
    CByteImage tmp;
    TypeConvert(img, tmp);
    ImageViewGL *view = new ImageViewGL(0, 0, img.Shape().width, img.Shape().height, winName);
    view->show();
    view->setImage(img);
}

// Refs:
// http://osdir.com/ml/lib.fltk.opengl/2004-12/msg00001.html

#endif // __IMAGEVIEW_GL_H__