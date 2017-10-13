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
#include "Docmala.h"
#include "DocmaPlugin.h"
#include "File.h"

#include <extension_system/ExtensionSystem.hpp>
#include <memory>

using namespace docmala;

Docmala::Docmala(const std::string& pluginDir)
    : _pluginLoader(new extension_system::ExtensionSystem())
    , _pluginDir(pluginDir) {
    _pluginLoader->searchDirectory(pluginDir, false);
}

Docmala::Docmala(const Document& other, const std::string& pluginDir)
    : Docmala(pluginDir) {
    _document.inheritFrom(other);
}

Docmala::~Docmala() = default;

void Docmala::setParameters(const ParameterList& parameters) {
    _parameters = parameters;
}

bool Docmala::parseFile(const std::string& fileName) {
    _file = std::make_unique<File>(fileName);
    return parse();
}

bool Docmala::parseData(const std::string& data, const std::string& fileName) {
    _file = std::make_unique<MemoryFile>(data, fileName);
    return parse();
}

bool Docmala::produceOutput(const std::string& pluginName) {
    auto plugin = _pluginLoader->createExtension<OutputPlugin>(pluginName);

    if (plugin) {
        produceOutput(plugin);
    } else {
        _errors.emplace_back(FileLocation(), "Unable to load output plugin '" + pluginName + "'.");
        return false;
    }
    return true;
}

bool Docmala::produceOutput(const std::shared_ptr<OutputPlugin>& plugin) {
    if (plugin) {
        ParameterList parameters = _parameters;

        parameters.insert(std::make_pair("inputFile", Parameter{"inputFile", _file->fileName(), FileLocation()}));
        plugin->write(parameters, _document);
        return true;
    }
    return false;
}

std::vector<std::string> Docmala::listOutputPlugins() const {
    auto                     plugins = _pluginLoader->extensions<OutputPlugin>();
    std::vector<std::string> knownOutputPlugins;

    knownOutputPlugins.reserve(plugins.size());
    for (const auto& plugin : plugins) {
        knownOutputPlugins.push_back(plugin.name() + "  -  " + plugin.description());
    }

    return knownOutputPlugins;
}

void Docmala::readComment() {
    while (!_file->isEoF()) {
        char c = _file->getch();
        if (c == '\n') {
            break;
        }
    }
}

bool Docmala::parse() {
    _document.clear();
    _errors.clear();
    _registeredPostprocessing.clear();

    if (!_file->isOpen()) {
        _errors.emplace_back(FileLocation(), "Unable to open file '" + _file->fileName() + "'.");
        return false;
    }

    enum class Mode { BeginOfLine } mode{Mode::BeginOfLine};

    while (!_file->isEoF()) {
        char c = _file->getch();

        if (mode == Mode::BeginOfLine) {
            switch (c) {
                case ' ':
                case '\t':
                    continue;
                case '\n':
                    if (!_document.last<DocumentPart::Paragraph>()) {
                        _document.addPart(DocumentPart::Paragraph());
                    }
                    continue;
                case '=':
                    readHeadLine();
                    break;
                case ';':
                    readComment();
                    break;
                case '.':
                    readCaption();
                    break;
                case '{':
                    readMetaData();
                    break;
                case '[':
                    if (_file->following() == '[') {
                        DocumentPart::Text text;
                        readText(c, text);
                        _document.addPart(text);
                    } else {
                        readPlugin();
                    }
                    break;
                case '#':
                    if (isWhitespace(_file->following()) || _file->following() == '#') {
                        readList(DocumentPart::List::Type::Numbered);
                    } else {
                        DocumentPart::Text text;
                        readText(c, text);
                        _document.addPart(text);
                    }
                    break;
                case '*':
                    if (isWhitespace(_file->following()) || _file->following() == '*') {
                        readList(DocumentPart::List::Type::Points);
                    } else {
                        DocumentPart::Text text;
                        readText(c, text);
                        _document.addPart(text);
                    }
                    break;
                case '<':
                default: {
                    DocumentPart::Text text;
                    readText(c, text);
                    _document.addPart(text);
                }
            }
        }
    }
    doPostprocessing();
    checkConsistency();
    return true;
}

void Docmala::doPostprocessing() {
    bool documentChanged = true;
    while (documentChanged) {
        documentChanged = false;
        for (auto& post : _registeredPostprocessing) {
            if (post.plugin->postProcessing() == DocumentPlugin::PostProcessing::DocumentChanged || !post.processed) {
                documentChanged = true;
                auto errors     = post.plugin->postProcess(post.parameters, post.location, _document);
                if (!errors.empty()) {
                    for (auto& error : errors) {
                        error.message = "    " + error.message;
                    }
                    _errors.emplace_back(post.location, "Errors occured during plugin execution");
                    _errors.insert(_errors.end(), errors.begin(), errors.end());
                }
                post.processed = true;
            }
        }
    }

    postProcessPartList(_document.parts());
}

