#pragma once

#include <extension_system/Extension.hpp>
#include <string>
#include <vector>

#include "Parameter.h"
#include "DocumentPart.h"

namespace docmala {
    class DocumentPlugin {
    public:
        enum class BlockProcessing {
            No,
            Required,
            Optional
        };

        virtual ~DocumentPlugin();
        virtual BlockProcessing blockProcessing() const { return BlockProcessing::No; }
        virtual bool process( const ParameterList &parameters, const FileLocation &location, std::vector<DocumentPart> &outDocument, const std::string &block = "" ) = 0;
    };
    DocumentPlugin::~DocumentPlugin() {}

    class OutputPlugin {
    public:
        virtual ~OutputPlugin() {}
        virtual bool write( const ParameterList &parameters, const std::vector<DocumentPart> &document ) = 0;
    };
}

EXTENSION_SYSTEM_INTERFACE(docmala::DocumentPlugin)
EXTENSION_SYSTEM_INTERFACE(docmala::OutputPlugin)
