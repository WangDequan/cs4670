#ifndef FeaturesDoc_H
#define FeaturesDoc_H

class Fl_Shared_Image;
class FeatureSet;
class ImageDatabase;
class FeaturesUI;

// The FeaturesDoc class controls the functionality of the project, and
// has methods for all major operations, like loading image and
// features, and performing queries.
class FeaturesDoc {
private:
	Fl_Shared_Image *queryImage;
	FeatureSet *queryFeatures;

	ImageDatabase *db;

	Fl_Shared_Image *resultImage;

	int matchType;

public:
	FeaturesUI *ui;

public:
	// Create a new document.
	FeaturesDoc();

	// Destroy the document.
	~FeaturesDoc();

	// Load an image, feature set, or database.
	void load_query_image(const char *name);
	void load_query_features(const char *name, bool sift);
	void load_image_database(const char *name, bool sift);

	// Perform a query on the currently loaded image and database.
	void perform_query();

	// Set the pointer to the UI.
	void set_ui(FeaturesUI *ui);

	// Select or deselect all query features.
	void select_all_query_features();
	void deselect_all_query_features();

	// Set the match algorithm.
    int get_match_algorithm() { return matchType; }
    void set_match_algorithm(int type);
};

#endif