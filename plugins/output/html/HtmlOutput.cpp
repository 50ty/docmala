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

//void writeText(std::stringstream &outFile, const DocumentPart::Text *printText, bool isGenerated)
//{

//}

void writeFormatedText(std::stringstream &outFile, const DocumentPart::FormatedText *text)
{
    if( text->bold ) {
        outFile << "<b>";
    }
    if( text->italic ) {
        outFile << "<i>";
    }
    if( text->underlined ) {
        outFile << "<u>";
    }
    if( text->stroked ) {
        outFile << "<del>";
    }
    if( text->monospaced ) {
        outFile << "<tt>";
    }
    std::string txt = text->text;
    replaceAll(txt, "<", "&lt;");
    replaceAll(txt, ">", "&gt;");
    outFile << txt;

    if( text->bold ) {
        outFile << "</b>";
    }
    if( text->italic ) {
        outFile << "</i>";
    }
    if( text->stroked ) {
        outFile << "</del>";
    }
    if( text->monospaced ) {
        outFile << "</tt>";
    }
    if( text->underlined ) {
        outFile << "</u>";
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

    outFile << cde;
    outFile << "</code> </pre>" << std::endl;

}


void HtmlOutput::writeTable(std::stringstream &outFile, const DocumentPart::Table *table)
{
    outFile << "<table>" << std::endl;
    bool firstRow = true;
    for( const auto &row : table->cells ) {
        bool firstColumn = true;
        outFile << "<tr>" << std::endl;
        std::string end;

        for( const auto &cell : row ) {
            std::string span;
            if( cell.isHiddenBySpan ) {
                continue;
            }

            if( cell.columnSpan > 0 ) {
                span = " colspan=\"" + std::to_string(cell.columnSpan+1) + "\"";
            }
            if( cell.rowSpan > 0 ) {
                span += " rowspan=\"" + std::to_string(cell.rowSpan+1) + "\"";
            }
            if( cell.isHeading && firstRow ) {
                outFile << "<th scope=\"col\""+span+">" << std::endl;
                end = "</th>";
            } else if( cell.isHeading && firstColumn ) {
                outFile << "<th scope=\"row\""+span+">" << std::endl;
                end = "</th>";
            } else {
                outFile << "<td"+span+">" << std::endl;
                end = "</td>";
            }

            writeDocumentParts(outFile, cell.content, true);
            outFile << std::endl << end << std::endl;

            firstColumn = false;
        }
        outFile << "</tr>" << std::endl;
        firstRow = false;
    }
    outFile << "</table>" << std::endl;
}

void HtmlOutput::writeList(std::stringstream &outFile, std::vector<DocumentPart>::const_iterator &start, const std::vector<DocumentPart> &documentParts, bool isGenerated, int currentLevel)
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
            writeList(outFile, start, documentParts, isGenerated, currentLevel);
            if( start +1 == documentParts.end() || (start+1)->type() != DocumentPart::Type::List || (start+1)->list()->level < currentLevel ) {
                break;
            } else {
                start++;
            }
        }
        outFile << "</" << type << ">" << std::endl;
        currentLevel--;
    } else if( currentLevel == list->level ) {
        for( auto entry = list->entries.begin(); entry != list->entries.end(); entry++ ) {
            outFile << "<li> ";
            writeDocumentParts(outFile, {*entry}, isGenerated);
            if( entry == list->entries.end()-1 ) {
                if( start +1 != documentParts.end() && (start+1)->type() == DocumentPart::Type::List && (start+1)->list()->level > currentLevel ) {
                    start++;
                    writeList(outFile, start, documentParts, isGenerated, currentLevel);
                }
            }
            outFile << " </li>" << std::endl;
        }
    }
}

