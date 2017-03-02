#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <docmala/Error.h>
#include <fstream>
#include <unordered_map>

using namespace docmala;

class ImagePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block) override;

    std::vector<Error> _errors;
    std::unordered_map<std::string, DocumentPart::Image> _cache;
};


DocumentPlugin::BlockProcessing ImagePlugin::blockProcessing() const {
    return BlockProcessing::No;
}

bool ImagePlugin::process(const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block)
{
    (void)block;
    (void)location;
    (void)document;

    std::string inputFile;

    auto inFileIter = parameters.find("inputFile");
    if( inFileIter != parameters.end() ) {
        inputFile = inFileIter->second.value;
    } else {
        _errors.push_back({location, "Parameter 'inputFile' is missing."});
        return false;
    }

    std::string baseDir = inputFile.substr(0, inputFile.find_last_of("\\/"));

    std::string fileName;
    auto fileNameIter = parameters.find("file");
    if( fileNameIter != parameters.end() ) {
        fileName = baseDir + "/" + fileNameIter->second.value;
    } else {
        _errors.push_back({location, "Parameter 'file' is missing."});
        return false;
    }

    std::string fileExtension = "";
    auto pos = fileName.find_last_of(".");
    if( pos != std::string::npos ) {
        fileExtension = fileName.substr(pos+1);
    } else {
        _errors.push_back({location, "Unable to determine file type."});
        return false;
    }

    std::string imageData;

    std::ifstream highlightReader(fileName, std::ios::in | std::ios::binary);
    if (highlightReader)
    {
        highlightReader.seekg(0, std::ios::end);
        imageData.resize( static_cast<std::string::size_type>(highlightReader.tellg()));
        highlightReader.seekg(0, std::ios::beg);
        highlightReader.read(&imageData[0], static_cast<std::streamsize>(imageData.size()));
        highlightReader.close();
    } else {
        _errors.push_back({location, "Unable to open file '" + fileName + "'."});
    }

    std::string format = fileExtension;

    if( format == "svg" ) {
        if( imageData.find("<?xml") != std::string::npos ) {
            format += "+xml";
        }
    }

    DocumentPart::Text text;
    text.text.push_back({fileName});
    DocumentPart::Image image(format, fileExtension, imageData, text);
    _cache.insert(std::make_pair(block, image));
    document.addPart( image );

    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, ImagePlugin, "image", 1, "Insert an image into the document", EXTENSION_SYSTEM_NO_USER_DATA )
