#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <docmala/Docmala.h>

using namespace docmala;

class IncludePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block) override;
    std::vector<Error> errors() const {
        return _errors;
    }
    std::vector<Error> _errors;
    std::unique_ptr<Docmala> parser;
};


DocumentPlugin::BlockProcessing IncludePlugin::blockProcessing() const {
    return BlockProcessing::No;
}

bool IncludePlugin::process(const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block)
{
    (void)block;
    (void)location;
    (void)document;

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

    auto doc = parser->document();
    int baseLevel = 1;

    if( !keepHeadlineLevel ) {
        for( auto iter = document.parts().rbegin(); iter != document.parts().rend(); iter++ ) {
            if( iter->type() == DocumentPart::Type::Headline ) {
                baseLevel = iter->headline()->level;
                break;
            }
        }
    }

    for( auto &part : doc.parts() ) {
        if( !keepHeadlineLevel && (part.type() == DocumentPart::Type::Headline) ) {
            DocumentPart::Headline headline = *part.headline();
            headline.level += baseLevel;
            document.addPart(headline);

        } else {
            document.addPart(part);
        }

    }


    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, IncludePlugin, "include", 1, "Include a file into an other one", EXTENSION_SYSTEM_NO_USER_DATA )
