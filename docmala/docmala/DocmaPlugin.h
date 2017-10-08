#pragma once

#include <extension_system/Extension.hpp>
#include <string>
#include <vector>

#include "Parameter.h"
#include "Document.h"
#include "Error.h"

namespace docmala {
class DocumentPlugin {
public:
    enum class BlockProcessing { No, Required, Optional };

    enum class PostProcessing {
        None, ///< No preprocessing is requested
        Once, ///< Preprocessing is done once
        DocumentChanged ///< Preprocessing is done when the document changed
    };

    virtual ~DocumentPlugin();
    virtual BlockProcessing blockProcessing() const {
        return BlockProcessing::No;
    }
    virtual bool process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block = "") = 0;

    /**
     * @brief Returns the post processing mode, a plugin requests
     * @return Requested post processing mode
     */
    virtual PostProcessing postProcessing() const {
        return PostProcessing::None;
    }

    /**
     * @brief postProcess will be called by docmala, when the whole document has been processed.
     *          The postprocess function may change the document's content. If the document has been changed, the
     *          function returns true.
     * @param document
     * @return true, if the document has been changed by post processing, false otherwise
     */
    virtual bool postProcess(const ParameterList& parameters, const FileLocation& location, Document& document) {
        (void)parameters;
        (void)location;
        (void)document;
        return true;
    }

    /**
     * @brief Get the list of errors, occured during the last process or postProcess call
     * @return List of errors
     */
    virtual const std::vector<Error> lastErrors() const {
        return std::vector<Error>();
    }
};
DocumentPlugin::~DocumentPlugin() {}

class OutputPlugin {
public:
    virtual ~OutputPlugin() {}
    virtual bool write(const ParameterList& parameters, const Document& document) = 0;
};
}

EXTENSION_SYSTEM_INTERFACE(docmala::DocumentPlugin)
EXTENSION_SYSTEM_INTERFACE(docmala::OutputPlugin)