void Docmala::postProcessPartList(const std::vector<DocumentPart::Variant>& parts) {
    auto visitor = make_visitor(
        // visitors
        [this](DocumentPart::Anchor& anchor) {
            auto prevAnchor = _document.anchors().find(anchor.name);
            if (prevAnchor != _document.anchors().end() && prevAnchor->second.location != anchor.location) {
                auto      loc = prevAnchor->second.location;
                ErrorData additionalInfo{loc,
                                         std::string("Previous definition of '") + anchor.name + "' is at " + loc.fileName + "("
                                             + std::to_string(loc.line) + ":" + std::to_string(loc.column) + ")"};
                _errors.push_back(
                    Error{anchor.location, std::string("Anchor with name '") + anchor.name + "' already defined.", {additionalInfo}});
            }

        },
        [this](const DocumentPart::Text& text) { postProcessPartList(text.text); },
        [this](const DocumentPart::Table& table) {
            for (auto row : table.cells) {
                for (auto cell : row) {
                    postProcessPartList(cell.content);
                }
            }
        },
        [](const auto&) {});
    for (auto part : parts) {
        boost::apply_visitor(visitor, part);
    }
}

void Docmala::checkConsistency() {
    for (const auto& part : _document.parts()) {
        auto link = boost::get<DocumentPart::Link>(&part);
        if (link) {
            if (link->type == DocumentPart::Link::Type::IntraFile) {
                if (_document.anchors().find(link->data) == _document.anchors().end()) {
                    _errors.emplace_back(link->location, "Unable to find anchor for link to: '" + link->data + "'.");
                }
            }
        }
    }
}

bool Docmala::readHeadLine() {
    int level = 1;
    while (!_file->isEoF()) {
        char c = _file->getch();
        if (isWhitespace(c)) {
            continue;
        }

        if (c == '\n') {
            _errors.emplace_back(_file->location(), std::string("Headline contains no text and is skipped."));
            return false;
        }

        if (c == '=') {
            level++;
        } else {
            DocumentPart::Text text;
            readText(c, text);
            _document.addPart(DocumentPart::Headline(text, level));
            return true;
        }
    }

    _errors.emplace_back(_file->location(), std::string("End of file reached while parsing headline."));
    return false;
}

bool Docmala::readCaption() {
    DocumentPart::Text text;
    if (!readText('\0', text)) {
        return false;
    }
    _document.addPart(DocumentPart::Caption(text));
    return true;
}

bool Docmala::readLine(std::string& destination) {
    while (!_file->isEoF()) {
        char c = _file->getch();
        if (c == '\n') {
            break;
        }
        destination.push_back(c);
    }
    return true;
}

bool Docmala::readPlugin() {
    enum class Mode { Begin, ParameterName, SkipUntilNewLine } mode{Mode::Begin};

    FileLocation  nameBegin;
    std::string   name;
    ParameterList parameters;
    parameters.insert(std::make_pair("inputFile", Parameter{"inputFile", _file->fileName(), FileLocation()}));
    parameters.insert(std::make_pair("pluginDir", Parameter{"pluginDir", _pluginDir, FileLocation()}));

    bool skipWarningPrinted = false;

    while (!_file->isEoF()) {
        char c = _file->getch();

        if (mode == Mode::Begin) {
            if (c == ' ' || c == '\t') {
                continue;
            }
            mode      = Mode::ParameterName;
            nameBegin = _file->location();
        }
        if (mode == Mode::ParameterName) {
            if (c == ',') {
                if (!readParameterList(parameters, ']')) {
                    return false;
                }
                mode = Mode::SkipUntilNewLine;
                continue;
            }
            if (c == ']') {
                mode = Mode::SkipUntilNewLine;
                continue;
            }
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-') {
                name.push_back(c);
            } else {
                _errors.emplace_back(
                    nameBegin,
                    std::string("Error while parsing plugin name. A valid name consists of ['a'-'z', 'A'-'Z', '0'-'9', '_', '-'] but a '")
                        + c + "' was found.");
                return false;
            }
        } else if (mode == Mode::SkipUntilNewLine) {
            if (c == '\n') {
                break;
            }
            if (!skipWarningPrinted) {
                _errors.emplace_back(_file->location(), std::string("Text found after ']' is skipped."));
                skipWarningPrinted = true;
            }
        }
    }

    std::shared_ptr<DocumentPlugin> plugin;
    if (_loadedDocumentPlugins.find(name) != _loadedDocumentPlugins.end()) {
        plugin = _loadedDocumentPlugins[name];
    } else {
        plugin = _pluginLoader->createExtension<DocumentPlugin>(name);
        _loadedDocumentPlugins.insert(std::make_pair(name, plugin));
    }
    if (!plugin) {
        _errors.emplace_back(nameBegin, std::string("Unable to load plugin with name: '") + name + "'.");
        return false;
    }
    if (plugin->postProcessing() != DocumentPlugin::PostProcessing::None) {
        PostProcessingInfo postProcessing;
        postProcessing.plugin     = plugin;
        postProcessing.parameters = parameters;
        postProcessing.location   = nameBegin;
        _registeredPostprocessing.push_back(postProcessing);
    }

    if (plugin->blockProcessing() == DocumentPlugin::BlockProcessing::Required
        || plugin->blockProcessing() == DocumentPlugin::BlockProcessing::Optional) {
        std::string block;
        if (!readBlock(block)) {
            return false;
        }
        auto errors = plugin->process(parameters, nameBegin, _document, block);
        if (!errors.empty()) {
            for (auto& error : errors) {
                error.message = "    " + error.message;
            }
            _errors.emplace_back(nameBegin, "Errors occured during plugin execution");
            _errors.insert(_errors.end(), errors.begin(), errors.end());
        }

    } else {
        auto errors = plugin->process(parameters, nameBegin, _document, "");
        if (!errors.empty()) {
            for (auto& error : errors) {
                error.message = "    " + error.message;
            }
            _errors.emplace_back(nameBegin, "Errors occured during plugin execution");
            _errors.insert(_errors.end(), errors.begin(), errors.end());
        }
    }

    return true;
}

