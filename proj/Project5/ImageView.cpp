#include "ImageView.h"

#include <FL/Fl.H>
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>

ImageView::ImageView(int x, int y, int w, int h, const char *l):
    Fl_Double_Window(x, y, w, h, l),
    _currImg(0)
{
    this->box(FL_DOWN_FRAME);
}

void
ImageView::setCurrentImageIndex(int idx)
{
    _currImg = idx;
    redraw();
}

void
ImageView::setImage(const CFloatImage &image)
{
    CByteImage tmp;
    TypeConvert(image, tmp);
    setImage(tmp);
}

void
ImageView::setImage(const CByteImage &image)
{
    std::vector<CByteImage> tmp;
    tmp.push_back(image);
    setImages(tmp);
}

void
ImageView::setImages(const std::vector<CFloatImage> &images)
{
    std::vector<CByteImage> tmp(images.size());
    for(int i = 0; i < images.size(); i++) {
        TypeConvert(images[i], tmp[i]);
    }
    setImages(tmp);
}

void
ImageView::setImages(const std::vector<CByteImage> &images)
{
    _imgs.resize(images.size());
    for(int im = 0; im < _imgs.size(); im++) {
        _imgs[im].ReAllocate(images[im].Shape());

        int nBands = images[im].Shape().nBands;
        int height = images[im].Shape().height;
        int width = images[im].Shape().width;

        for (int i = 0; i < images[im].Shape().height; i++) {
            uint8_t *src = (uint8_t *) images[im].PixelAddress(0, i, 0);
            uint8_t *dst = (uint8_t *) _imgs[im].PixelAddress(0, height - i - 1, 0);
            for (int j = 0; j < images[im].Shape().width; j++, src += nBands, dst += nBands) {
                if(nBands == 3) {
                    dst[2] = src[0];
                    dst[1] = src[1];
                    dst[0] = src[2];
                } else {
                    dst[0] = src[0];
                }

            }
        }
    }

    redraw();
}

void
ImageView::draw()
{
    fl_draw_box(FL_DOWN_BOX, 0, 0, this->w(), this->h(), FL_DARK3);

    if (_imgs.size() != 0) {
        fl_draw_image((unsigned char *)_imgs[_currImg].PixelAddress(0, 0, 0),
                      BOX_WIDTH, BOX_WIDTH,
                      _imgs[_currImg].Shape().width, _imgs[_currImg].Shape().height,
                      _imgs[_currImg].Shape().nBands, _imgs[_currImg].RowSize());
    }
}
