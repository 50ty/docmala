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
