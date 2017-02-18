#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <docmala/Docmala.h>

using namespace docmala;

class CodePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block) override;
    std::vector<Error> errors() const {
        return _errors;
    }
    std::vector<Error> _errors;
};


DocumentPlugin::BlockProcessing CodePlugin::blockProcessing() const {
    return BlockProcessing::Required;
}

bool CodePlugin::process(const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block)
{
    (void)block;
    (void)location;
    (void)document;
    (void)parameters;

    DocumentPart::Code code;

    auto inFileIter = parameters.find("type");
    if( inFileIter != parameters.end() ) {
        code.type = inFileIter->second.value;
    }

    code.code = block;
    document.addPart(code);

    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, CodePlugin, "code", 1, "Adds code from subsequent block to document", EXTENSION_SYSTEM_NO_USER_DATA )
