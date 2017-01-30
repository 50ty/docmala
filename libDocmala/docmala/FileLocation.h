#pragma once

#include <string>

namespace docmala {

    struct FileLocation {
        FileLocation(int line, int column, const std::string &fileName)
            : line(line)
            , column(column)
            , fileName(fileName)
        {}

        FileLocation() = default;
        int line = 0;
        int column = 0;
        std::string fileName;
    };

}
