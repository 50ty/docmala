#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>

using namespace docmala;

class HidePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block) override;
};


DocumentPlugin::BlockProcessing HidePlugin::blockProcessing() const {
    return BlockProcessing::Optional;
}

bool HidePlugin::process(const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block)
{
    (void)block;
    (void)location;
    (void)document;
    (void)parameters;
    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, HidePlugin, "hide", 1, "Hides text of the subsequent block from output", EXTENSION_SYSTEM_NO_USER_DATA )
