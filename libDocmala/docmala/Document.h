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
            if( part.type() == DocumentPart::Type::Anchor ) {
                _anchors.insert(std::make_pair(part.anchor()->name, *part.anchor()));
            } else if( part.type() == DocumentPart::Type::GeneratedDocument ) {
                for( const auto &p : part.generatedDocument()->document ) {
                    if( p.type() == DocumentPart::Type::Anchor ) {
                        _anchors.insert(std::make_pair(p.anchor()->name, *p.anchor()));
                    }
                }
            }
        }

        void addMetaData(const MetaData& metaData) {
            auto &data = _metaData[metaData.key];
            if( data.mode == MetaData::Mode::None ) {
                data = metaData;
                data.firstLocation = metaData.data.front().location;
            } else if( data.mode == MetaData::Mode::List ) {
                data.data.push_back(metaData.data.front());
            }
        }

        void inheritFrom(const Document &other) {
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

        const std::vector<DocumentPart>& parts() const{
            return _parts;
        }

        const std::map<std::string, DocumentPart::Anchor>& anchors() const {
            return _anchors;
        }

        const std::map<std::string, MetaData>& metaData() const {
            return _metaData;
        }
    private:
        std::vector<DocumentPart> _parts;
        std::map<std::string, DocumentPart::Anchor> _anchors;
        std::map<std::string, MetaData> _metaData;
    };

}