bool Docmala::readAnchor(IFile* file, std::vector<Error>& errors, DocumentPart::Text& outText) {
    enum class Mode { Begin, Name, EndTag1, EndTag2 } mode{Mode::Begin};

    std::string name;
    auto        anchorLocation = file->location();

    while (!file->isEoF()) {
        auto location = file->location();
        char c        = file->getch();

        if (mode == Mode::Begin) {
            if (c == '[') {
                mode = Mode::Name;
                continue;
            }
            errors.emplace_back(location, std::string("Expected '[' but got '") + c + "'. This is an error in Docmala.");
            return false;
        }
        if (mode == Mode::Name) {
            if (isWhitespace(c) && name.empty()) {
                continue;
            }
            if (c == ']') {
                mode = Mode::EndTag1;
                continue;
            }
            if (isWhitespace(c) && !name.empty()) {
                mode = Mode::EndTag2;
                continue;
            }
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-') {
                name.push_back(c);
            } else {
                errors.emplace_back(file->location(),
                                    std::string("Error while parsing anchor. A valid anchor name consisting of [a'-'z', 'A'-'Z', "
                                                "'0'-'9', '_', '-'] or a anchor end']]' was expected but a '")
                                        + c + "' was found.");
                return false;
            }
        } else if (mode == Mode::EndTag1) {
            if (c != ']') {
                errors.emplace_back(location, std::string("Error while parsing anchor. Expected ']' but got '") + c + "'.");
                return false;
            }
            outText.text.emplace_back(DocumentPart::Anchor{name, anchorLocation});
            return true;

        } else if (mode == Mode::EndTag2) {
            if (isWhitespace(c)) {
                continue;
            }
            if (c == ']') {
                mode = Mode::EndTag1;
                continue;
            }
            errors.emplace_back(location, std::string("Error while parsing anchor. Expected ']' but got '") + c + "'.");
            return false;
        }
    }

    errors.emplace_back(file->location(), std::string("Error while parsing anchor. Unexpected end of file."));
    return false;
}

bool Docmala::readLink(DocumentPart::Text& outText) {
    return readLink(_file.get(), _errors, outText);
}

