#pragma once

#include <string>
#include "FileLocation.h"

namespace docmala {

    struct Error {
        FileLocation location;
        std::string message;
    };

}
