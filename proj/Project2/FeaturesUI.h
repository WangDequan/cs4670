#ifndef FeaturesUI_H
#define FeaturesUI_H

class Fl_Window;
class Fl_Menu_;
class Fl_Menu_Bar;
class ImageView;
class FeatureSet;
class FeaturesDoc;

// The FeaturesUI class controls the UI.  Feel free to play around with
// the UI if you'd like.
class FeaturesUI {
public:
	FeaturesDoc *doc;

	Fl_Window *mainWindow;
	Fl_Menu_Bar *menuBar;

	ImageView *queryView;
	ImageView *resultView;

public:
	// Create the UI.
	FeaturesUI();

	// Begin displaying the UI.
	void show();

	// Refresh the window.
	void refresh();

	// Resize the image windows.
	void resize_windows(int w1, int w2, int h);

	// Set the document pointer.
	void set_document(FeaturesDoc *doc);

	// Set the pointers to the two images.
	void set_images(Fl_Image *queryImage, Fl_Image *resultImage);

	// Set the pointers to the two feature sets.
	void set_features(FeatureSet *queryFeatures, FeatureSet *resultFeatures);

	// Get the current match type.
	int get_match_type() const;

private:
	// Return the UI, given a menu item.
	static FeaturesUI *who_am_i(Fl_Menu_ *o);

	// Here are the callback functions.
	static void cb_load_query_image(Fl_Menu_ *o, void *v);
	static void cb_load_query_features(Fl_Menu_ *o, void *v);
	static void cb_load_query_features_sift(Fl_Menu_ *o, void *v);
	static void cb_load_image_database(Fl_Menu_ *o, void *v);
	static void cb_load_image_database_sift(Fl_Menu_ *o, void *v);
	static void cb_exit(Fl_Menu_ *o, void *v);
	static void cb_select_all_features(Fl_Menu_ *o, void *v);
	static void cb_deselect_all_features(Fl_Menu_ *o, void *v);
	static void cb_toggle_features(Fl_Menu_ *o, void *v);
	static void cb_perform_query(Fl_Menu_ *o, void *v);
	static void cb_match_algorithm_1(Fl_Menu_ *o, void *v);
	static void cb_match_algorithm_2(Fl_Menu_ *o, void *v);
	static void cb_about(Fl_Menu_ *o, void *v);

	// Here is the array of menu items.
	static Fl_Menu_Item menuItems[];
};

#endif