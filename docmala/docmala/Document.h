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

#include <vector>
#include <map>
#include "DocumentPart.h"
#include "MetaData.h"

namespace docmala {

class Document {
public:
    void addPart(const DocumentPart& part) {
        _parts.push_back(part);
        addAnchors(part);
    }

    void addMetaData(const MetaData& metaData) {
        auto& data = _metaData[metaData.key];
        if (data.mode == MetaData::Mode::None) {
            data               = metaData;
            data.firstLocation = metaData.data.front().location;
        } else if (data.mode == MetaData::Mode::List) {
            data.data.push_back(metaData.data.front());
        }
    }

    void inheritFrom(const Document& other) {
        _anchors = other.anchors();
    }

    void clear() {
        _parts.clear();
        _anchors.clear();
    }

    bool empty() const {
        return _parts.empty();
    }

    DocumentPart& last() {
        return _parts.back();
    }

    std::vector<DocumentPart>& parts() {
        return _parts;
    }

    const std::vector<DocumentPart>& parts() const {
        return _parts;
    }

    const std::map<std::string, DocumentPart::Anchor>& anchors() const {
        return _anchors;
    }

    const std::map<std::string, MetaData>& metaData() const {
        return _metaData;
    }

private:
    void addAnchors(const DocumentPart& part) {
        if (part.type() == DocumentPart::Type::Anchor) {
            _anchors.insert(std::make_pair(part.anchor()->name, *part.anchor()));
        } else if (part.type() == DocumentPart::Type::GeneratedDocument) {
            for (const auto& p : part.generatedDocument()->document) {
                addAnchors(p);
            }
        } else if (part.type() == DocumentPart::Type::Text) {
            for (const auto& p : part.text()->text) {
                addAnchors(p);
            }
        } else if (part.type() == DocumentPart::Type::Table) {
            for (auto row : part.table()->cells) {
                for (auto cell : row) {
                    for (const auto& p : cell.content) {
                        addAnchors(p);
                    }
                }
            }
        }
    }

    std::vector<DocumentPart>                   _parts;
    std::map<std::string, DocumentPart::Anchor> _anchors;
    std::map<std::string, MetaData>             _metaData;
};
}
