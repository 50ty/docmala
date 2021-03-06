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
    std::vector<Error> process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) override;

    PostProcessing     postProcessing() const override;
    std::vector<Error> postProcess(const ParameterList& parameters, const FileLocation& location, Document& document) override;

    static void escapeFileName(std::string& fileName) {
        std::replace(fileName.begin(), fileName.end(), '.', '_');
    }

    std::unique_ptr<Docmala> parser;

    void updateDocumentParts(const std::string&                        identifier,
                             document_part::GeneratedDocument&          out,
                             const std::vector<document_part::Variant>& parts,
                             bool                                      keepHeadlineLevel,
                             int                                       baseLevel);
    void postProcessParts(const std::string& identifier, std::vector<document_part::Variant>& parts);
};

DocumentPlugin::BlockProcessing IncludePlugin::blockProcessing() const {
    return BlockProcessing::No;
}

void IncludePlugin::updateDocumentParts(const std::string&                        identifier,
                                        document_part::GeneratedDocument&          out,
                                        const std::vector<document_part::Variant>& parts,
                                        bool                                      keepHeadlineLevel,
                                        int                                       baseLevel) {
    int  currentLevel = 0;
    auto visitor      = make_visitor(
        // visitors
        [&](document_part::Headline& headline) {
            if (!keepHeadlineLevel) {
                headline.level += baseLevel;
                out.document.emplace_back(headline);
                currentLevel = headline.level;
            }
        },
        [&](document_part::Anchor& anchor) {
            anchor.name = identifier + ":" + anchor.name;
            out.document.emplace_back(anchor);

        },
        [&](const document_part::GeneratedDocument& doc) {
            document_part::GeneratedDocument outDoc(doc.location);
            updateDocumentParts(identifier, outDoc, doc.document, keepHeadlineLevel, currentLevel);
            out.document.emplace_back(outDoc);
        },
        [&](const auto& part) { out.document.push_back(part); });

    for (auto& part : parts) {
        boost::apply_visitor(visitor, part);
    }
}

std::vector<Error>
IncludePlugin::process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) {
    (void)block;

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
        return {{location, "Parameter 'inputFile' is missing."}};
    }

    std::string baseDir = inputFile.substr(0, inputFile.find_last_of("\\/"));

    auto includeFileIter = parameters.find("file");

    if (includeFileIter != parameters.end()) {
        includeFile = includeFileIter->second.value;
    } else {
        return {{location, "Parameter 'file' is missing."}};
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

    auto                            errors    = parser->errors();
    auto                            doc       = parser->document();
    int                             baseLevel = 1;
    document_part::GeneratedDocument generated(location);

    if (!keepHeadlineLevel) {
        for (auto iter = document.parts().rbegin(); iter != document.parts().rend(); iter++) {
            auto headline = boost::get<document_part::Headline>(&(*iter));
            if (headline) {
                baseLevel = headline->level;
                break;
            }
        }
    }

    updateDocumentParts(identifier, generated, doc.parts(), keepHeadlineLevel, baseLevel);

    document.addPart(generated);

    return errors;
}

DocumentPlugin::PostProcessing IncludePlugin::postProcessing() const {
    return PostProcessing::Once;
}

void IncludePlugin::postProcessParts(const std::string& identifier, std::vector<document_part::Variant>& parts) {

    auto visitor = make_visitor(
        // visitors
        [&](document_part::Link& link) {
            if (link.type == document_part::Link::Type::InterFile) {
                std::string fileName = link.data.substr(0, link.data.find(':'));
                if (fileName == identifier) {
                    link.type = document_part::Link::Type::IntraFile;
                }
            }
        },
        [&](document_part::GeneratedDocument& doc) { postProcessParts(identifier, doc.document); },
        [&](document_part::Text& text) { postProcessParts(identifier, text.text); },
        [&](const document_part::Table& table) {
            for (auto row : table.cells) {
                for (auto cell : row) {
                    postProcessParts(identifier, cell.content);
                }
            }
        },
        [](const auto&) {});

    for (auto& part : parts) {
        boost::apply_visitor(visitor, part);
    }
}

std::vector<Error> IncludePlugin::postProcess(const ParameterList& parameters, const FileLocation& location, Document& document) {
    std::string includeFile;
    std::string inputFile;

    auto inFileIter = parameters.find("inputFile");
    if (inFileIter != parameters.end()) {
        inputFile = inFileIter->second.value;
    } else {
        return {{location, "Parameter 'inputFile' is missing."}};
    }

    std::string baseDir = inputFile.substr(0, inputFile.find_last_of("\\/"));

    auto includeFileIter = parameters.find("file");

    if (includeFileIter != parameters.end()) {
        includeFile = includeFileIter->second.value;
    } else {
        return {{location, "Parameter 'file' is missing."}};
    }

    std::string identifier     = includeFile;
    auto        identifierIter = parameters.find("as");

    if (identifierIter != parameters.end()) {
        identifier = identifierIter->second.value;
    }

    postProcessParts(identifier, document.parts());
    return {};
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, IncludePlugin, "include", 1, "Include a file into an other one", EXTENSION_SYSTEM_NO_USER_DATA)
