#include <fstream>
#include <Fl/filename.H>
#include "ImageDatabase.h"

// Create a database.
ImageDatabase::ImageDatabase() {
}

// Load a database from file.  The database file contains a list of
// image file names and feature file names.  Each image file name is
// followed by the corresponding feature file name.  The file names in
// the database must be relative to the database path or it won't work.
// I apologize for this annoyance.
bool ImageDatabase::load(const char *name, bool sift) {
    DatabaseItem d;
    string s;
	
    // Clear all entries from the database.
    clear();

    // Open the file.
    ifstream f(name);

    if (!f.is_open()) {
        return false;
    }

    // Get the path of the database file.
    const char *dirStart = name;
    const char *dirStop = fl_filename_name(name);
    string dir = "";

    for (const char *c=dirStart; c<dirStop; c++) {
        dir += (*c);
    }

    while (f.peek() != EOF) {
        // Read the image name.
        f >> d.name;
        d.name = dir + d.name;

        // Read the feature file name.
        f >> s;
        s = dir + s;

        // This really shouldn't be necessary.  I'm not sure what's
        // going on.
        if (f.peek() == EOF) {
            break;
        }

        // Load the features from file.
        if (((!sift) && (!d.features.load(s.c_str()))) || ((sift) && (!d.features.load_sift(s.c_str())))) {
            clear();
            f.close();
            return false;
        }

        push_back(d);
    }

    f.close();
    return true;
}
