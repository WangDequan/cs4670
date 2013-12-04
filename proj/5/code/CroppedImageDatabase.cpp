#include "CroppedImageDatabase.h"

using namespace std;

static const char *SIGNATURE = "CroppedImageDatabase";


CroppedImageDatabase::CroppedImageDatabase():
    _positivesCount(0), _negativesCount(0)
{
}

CroppedImageDatabase::CroppedImageDatabase(const char *dbFilename):
    _positivesCount(0), _negativesCount(0)
{
    load(dbFilename);
}

CroppedImageDatabase::CroppedImageDatabase(const vector<float> &labels, const vector<string> &filenames):
    _positivesCount(0), _negativesCount(0)
{
    _labels = labels;
    _filenames = filenames;

    for(vector<float>::iterator i = _labels.begin(); i != _labels.end(); i++) {
        if(*i > 0) _positivesCount++;
        else if(*i < 0) _negativesCount++;
    }
}

void
CroppedImageDatabase::load(const char *dbFilename)
{
    _dbFilename = string(dbFilename);

    _negativesCount = 0;
    _positivesCount = 0;

    ifstream f(dbFilename);
    if(!f.is_open()) {
        throw CError("Could not open file %s for reading", dbFilename);
    }

    char sig[200];
    f.read(sig, strlen(SIGNATURE));
    sig[strlen(SIGNATURE)] = '\0';
    if (strcmp(sig, SIGNATURE) != 0) {
        throw CError("Bad signature for file, expecting \"%s\" but got \"%s\"", SIGNATURE, sig);
    }

    f >> _roiSize.width >> _roiSize.height;

    int nItems;
    f >> nItems;

    assert(nItems > 0);
    _labels.resize(nItems);
    _filenames.resize(nItems);

    for(int i = 0; i < nItems; i++) {
        f >> _labels[i] >> _filenames[i];

        if(_labels[i] < 0) _negativesCount++;
        else if(_labels[i] > 0) _positivesCount++;
    }
}

void
CroppedImageDatabase::save(const char *dbFilename)
{
    ofstream f(dbFilename);
    if(!f.is_open()) {
        throw CError("Could not open file %s for writing", dbFilename);
    }

    f << SIGNATURE << "\n";
    f << _labels.size() << "\n";
    for(int i = 0; i < _labels.size(); i++) {
        f << _labels[i] << " " << _filenames[i] << "\n";
    }
}

std::ostream &
operator<<(std::ostream &s, const CroppedImageDatabase &db)
{
    s << "DATABASE INFO\n"
      << setw(20) << "Original filename:" << " " << db.getDatabaseFilename() << "\n"
      << setw(20) << "ROI Size:" << setw(5) << right << db.getROISize().width << " x " << db.getROISize().height << "\n"
      << setw(20) << "Positives:" << setw(5) << right << db.getPositivesCount() << "\n"
      << setw(20) << "Negaties:"   << setw(5) << right << db.getNegativesCount() << "\n"
      << setw(20) << "Unlabeled:"  << setw(5) << right << db.getUnlabeledCount() << "\n"
      << setw(20) << "Total:"      << setw(5) << right << db.getSize() << "\n";

    return s;
}