#ifndef PARAMETERS_MAP
#define PARAMETERS_MAP

#include "Common.h"

class ParametersMap : public std::map<std::string, std::string>
{
public:
    ParametersMap() {};

    void set(const std::string &key, double val);
    void set(const std::string &key, int val);
    void set(const std::string &key, const std::string &val);

    int getInt(const std::string &key) const;
    double getFloat(const std::string &key) const;
    const std::string &getStr(const std::string &key) const;

    void save(const std::string &fname) const;
    void save(FILE *f) const;
    void load(FILE *f);
};

// Save dictionary of paramater maps to file
void saveToFile(const std::string &fname, const std::map<std::string, ParametersMap> &params);

void loadFromFile(const std::string &fname, std::map<std::string, ParametersMap> &params);

#endif // PARAMETERS_MAP