void HtmlOutput::writeDocumentParts(std::stringstream &outFile, const std::vector<DocumentPart> &documentParts, bool isGenerated)
{
    bool paragraphOpen = false;
    std::vector<DocumentPart>::const_iterator previous = documentParts.end();

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
            if( !isGenerated ) {
                outFile << "<span " << id(headline) << ">";
            }
            writeDocumentParts(outFile, headline->text, isGenerated);
            if( !isGenerated ) {
                outFile << "</span>" << std::endl;
            }
            outFile << "</h" << headline->level << ">" << std::endl;
            break;
        }
        case DocumentPart::Type::FormatedText: {
            auto text = part->formatedText();
            writeFormatedText(outFile, text);
            break;
        }
        case DocumentPart::Type::Text: {
            auto text = part->text();

            if( !isGenerated ) {
                outFile << "<span " << id(text) << ">";
            }

            writeDocumentParts(outFile, text->text, isGenerated);

            if( !isGenerated ) {
                outFile << "</span>" << std::endl;
            }
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
            if( _embedImages ) {
                outFile << "<img src=\"data:image/" << image->format << ";base64,";
                outFile << base64_encode( image->data );
                outFile << "\">";
            } else {
                std::ofstream imgFile;
                std::stringstream fileName;

                fileName << _nameBase << "_image_" << _imageCounter << "." << image->fileExtension;

                imgFile.open(fileName.str(), std::ofstream::binary | std::ofstream::out);
                imgFile << image->data;
                imgFile.close();

                std::string imageImportName = fileName.str();
                std::replace( imageImportName.begin(), imageImportName.end(), '\\', '/');
                if( imageImportName.find_last_of('/') != std::string::npos ) {
                    imageImportName = imageImportName.substr(imageImportName.find_last_of('/')+1);
                }

                outFile << "<img src=\"" << imageImportName <<"\">" << std::endl;
            }
            if( previous->type() == DocumentPart::Type::Caption ) {
                outFile << "<figcaption>Figure " << _figureCounter << ": ";
                if( !isGenerated ) {
                    outFile << "<span " << id(previous->caption()) << ">";
                }
                writeDocumentParts(outFile, previous->caption()->text, isGenerated );
                if( !isGenerated ) {
                    outFile << "</span>" << std::endl;
                }
                outFile << "</figcaption>" << std::endl;
                _figureCounter++;
            }
            outFile << "</figure>" << std::endl;
            _imageCounter++;
            break;
        }
        case DocumentPart::Type::List: {
            writeList(outFile, part, documentParts, isGenerated);
            break;
        }
        case DocumentPart::Type::GeneratedDocument: {
            auto generated = part->generatedDocument();
            outFile << "<div " << id(generated) << ">" << std::endl;
            writeDocumentParts(outFile, generated->document, true);
            outFile << "</div>" << std::endl;
            break;
        }
        case DocumentPart::Type::Code: {
            auto code = part->code();

            outFile << "<figure"<<id(code)<<">" << std::endl;
            writeCode(outFile, code);
            if( previous->type() == DocumentPart::Type::Caption ) {
                outFile << "<figcaption>Listing " << _listingCounter << ": ";
                if( !isGenerated ) {
                    outFile << "<span " << id(previous->caption()) << ">";
                }
                writeDocumentParts(outFile, previous->caption()->text, isGenerated );
                if( !isGenerated ) {
                    outFile << "</span>" << std::endl;
                }
                outFile << "</figcaption>" << std::endl;
                _listingCounter++;
            }
            outFile << "</figure>" << std::endl;
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
        case DocumentPart::Type::Table: {
            auto table = part->table();
            outFile << "<figure"<<id(table)<<">" << std::endl;
            writeTable(outFile, table);
            if( previous->type() == DocumentPart::Type::Caption ) {
                outFile << "<figcaption>Table " << _tableCounter << ": ";
                if( !isGenerated ) {
                    outFile << "<span " << id(previous->caption()) << ">";
                }
                writeDocumentParts(outFile, previous->caption()->text, isGenerated );
                if( !isGenerated ) {
                    outFile << "</span>" << std::endl;
                }
                outFile << "</figcaption>" << std::endl;
                _tableCounter++;
            }
            outFile << "</figure>" << std::endl;
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

    std::string pluginDir = "./";
    auto pluginDirIter = parameters.find("pluginDir");
    if( pluginDirIter != parameters.end() ) {
        pluginDir =  pluginDirIter->second.value + '/';
    }

    _embedImages = parameters.find("embedImages") != parameters.end();

    std::string codeHighlightScript;
    std::string codeHighlightCSS;
    std::string generalCSS;

    std::ifstream highlightReader(pluginDir + "outputPluginHtmlCodeHighlight.js", std::ios::in | std::ios::binary);
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


    std::ifstream codeHighlightReader(pluginDir + "outputPluginHtmlDefaultStyle.css", std::ios::in | std::ios::binary);
    if (codeHighlightReader)
    {
        codeHighlightReader.seekg(0, std::ios::end);
        codeHighlightCSS.resize( static_cast<std::string::size_type>(codeHighlightReader.tellg()));
        codeHighlightReader.seekg(0, std::ios::beg);
        codeHighlightReader.read(&codeHighlightCSS[0], static_cast<std::streamsize>(codeHighlightCSS.size()));
        codeHighlightReader.close();
    }

    std::ifstream generalCSSReader(pluginDir + "outputPluginHtmlCodeHighlight.css", std::ios::in | std::ios::binary);
    if (generalCSSReader)
    {
        generalCSSReader.seekg(0, std::ios::end);
        generalCSS.resize( static_cast<std::string::size_type>(generalCSSReader.tellg()));
        generalCSSReader.seekg(0, std::ios::beg);
        generalCSSReader.read(&generalCSS[0], static_cast<std::streamsize>(generalCSS.size()));
        generalCSSReader.close();
    }

    HtmlDocument html;

    std::stringstream head;
    std::stringstream body;


    head << "<meta charset=\"utf-8\">" << std::endl;
    if( document.metaData().find("title") != document.metaData().end() ) {
        head << "<title>"+document.metaData().at("title").data.front().value+"</title>" << std::endl;
    } else {
        head << "<title>[No title]</title>" << std::endl;
    }

    head << "<style>" << std::endl;
    head << codeHighlightCSS << std::endl;
    head << generalCSS << std::endl;
    head << "</style>" << std::endl;

    head << "<script>" << std::endl;
    head << scripts << std::endl;
    head << "</script>" << std::endl;
    head << "<script>" << std::endl;
    head << codeHighlightScript << std::endl;
    head << "</script>" << std::endl;
    head << "<script>hljs.initHighlightingOnLoad();</script>" << std::endl;

    writeDocumentParts(body, document.parts() );

    html.head = head.str();
    html.body = body.str();
    return html;
}

EXTENSION_SYSTEM_EXTENSION(docmala::OutputPlugin, HtmlOutputPlugin, "html", 1, "Write document to a HTML file", EXTENSION_SYSTEM_NO_USER_DATA )
