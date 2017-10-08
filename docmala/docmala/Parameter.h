#pragma once

#include <string>
#include <map>
#include "FileLocation.h"

namespace docmala {

struct Parameter {
    std::string  key;
    std::string  value;
    FileLocation location;
};

typedef std::map<std::string, Parameter> ParameterList;
}
