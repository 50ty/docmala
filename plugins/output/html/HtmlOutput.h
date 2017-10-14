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

#include "HtmlOutput_global.h"
#include <vector>
#include <string>
#include <map>
#include <docmala/Document.h>
#include <docmala/Parameter.h>

namespace docmala {

class HTMLOUTPUT_API HtmlOutput {
    // OutputPlugin interface
public:
    struct HtmlDocument {
        std::string head;
        std::string body;
    };

    HtmlDocument produceHtml(const ParameterList& parameters, const Document& document, const std::string& scripts = "");

private:
    void prepare(const std::vector<document_part::Variant>& documentParts);

    void writeDocumentParts(std::stringstream& outFile, const std::vector<document_part::Variant>& documentParts, bool isGenerated = false);

    void writeTable(std::stringstream& outFile, const document_part::Table& table);
    void writeList(std::stringstream& outFile, const document_part::List& list, bool isGenerated);
    void writeListEntries(std::stringstream& outFile, const std::vector<document_part::List::Entry>& entries, bool isGenerated);

    unsigned int         _imageCounter                       = 1;
    unsigned int         _figureCounter                      = 1;
    unsigned int         _listingCounter                     = 1;
    unsigned int         _tableCounter                       = 1;
    static constexpr int _maxHeadlineLevels                  = 32;
    int                  _headlineLevels[_maxHeadlineLevels] = {0};

    std::string _nameBase    = "outfile";
    bool        _embedImages = false;

    struct TitleData {
        std::string        id;
        document_part::Text text;
    };

    std::map<std::string, TitleData>  _anchorData;
    std::map<FileLocation, TitleData> _titleData;
};
}
