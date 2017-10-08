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
#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <docmala/Docmala.h>

using namespace docmala;

class CodePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) override;
    std::vector<Error> errors() const {
        return _errors;
    }
    std::vector<Error> _errors;
};

DocumentPlugin::BlockProcessing CodePlugin::blockProcessing() const {
    return BlockProcessing::Required;
}

bool CodePlugin::process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) {
    (void)block;
    (void)location;
    (void)document;
    (void)parameters;

    DocumentPart::Code code(location);

    auto inFileIter = parameters.find("type");
    if (inFileIter != parameters.end()) {
        code.type = inFileIter->second.value;
    }

    code.code = block;
    document.addPart(code);

    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, CodePlugin, "code", 1, "Adds code from subsequent block to document", EXTENSION_SYSTEM_NO_USER_DATA)
