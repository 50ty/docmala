#pragma once

#include <vector>
#include <string>
#include <docmala/Document.h>
#include <docmala/Parameter.h>

namespace docmala {

    class HtmlOutput
    {
        // OutputPlugin interface
    public:
        struct HtmlDocument {
            std::string head;
            std::string body;
        };

        HtmlDocument produceHtml(const ParameterList &parameters, const Document &document, const std::string& scripts = "");

    private:
        void writeDocumentParts(std::stringstream &outFile,
                                const ParameterList &parameters,
                                const Document &document,
                                const std::vector<DocumentPart> &documentParts,
                                bool isGenerated = false);

        void writeTable(std::stringstream &outFile, const DocumentPart::Table *table, const ParameterList &parameters, const Document &document);


        unsigned int _imageCounter = 1;
        unsigned int _figureCounter = 1;
        std::string _nameBase = "outfile";

    };

}
