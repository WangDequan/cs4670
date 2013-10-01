#include <assert.h>
#include <Fl/Fl.H>
#include <Fl/Fl_Shared_Image.H>
#include <FL/fl_ask.H>
#include "features.h"
#include "FeaturesUI.h"
#include "FeaturesDoc.h"

// Create a new document.
FeaturesDoc::FeaturesDoc() {
    queryImage = NULL;
    queryFeatures = NULL;

    db = NULL;

    resultImage = NULL;

    ui = NULL;

    matchType = 1;
}

// Load an image file for use as the query image.
void FeaturesDoc::load_query_image(const char *name) {
    ui->set_images(NULL, NULL);
    ui->set_features(NULL, NULL);

    // Delete the current query image.
    if (queryImage != NULL) {
        queryImage->release();
        queryImage = NULL;
    }

    // Delete the current query image features.
    if (queryFeatures != NULL) {
        delete queryFeatures;
        queryFeatures = NULL;
    }

    // Delete the current result image.
    if (resultImage != NULL) {
        resultImage->release();
        resultImage = NULL;
    }

    // Load the image.
    queryImage = Fl_Shared_Image::get(name);

    if (queryImage == NULL) {
        fl_alert("couldn't load image file");
    }
    else {
        // Update the UI.
        ui->resize_windows(queryImage->w(), 0, queryImage->h());
        ui->set_images(queryImage, NULL);
    }

    ui->refresh();
}

// Load a set of features for the query image.
void FeaturesDoc::load_query_features(const char *name, bool sift) {
    if (queryImage == NULL) {
        fl_alert("no query image loaded");
    }
    else {
        ui->set_images(queryImage, NULL);
        ui->set_features(NULL, NULL);

        // Delete the current query image features.
        if (queryFeatures != NULL) {
            delete queryFeatures;
            queryFeatures = NULL;
        }

        // Delete the current result image.
        if (resultImage != NULL) {
            resultImage->release();
            resultImage = NULL;
        }

        queryFeatures = new FeatureSet();

        // Load the feature set.
        if (((!sift) && (queryFeatures->load(name))) || ((sift) && (queryFeatures->load_sift(name)))) {
            ui->set_features(queryFeatures, NULL);
        }
        else {
            delete queryFeatures;
            queryFeatures = NULL;

            fl_alert("couldn't load feature data file");
        }
    }

    ui->refresh();
}

// Load an image database.
void FeaturesDoc::load_image_database(const char *name, bool sift) {
    ui->set_images(queryImage, NULL);
    ui->set_features(queryFeatures, NULL);

    // Delete the current database.
    if (db != NULL) {
        delete db;
        db = NULL;
    }

    // Delete the current result image.
    if (resultImage != NULL) {
        resultImage->release();
        resultImage = NULL;
    }

    db = new ImageDatabase();

    // Load the database.
    if (!db->load(name, sift)) {
        delete db;
        db = NULL;

        fl_alert("couldn't load database");
    }

    ui->refresh();
}

// Perform a query on the loaded database.
void FeaturesDoc::perform_query() {
    ui->set_images(queryImage, NULL);
    ui->set_features(queryFeatures, NULL);

    if (queryImage == NULL) {
        fl_alert("no query image loaded");
    }
    else if (queryFeatures == NULL) {
        fl_alert("no query features loaded");
    }
    else if (db == NULL) {
        fl_alert("no image database loaded");
    }
    else {
        FeatureSet selectedFeatures;
        queryFeatures->get_selected_features(selectedFeatures);

        if (selectedFeatures.size() == 0) {
            fl_alert("no features selected");
        }
        else {
            int index;
            vector<FeatureMatch> matches;
            double distance;

            if (!performQuery(selectedFeatures, *db, index, matches, distance, ui->get_match_type())) {
                fl_alert("query failed");
            }
            else {
				printf("Found %d matches\n", (int) matches.size());

                // Delete the current result image.
                if (resultImage != NULL) {
                    resultImage->release();
                    resultImage = NULL;
                }

                // Load the image.
                resultImage = Fl_Shared_Image::get((*db)[index].name.c_str());

                if (resultImage == NULL) {
                    fl_alert("couldn't load result image file");
                }
                else {
                    (*db)[index].features.deselect_all();
                    (*queryFeatures).deselect_all();

                    // Select the matched features.
                    for (unsigned int i=0; i<matches.size(); i++) {
                        (*queryFeatures)[matches[i].id1].selected = true;
                        (*db)[index].features[matches[i].id2].selected = true;
                    }

                    // Update the UI.
                    if (queryImage->h() > resultImage->h()) {
                        ui->resize_windows(queryImage->w(), resultImage->w(), queryImage->h());
                    }
                    else {
                        ui->resize_windows(queryImage->w(), resultImage->w(), resultImage->h());
                    }
					
                    ui->set_images(queryImage, resultImage);
                    ui->set_features(queryFeatures, &((*db)[index].features));
                }
            }
        }
    }

    ui->refresh();
}

// Set the UI pointer.
void FeaturesDoc::set_ui(FeaturesUI *ui) {
    this->ui = ui;
}

// Select all query features.
void FeaturesDoc::select_all_query_features() {
    if (queryFeatures == NULL) {
        fl_alert("no query features loaded");
    }
    else {
        queryFeatures->select_all();
        ui->refresh();
    }
}

// Deselect all query features.
void FeaturesDoc::deselect_all_query_features() {
    if (queryFeatures == NULL) {
        fl_alert("no query features loaded");
    }
    else {
        queryFeatures->deselect_all();
        ui->refresh();
    }
}

// Set the match algorithm.
void FeaturesDoc::set_match_algorithm(int type) {
    matchType = type;
}
