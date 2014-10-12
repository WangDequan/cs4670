#include "ParametersMap.h"

template<typename T>
std::string
toString(const std::string &fmt, T val)
{
    char buffer[256];
    sprintf(buffer, fmt.c_str(), val);
    return std::string(buffer);
}

void
ParametersMap::save(const std::string &fname) const
{
    FILE *f = fopen(fname.c_str(), "w");
    if(f == NULL) {
        throw CError("Could not open file %s for writing", fname.c_str());
    }

    save(f);
    fclose(f);
}

void
ParametersMap::save(FILE *f) const
{
    fprintf(f, "%lu\n", size());

    for(ParametersMap::const_iterator i = begin(); i != end(); i++) {
        fprintf(f, "%s %s\n", i->first.c_str(), i->second.c_str());
    }
}

void
ParametersMap::load(FILE *f)
{
    int n;
    fscanf(f, "%d", &n);

    for(int i = 0; i < n; i++) {
        char key[256], val[256];
        fscanf(f, "%s %s\n", key, val);

        (*this)[key] = val;
    }
}

void
ParametersMap::set(const std::string &key, double val)
{
    // TODO: throw exception if key has a space in it
    (*this)[key] = toString("%f", val);
}

void
ParametersMap::set(const std::string &key, int val)
{
    (*this)[key] = toString("%d", val);
}

void
ParametersMap::set(const std::string &key, const std::string &val)
{
    (*this)[key] = val;
}

int
ParametersMap::getInt(const std::string &key) const
{
    return atoi(this->at(key).c_str());
}

double
ParametersMap::getFloat(const std::string &key) const
{
    return atof(this->at(key).c_str());
}

const std::string &
ParametersMap::getStr(const std::string &key) const
{
    return this->at(key);
}

void
saveToFile(const std::string &fname, const std::map<std::string, ParametersMap> &params)
{
    using namespace std;

    FILE *f = fopen(fname.c_str(), "w");
    if(f == NULL) {
        throw CError("Could not open file %s for writing", fname.c_str());
    }

    fprintf(f, "%d\n", params.size());
    for(map<string, ParametersMap>::const_iterator i = params.begin(); i != params.end(); i++) {
        fprintf(f, "%s\n", i->first.c_str());
        i->second.save(f);
    }

    fclose(f);
}

void
loadFromFile(const std::string &fname, std::map<std::string, ParametersMap> &params)
{
    using namespace std;

    FILE *f = fopen(fname.c_str(), "r");
    if(f == NULL) {
        throw CError("Could not open file %s for reading", fname.c_str());
    }

    int nParamMaps = 0;
    fscanf(f, "%d", &nParamMaps);
    for(int i = 0; i < nParamMaps; i++) {
        char key[100];
        fscanf(f, "%s", key);
        ParametersMap tmp;
        tmp.load(f);

        params[key] = tmp;
    }

    fclose(f);
}