bool Docmala::readLink(IFile* file, std::vector<Error>& errors, DocumentPart::Text& outText) {
    enum class Mode { Begin, Data, Text, EndTag1, EndTag2 } mode{Mode::Begin};

    std::string text;
    std::string data;
    auto        linkLocation = file->location();

    while (!file->isEoF()) {
        auto location = file->location();
        char c        = file->getch();

        if (mode == Mode::Begin) {
            if (c == '<') {
                mode = Mode::Data;
                continue;
            }
            errors.emplace_back(location, std::string("Expected '<' but got '") + c + "'. This is an error in Docmala.");
            return false;
        }
        if (mode == Mode::Data) {
            if (isWhitespace(c) && data.empty()) {
                continue;
            }
            if (c == ',') {
                mode = Mode::Text;
                continue;
            }
            if (c == '>') {
                mode = Mode::EndTag1;
                continue;
            }
            if (isWhitespace(c) && !data.empty()) {
                mode = Mode::EndTag2;
                continue;
            }
            data.push_back(c);

        } else if (mode == Mode::Text) {
            if (text.empty() && isWhitespace(c)) {
                continue;
            }
            if (c == '>') {
                if (text.empty()) {
                    errors.emplace_back(file->location(), std::string("Error while parsing link. Text after colon is not allowed to be empty."));
                }
                mode = Mode::EndTag1;
                continue;
            }
            if (c == '\n') {
                errors.emplace_back(file->location(),
                                    std::string("Error while parsing link. A end tag '>>' was expected but a 'newline' was found."));
                return false;
            }
            text.push_back(c);

        } else if (mode == Mode::EndTag1) {
            if (c != '>') {
                errors.emplace_back(location, std::string("Error while parsing link. Expected '>' but got '") + c + "'.");
                return false;
            }
            auto type = DocumentPart::Link::Type::IntraFile;

            if (data.find("://") != std::string::npos) {
                type = DocumentPart::Link::Type::Web;
            } else if (data.find(':') != std::string::npos && data.find('.') != std::string::npos) {
                type = DocumentPart::Link::Type::InterFile;
            }

            if (data.empty()) {
                errors.emplace_back(linkLocation, std::string("Error while parsing link. Link data is not allowed to be empty."));
                return false;
            }

            outText.text.emplace_back(DocumentPart::Link{data, text, type, linkLocation});
            return true;

        } else if (mode == Mode::EndTag2) {
            if (isWhitespace(c)) {
                continue;
            }
            if (c == '>') {
                mode = Mode::EndTag1;
                continue;
            }
            errors.emplace_back(location, std::string("Error while parsing anchor. Expected '>' but got '") + c + "'.");
            return false;
        }
    }

    errors.emplace_back(file->location(), std::string("Error while parsing anchor. Unexpected end of file."));
    return false;
}

bool Docmala::readMetaData() {
    enum class Mode { ParameterName, EndOrAssign, ParameterValue, SkipUntilNewLine } mode{Mode::ParameterName};

    MetaData metaData;
    bool     skipWarningPrinted = false;
    bool     addMetaData        = true;
    bool     ok                 = true;
    auto     loc                = _file->location();

    while (!_file->isEoF()) {
        char c = _file->getch();
        if (mode == Mode::ParameterName) {
            if (c == ' ' || c == '\t') {
                if (metaData.key.empty()) { // skip leading white spaces
                    continue;
                }
                mode = Mode::EndOrAssign;
                continue;
            }
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-') {
                metaData.key.push_back(c);
            } else if (c == '=' || c == '+') {
                if (metaData.key.empty()) {
                    _errors.emplace_back(_file->location(),
                                         std::string("Error while parsing meta data key. A key consisting of ['a'-'z', 'A'-'Z', "
                                                     "'0'-'9', '_', '-'] was expected but a '")
                                             + c + "' was found.");
                    return false;
                }
                mode = Mode::EndOrAssign;

            } else if (c == '}') {
                mode = Mode::EndOrAssign;
            } else {
                _errors.emplace_back(_file->location(),
                                     std::string("Error while parsing meta data key. A valid meta data key consisting of [a'-'z', "
                                                 "'A'-'Z', '0'-'9', '_', '-'] was expected but a '")
                                         + c + "' was found.");
                return false;
            }
        }
        if (mode == Mode::EndOrAssign) {
            if (c == ' ' || c == '\t') {
                continue;
            }

            if (c == '+') {
                metaData.mode = MetaData::Mode::List;
            } else if (c == '=') {
                metaData.mode = MetaData::Mode::First;
            } else if (c == '}') {
                metaData.mode = MetaData::Mode::Flag;
                metaData.data.push_back({loc, ""});
                mode = Mode::SkipUntilNewLine;
                continue;
            }
            mode = Mode::ParameterValue;
            continue;
        }
        if (mode == Mode::ParameterValue) {
            if ((c == ' ' || c == '\t') && metaData.data.empty()) {
                continue;
            }
            if (c == '}') {
                if (metaData.data.empty()) {
                    _errors.emplace_back(_file->location(), std::string("A value was expected but '") + c + "' was found.");
                    return false;
                }
                mode = Mode::SkipUntilNewLine;
                continue;
            }
            if (metaData.data.empty()) {
                metaData.data.push_back({loc, ""});
            }
            metaData.data.front().value.push_back(c);
        }
        if (mode == Mode::SkipUntilNewLine) {
            if (addMetaData) {
                addMetaData         = false;
                const auto& current = _document.metaData().find(metaData.key);
                if (current == _document.metaData().end()) {
                    _document.addMetaData(metaData);
                } else {
                    if (current->second.mode != metaData.mode) {
                        ok            = false;
                        auto      loc = current->second.firstLocation;
                        ErrorData ext = {current->second.data.front().location,
                                         std::string("First definition of '") + metaData.key + "' is at " + loc.fileName + "("
                                             + std::to_string(loc.line) + ":" + std::to_string(loc.column) + ")"};
                        _errors.push_back(
                            {_file->location(), std::string("Assignment mode of metadata does not match first definition"), {ext}});
                    } else {
                        _document.addMetaData(metaData);
                    }
                }
            }
            if (c == '\n') {
                return ok;
            }
            if (!skipWarningPrinted) {
                _errors.emplace_back(_file->location(), std::string("Text found after '}' is skipped."));
                skipWarningPrinted = true;
            }
        }
    }
    _errors.emplace_back(_file->location(), std::string("Error while parsing meta data. Unexpected end of file."));
    return false;
}

