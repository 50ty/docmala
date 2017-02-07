#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <fstream>
#include <sstream>

using namespace docmala;

class HtmlOutput : public OutputPlugin
{


    // OutputPlugin interface
public:
    bool write(const ParameterList &parameters, const std::vector<DocumentPart> &document) override;

    void writeText(std::ofstream &outFile, const DocumentPart::Text *printText);
    void writeList(std::ofstream &outFile, std::vector<DocumentPart>::const_iterator &start, const std::vector<DocumentPart> &document, int currentLevel = 0);

};


bool HtmlOutput::write(const ParameterList &parameters, const std::vector<DocumentPart> &document)
{
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

    unsigned int imageCounter = 1;
    unsigned int figureCounter = 1;

    if( outFile.is_open() ) {
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
        auto previous = document.end();

        for( auto part = document.begin(); part != document.end(); part++ ) {
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
                std::ofstream imgFile;
                std::stringstream fileName;
                fileName << nameBase << "_image_" << imageCounter << "." << image->format;

                imgFile.open(fileName.str());
                imgFile << image->data;
                imgFile.close();

                outFile << "<figure>" << std::endl;
                outFile << "<img src=\"" << fileName.str() <<"\">" << std::endl;
                if( previous != document.end() && previous->type() == DocumentPart::Type::Caption ) {
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
    } else {
        return false;
    }

    return true;
}

void HtmlOutput::writeText(std::ofstream &outFile, const DocumentPart::Text *printText)
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

void HtmlOutput::writeList(std::ofstream &outFile, std::vector<DocumentPart>::const_iterator &start, const std::vector<DocumentPart> &document, int currentLevel)
{
    for( ; start != document.end(); start++ ) {
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

EXTENSION_SYSTEM_EXTENSION(docmala::OutputPlugin, HtmlOutput, "html", 1, "Write document to a HTML file", EXTENSION_SYSTEM_NO_USER_DATA )
