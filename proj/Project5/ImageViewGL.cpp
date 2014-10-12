#include "ImageViewGL.h"

void
checkGLErrors()
{
    GLenum err = glGetError();
    if ( err != GL_NO_ERROR ) {
        fprintf(stderr, "GL ERROR = %d\n", (int)err);
        fprintf(stderr, "%s\n", gluErrorString(err));
    }
}

// ============================================================================
// Texture
// ============================================================================

// ref: http://www.nullterminator.net/gltexture.html

TextureGL::TextureGL():
    _imW(0), _imH(0), _aspect(0)
{
    glGenTextures(1, &_textureID);
    bind();
}

TextureGL::~TextureGL()
{
    glDeleteTextures( 1, &_textureID );
}

void
TextureGL::bind()
{
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, _textureID );
}

void
TextureGL::setImage(const CByteImage &img)
{
    _imW = img.Shape().width;
    _imH = img.Shape().height;
    _aspect = double(_imH) / _imW;

    // ref: http://www.gamedev.net/topic/521924-uploading-texture-data-with-stride/
    glPixelStorei(GL_UNPACK_ROW_LENGTH, img.RowSize() / img.Shape().nBands);

    switch(img.Shape().nBands) {
    case 1:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.Shape().width, img.Shape().height, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE,
                     (unsigned char *)img.PixelAddress(0, 0, 0));
        break;
    case 3:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.Shape().width, img.Shape().height, 0,
                     GL_BGR, GL_UNSIGNED_BYTE,
                     (unsigned char *)img.PixelAddress(0, 0, 0));
        break;
    case 4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.Shape().width, img.Shape().height, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE,
                     (unsigned char *)img.PixelAddress(0, 0, 0));
        break;
    default:
        assert(false);
    }

    // min mag
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    // borders
    //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

    glEnable( GL_TEXTURE_2D );

    checkGLErrors();
}

// ============================================================================
// ImageViewGL
// ============================================================================

ImageViewGL::ImageViewGL(int X, int Y, int W, int H, const char *L)
    : Fl_Gl_Window(X, Y, W, H, L),
      _mode(IDLE),
      _viewScale(1.0),
      _texture(NULL),
      _currImg(0),
      _clickCallback(NULL),
      _callbackData(NULL)
{
}

ImageViewGL::~ImageViewGL()
{
    if(_texture != NULL) {
        delete _texture;
        _texture = NULL;
    }
}

void
ImageViewGL::resetScale()
{
    _viewScale = std::min(double(h() / _texture->aspect()), (double) w() );
}

void
ImageViewGL::setCurrentImageIndex(int idx)
{
    _currImg = idx;
    if(this->parent()->visible() != 0) damage(FL_DAMAGE_ALL);
    if(_texture != NULL) {
        delete _texture;
        _texture = NULL;
    }
}

void
ImageViewGL::setImage(const CFloatImage &image)
{
    CByteImage tmp;
    TypeConvert(image, tmp);
    setImage(tmp);
}

void
ImageViewGL::setImage(const CByteImage &img)
{
    _imgs.resize(1);
    _imgs[0] = img;
    damage(FL_DAMAGE_ALL);
    _createTexture();
    resetScale();
}

void
ImageViewGL::_createTexture()
{
    if(_texture != NULL) {
        delete _texture;
        _texture = NULL;
    }

    if(_currImg < _imgs.size()) {
        _texture = new TextureGL();
        _texture->setImage(_imgs[_currImg]);
    }

    if( (this->parent() == NULL) || (this->parent()->visible() != 0) ) damage(FL_DAMAGE_ALL);
}

void
ImageViewGL::setImages(const std::vector<CFloatImage> &images)
{
    std::vector<CByteImage> tmp(images.size());
    for(int i = 0; i < images.size(); i++) {
        TypeConvert(images[i], tmp[i]);
    }
    setImages(tmp);
}

void
ImageViewGL::setImages(const std::vector<CByteImage> &images)
{
    _imgs = images;
    _createTexture();
    resetScale();
}

