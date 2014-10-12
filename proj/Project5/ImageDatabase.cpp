#include "ImageDatabase.h"

static const char *SIGNATURE = "ImageDataset";

ImageDatabase::ImageDatabase()
{
}

ImageDatabase::ImageDatabase(const std::string &dbFilename)
{
    load(dbFilename);
}

ImageDatabase::ImageDatabase(const std::vector<std::vector<Detection> > &dets,
                             const std::vector<std::string> &fnames):
    _detections(dets),
    _filenames(fnames)
{
}

void
ImageDatabase::load(const std::string &dbFilename)
{
    _dbFilename = dbFilename;

    ifstream f(dbFilename.c_str());
    if(!f.is_open()) {
        throw CError("Could not open file %s for reading", dbFilename.c_str());
    }

    char sig[200];
    f.read(sig, strlen(SIGNATURE));
    sig[strlen(SIGNATURE)] = '\0';
    if (strcmp(sig, SIGNATURE) != 0) {
        throw CError("Bad signature for file, expecting \"%s\" but got \"%s\"", SIGNATURE, sig);
    }

    int nItems;
    f >> nItems;

    assert(nItems > 0);

    _filenames.resize(nItems);
    _detections.resize(nItems);
    for(int i = 0; i < nItems; i++) {
        f >> _filenames[i];

        int nDets = 0;
        f >> nDets;

        _detections[i].resize(nDets);
        for (int j = 0; j < nDets; j++) f >> _detections[i][j];
    }
}

void
ImageDatabase::save(const std::string &dbFilename)
{
    ofstream f(dbFilename.c_str());
    if(!f.is_open()) {
        throw CError("Could not open file %s for writing", dbFilename.c_str());
    }

    f << SIGNATURE << "\n";
    f << getSize() << "\n";

    for(int i = 0; i < getSize(); i++) {
        f << _filenames[i] << " ";
        f << _detections[i].size() << " ";
        for (int j = 0; j < _detections[i].size(); j++) {
            f << _detections[i][j] << " ";
        }
        f << "\n";
    }
}
