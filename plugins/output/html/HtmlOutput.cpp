#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <fstream>
#include <sstream>

#include "HtmlOutput.h"

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

std::string base64_encode(const std::string& inputData) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for( auto data = inputData.begin(); data != inputData.end(); data++)
    {
        char_array_3[i++] = *data;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] =   char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;
}


using namespace docmala;

class HtmlOutputPlugin : public OutputPlugin
{
    // OutputPlugin interface
public:
    bool write(const ParameterList &parameters, const Document &document) override {
        std::string nameBase = "outfile";
        std::string outputFileName = "outfile.html";

        auto inputFile = parameters.find("inputFile");

        if( inputFile != parameters.end() ) {
            outputFileName = inputFile->second.value;
            nameBase = outputFileName.substr(0, outputFileName.find_last_of("."));
            outputFileName = nameBase + ".html";
        }

        std::ofstream outFile;

        outFile.open(outputFileName);

        if( outFile.is_open() ) {
            HtmlOutput output;
            outFile << output.produceHtml(parameters, document);
            return true;
        }
        return false;
    }
};




void writeText(std::stringstream &outFile, const DocumentPart::Text *printText)
{
    for( const auto &text : printText->text )
    {
        if( text.bold ) {
            outFile << "<b>";
        }
        if( text.italic ) {
            outFile << "<i>";
        }
        if( text.crossedOut ) {
            outFile << "<del>";
        }
        outFile << text.text << std::endl;

        if( text.bold ) {
            outFile << "</b>";
        }
        if( text.italic ) {
            outFile << "</i>";
        }
        if( text.crossedOut ) {
            outFile << "</del>";
        }
    }
}

void writeList(std::stringstream &outFile, std::vector<DocumentPart>::const_iterator &start, const Document &document, int currentLevel = 0)
{
    for( ; start != document.parts().end(); start++ ) {
        if( start->type() != DocumentPart::Type::List ) {
            start--;
            return;
        }

        // TODO: Currently, mixed lists are not upported, meaning that:
        // * text
        // # text 2
        // will be treated as:
        // * text
        // * text 2

        auto list = start->list();
        if( currentLevel < list->level) {
            std::string type = "ul";
            std::string style;
            switch( list->type ) {
            case DocumentPart::List::Type::Points:
                type = "ul";
                break;
            case DocumentPart::List::Type::Dashes:
                type = "ul";
                style = "class=\"dash\"";
                break;
            case DocumentPart::List::Type::Numbered:
                type = "ol";
                break;

            }
            currentLevel++;
            outFile << "<" << type << " " << style << ">" << std::endl;
            writeList(outFile, start, document, currentLevel);
            outFile << "</" << type << ">" << std::endl;
            currentLevel--;
        } else if( currentLevel == list->level ) {
            for( auto entry : list->entries ) {
                outFile << "<li> ";
                writeText(outFile, &entry);
                outFile << " </li>" << std::endl;
            }
        } else {
            start--;
            return;
        }

    }
}



std::string HtmlOutput::produceHtml(const ParameterList &parameters, const Document &document)
{
    std::string nameBase = "outfile";
    std::string outputFileName = "outfile.html";

    auto inputFile = parameters.find("inputFile");

    if( inputFile != parameters.end() ) {
        outputFileName = inputFile->second.value;
        nameBase = outputFileName.substr(0, outputFileName.find_last_of("."));
        outputFileName = nameBase + ".html";
    }

    bool embedImages = parameters.find("embedImages") != parameters.end();


    std::stringstream outFile;

    unsigned int imageCounter = 1;
    unsigned int figureCounter = 1;

    outFile << "<!doctype html>" << std::endl;
    outFile << "<html>" << std::endl;
    outFile << "<head>" << std::endl;
    outFile << "<meta charset=\"utf-8\">" << std::endl;
    outFile << "<title>No title yet</title>" << std::endl;

    outFile << "<style>" << std::endl;
    outFile << "ul.dash { list-style-type: none; }" << std::endl;

    outFile << "ul.dash li:before { content: '-'; position: absolute; margin-left: -15px; }" << std::endl;
    outFile << "</style>" << std::endl;

    outFile << "</head>" << std::endl;
    outFile << "<body>" << std::endl;
    outFile << "" << std::endl;
    outFile << "" << std::endl;

    bool paragraphOpen = false;
    auto previous = document.parts().end();

    for( auto part = document.parts().begin(); part != document.parts().end(); part++ ) {
        switch(part->type() ) {
        case DocumentPart::Type::Invalid:
            break;
        case DocumentPart::Type::Custom:
            break;
        case DocumentPart::Type::Headline: {
            if( paragraphOpen ) {
                outFile << "</p>" << std::endl;
                paragraphOpen = false;
            }
            auto headline = part->headline();
            outFile << "<h" << headline->level << ">";
            writeText(outFile, headline);
            outFile << "</h" << headline->level << ">" << std::endl;
            break;
        }
        case DocumentPart::Type::Text: {
            auto text = part->text();
            writeText(outFile, text);
            break;
        }
        case DocumentPart::Type::Paragraph:
            if( paragraphOpen ) {
                outFile << "</p>" << std::endl;
                paragraphOpen = false;
            }
            outFile << "<p>" << std::endl;
            paragraphOpen = true;
            break;
        case DocumentPart::Type::Image: {
            auto image = part->image();
            outFile << "<figure>" << std::endl;
            if( embedImages ) {
                outFile << "<img src=\"data:image/" << image->format << ";base64,";
                outFile << base64_encode( image->data );
                outFile << "\">";
            } else {
                std::ofstream imgFile;
                std::stringstream fileName;
                fileName << nameBase << "_image_" << imageCounter << "." << image->format;

                imgFile.open(fileName.str());
                imgFile << image->data;
                imgFile.close();
                outFile << "<img src=\"" << fileName.str() <<"\">" << std::endl;
            }
            if( previous != document.parts().end() && previous->type() == DocumentPart::Type::Caption ) {
                outFile << "<figcaption>Figure " << figureCounter << ": ";
                writeText(outFile, previous->caption() );
                outFile << "</figcaption>" << std::endl;
                figureCounter++;
            }
            outFile << "</figure>" << std::endl;
            imageCounter++;
            break;
        }
        case DocumentPart::Type::List: {
            writeList(outFile, part, document);
            break;
        }
        default:
            break;
        }
        previous = part;
    }

    if( paragraphOpen ) {
        outFile << "</p>" << std::endl;
        paragraphOpen = false;
    }

    outFile << "</body>" << std::endl;

    return outFile.str();
}

EXTENSION_SYSTEM_EXTENSION(docmala::OutputPlugin, HtmlOutputPlugin, "html", 1, "Write document to a HTML file", EXTENSION_SYSTEM_NO_USER_DATA )
