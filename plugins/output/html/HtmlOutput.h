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
        std::string produceHtml(const ParameterList &parameters, const Document &document, const std::string scripts = "");

    //    void writeText(std::ofstream &outFile, const DocumentPart::Text *printText);
    //    void writeList(std::ofstream &outFile, std::vector<DocumentPart>::const_iterator &start, const std::vector<DocumentPart> &document, int currentLevel = 0);
    private:
        void writeDocumentParts(std::stringstream &outFile,
                                const ParameterList &parameters,
                                const Document &document,
                                const std::vector<DocumentPart> &documentParts,
                                bool isGenerated = false);


        unsigned int _imageCounter = 1;
        unsigned int _figureCounter = 1;
        std::string _nameBase = "outfile";

    };

}
