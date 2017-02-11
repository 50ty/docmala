#pragma once

#include <extension_system/Extension.hpp>
#include <string>
#include <vector>

#include "Parameter.h"
#include "Document.h"

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
        virtual bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block = "" ) = 0;
    };
    DocumentPlugin::~DocumentPlugin() {}

    class OutputPlugin {
    public:
        virtual ~OutputPlugin() {}
        virtual bool write( const ParameterList &parameters, const Document &document ) = 0;
    };
}

EXTENSION_SYSTEM_INTERFACE(docmala::DocumentPlugin)
EXTENSION_SYSTEM_INTERFACE(docmala::OutputPlugin)
