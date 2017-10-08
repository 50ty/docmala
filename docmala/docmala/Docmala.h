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
#include <map>
#include <fstream>
#include <memory>

#include "docmala_global.h"
#include "Parameter.h"
#include "Document.h"
#include "Error.h"

namespace extension_system {
class ExtensionSystem;
}
namespace docmala {
class IFile;
class DocumentPlugin;
class OutputPlugin;

class DOCMALA_API Docmala {
public:
    Docmala(const std::string& pluginDir = "./");
    Docmala(const Document other, const std::string& pluginDir = "./");
    ~Docmala();

    void setParameters(const ParameterList& parameters);
    bool parseFile(const std::string& fileName);
    bool parseData(const std::string& data, const std::string& fileName = "");

    bool produceOutput(const std::string& pluginName);
    bool produceOutput(std::shared_ptr<OutputPlugin> plugin);

    std::vector<std::string> listOutputPlugins() const;

    std::vector<Error> errors() const {
        return _errors;
    }

    const Document& document() const {
        return _document;
    }

    const std::string& pluginDir() const {
        return _pluginDir;
    }
    void readComment();

    static bool readText(IFile* file, std::vector<Error>& errors, char startCharacter, DocumentPart::Text& text);

private:
    bool parse();
    void doPostprocessing();
    void postProcessPartList(const std::vector<DocumentPart>& parts);
    void checkConsistency();

    bool readHeadLine();
    bool readCaption();
    bool readLine(std::string& destination);
    bool readPlugin();
    // bool readAnchor();
    static bool readAnchor(IFile* file, std::vector<Error>& errors, DocumentPart::Text& outText);
    bool        readLink(DocumentPart::Text& outText);
    static bool readLink(IFile* file, std::vector<Error>& errors, DocumentPart::Text& outText);
    bool        readMetaData();
    bool        readText(char startCharacter, DocumentPart::Text& text);

    bool readParameterList(ParameterList& parameters, char blockEnd);
    bool readBlock(std::string& block);
    bool readList(DocumentPart::List::Type type);

    static bool isWhitespace(char c, bool allowEndline = false);

    /**
     * A document consists of many document parts
     * All of these parts are stored in this variable
     */
    Document                                               _document;
    std::unique_ptr<IFile>                                 _file;
    std::vector<Error>                                     _errors;
    std::unique_ptr<extension_system::ExtensionSystem>     _pluginLoader;
    std::string                                            _outputDir;
    std::map<std::string, std::shared_ptr<DocumentPlugin>> _loadedDocumentPlugins;
    ParameterList                                          _parameters;

    struct PostProcessingInfo {
        std::shared_ptr<DocumentPlugin> plugin;
        ParameterList                   parameters;
        FileLocation                    location;
        bool                            processed = false;
    };

    std::vector<PostProcessingInfo> _registeredPostprocessing;
    std::string                     _pluginDir;
};
}
