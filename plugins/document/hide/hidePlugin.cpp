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
#include <docmala/DocmaPlugin.h>
#include <extension_system/Extension.hpp>

using namespace docmala;

class HidePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) override;
};

DocumentPlugin::BlockProcessing HidePlugin::blockProcessing() const {
    return BlockProcessing::Optional;
}

bool HidePlugin::process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) {
    (void)block;
    (void)location;
    (void)document;
    (void)parameters;
    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, HidePlugin, "hide", 1, "Hides text of the subsequent block from output", EXTENSION_SYSTEM_NO_USER_DATA)
