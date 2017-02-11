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
        std::string produceHtml(const ParameterList &parameters, const Document &document);

    //    void writeText(std::ofstream &outFile, const DocumentPart::Text *printText);
    //    void writeList(std::ofstream &outFile, std::vector<DocumentPart>::const_iterator &start, const std::vector<DocumentPart> &document, int currentLevel = 0);

    };

}
