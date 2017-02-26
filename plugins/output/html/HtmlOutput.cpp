#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <fstream>
#include <sstream>
#include <algorithm>

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

std::string escapeAnchor( const std::string anchor ) {
    std::string escape = anchor;
    std::replace( escape.begin(), escape.end(), '.', 'd');
    std::replace( escape.begin(), escape.end(), ':', 'c');
    return escape;
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
            auto html = output.produceHtml(parameters, document);

            outFile << "<!doctype html>" << std::endl;
            outFile << "<html>" << std::endl;
            outFile << "<head>" << std::endl;
            outFile << html.head << std::endl;
            outFile << "</head>" << std::endl;

            outFile << "<body>" << std::endl;
            outFile << html.body << std::endl;
            outFile << "</body>" << std::endl;
            return true;
        }
        return false;
    }
};

void replaceAll( std::string& source, const std::string& from, const std::string& to )
{
    std::string newString;
    newString.reserve( source.length() );  // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos = 0;

    while( std::string::npos != ( findPos = source.find( from, lastPos ))) {
        newString.append( source, lastPos, findPos - lastPos );
        newString += to;
        lastPos = findPos + from.length();
    }

    newString += source.substr( lastPos );

    source.swap( newString );
}

std::string id(const DocumentPart::VisualElement *element)
{
    if( element->line > 0 )
        return std::string(" id=\"line_") + std::to_string(element->line) +"\"";
    return "";
}

