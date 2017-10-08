#pragma once

#include <string>

namespace docmala {

struct FileLocation {
    FileLocation(int line, int column, const std::string& fileName)
        : line(line)
        , column(column)
        , fileName(fileName) {}

    FileLocation()       = default;
    int         line     = 0;
    int         column   = 0;
    std::string fileName = "internal";

    bool valid() const {
        return line != 0;
    }

    bool operator<(const FileLocation& other) const {
        if (fileName != other.fileName)
            return fileName < other.fileName;
        if (line != other.line)
            return line < other.line;
        return column < other.column;
    }

    bool operator==(const FileLocation& other) const {
        return line == other.line && column == other.column && fileName == other.fileName;
    }

    bool operator!=(const FileLocation& other) const {
        return !(*this == other);
    }
};
}
