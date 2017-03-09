#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <docmala/Docmala.h>
#include <algorithm>

using namespace docmala;

class IncludePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block) override;

    PostProcessing postProcessing() const override;
    bool postProcess(const ParameterList &parameters, const FileLocation &location, Document &document) override;

    void escapeFileName(std::string &fileName) {
        std::replace( fileName.begin(), fileName.end(), '.', '_');
    }

    const std::vector<Error> lastErrors() const override {return _errors;}

    std::vector<Error> _errors;
    std::unique_ptr<Docmala> parser;
    void updateDocumentParts(const std::string &includeFile, DocumentPart::GeneratedDocument &out, const std::vector<DocumentPart> &parts, bool keepHeadlineLevel, int baseLevel);
};


DocumentPlugin::BlockProcessing IncludePlugin::blockProcessing() const {
    return BlockProcessing::No;
}



void IncludePlugin::updateDocumentParts(const std::string &includeFile, DocumentPart::GeneratedDocument &out, const std::vector<DocumentPart> &parts, bool keepHeadlineLevel, int baseLevel)
{
    int currentLevel = 0;
    for( auto &part : parts ) {
        if( !keepHeadlineLevel && (part.type() == DocumentPart::Type::Headline) ) {
            DocumentPart::Headline headline = *part.headline();
            headline.level += baseLevel;
            out.document.push_back(headline);
            currentLevel = headline.level;
        } else if( part.type() == DocumentPart::Type::Anchor && !includeFile.empty()) {
            auto anchor = *part.anchor();
            anchor.name = includeFile + ":" + anchor.name;
            out.document.push_back(anchor);
        } else if( part.type() == DocumentPart::Type::GeneratedDocument ){
            auto generated = part.generatedDocument();
            DocumentPart::GeneratedDocument outDoc(0);
            updateDocumentParts("",  outDoc, generated->document, keepHeadlineLevel, currentLevel);
            out.document.push_back(outDoc);
        } else {
            out.document.push_back(part);
        }
    }
}

bool IncludePlugin::process(const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block)
{
    (void)block;

    _errors.clear();

    std::string pluginDir = "./plugins";
    auto pluginDirIter = parameters.find("pluginDir");
    if( pluginDirIter != parameters.end() ) {
        pluginDir =  pluginDirIter->second.value + '/';
    }

    std::string includeFile;
    std::string inputFile;
    bool keepHeadlineLevel = false;

    auto inFileIter = parameters.find("inputFile");
    if( inFileIter != parameters.end() ) {
        inputFile = inFileIter->second.value;
    } else {
        _errors.push_back({location, "Parameter 'inputFile' is missing."});
        return false;
    }

    std::string baseDir = inputFile.substr(0, inputFile.find_last_of("\\/"));

    auto includeFileIter = parameters.find("file");

    if( includeFileIter != parameters.end() ) {
        includeFile = includeFileIter->second.value;
    } else {
        _errors.push_back({location, "Parameter 'file' is missing."});
        return false;
    }

    if( parameters.find("keepHeadlineLevel") != parameters.end() ) {
        keepHeadlineLevel = true;
    }

    if( !parser ) {
        parser.reset(new Docmala(pluginDir));
    }

    parser->parseFile(baseDir + "/" + includeFile);

    _errors = parser->errors();
    auto doc = parser->document();
    int baseLevel = 1;
    DocumentPart::GeneratedDocument generated(location.line);


    if( !keepHeadlineLevel ) {
        for( auto iter = document.parts().rbegin(); iter != document.parts().rend(); iter++ ) {
            if( iter->type() == DocumentPart::Type::Headline ) {
                baseLevel = iter->headline()->level;
                break;
            }
        }
    }

    updateDocumentParts(includeFile, generated, doc.parts(), keepHeadlineLevel, baseLevel);

    document.addPart(generated);

    return true;
}

DocumentPlugin::PostProcessing IncludePlugin::postProcessing() const
{
    return PostProcessing::Once;
}

bool IncludePlugin::postProcess(const ParameterList &parameters, const FileLocation &location, Document &document)
{
    _errors.clear();

    std::string includeFile;
    std::string inputFile;

    auto inFileIter = parameters.find("inputFile");
    if( inFileIter != parameters.end() ) {
        inputFile = inFileIter->second.value;
    } else {
        _errors.push_back({location, "Parameter 'inputFile' is missing."});
        return false;
    }

    std::string baseDir = inputFile.substr(0, inputFile.find_last_of("\\/"));

    auto includeFileIter = parameters.find("file");

    if( includeFileIter != parameters.end() ) {
        includeFile = includeFileIter->second.value;
    } else {
        _errors.push_back({location, "Parameter 'file' is missing."});
        return false;
    }



    for( auto &part : document.parts() ) {
        if( part.type() == DocumentPart::Type::Link ) {
            auto link = part.link();

            if( link->type == DocumentPart::Link::Type::InterFile ) {
                std::string fileName = link->data.substr(0, link->data.find(":"));
                if( fileName == includeFile ) {
                    link->type = DocumentPart::Link::Type::IntraFile;
                }
            }
        }
    }
    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, IncludePlugin, "include", 1, "Include a file into an other one", EXTENSION_SYSTEM_NO_USER_DATA )
