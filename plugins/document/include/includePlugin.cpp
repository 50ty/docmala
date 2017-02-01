#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <docmala/Docmala.h>

using namespace docmala;

class IncludePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, std::vector<DocumentPart> &outDocument, const std::string &block) override;
    std::vector<Error> errors() const {
        return _errors;
    }
    std::vector<Error> _errors;
};


DocumentPlugin::BlockProcessing IncludePlugin::blockProcessing() const {
    return BlockProcessing::No;
}

bool IncludePlugin::process( const ParameterList &parameters, const FileLocation &location, std::vector<DocumentPart> &outDocument, const std::string &block)
{
    (void)block;
    (void)location;
    (void)outDocument;

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

    Docmala parser;
    parser.parseFile(baseDir + "/" + includeFile, "");

    std::vector<DocumentPart> doc = parser.document();
    int baseLevel = 1;

    if( !keepHeadlineLevel ) {
        for( auto iter = outDocument.rbegin(); iter != outDocument.rend(); iter++ ) {
            if( iter->type() == DocumentPart::Type::Headline ) {
                baseLevel = iter->headline()->level;
                break;
            }
        }
    }

    for( auto &part : doc ) {
        if( !keepHeadlineLevel && (part.type() == DocumentPart::Type::Headline) ) {
            DocumentPart::Headline headline = *part.headline();
            headline.level += baseLevel;
            outDocument.push_back(headline);

        } else {
            outDocument.push_back(part);
        }

    }


    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, IncludePlugin, "include", 1, "Include a file into an other one", EXTENSION_SYSTEM_NO_USER_DATA )
