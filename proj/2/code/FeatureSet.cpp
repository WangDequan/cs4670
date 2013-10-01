/* FeatureSet.cpp */

#include <fstream>
#include <math.h>

#include <FL/fl_draw.H>
#include "FeatureSet.h"

static int iround(double x) {
    if (x < 0.0) {
        return (int) (x - 0.5);
    } else {
        return (int) (x + 0.5);
    }
}

// Create a feature.
Feature::Feature() {
    selected = false;
}

// Draw a feature.  Currently, this just draws a green box for selected
// features, and a red box for unselected features.
void Feature::draw() const {
    if (selected) {
        fl_color(FL_GREEN);
    }
    else {
        fl_color(FL_RED);
    }

    // fl_rect(x-3, y-3, 7, 7);
    int x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
    
    double d1[2] = {  5.0,  5.0 };
    double d2[2] = {  5.0, -5.0 };
    double d3[2] = { -5.0, -5.0 };
    double d4[2] = { -5.0,  5.0 };
    double d5[2] = { 5.0, 0.0 };

    double s = sin(angleRadians);
    double c = cos(angleRadians);

    x1 = iround(x + (d1[0] * c - d1[1] * s));
    y1 = iround(y + (d1[0] * s + d1[1] * c));

    x2 = iround(x + (d2[0] * c - d2[1] * s));
    y2 = iround(y + (d2[0] * s + d2[1] * c));

    x3 = iround(x + (d3[0] * c - d3[1] * s));
    y3 = iround(y + (d3[0] * s + d3[1] * c));

    x4 = iround(x + (d4[0] * c - d4[1] * s));
    y4 = iround(y + (d4[0] * s + d4[1] * c));

    x5 = iround(x + (d5[0] * c - d5[1] * s));
    y5 = iround(y + (d5[0] * s + d5[1] * c));

    fl_line(x1, y1, x2, y2);
    fl_line(x2, y2, x3, y3);
    fl_line(x3, y3, x4, y4);
    fl_line(x4, y4, x1, y1);
    fl_line(x, y, x5, y5);
}

// Print the (x,y) location of a feature.  You can modify this to print
// whatever attributes you use.
void Feature::print() const {
    printf("(%d,%d)\n", x, y);
}

// Reads a SIFT feature.
void Feature::read_sift(istream &is) {
    // Let's use type 9 for SIFT features.
    type = 9;

    double xSub;
    double ySub;
    double scale;
    double rotation;

    // Read the feature location, scale, and orientation.
    is >> xSub >> ySub >> scale >> rotation;

    // They give row first, then column.
    x = (int) (ySub + 0.5);
    y = (int) (xSub + 0.5);

    data.resize(128);

    // Read the descriptor vector.
    for (int i=0; i<128; i++) {
        is >> data[i];
    }
}

// Write the feature to an output stream.
ostream &operator<<(ostream &os, const Feature &f) {
    os << f.type << '\n';
    os << f.id << '\n';
    os << f.x << ' ' << f.y << '\n';
    os << f.angleRadians << '\n';

    os << f.data.size() << '\n';

    for (unsigned int i=0; i<f.data.size(); i++) {
        os << f.data[i] << '\n';
    }

    return os;
}

// Read the feature from an input stream.
istream &operator>>(istream &is, Feature &f) {
    int n;

    is >> f.type;
    is >> f.id;
    is >> f.x >> f.y;
    is >> f.angleRadians;

    is >> n;

    f.data.clear();
    f.data.resize(n);

    for (int i=0; i<n; i++) {
        is >> f.data[i];
    }

    return is;
}

// Create a feature set.
FeatureSet::FeatureSet() {
}

// Load a feature set from a file.
bool FeatureSet::load(const char *name) {
    int n;

    // Clear the currently loaded features.
    clear();

    // Open the file.
    ifstream f(name);

    if (!f.is_open()) {
        return false;
    }

    // Read the total number of features.
    f >> n;

    // Resize the vector of features.
    resize(n);

    // Read each of the features.
    iterator i = begin();

    while (i != end()) {
        f >> (*i);
        i++;
    }

    // Close the file.
    f.close();

    return true;
}

// Load a SIFT feature set.
bool FeatureSet::load_sift(const char *name) {
    int n;
    int m;

    // Clear the currently loaded features.
    clear();

    // Open the file.
    ifstream f(name);

    if (!f.is_open()) {
        return false;
    }

    // Read the total number of features.
    f >> n;

    // Read the length of each feature.  It better be 128.
    f >> m;

    if (m != 128) {
        f.close();
        return false;
    }

    // Resize the vector of features.
    resize(n);

    // Read each of the features.
    iterator i = begin();
    int id = 1;

    while (i != end()) {
        (*i).read_sift(f);
        (*i).id = id;

        i++;
        id++;
    }

    // Close the file.
    f.close();

    return true;
}

// Save a feature set to file.
bool FeatureSet::save(const char *name) const {
    // Open the file.
    ofstream f(name);

    if (!f.is_open()) {
        return false;
    }

    // Write the number of features.
    f << size() << '\n';

    // Write each of the features.
    const_iterator i = begin();

    while (i != end()) {
        f << (*i);
        i++;
    }

    // Close the file.
    f.close();

    return true;
}

// Select (or deselect) features at a location.
void FeatureSet::select_point(int x, int y) {
    iterator i = begin();

    while (i != end()) {
        // If the given point is within 5 pixels of the feature, then
        // select it.  This can select multiple features if they are
        // nearly colocated.
        if ((fabs((double)(*i).x-x) <= 5) && (fabs((double)(*i).y-y) <= 5)) {
            (*i).selected = (!(*i).selected);

            printf("selecting feature %d [orient: %0.3f]\n", (*i).id, (*i).angleRadians);
            fflush(stdout);
        }

        i++;
    }
}

// Select (or deselect) features inside a box.
void FeatureSet::select_box(int xMin, int xMax, int yMin, int yMax) {
    iterator i = begin();

    while (i != end()) {
        if (((*i).x >= xMin) && ((*i).x <= xMax) && ((*i).y >= yMin) && ((*i).y <= yMax)) {
            (*i).selected = (!(*i).selected);
        }

        i++;
    }
}

// Select all features.
void FeatureSet::select_all() {
    iterator i = begin();

    while (i != end()) {
        (*i).selected = true;
        i++;
    }
}

// Deselect all features.
void FeatureSet::deselect_all() {
    iterator i = begin();

    while (i != end()) {
        (*i).selected = false;
        i++;
    }
}

// Take only the selected features.
void FeatureSet::get_selected_features(FeatureSet &f) {
    f.clear();

    iterator i = begin();

    while (i != end()) {
        if ((*i).selected) {
            f.push_back((*i));
        }

        i++;
    }
}
