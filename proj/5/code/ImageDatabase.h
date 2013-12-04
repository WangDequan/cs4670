#ifndef IMAGE_DATABASE_H
#define IMAGE_DATABASE_H

#include "Common.h"
#include "Detection.h"

class ImageDatabase
{
private:
    std::vector<std::string> _filenames;
    std::vector<std::vector<Detection> > _detections;
    std::string _dbFilename;

public:
    // Create a new database.
    ImageDatabase();
    ImageDatabase(const std::string &dbFilename);
    ImageDatabase(const std::vector<std::vector<Detection> > &dets, const std::vector<std::string> &fnames);

    // Load a database from file.
    void load(const std::string &dbFilename);
    void save(const std::string &dbFilename);

    // Accessors
    const std::vector<std::vector<Detection> > &getDetections() const { return _detections; }
    const std::vector<std::string> &getFilenames() const { return _filenames; }

    // Info about the database
    int getSize() const { return _detections.size(); }
    std::string getDatabaseFilename() const { return _dbFilename; }
};

#endif // IMAGE_DATABASE_H