bool isFormatSpecifier(char c) {
    return c == '_' || c == '*' || c == '-' || c == '/' || c == '\'';
}

// bool Docmala::readText(char startCharacter, DocumentPart::Text &text)
//{
//    char c = startCharacter;
//    DocumentPart::FormatedText formatedText;

//    if( startCharacter == '\0' ) {
//        c = _file->getch();
//    }

//    text.line = _file->location().line;
//    while( true ) {
//        if( isFormatSpecifier(c) ) {
//            const char previous = _file->previous();
//            const char following = _file->following();
//            if( (isWhitespace(previous, true) || isFormatSpecifier(previous)) && !isWhitespace(following, true) ) {
//                auto store = formatedText;
//                bool ok = false;
//                switch(c) {
//                case '_':
//                    ok = !formatedText.bold;
//                    formatedText.bold = true;
//                    break;
//                case '*':
//                    ok = !formatedText.italic;
//                    formatedText.italic = true;
//                    break;
//                case '-':
//                    ok = !formatedText.crossedOut;
//                    formatedText.crossedOut = true;
//                    break;
//                }
//                if( !ok ) {
//                    formatedText.text.push_back(c);
//                } else {
//                    if( !store.text.empty() ) {
//                        text.text.push_back(store);
//                        formatedText.text.clear();
//                    }
//                }
//            } else if( (!isWhitespace(previous) ) && ( isWhitespace(following, true) || isFormatSpecifier(following)) ) {
//                auto store = formatedText;
//                bool ok = false;
//                switch(c) {
//                case '_':
//                    ok = formatedText.bold;
//                    formatedText.bold = false;
//                    break;
//                case '*':
//                    ok = formatedText.italic;
//                    formatedText.italic = false;
//                    break;
//                case '-':
//                    ok = formatedText.crossedOut;
//                    formatedText.crossedOut = false;
//                    break;
//                }
//                if( !ok ) {
//                    formatedText.text.push_back(c);
//                } else {
//                    if( !store.text.empty() ) {
//                        text.text.push_back(store);
//                        formatedText.text.clear();
//                    }
//                }
//            } else {
//                formatedText.text.push_back(c);
//            }

//        } else if( c == '\n' ) {
//            if( !formatedText.text.empty() )
//                text.text.push_back(formatedText);
//            bool ok = true;
//            if( formatedText.bold ) {
//                _errors.push_back(Error{_file->location(), std::string("Bold formating ('_') was not closed.") });
//                ok = false;
//            }
//            if( formatedText.italic ) {
//                _errors.push_back(Error{_file->location(), std::string("Italic formating ('*') was not closed.") });
//                ok = false;
//            }
//            if( formatedText.crossedOut ) {
//                _errors.push_back(Error{_file->location(), std::string("Crossed out formating ('-') was not closed.") });
//                ok = false;
//            }
//            return ok;
//        } else if( c == '[' && _file->following() == '[' ) {
//            if( !formatedText.text.empty() )
//                text.text.push_back(formatedText);
//            readAnchor();
//            formatedText.text.clear();
//        } else if( c == '<' && _file->following() == '<' ) {
//            if( !formatedText.text.empty() ) {
//                text.text.push_back(formatedText);
//            }
//            readLink();
//            formatedText.text.clear();
//        } else {
//            formatedText.text.push_back(c);
//        }

//        if( _file->isEoF() )
//            break;

//        c = _file->getch();
//    }

//    // end of file is no error, when parsing text
//    if( !formatedText.text.empty() )
//        text.text.push_back(formatedText);
//    return true;
//}