void writeText(std::stringstream &outFile, const DocumentPart::Text *printText, bool isGenerated)
{
    if( !isGenerated ) {
        outFile << "<span " << id(printText) << ">";
    }

    for( const auto &text : printText->text ) {
        if( text.bold ) {
            outFile << "<b>";
        }
        if( text.italic ) {
            outFile << "<i>";
        }
        if( text.crossedOut ) {
            outFile << "<del>";
        }
        outFile << text.text;

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

    if( !isGenerated ) {
        outFile << "</span>" << std::endl;
    }
}

void writeCode(std::stringstream &outFile, const DocumentPart::Code *code)
{
    if( !code->type.empty() ) {
        outFile << "<pre"<<id(code)<<"> <code class=\""<<code->type<<"\">" << std::endl;
    } else {
        outFile << "<pre> <code>" << std::endl;
    }

    std::string cde = code->code;

    replaceAll(cde, "<", "&lt;");
    replaceAll(cde, ">", "&gt;");

    outFile << cde << std::endl;
    outFile << "</code> </pre>" << std::endl;

}

void writeList(std::stringstream &outFile, std::vector<DocumentPart>::const_iterator &start, const Document &document, bool isGenerated, int currentLevel = 0)
{
    auto list = start->list();
    if( currentLevel < list->level ) {
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
        while( true ) {
            writeList(outFile, start, document, isGenerated, currentLevel);
            if( start +1 == document.parts().end() || (start+1)->type() != DocumentPart::Type::List || (start+1)->list()->level < currentLevel ) {
                break;
            } else {
                start++;
            }
        }
        outFile << "</" << type << ">" << std::endl;
        currentLevel--;
    } else if( currentLevel == list->level ) {
        for( auto entry : list->entries ) {
            outFile << "<li> ";
            writeText(outFile, &entry, isGenerated);
            outFile << " </li>" << std::endl;
        }
    }
}

void HtmlOutput::writeDocumentParts(std::stringstream &outFile, const ParameterList &parameters, const Document &document, const std::vector<DocumentPart> &documentParts, bool isGenerated)
{
    bool paragraphOpen = false;
    auto previous = document.parts().end();

    bool embedImages = parameters.find("embedImages") != parameters.end();


    for( std::vector<DocumentPart>::const_iterator part = documentParts.begin(); part != documentParts.end(); part++ ) {
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
            writeText(outFile, headline, isGenerated);
            outFile << "</h" << headline->level << ">" << std::endl;
            break;
        }
        case DocumentPart::Type::Text: {
            auto text = part->text();
            writeText(outFile, text, isGenerated);
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
            outFile << "<figure"<<id(image)<<">" << std::endl;
            if( embedImages ) {
                outFile << "<img src=\"data:image/" << image->format << ";base64,";
                outFile << base64_encode( image->data );
                outFile << "\">";
            } else {
                std::ofstream imgFile;
                std::stringstream fileName;
                fileName << _nameBase << "_image_" << _imageCounter << "." << image->format;

                imgFile.open(fileName.str());
                imgFile << image->data;
                imgFile.close();
                outFile << "<img src=\"" << fileName.str() <<"\">" << std::endl;
            }
            if( previous != document.parts().end() && previous->type() == DocumentPart::Type::Caption ) {
                outFile << "<figcaption>Figure " << _figureCounter << ": ";
                writeText(outFile, previous->caption(), isGenerated );
                outFile << "</figcaption>" << std::endl;
                _figureCounter++;
            }
            outFile << "</figure>" << std::endl;
            _imageCounter++;
            break;
        }
        case DocumentPart::Type::List: {
            writeList(outFile, part, document, isGenerated);
            break;
        }
        case DocumentPart::Type::GeneratedDocument: {
            auto generated = part->generatedDocument();
            outFile << "<div " << id(generated) << ">" << std::endl;
            writeDocumentParts(outFile, parameters, document, generated->document, true);
            outFile << "</div>" << std::endl;
            break;
        }
        case DocumentPart::Type::Code: {
            auto code = part->code();

            writeCode(outFile, code);
            break;
        }
        case DocumentPart::Type::Anchor: {
            auto anchor = part->anchor();
            outFile << "<a id=\"" << escapeAnchor(anchor->name) << "\"/>" << std::endl;
            break;
        }
        case DocumentPart::Type::Link: {
            auto link = part->link();
            std::string text = link->text;
            if( text.empty() ) {
                text = link->data;
            }
            if( link->type == DocumentPart::Link::Type::IntraFile ) {
                outFile << "<a href=\"#" << escapeAnchor(link->data) << "\">" << text << "</a>" << std::endl;
            } else if( link->type == DocumentPart::Link::Type::InterFile ) {
                std::string data = link->data;
                data[data.find(":")] = '#';
                outFile << "<a href=\"" << data << "\">" << text << "</a>" << std::endl;
            } else if( link->type == DocumentPart::Link::Type::Web ) {
                outFile << "<a href=\"" << link->data << "\">" << text << "</a>" << std::endl;
            }

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

}

HtmlOutput::HtmlDocument HtmlOutput::produceHtml(const ParameterList &parameters, const Document &document, const std::string &scripts)
{
    std::string outputFileName = "outfile.html";

    auto inputFile = parameters.find("inputFile");

    if( inputFile != parameters.end() ) {
        outputFileName = inputFile->second.value;
        _nameBase = outputFileName.substr(0, outputFileName.find_last_of("."));
        outputFileName = _nameBase + ".html";
    }

    std::string pluginDir;
    auto pluginDirIter = parameters.find("pluginDir");
    if( pluginDirIter != parameters.end() ) {
        pluginDir =  pluginDirIter->second.value + '/';
    }

    std::string codeHighlightScript;
    std::string codeHighlightCSS;

    std::ifstream highlightReader(pluginDir + "/htmlOutputPluginCodeHighlight.js", std::ios::in | std::ios::binary);
    if (highlightReader)
    {
        highlightReader.seekg(0, std::ios::end);
        codeHighlightScript.resize( static_cast<std::string::size_type>(highlightReader.tellg()));
        highlightReader.seekg(0, std::ios::beg);
        highlightReader.read(&codeHighlightScript[0], static_cast<std::streamsize>(codeHighlightScript.size()));
        highlightReader.close();

        replaceAll( codeHighlightScript, "<script", "&lt;script");
        replaceAll( codeHighlightScript, "</script", "&lt;/script");
    }


    std::ifstream cssReader(pluginDir + "/htmlOutputPluginCodeHighlight.css", std::ios::in | std::ios::binary);
    if (cssReader)
    {
        cssReader.seekg(0, std::ios::end);
        codeHighlightCSS.resize( static_cast<std::string::size_type>(cssReader.tellg()));
        cssReader.seekg(0, std::ios::beg);
        cssReader.read(&codeHighlightCSS[0], static_cast<std::streamsize>(codeHighlightCSS.size()));
        cssReader.close();
    }

    HtmlDocument html;

    std::stringstream head;
    std::stringstream body;


    head << "<meta charset=\"utf-8\">" << std::endl;
    head << "<title>No title yet</title>" << std::endl;

    head << "<style>" << std::endl;
    head << "ul.dash { list-style-type: none; }" << std::endl;
    head << "ul.dash li:before { content: '-'; position: absolute; margin-left: -15px; }" << std::endl;
    head << codeHighlightCSS << std::endl;
    head << "</style>" << std::endl;

    head << "<script>" << std::endl;
    head << scripts << std::endl;
    head << "</script>" << std::endl;
    head << "<script>" << std::endl;
    head << codeHighlightScript << std::endl;
    head << "</script>" << std::endl;
    head << "<script>hljs.initHighlightingOnLoad();</script>" << std::endl;

    writeDocumentParts(body, parameters, document, document.parts() );

    html.head = head.str();
    html.body = body.str();
    return html;
}

EXTENSION_SYSTEM_EXTENSION(docmala::OutputPlugin, HtmlOutputPlugin, "html", 1, "Write document to a HTML file", EXTENSION_SYSTEM_NO_USER_DATA )
