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
#include <vector>
#include "FileLocation.h"

namespace docmala {

struct ErrorData {
    FileLocation location;
    std::string  message;
};

struct Error : public ErrorData {
    Error(const FileLocation& loc, const std::string& message)
        : ErrorData({loc, message}) {}

    Error(const FileLocation& loc, const std::string& message, const std::vector<ErrorData>& extended)
        : ErrorData({loc, message})
        , extendedInformation(extended) {}

    std::vector<ErrorData> extendedInformation;
};
}
