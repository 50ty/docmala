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
#include <extension_system/Extension.hpp>
#include <fstream>
#include <sstream>

#include "HtmlOutput.h"

namespace {
const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789+/";

std::string base64_encode(const std::string& inputData) {
    std::string   ret;
    int           i = 0;
    int           j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (char data : inputData) {
        char_array_3[i++] = static_cast<unsigned char>(data);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = static_cast<unsigned char>(((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4));
            char_array_4[2] = static_cast<unsigned char>(((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6));
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++) {
                ret += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i != 0) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = static_cast<unsigned char>(((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4));
        char_array_4[2] = static_cast<unsigned char>(((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6));
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) {
            ret += base64_chars[char_array_4[j]];
        }

        while ((i++ < 3)) {
            ret += '=';
        }
    }

    return ret;
}

std::string escapeAnchor(const std::string& anchor) {
    std::string escape = anchor;
    std::replace(escape.begin(), escape.end(), '.', 'd');
    std::replace(escape.begin(), escape.end(), ':', 'c');
    return escape;
}
} // namespace

using namespace docmala;

class HtmlOutputPlugin : public OutputPlugin {
    // OutputPlugin interface
public:
    bool write(const ParameterList& parameters, const Document& document) override;
};

void replaceAll(std::string& source, const std::string& from, const std::string& to) {
    std::string newString;
    newString.reserve(source.length()); // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos = 0;

    while (std::string::npos != (findPos = source.find(from, lastPos))) {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    newString += source.substr(lastPos);

    source.swap(newString);
}

std::string id(const document_part::VisualElement& element) {
    if (element.location.valid()) {
        return std::string(" id=\"line_") + std::to_string(element.location.line) + "\"";
    }
    return "";
}

void writeFormatedText(std::stringstream& outFile, const document_part::FormatedText& text) {
    if (text.bold) {
        outFile << "<b>";
    }
    if (text.italic) {
        outFile << "<i>";
    }
    if (text.underlined) {
        outFile << "<u>";
    }
    if (text.stroked) {
        outFile << "<del>";
    }
    if (text.monospaced) {
        outFile << "<tt>";
    }
    std::string txt = text.text;
    replaceAll(txt, "<", "&lt;");
    replaceAll(txt, ">", "&gt;");
    outFile << txt;

    if (text.bold) {
        outFile << "</b>";
    }
    if (text.italic) {
        outFile << "</i>";
    }
    if (text.stroked) {
        outFile << "</del>";
    }
    if (text.monospaced) {
        outFile << "</tt>";
    }
    if (text.underlined) {
        outFile << "</u>";
    }
}

void writeCode(std::stringstream& outFile, const document_part::Code& code) {
    if (!code.type.empty()) {
        outFile << "<pre" << id(code) << "> <code class=\"" << code.type << "\">\n";
    } else {
        outFile << "<pre> <code>\n";
    }

    std::string cde = code.code;

    replaceAll(cde, "<", "&lt;");
    replaceAll(cde, ">", "&gt;");

    outFile << cde;
    outFile << "</code> </pre>\n";
}

void HtmlOutput::writeTable(std::stringstream& outFile, const document_part::Table& table) {
    outFile << "<table>\n";
    bool firstRow = true;
    for (const auto& row : table.cells) {
        bool firstColumn = true;
        outFile << "<tr>\n";
        std::string end;

        for (const auto& cell : row) {
            std::string span;
            if (cell.isHiddenBySpan) {
                continue;
            }

            if (cell.columnSpan > 0) {
                span = " colspan=\"" + std::to_string(cell.columnSpan + 1) + "\"";
            }
            if (cell.rowSpan > 0) {
                span += " rowspan=\"" + std::to_string(cell.rowSpan + 1) + "\"";
            }
            if (cell.isHeading && firstRow) {
                outFile << "<th scope=\"col\"" + span + ">\n";
                end = "</th>";
            } else if (cell.isHeading && firstColumn) {
                outFile << "<th scope=\"row\"" + span + ">\n";
                end = "</th>";
            } else {
                outFile << "<td" + span + ">\n";
                end = "</td>";
            }

            writeDocumentParts(outFile, cell.content, true);
            outFile << "\n" << end << "\n";

            firstColumn = false;
        }
        outFile << "</tr>\n";
        firstRow = false;
    }
    outFile << "</table>\n";
}

void HtmlOutput::writeListEntries(std::stringstream& outFile, const std::vector<document_part::List::Entry>& entries, bool isGenerated) {
    if (entries.empty())
        return;

    std::string type = "ul";
    std::string style;
    switch (entries.front().type) {
        case document_part::List::Type::Points:
            type = "ul";
            break;
        case document_part::List::Type::Dashes:
            type  = "ul";
            style = "class=\"dash\"";
            break;
        case document_part::List::Type::Numbered:
            type = "ol";
            break;
    }
    outFile << "<" << type << " " << style << ">\n";

    for (const auto& entry : entries) {
        outFile << "<li> ";
        writeDocumentParts(outFile, {entry.text}, isGenerated);
        writeListEntries(outFile, entry.entries, isGenerated);
        outFile << " </li>\n";
    }
    outFile << "</" << type << ">\n";
}

void HtmlOutput::writeList(std::stringstream& outFile, const document_part::List& list, bool isGenerated) {
    writeListEntries(outFile, list.entries, isGenerated);
}

void HtmlOutput::prepare(const std::vector<document_part::Variant>& documentParts) {
    auto previous = documentParts.end();

    for (auto part = documentParts.begin(); part != documentParts.end(); previous = part, part++) {
        if (auto doc = boost::get<document_part::GeneratedDocument>(&*part)) {
            prepare(doc->document);
        }
        auto headline = boost::get<document_part::Headline>(&*part);
        auto caption  = boost::get<document_part::Caption>(&*part);

        if (headline) {
            for (int level = headline->level; level < _maxHeadlineLevels; level++) {
                _headlineLevels[level] = 0;
            }
            _headlineLevels[headline->level - 1]++;
        }

        if (headline || caption) {
            document_part::Anchor const* anchor = nullptr;

            if (previous != documentParts.end()) {
                auto previousText = boost::get<document_part::Text>(&*previous);
                if (previousText && !previousText->text.empty()) {
                    anchor = boost::get<document_part::Anchor>(&previousText->text.front());
                }
            }

            TitleData    title;
            FileLocation location;
            if (caption) {
                auto next = part + 1;
                if (next != documentParts.end()) {
                    if (auto image = boost::get<document_part::Image>(&*next)) {
                        location            = image->location;
                        title.id            = "Figure " + std::to_string(_figureCounter);
                        title.text.text     = caption->text;
                        title.text.location = caption->location;
                        _figureCounter++;
                    } else if (auto table = boost::get<document_part::Table>(&*next)) {
                        location            = table->location;
                        title.id            = "Table " + std::to_string(_tableCounter);
                        title.text.text     = caption->text;
                        title.text.location = caption->location;
                        _tableCounter++;
                    } else if (auto code = boost::get<document_part::Code>(&*next)) {
                        location            = code->location;
                        title.id            = "Listing " + std::to_string(_listingCounter);
                        title.text.text     = caption->text;
                        title.text.location = caption->location;
                        _listingCounter++;
                    } else {
                        location            = caption->location;
                        title.text.text     = caption->text;
                        title.text.location = caption->location;
                    }
                }
            } else if (headline) {
                location = headline->location;
                // title.id = "Chapter ";
                for (int level = 0; level < headline->level; level++) {
                    title.id += std::to_string(_headlineLevels[level]) + ".";
                }
                title.text.text     = headline->text;
                title.text.location = headline->location;
            }

            if (!title.id.empty() || !title.text.text.empty()) {
                _titleData.insert(std::make_pair(location, title));
            }

            if (anchor) {
                _anchorData.insert(std::make_pair(anchor->name, title));
            }
        }
    }
}

void HtmlOutput::writeDocumentParts(std::stringstream& outFile, const std::vector<document_part::Variant>& documentParts, bool isGenerated) {
    bool paragraphOpen = false;
    auto previous      = documentParts.end();
    auto part          = documentParts.begin();

    auto visitor = make_visitor(
        // visitors
        [&](const document_part::Headline& headline) {
            if (paragraphOpen) {
                outFile << "</p>\n";
                paragraphOpen = false;
            }
            auto headlineTitle = _titleData.find(headline.location);

            outFile << "<h" << headline.level << ">";
            if (!isGenerated) {
                outFile << "<span " << id(headline) << ">";
            }
            if (headlineTitle != _titleData.end()) {
                outFile << headlineTitle->second.id << " ";
            }
            writeDocumentParts(outFile, headline.text, isGenerated);
            if (!isGenerated) {
                outFile << "</span>\n";
            }
            outFile << "</h" << headline.level << ">\n";
        },
        [&](const document_part::FormatedText& text) { writeFormatedText(outFile, text); },
        [&](const document_part::Caption& caption) {
            if (_titleData.find(caption.location) != _titleData.end()) {
                outFile << "<span style=\"font-size:110%;font-weight:bold;\" ";

                if (!isGenerated) {
                    outFile << id(caption);
                }
                outFile << ">";

                writeDocumentParts(outFile, caption.text, isGenerated);

                if (!isGenerated) {
                    outFile << "</span>\n";
                }
            }
        },
        [&](const document_part::Text& text) {
            if (!isGenerated) {
                outFile << "<span " << id(text) << ">";
            }

            writeDocumentParts(outFile, text.text, isGenerated);

            if (!isGenerated) {
                outFile << "</span>\n";
            }
        },
        [&](const document_part::Paragraph&) {
            if (paragraphOpen) {
                outFile << "</p>\n";
                paragraphOpen = false;
            }
            outFile << "<p>\n";
            paragraphOpen = true;
        },
        [&](const document_part::Image& image) {
            outFile << "<figure" << id(image) << ">\n";
            if (_embedImages) {
                outFile << "<img src=\"data:image/" << image.format << ";base64,";
                outFile << base64_encode(image.data);
                outFile << "\">";
            } else {
                std::ofstream     imgFile;
                std::stringstream fileName;

                fileName << _nameBase << "_image_" << _imageCounter << "." << image.fileExtension;

                imgFile.open(fileName.str(), std::ofstream::binary | std::ofstream::out);
                imgFile << image.data;
                imgFile.close();

                std::string imageImportName = fileName.str();
                std::replace(imageImportName.begin(), imageImportName.end(), '\\', '/');
                if (imageImportName.find_last_of('/') != std::string::npos) {
                    imageImportName = imageImportName.substr(imageImportName.find_last_of('/') + 1);
                }

                outFile << "<img src=\"" << imageImportName << "\">\n";
            }
            auto title = _titleData.find(image.location);
            if (title != _titleData.end()) {
                outFile << "<figcaption>" << title->second.id << ": ";
                if (!isGenerated) {
                    if (auto caption = boost::get<document_part::Caption>(&*previous)) {
                        outFile << "<span " << id(*caption) << ">";
                    }
                }
                writeDocumentParts(outFile, title->second.text.text, isGenerated);
                if (!isGenerated) {
                    outFile << "</span>\n";
                }
                outFile << "</figcaption>\n";
            }
            outFile << "</figure>\n";
            _imageCounter++;
        },
        [&](const document_part::List& list) { writeList(outFile, list, isGenerated); },
        [&](const document_part::GeneratedDocument& doc) {
            outFile << "<div " << id(doc) << ">\n";
            writeDocumentParts(outFile, doc.document, true);
            outFile << "</div>\n";
        },
        [&](const document_part::Code& code) {
            outFile << "<figure" << id(code) << ">\n";
            writeCode(outFile, code);
            auto title = _titleData.find(code.location);
            if (title != _titleData.end()) {
                outFile << "<figcaption>" << title->second.id << ": ";
                if (!isGenerated) {
                    if (auto caption = boost::get<document_part::Caption>(&*previous)) {
                        outFile << "<span " << id(*caption) << ">";
                    }
                }
                writeDocumentParts(outFile, title->second.text.text, isGenerated);
                if (!isGenerated) {
                    outFile << "</span>\n";
                }
                outFile << "</figcaption>\n";
            }

            outFile << "</figure>\n";
        },
        [&](const document_part::Anchor& anchor) { outFile << "<a id=\"" << escapeAnchor(anchor.name) << "\"/>\n"; },
        [&](const document_part::Link& link) {
            std::string text = link.text;
            TitleData   titleData;

            if (text.empty()) {
                text = link.data;
            } else if (text == "#" || text == "*" || text == "#*") {
                auto anchorData = _anchorData.find(link.data);
                if (anchorData != _anchorData.end()) {
                    titleData = anchorData->second;
                }
            }

            if (link.type == document_part::Link::Type::IntraFile) {
                outFile << "<a href=\"#" << escapeAnchor(link.data) << "\">";
                if ((text == "#" || text == "#*") && !titleData.id.empty()) {
                    outFile << titleData.id;
                    if (text == "#*") {
                        if (titleData.id.back() != '.') {
                            outFile << ":";
                        }
                        outFile << " ";
                    }
                }

                if (text == "*" || link.text == "#*" || titleData.id.empty()) {
                    writeDocumentParts(outFile, titleData.text.text, true);
                } else if (text != "#") {
                    outFile << text;
                }
                outFile << "</a>\n";
            } else if (link.type == document_part::Link::Type::InterFile) {
                std::string data     = link.data;
                data[data.find(':')] = '#';
                outFile << "<a href=\"" << data << "\">" << text << "</a>\n";
            } else if (link.type == document_part::Link::Type::Web) {
                outFile << "<a href=\"" << link.data << "\">" << text << "</a>\n";
            }
        },
        [&](const document_part::Table& table) {
            outFile << "<figure" << id(table) << ">\n";
            writeTable(outFile, table);
            auto title = _titleData.find(table.location);
            if (title != _titleData.end()) {
                outFile << "<figcaption>" << title->second.id << ": ";

                if (!isGenerated) {
                    if (auto caption = boost::get<document_part::Caption>(&*previous)) {
                        outFile << "<span " << id(*caption) << ">";
                    }
                }
                writeDocumentParts(outFile, title->second.text.text, isGenerated);
                if (!isGenerated) {
                    outFile << "</span>\n";
                }
                outFile << "</figcaption>\n";
            }

            outFile << "</figure>\n";
        },
        [](const auto&) {});

    for (; part != documentParts.end(); part++) {
        boost::apply_visitor(visitor, *part);
        previous = part;
    }

    if (paragraphOpen) {
        outFile << "</p>\n";
        paragraphOpen = false;
    }
}

HtmlOutput::HtmlDocument HtmlOutput::produceHtml(const ParameterList& parameters, const Document& document, const std::string& scripts) {
    std::string outputFileName = "outfile.html";

    auto inputFile = parameters.find("inputFile");

    if (inputFile != parameters.end()) {
        outputFileName = inputFile->second.value;
        _nameBase      = outputFileName.substr(0, outputFileName.find_last_of('.'));
        outputFileName = _nameBase + ".html";
    }

    std::string pluginDir     = "./";
    auto        pluginDirIter = parameters.find("pluginDir");
    if (pluginDirIter != parameters.end()) {
        pluginDir = pluginDirIter->second.value + '/';
    }

    _embedImages = parameters.find("embedImages") != parameters.end();

    std::string codeHighlightScript;
    std::string codeHighlightCSS;
    std::string generalCSS;

    std::ifstream highlightReader(pluginDir + "outputPluginHtmlCodeHighlight.js", std::ios::in | std::ios::binary);
    if (highlightReader) {
        highlightReader.seekg(0, std::ios::end);
        codeHighlightScript.resize(static_cast<std::string::size_type>(highlightReader.tellg()));
        highlightReader.seekg(0, std::ios::beg);
        highlightReader.read(&codeHighlightScript[0], static_cast<std::streamsize>(codeHighlightScript.size()));
        highlightReader.close();

        replaceAll(codeHighlightScript, "<script", "&lt;script");
        replaceAll(codeHighlightScript, "</script", "&lt;/script");
    }

    std::ifstream codeHighlightReader(pluginDir + "outputPluginHtmlDefaultStyle.css", std::ios::in | std::ios::binary);
    if (codeHighlightReader) {
        codeHighlightReader.seekg(0, std::ios::end);
        codeHighlightCSS.resize(static_cast<std::string::size_type>(codeHighlightReader.tellg()));
        codeHighlightReader.seekg(0, std::ios::beg);
        codeHighlightReader.read(&codeHighlightCSS[0], static_cast<std::streamsize>(codeHighlightCSS.size()));
        codeHighlightReader.close();
    }

    std::ifstream generalCSSReader(pluginDir + "outputPluginHtmlCodeHighlight.css", std::ios::in | std::ios::binary);
    if (generalCSSReader) {
        generalCSSReader.seekg(0, std::ios::end);
        generalCSS.resize(static_cast<std::string::size_type>(generalCSSReader.tellg()));
        generalCSSReader.seekg(0, std::ios::beg);
        generalCSSReader.read(&generalCSS[0], static_cast<std::streamsize>(generalCSS.size()));
        generalCSSReader.close();
    }

    HtmlDocument html;

    std::stringstream head;
    std::stringstream body;

    head << "<meta charset=\"utf-8\">\n";
    if (document.metaData().find("title") != document.metaData().end()) {
        head << "<title>" + document.metaData().at("title").data.front().value + "</title>\n";
    } else {
        head << "<title>[No title]</title>\n";
    }

    head << "<style>\n";
    head << codeHighlightCSS << "\n";
    head << generalCSS << "\n";
    head << "</style>\n";

    head << "<script>\n";
    head << scripts << "\n";
    head << "</script>\n";
    head << "<script>\n";
    head << codeHighlightScript << "\n";
    head << "</script>\n";
    head << "<script>hljs.initHighlightingOnLoad();</script>\n";

    prepare(document.parts());
    writeDocumentParts(body, document.parts());

    html.head = head.str();
    html.body = body.str();
    return html;
}



bool HtmlOutputPlugin::write(const ParameterList &parameters, const Document &document) {
    std::string nameBase       = "outfile";
    std::string outputFileName = "outfile.html";

    auto inputFile = parameters.find("inputFile");

    if (inputFile != parameters.end()) {
        outputFileName = inputFile->second.value;
        nameBase       = outputFileName.substr(0, outputFileName.find_last_of('.'));
        outputFileName = nameBase + ".html";
    }

    std::ofstream outFile;

    outFile.open(outputFileName);

    if (outFile.is_open()) {
        HtmlOutput output;
        auto       html = output.produceHtml(parameters, document);

        outFile << "<!doctype html>\n";
        outFile << "<html>\n";
        outFile << "<head>\n";
        outFile << html.head << "\n";
        outFile << "</head>\n";

        outFile << "<body>\n";
        outFile << html.body << "\n";
        outFile << "</body>\n";
        return true;
    }
    return false;
}

EXTENSION_SYSTEM_EXTENSION(docmala::OutputPlugin, HtmlOutputPlugin, "html", 1, "Write document to a HTML file", EXTENSION_SYSTEM_NO_USER_DATA)
