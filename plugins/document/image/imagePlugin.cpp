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
#include <docmala/DocmaPlugin.h>
#include <docmala/Error.h>
#include <extension_system/Extension.hpp>
#include <fstream>
#include <unordered_map>

using namespace docmala;

class ImagePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    std::vector<Error> process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) override;

    std::unordered_map<std::string, DocumentPart::Image> _cache;
};

DocumentPlugin::BlockProcessing ImagePlugin::blockProcessing() const {
    return BlockProcessing::No;
}

std::vector<Error> ImagePlugin::process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) {
    (void)block;

    std::vector<Error> errors;

    errors.clear();
    std::string inputFile;

    auto inFileIter = parameters.find("inputFile");
    if (inFileIter != parameters.end()) {
        inputFile = inFileIter->second.value;
    } else {
        errors.emplace_back(location, "Parameter 'inputFile' is missing.");
        return errors;
    }

    std::string baseDir = inputFile.substr(0, inputFile.find_last_of("\\/"));

    std::string fileName;
    auto        fileNameIter = parameters.find("file");
    if (fileNameIter != parameters.end()) {
        fileName = baseDir + "/" + fileNameIter->second.value;
    } else {
        errors.emplace_back(location, "Parameter 'file' is missing.");
        return errors;
    }

    std::string fileExtension;
    auto        pos = fileName.find_last_of('.');
    if (pos != std::string::npos) {
        fileExtension = fileName.substr(pos + 1);
    } else {
        errors.emplace_back(location, "Unable to determine file type.");
        return errors;
    }

    std::string imageData;

    std::ifstream highlightReader(fileName, std::ios::in | std::ios::binary);
    if (highlightReader) {
        highlightReader.seekg(0, std::ios::end);
        imageData.resize(static_cast<std::string::size_type>(highlightReader.tellg()));
        highlightReader.seekg(0, std::ios::beg);
        highlightReader.read(&imageData[0], static_cast<std::streamsize>(imageData.size()));
        highlightReader.close();
    } else {
        errors.emplace_back(location, "Unable to open file '" + fileName + "'.");
    }

    std::string format = fileExtension;

    if (format == "svg") {
        if (imageData.find("<?xml") != std::string::npos) {
            format += "+xml";
        }
    }

    DocumentPart::Text text;
    text.text.emplace_back(fileName);
    DocumentPart::Image image(format, fileExtension, imageData, text);
    _cache.insert(std::make_pair(block, image));
    document.addPart(image);

    return errors;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, ImagePlugin, "image", 1, "Insert an image into the document", EXTENSION_SYSTEM_NO_USER_DATA)
