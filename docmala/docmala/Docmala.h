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
