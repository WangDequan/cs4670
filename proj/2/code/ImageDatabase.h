#ifndef IMAGEDATABASE_H
#define IMAGEDATABASE_H

#include <string>
#include "FeatureSet.h"

// A DatabaseItem holds the name of an image, and the corresponding
// feature set.  The images themselves are not stored in memory.
struct DatabaseItem {
	string name;
	FeatureSet features;
};

// The ImageDatabase class is a vector of database items.
class ImageDatabase : public vector<DatabaseItem> {
public:
	// Create a new database.
	ImageDatabase();

	// Load a database from file.
	bool load(const char *name, bool sift);
};

#endif