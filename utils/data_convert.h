#ifndef _DATA_CONVERT_H
#define _DATA_CONVERT_H

#include <string>

class DataConvert {
public:
    static char * convert(std::string p) {
        char *res = new char[p.length() + 1];
        std::copy(p.begin(), p.end(), res);
        res[p.length()] = 0;
        return res;
    }
};

#endif