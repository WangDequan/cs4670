#include "FileIO.h"

void
saveToFile(const std::string &filename, const SupportVectorMachine &svm, const FeatureExtractor *feat)
{
    FILE *f = fopen(filename.c_str(), "wb");
    if(f == NULL) {
        throw CError("Could not open file %s for writing", filename.c_str());
    }

    FeatureExtractor::save(f, feat);

    svm.save(f);
    fclose(f);
}

void
loadFromFile(const std::string &filename, SupportVectorMachine &svm, FeatureExtractor **feat)
{
    FILE *f = fopen(filename.c_str(), "rb");
    if(f == NULL) {
        throw CError("Could not open file %s for reading", filename.c_str());
    }

    char buff[100];

    assert(*feat == NULL);
    *feat = FeatureExtractor::load(f);

    svm.load(f);

    fclose(f);
}

void
saveToFile(const std::string &filename, const std::vector<Detection> &dets)
{
    std::ofstream f(filename.c_str());

    f << "detections\n";
    f << dets.size() << "\n";
    for (int i = 0; i < dets.size(); i++) {
        f << dets[i] << "\n";
    }
}
