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
#include <algorithm>
#include <docmala/DocmaPlugin.h>
#include <docmala/Docmala.h>
#include <extension_system/Extension.hpp>
#include <memory>

using namespace docmala;

class IncludePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) override;

    PostProcessing postProcessing() const override;
    bool           postProcess(const ParameterList& parameters, const FileLocation& location, Document& document) override;

    static void escapeFileName(std::string& fileName) {
        std::replace(fileName.begin(), fileName.end(), '.', '_');
    }

    std::vector<Error> lastErrors() const override {
        return _errors;
    }

    std::vector<Error>       _errors;
    std::unique_ptr<Docmala> parser;
    void                     updateDocumentParts(const std::string&               identifier,
                                                 DocumentPart::GeneratedDocument& out,
                                                 const std::vector<DocumentPart>& parts,
                                                 bool                             keepHeadlineLevel,
                                                 int                              baseLevel);
    void                     postProcessParts(const std::string& identifier, std::vector<DocumentPart>& parts);
};

DocumentPlugin::BlockProcessing IncludePlugin::blockProcessing() const {
    return BlockProcessing::No;
}

void IncludePlugin::updateDocumentParts(const std::string&               identifier,
                                        DocumentPart::GeneratedDocument& out,
                                        const std::vector<DocumentPart>& parts,
                                        bool                             keepHeadlineLevel,
                                        int                              baseLevel) {
    int currentLevel = 0;
    for (auto& part : parts) {
        if (!keepHeadlineLevel && (part.type() == DocumentPart::Type::Headline)) {
            DocumentPart::Headline headline = *part.headline();
            headline.level += baseLevel;
            out.document.emplace_back(headline);
            currentLevel = headline.level;
        } else if (part.type() == DocumentPart::Type::Anchor) {
            auto anchor = *part.anchor();
            anchor.name = identifier + ":" + anchor.name;
            out.document.emplace_back(anchor);
        } else if (part.type() == DocumentPart::Type::GeneratedDocument) {
            auto                            generated = part.generatedDocument();
            DocumentPart::GeneratedDocument outDoc(part.generatedDocument()->location);
            updateDocumentParts(identifier, outDoc, generated->document, keepHeadlineLevel, currentLevel);
            out.document.emplace_back(outDoc);
        } else {
            out.document.push_back(part);
        }
    }
}

bool IncludePlugin::process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) {
    (void)block;

    _errors.clear();

    std::string pluginDir     = "./plugins";
    auto        pluginDirIter = parameters.find("pluginDir");
    if (pluginDirIter != parameters.end()) {
        pluginDir = pluginDirIter->second.value + '/';
    }

    std::string includeFile;
    std::string inputFile;
    bool        keepHeadlineLevel = false;

    auto inFileIter = parameters.find("inputFile");
    if (inFileIter != parameters.end()) {
        inputFile = inFileIter->second.value;
    } else {
        _errors.emplace_back(location, "Parameter 'inputFile' is missing.");
        return false;
    }

    std::string baseDir = inputFile.substr(0, inputFile.find_last_of("\\/"));

    auto includeFileIter = parameters.find("file");

    if (includeFileIter != parameters.end()) {
        includeFile = includeFileIter->second.value;
    } else {
        _errors.emplace_back(location, "Parameter 'file' is missing.");
        return false;
    }

    std::string identifier     = includeFile;
    auto        identifierIter = parameters.find("as");

    if (identifierIter != parameters.end()) {
        identifier = identifierIter->second.value;
    }

    if (parameters.find("keepHeadlineLevel") != parameters.end()) {
        keepHeadlineLevel = true;
    }

    if (!parser) {
        parser = std::make_unique<Docmala>(pluginDir);
    }

    parser->parseFile(baseDir + "/" + includeFile);

    _errors                                   = parser->errors();
    auto                            doc       = parser->document();
    int                             baseLevel = 1;
    DocumentPart::GeneratedDocument generated(location);

    if (!keepHeadlineLevel) {
        for (auto iter = document.parts().rbegin(); iter != document.parts().rend(); iter++) {
            if (iter->type() == DocumentPart::Type::Headline) {
                baseLevel = iter->headline()->level;
                break;
            }
        }
    }

    updateDocumentParts(identifier, generated, doc.parts(), keepHeadlineLevel, baseLevel);

    document.addPart(generated);

    return true;
}

DocumentPlugin::PostProcessing IncludePlugin::postProcessing() const {
    return PostProcessing::Once;
}

void IncludePlugin::postProcessParts(const std::string& identifier, std::vector<DocumentPart>& parts) {
    for (auto& part : parts) {
        if (part.type() == DocumentPart::Type::Link) {
            auto link = part.link();

            if (link->type == DocumentPart::Link::Type::InterFile) {
                std::string fileName = link->data.substr(0, link->data.find(':'));
                if (fileName == identifier) {
                    link->type = DocumentPart::Link::Type::IntraFile;
                }
            }
        } else if (part.generatedDocument()) {
            postProcessParts(identifier, part.generatedDocument()->document);
        } else if (part.text()) {
            postProcessParts(identifier, part.text()->text);
        } else if (part.table()) {
            auto table = part.table();
            for (auto row : table->cells) {
                for (auto cell : row) {
                    postProcessParts(identifier, cell.content);
                }
            }
        }
    }
}

bool IncludePlugin::postProcess(const ParameterList& parameters, const FileLocation& location, Document& document) {
    _errors.clear();

    std::string includeFile;
    std::string inputFile;

    auto inFileIter = parameters.find("inputFile");
    if (inFileIter != parameters.end()) {
        inputFile = inFileIter->second.value;
    } else {
        _errors.emplace_back(location, "Parameter 'inputFile' is missing.");
        return false;
    }

    std::string baseDir = inputFile.substr(0, inputFile.find_last_of("\\/"));

    auto includeFileIter = parameters.find("file");

    if (includeFileIter != parameters.end()) {
        includeFile = includeFileIter->second.value;
    } else {
        _errors.emplace_back(location, "Parameter 'file' is missing.");
        return false;
    }

    std::string identifier     = includeFile;
    auto        identifierIter = parameters.find("as");

    if (identifierIter != parameters.end()) {
        identifier = identifierIter->second.value;
    }

    postProcessParts(identifier, document.parts());
    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, IncludePlugin, "include", 1, "Include a file into an other one", EXTENSION_SYSTEM_NO_USER_DATA)
