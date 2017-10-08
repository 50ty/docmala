/**
    @file
    @copyright
        Copyright (C) 2017 Michael Adam
        Copyright (C) 2017 Bernd Amend
        Copyright (C) 2017 Stefan Rommel

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
