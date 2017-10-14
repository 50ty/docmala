/**
    @file
    @copyright
        Copyright (C) 2017 Michael Adam
        Copyright (C) 2017 Bernd Amend
        Copyright (C) 2017 Stefan Rommel

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or any
   later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "DocumentPart.h"
#include "MetaData.h"
#include <boost/hana.hpp>
#include <map>
#include <vector>

namespace docmala {

template <typename... TFs>
auto make_visitor(TFs&&... fs) {
    return boost::hana::overload(std::forward<decltype(fs)>(fs)...);
}

class Document {
public:
    void addPart(const document_part::Variant& part) {
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

    document_part::Variant& last() {
        return _parts.back();
    }

    template< typename T>
    T* last() {
        if( _parts.empty() )
            return nullptr;

        return boost::get<T>(&_parts.back());
    }

    std::vector<document_part::Variant>& parts() {
        return _parts;
    }

    const std::vector<document_part::Variant>& parts() const {
        return _parts;
    }

    const std::map<std::string, document_part::Anchor>& anchors() const {
        return _anchors;
    }

    const std::map<std::string, MetaData>& metaData() const {
        return _metaData;
    }

private:
    void addAnchors(const document_part::Variant& part) {
        auto visitor = make_visitor(
            // visitors
            [this](const document_part::Anchor& anchor) { _anchors.insert(std::make_pair(anchor.name, anchor)); },
            [this](const document_part::GeneratedDocument& doc) {
                for (const auto& p : doc.document) {
                    addAnchors(p);
                }
            },
            [this](const document_part::Text& text) {
                for (const auto& p : text.text) {
                    addAnchors(p);
                }
            },
            [this](const document_part::Table& table) {
                for (const auto& row : table.cells) {
                    for (const auto& cell : row) {
                        for (const auto& p : cell.content) {
                            addAnchors(p);
                        }
                    }
                }
            },
            [](const auto&) {});
        boost::apply_visitor(visitor, part);
    }

    std::vector<document_part::Variant>          _parts;
    std::map<std::string, document_part::Anchor> _anchors;
    std::map<std::string, MetaData>             _metaData;
};
} // namespace docmala