bool Docmala::readText(char startCharacter, DocumentPart::Text& text) {
    return readText(_file.get(), _errors, startCharacter, text);
    /*    char c = startCharacter;
        DocumentPart::FormatedText formatedText;

        if( startCharacter == '\0' ) {
            c = _file->getch();
        }

        text.line = _file->location().line;
        while( true ) {
            if( isFormatSpecifier(c) ) {
                const char following = _file->following();
                if( following == c && _file->previous() != '\\') {
                    c = _file->getch();
                    auto store = formatedText;
                    switch(c) {
                    case '_':
                        formatedText.underlined = !formatedText.underlined;
                        break;
                    case '/':
                        formatedText.italic = !formatedText.italic;
                        break;
                    case '\'':
                        formatedText.monospaced = !formatedText.monospaced;
                        break;
                    case '*':
                        formatedText.bold = !formatedText.bold;
                        break;
                    case '-':
                        formatedText.stroked = !formatedText.stroked;
                        break;
                    }
                    if( !store.text.empty() ) {
                        text.text.push_back(store);
                        formatedText.text.clear();
                    }
                } else {
                    formatedText.text.push_back(c);
                }
            } else if( c == '\n' ) {
                if( !formatedText.text.empty() )
                    text.text.push_back(formatedText);
                bool ok = true;
                if( formatedText.bold ) {
                    _errors.push_back(Error{_file->location(), std::string("Bold formating (\"**'\") was not closed.") });
                    ok = false;
                }
                if( formatedText.italic ) {
                    _errors.push_back(Error{_file->location(), std::string("Italic formating (\"//\") was not closed.") });
                    ok = false;
                }
                if( formatedText.monospaced ) {
                    _errors.push_back(Error{_file->location(), std::string("Monospace formating (\"''\") was not closed.") });
                    ok = false;
                }
                if( formatedText.stroked ) {
                    _errors.push_back(Error{_file->location(), std::string("Stroked formating (\"--\") was not closed.") });
                    ok = false;
                }
                if( formatedText.underlined ) {
                    _errors.push_back(Error{_file->location(), std::string("Underlined formating (\"--\") was not closed.") });
                    ok = false;
                }
                return ok;
            } else if( c == '[' && _file->following() == '[' && _file->previous() != '\\') {
                if( !formatedText.text.empty() )
                    text.text.push_back(formatedText);
                readAnchor();
                formatedText.text.clear();
            } else if( c == '<' && _file->following() == '<' && _file->previous() != '\\') {
                if( !formatedText.text.empty() ) {
                    text.text.push_back(formatedText);
                }
                readLink(text);
                formatedText.text.clear();
            } else if( c == '\\' ) {
                if( _file->following() == '\\' ) {
                    c = _file->getch();
                    formatedText.text.push_back(c);
                }
            } else {
                formatedText.text.push_back(c);
            }

            if( _file->isEoF() )
                break;

            c = _file->getch();
        }

        // end of file is no error, when parsing text
        if( !formatedText.text.empty() )
            text.text.push_back(formatedText);
        return true;
        */
}

bool Docmala::readText(IFile* file, std::vector<Error>& errors, char startCharacter, DocumentPart::Text& text) {
    char                       c = startCharacter;
    DocumentPart::FormatedText formatedText;

    if (startCharacter == '\0') {
        c = file->getch();
    }

    text.location = file->location();
    while (true) {
        if (isFormatSpecifier(c)) {
            const char following = file->following();
            if (following == c && file->previous() != '\\') {
                c          = file->getch();
                auto store = formatedText;
                switch (c) {
                    case '_':
                        formatedText.underlined = !formatedText.underlined;
                        break;
                    case '/':
                        formatedText.italic = !formatedText.italic;
                        break;
                    case '\'':
                        formatedText.monospaced = !formatedText.monospaced;
                        break;
                    case '*':
                        formatedText.bold = !formatedText.bold;
                        break;
                    case '-':
                        formatedText.stroked = !formatedText.stroked;
                        break;
                }
                if (!store.text.empty()) {
                    text.text.emplace_back(store);
                    formatedText.text.clear();
                }
            } else {
                formatedText.text.push_back(c);
            }
        } else if (c == '\n') {
            if (!formatedText.text.empty()) {
                text.text.emplace_back(formatedText);
            }
            bool ok = true;
            if (formatedText.bold) {
                errors.emplace_back(file->location(), std::string("Bold formating (\"**'\") was not closed."));
                ok = false;
            }
            if (formatedText.italic) {
                errors.emplace_back(file->location(), std::string("Italic formating (\"//\") was not closed."));
                ok = false;
            }
            if (formatedText.monospaced) {
                errors.emplace_back(file->location(), std::string("Monospace formating (\"''\") was not closed."));
                ok = false;
            }
            if (formatedText.stroked) {
                errors.emplace_back(file->location(), std::string("Stroked formating (\"--\") was not closed."));
                ok = false;
            }
            if (formatedText.underlined) {
                errors.emplace_back(file->location(), std::string("Underlined formating (\"--\") was not closed."));
                ok = false;
            }
            return ok;
        } else if (c == '[' && file->following() == '[' && file->previous() != '\\') {
            if (!formatedText.text.empty()) {
                text.text.emplace_back(formatedText);
            }
            readAnchor(file, errors, text);
            formatedText.text.clear();
        } else if (c == '<' && file->following() == '<' && file->previous() != '\\') {
            if (!formatedText.text.empty()) {
                text.text.emplace_back(formatedText);
            }
            readLink(file, errors, text);
            formatedText.text.clear();
        } else if (c == '\\') {
            if (file->following() == '\\') {
                c = file->getch();
                formatedText.text.push_back(c);
            }
        } else {
            formatedText.text.push_back(c);
        }

        if (file->isEoF()) {
            break;
        }

        c = file->getch();
    }

    // end of file is no error, when parsing text
    if (!formatedText.text.empty()) {
        text.text.emplace_back(formatedText);
    }
    return true;
}