void
ImageViewGL::draw()
{
    if(!visible()) return;

    if(((!valid()) || _texture == NULL) && _imgs.size()) {
        _createTexture();
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glScalef(1., -1., 1.);
    glViewport(0, 0, w(), h());
    glOrtho(0, w(), 0, h(), -1, 1);

    Point delta(_mousePos.x - _mousePrevPos.x, _mousePos.y - _mousePrevPos.y);
    _mousePrevPos = _mousePos;

    switch(_mode) {
    case TRANSLATE_IMAGE:
        _viewPos.x += delta.x;
        _viewPos.y += delta.y;
        break;
    case SCALE_IMAGE:
        _viewScale *= std::min(std::max(0.1, pow(10.0, delta.y * 0.01)), 100.0);
        break;
    default:
        break;
    }

    glTranslatef( _viewPos.x, _viewPos.y, 0.0 );
    glScalef(_viewScale, _viewScale, _viewScale);

    // Clear screen
    glClearColor(.5, .5, .5, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(_texture) {
        _texture->bind();

        glBegin( GL_QUADS );
        {
            glTexCoord2f(0.0, 0.0);
            glVertex2f(0, _texture->aspect());
            glTexCoord2f(1.0, 0.0);
            glVertex2f(1.0, _texture->aspect());
            glTexCoord2f(1.0, 1.0);
            glVertex2f(1.0, 0);
            glTexCoord2f(0.0, 1.0);
            glVertex2f(0, 0);
        }
        glEnd();
    }

    checkGLErrors();
}

void
ImageViewGL::resize(int X, int Y, int W, int H)
{
    Fl_Gl_Window::resize(X, Y, W, H);
    glLoadIdentity();
    glViewport(0, 0, W, H);
    glOrtho(-W, W, -H, H, -1, 1);
    redraw();
}

void
ImageViewGL::_toImageCoords(const Point &view, Point &img )
{
    double scale = _texture->width() / _viewScale;
    img = Point(double(view.x - _viewPos.x) * scale, _texture->height() - double(view.y - _viewPos.y) * scale);
}

int
ImageViewGL::handle(int event)
{
    // fprintf(stderr, "EVENT: %s (%d)\n", fl_eventnames[event], event);
    switch(event) {
    case FL_PUSH:
        _mousePrevPos = Point(Fl::event_x(), Fl::event_y());
        _mousePos = _mousePrevPos;
        _mouseFirstPos = _mousePos;

        if(Fl::get_key(FL_Shift_L) || Fl::get_key(FL_Shift_R)) {
            _mode = SCALE_IMAGE;
        } else if(Fl::get_key(FL_Alt_L) || Fl::get_key(FL_Alt_R)) {
            // TODO: change the trigger to double click
            if(_clickCallback != NULL && _texture != NULL) {
                Point eventXY(Fl::event_x(), Fl::event_y());
                Point imgXY;
                _toImageCoords(eventXY, imgXY);

                (*_clickCallback)(this, _currImg, imgXY.x, imgXY.y, _callbackData);
            }
        } else {
            _mode = TRANSLATE_IMAGE;
        }
        redraw();
        return 1;

    case FL_RELEASE:
        _mousePrevPos = Point();
        _mousePos = _mousePrevPos;
        _mode = IDLE;
        redraw();
        return 1;

    case FL_SHORTCUT:
        if(Fl::get_key('m') || Fl::get_key('M')) {
            resetScale();
            _viewPos = Point();
            redraw();
            return 1;
        }
        _mode = IDLE;
        break;

    case FL_DRAG:
        _mousePos = Point(Fl::event_x(), Fl::event_y());
        redraw();
        return 1;
    }
    return Fl_Gl_Window::handle(event);;
}

void
ImageViewGL::setClickCallback(ImageViewGLClickCallback *cb, void *data)
{
    _clickCallback = cb;
    _callbackData = data;
}
