#pragma once

#include <vector>
#include <map>
#include "DocumentPart.h"

namespace docmala {

    class Document {
    public:
        void addPart(const DocumentPart& part) {
            _parts.push_back(part);
            if( part.type() == DocumentPart::Type::Anchor ) {
                _anchors.insert(std::make_pair(part.anchor()->name, *part.anchor()));
            }
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
    private:
        std::vector<DocumentPart> _parts;
        std::map<std::string, DocumentPart::Anchor> _anchors;
    };

}