bool Docmala::readParameterList(ParameterList& parameters, char blockEnd) {
    enum class Mode { ParameterName, EndOrEquals, ParameterValue, EndOrNext } mode{Mode::ParameterName};

    enum class ValueMode { Normal, Extended } valueMode{ValueMode::Normal};

    Parameter parameter;

    while (!_file->isEoF()) {
        char c = _file->getch();
        if (c == '\n') {
            _errors.emplace_back(
                _file->location(),
                std::string("Error while parsing parameters. A valid parameter definition was expected but a 'newline' was found."));
            return false;
        }
        if (mode == Mode::ParameterName) {
            if (c == ' ' || c == '\t') {
                if (parameter.key.empty()) { // skip leading white spaces
                    continue;
                }
                mode = Mode::EndOrEquals;
                continue;
            }
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-') {
                parameter.key.push_back(c);
            } else if (c == '=') {
                if (parameter.key.empty()) {
                    _errors.emplace_back(_file->location(),
                                         std::string("Error while parsing parameter name. A name consisting of ['a'-'z', 'A'-'Z', "
                                                     "'0'-'9', '_', '-'] was expected but a '")
                                             + c + "' was found.");
                    return false;
                }
                mode = Mode::ParameterValue;
                continue;

            } else if (c == ',') {
                if (parameter.key.empty()) {
                    _errors.emplace_back(_file->location(),
                                         std::string("Error while parsing parameter name. A name consisting of ['a'-'z', 'A'-'Z', "
                                                     "'0'-'9', '_', '-'] was expected but a '")
                                             + c + "' was found.");
                    return false;
                }
                parameters.insert(std::make_pair(parameter.key, parameter));
                parameter = Parameter();
                mode      = Mode::ParameterName;
                continue;

            } else if (c == blockEnd) {
                if (parameter.key.empty()) {
                    _errors.emplace_back(_file->location(),
                                         std::string("Error while parsing parameter name. A valid name consisting of ['a'-'z', 'A'-'Z', "
                                                     "'0'-'9', '_', '-'] was expected but a ']' was found."));
                    return false;
                }
                parameters.insert(std::make_pair(parameter.key, parameter));
                parameter = Parameter();
                return true;

            } else {
                _errors.emplace_back(
                    _file->location(),
                    std::string(
                        "Error while parsing parameter name. A valid name consisting of [a'-'z', 'A'-'Z', '0'-'9', '_', '-'] or a '")
                        + blockEnd + "' was expected but a '" + c + "' was found.");
                return false;
            }
        } else if (mode == Mode::EndOrEquals) {
            if (c == ' ' || c == '\t') {
                continue;
            }
            if (c == '=') {
                mode = Mode::ParameterValue;
                continue;
            }
            if (c == blockEnd) {
                parameters.insert(std::make_pair(parameter.key, parameter));
                parameter = Parameter();
                return true;
            }
            _errors.emplace_back(_file->location(),
                                 std::string("Error while parsing parameter. A '=' or '") + blockEnd + "' was expected but a '" + c
                                     + "' was found.");
            return false;

        } else if (mode == Mode::EndOrNext) {
            if (c == ' ' || c == '\t') {
                continue;
            }
            if (c == ',') {
                mode = Mode::ParameterName;
                parameters.insert(std::make_pair(parameter.key, parameter));
                parameter = Parameter();
                continue;
            }
            if (c == blockEnd) {
                parameters.insert(std::make_pair(parameter.key, parameter));
                parameter = Parameter();
                return true;
            }
            _errors.emplace_back(_file->location(),
                                 std::string("Error while parsing parameter. A '=' or '") + blockEnd + "' was expected but a '" + c
                                     + "' was found.");
            return false;

        } else if (mode == Mode::ParameterValue) {
            if (parameter.value.empty() && (c == ' ' || c == '\t')) {
                continue;
            }
            if (parameter.value.empty() && valueMode == ValueMode::Normal) {
                if (c == '"') {
                    valueMode = ValueMode::Extended;
                    continue;
                }
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                    valueMode = ValueMode::Normal;
                    parameter.value.push_back(c);
                    continue;
                }
                _errors.emplace_back(_file->location(),
                                     std::string("Error while parsing parameter value. ")
                                         + "A valid value has to start with ['a'-'z', 'A'-'Z', '0'-'9'] or "
                                         + " has to be placed in '\"' but a " + c + "' was found.");
                return false;
            }
            if (valueMode == ValueMode::Extended) {
                if (c == '"') {
                    if (parameter.value.back() == '\\') {
                        parameter.value.back() = '"';
                        continue;
                    }
                    mode      = Mode::EndOrNext;
                    valueMode = ValueMode::Normal;
                    continue;
                }
                if (c == '\n') {
                    _errors.emplace_back(_file->location(),
                                         std::string(
                                             "Error while parsing parameter value. A '\"' was expected but a 'newline' was found."));
                    return false;
                }
                parameter.value.push_back(c);
                continue;
            }
            if (valueMode == ValueMode::Normal) {
                if (c == ' ' || c == '\t' || c == ',' || c == blockEnd) {
                    mode = Mode::ParameterName;
                    parameters.insert(std::make_pair(parameter.key, parameter));
                    parameter = Parameter();
                    if (c == blockEnd) {
                        return true;
                    }
                    continue;
                }
                parameter.value.push_back(c);
                continue;
            }
        }
    }
    _errors.emplace_back(_file->location(),
                         std::string(
                             "Error while parsing parameters. A valid parameter definition was expected but an 'end of file' was found."));
    return false;
}

