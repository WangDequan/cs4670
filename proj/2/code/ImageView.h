#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <FL/Fl_Double_Window.H>

class Fl_Image;
class FeatureSet;

// The ImageView class is responsible for drawing an image and its
// associated features.
class ImageView : public Fl_Double_Window {
private:
	Fl_Image *image;
	FeatureSet *features;

	bool showFeatures;

	bool mouseDown;

	int xLast;
	int yLast;
	int xCurrent;
	int yCurrent;

public:
	// Create an ImageView object.
	ImageView(int x, int y, int w, int h, const char *l);
	
	// Draw the image and features.
	void draw();

	// Refresh the window.
	void refresh();

	// Handle mouse events.
	int handle(int event);

	// Set the pointer to the image.
	void setImage(Fl_Image *image);

	// Set the pointer to the feature set.
	void setFeatures(FeatureSet *features);

	// Change whether or not the features get drawn.
	void toggleFeatures();
};

#endif