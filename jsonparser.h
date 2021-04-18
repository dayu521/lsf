#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>

namespace lsf {

class Token;

class JsonParser
{
public:
    JsonParser();
    bool parser();
};

}
#endif // JSONPARSER_H
