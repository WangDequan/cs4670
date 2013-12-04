#ifndef CROPPED_IMAGE_DATABASE_H
#define CROPPED_IMAGE_DATABASE_H

#include "Common.h"

// Stores the datasets used for training and testing the Support
// Vector Machine class. Contains basically a list of image filenames
// together with their true or predicted labels.
class CroppedImageDatabase
{
private:
    std::vector<std::string> _filenames;
    std::vector<float> _labels;
    int _positivesCount;
    int _negativesCount;
    std::string _dbFilename;

    // What is the size of the object in the image (object is assumed to be
    // centered and of the same size in all images)
    Size _roiSize;

public:
    // Create a new database.
    CroppedImageDatabase();
    CroppedImageDatabase(const char *dbFilename);
    CroppedImageDatabase(const std::vector<float> &labels, const std::vector<std::string> &filenames);

    // Load a database from file.
    void load(const char *dbFilename);
    void save(const char *dbFilename);

    // Accessors
    const int getLabel(int idx) const { return _labels[idx]; }
    const std::vector<float> &getLabels() const { return _labels; }
    const std::string getFilename(int idx) const { return _filenames[idx]; }
    const std::vector<std::string> &getFilenames() const { return _filenames; }
    const Size &getROISize() const { return _roiSize; }

    // Info about the database
    int getPositivesCount() const { return _positivesCount; }
    int getNegativesCount() const { return _negativesCount; }
    int getUnlabeledCount() const { return _labels.size() - _positivesCount - _negativesCount; }
    int getSize() const { return _labels.size(); }
    std::string getDatabaseFilename() const { return _dbFilename; }
};

// Prints information about the dataset
std::ostream &operator<<(std::ostream &s, const CroppedImageDatabase &db);

#endif // CROPPED_IMAGE_DATABASE_H