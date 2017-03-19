#pragma once

#include "HtmlOutput_global.h"
#include <vector>
#include <string>
#include <docmala/Document.h>
#include <docmala/Parameter.h>

namespace docmala {

    class HTMLOUTPUT_API HtmlOutput
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
                                const std::vector<DocumentPart> &documentParts,
                                bool isGenerated = false);

        void writeTable(std::stringstream &outFile, const DocumentPart::Table *table);
        void writeList(std::stringstream &outFile, std::vector<DocumentPart>::const_iterator &start, const std::vector<DocumentPart> &documentParts, bool isGenerated, int currentLevel = 0);


        unsigned int _imageCounter = 1;
        unsigned int _figureCounter = 1;
        unsigned int _listingCounter = 1;
        unsigned int _tableCounter = 1;
        std::string _nameBase = "outfile";
        bool _embedImages = false;
    };

}
