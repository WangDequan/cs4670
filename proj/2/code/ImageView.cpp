/* ImageView.cpp */

#include <FL/Fl.H>
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>
#include "FeatureSet.h"
#include "ImageView.h"

// Create a new ImageView object.
ImageView::ImageView(int x, int y, int w, int h, const char *l) : Fl_Double_Window(x,y,w,h,l) {
    image = NULL;
    features = NULL;
	
    showFeatures = true;

    mouseDown = false;
    // This call is necessary to prevent any additional UI widgets
    // from becoming subcomponents of this window.
    end();
}

// Draw the image and features.
void ImageView::draw() {
    if (image != NULL) {
        image->draw(0, 0);
    }

    if ((features != NULL) && (showFeatures)) {
        for (unsigned int i=0; i<features->size(); i++) {
            (*features)[i].draw();
        }
    }

    // If the user is selecting a box of features, draw the box.
    if (mouseDown) {
        int xMin = xLast;
        int yMin = yLast;
        int width = xCurrent - xLast + 1;
        int height = yCurrent - yLast + 1;

        if (xCurrent < xLast) {
            xMin = xCurrent;
            width = xLast - xCurrent + 1;
        }

        if (yCurrent < yLast) {
            yMin = yCurrent;
            height = yLast - yCurrent + 1;
        }

        fl_color(FL_WHITE);
        fl_rect(xMin, yMin, width, height);
    }
}

// Refresh the window.
void ImageView::refresh() {
    redraw();
}

// Handle mouse events.  Yeah, maybe this should be done somewhere
// else, but for now here will have to do.
int ImageView::handle(int event) {
    if (event == FL_PUSH) {
        if (Fl::event_button() == FL_LEFT_MOUSE) {
            // Select a feature by point.
            if (features != NULL) {
                features->select_point(Fl::event_x(), Fl::event_y());
            }

            refresh();
            return 1;
        }
        else if (Fl::event_button() == FL_RIGHT_MOUSE) {
            // Begin to select features by box.
            mouseDown = true;
            xCurrent = xLast = Fl::event_x();
            yCurrent = yLast = Fl::event_y();

            refresh();
            return 1;
        }
    }
    else if (event == FL_RELEASE) {
        if (Fl::event_button() == FL_RIGHT_MOUSE) {
            // Select features by box.
            mouseDown = false;

            if (features != NULL) {
                int xMin = xLast;
                int yMin = yLast;
                int xMax = Fl::event_x();
                int yMax = Fl::event_y();

                if (Fl::event_x() < xLast) {
                    xMin = Fl::event_x();
                    xMax = xLast;
                }

                if (Fl::event_y() < yLast) {
                    yMin = Fl::event_y();
                    yMax = yLast;
                }

                features->select_box(xMin, xMax, yMin, yMax);
            }

            refresh();
            return 1;
        }
    }
    else if (event == FL_DRAG) {
        if (Fl::event_button() == FL_RIGHT_MOUSE) {
            // Update the selection box.
            xCurrent = Fl::event_x();
            yCurrent = Fl::event_y();
        }

        refresh();
        return 1;
    }

    return 0;
}

// Set the pointer to the image.
void ImageView::setImage(Fl_Image *image) {
    this->image = image;
}

// Set the pointer to the feature set.
void ImageView::setFeatures(FeatureSet *features) {
    this->features = features;
}

// Change whether or not the features get drawn.
void ImageView::toggleFeatures() {
    showFeatures = (!showFeatures);
}
