/**
    @file
    @copyright
        Copyright (C) 2017 Michael Adam
        Copyright (C) 2017 Bernd Amend
        Copyright (C) 2017 Stefan Rommel

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
    virtual std::vector<Error> lastErrors() const {
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
