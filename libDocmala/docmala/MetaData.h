#pragma once

#include <string>
#include <vector>
#include "FileLocation.h"


namespace docmala {
    struct MetaData {
        enum class Mode {
            None,
            Flag,
            First,
            List
        };

        struct Data {
            FileLocation location;
            std::string value;
        };

        Mode mode = Mode::None;
        std::string key;
        std::vector<Data> data;
        FileLocation firstLocation;
    };
} /* docmala */