bool Docmala::readBlock(std::string& block) {
    const std::string delimiter = "----";

    std::string potentialDelimiter;
    bool        searchingEnd = false;

    while (!_file->isEoF()) {
        char c = _file->getch();

        if (c == '-' || (c == '\\' && potentialDelimiter.empty())) {
            potentialDelimiter.push_back(c);
            continue;
        }
        if (potentialDelimiter.substr(0, 4) == "----") {
            if (c == '\n') {
                potentialDelimiter.clear();
                if (searchingEnd) {
                    return true;
                }
                searchingEnd = true;
                continue;
            }
            _errors.emplace_back(_file->location(),
                                 std::string("Error while reading block. A valid delimiter may only contain '-' but a '") + c + "' was found.");
            return false;
        }
        if (!searchingEnd) {
            _errors.emplace_back(_file->location(),
                                 std::string("Error while reading block. A valid delimiter ('") + delimiter + "') was expected but a '" + c
                                     + "' was found.");
            return false;
        }
        if (!potentialDelimiter.empty() && potentialDelimiter.front() == '\\') {
            block.append(potentialDelimiter.substr(1));
        } else {
            block.append(potentialDelimiter);
        }
        block.push_back(c);
        potentialDelimiter.clear();
    }
    _errors.emplace_back(_file->location(),
                         std::string("Error while parsing block. A valid block definition was expected but an 'end of file' was found."));
    return false;
}

bool Docmala::readList(DocumentPart::List::Type type) {
    int level = 1;

    while (!_file->isEoF()) {
        char c = _file->getch();

        if ((c == '*' && type == DocumentPart::List::Type::Points) || (c == '#' && type == DocumentPart::List::Type::Numbered)) {
            level++;
        } else if (isWhitespace(c)) {
            DocumentPart::Text text;
            readText('\0', text);

            auto list = _document.last<DocumentPart::List>();
            if (list) {
                std::vector<DocumentPart::List::Entry>* entries = &list->entries;
                for (int i = 1; i < level; i++) {
                    if (entries->empty()) {
                        entries->push_back({{}, type, {}});
                    }
                    entries = &entries->back().entries;
                }
                entries->push_back({text, type, {}});
                return true;
            }
            _document.addPart(DocumentPart::List(text, type));
            return true;
        }
    }
    return false;
}

bool Docmala::isWhitespace(char c, bool allowEndline) {
    return c == ' ' || c == '\t' || (allowEndline && (c == '\n' || c == '\0'));
}
