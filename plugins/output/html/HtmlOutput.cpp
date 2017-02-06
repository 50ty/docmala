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
        outFile << "</head>" << std::endl;
        outFile << "<body>" << std::endl;
        outFile << "" << std::endl;
        outFile << "" << std::endl;

        bool paragraphOpen = false;
        DocumentPart previous;

        for( const auto &part : document ) {
            switch(part.type() ) {
            case DocumentPart::Type::Invalid:
                break;
            case DocumentPart::Type::Custom:
                break;
            case DocumentPart::Type::Headline: {
                if( paragraphOpen ) {
                    outFile << "</p>" << std::endl;
                    paragraphOpen = false;
                }
                auto headline = part.headline();
                outFile << "<h" << headline->level << ">";
                outFile << headline->text;
                outFile << "</h" << headline->level << ">" << std::endl;
                break;
            }
            case DocumentPart::Type::Text: {
                auto text = part.text();

                if( text->bold ) {
                    outFile << "<b>";
                }
                if( text->italic ) {
                    outFile << "<i>";
                }
                if( text->crossedOut ) {
                    outFile << "<del>";
                }
                outFile << text->text << std::endl;

                if( text->bold ) {
                    outFile << "</b>";
                }
                if( text->italic ) {
                    outFile << "</i>";
                }
                if( text->crossedOut ) {
                    outFile << "</del>";
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
                auto image = part.image();
                std::ofstream imgFile;
                std::stringstream fileName;
                fileName << nameBase << "_image_" << imageCounter << "." << image->format;

                imgFile.open(fileName.str());
                imgFile << image->data;
                imgFile.close();

                outFile << "<figure>" << std::endl;
                outFile << "<img src=\"" << fileName.str() <<"\" alt=\""<< image->text <<"\">" << std::endl;
                if( previous.type() == DocumentPart::Type::Caption ) {
                    outFile << "<figcaption>Figure " << figureCounter << ": " << previous.caption()->text << "</figcaption>" << std::endl;
                    figureCounter++;
                }
                outFile << "</figure>" << std::endl;
                imageCounter++;
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

EXTENSION_SYSTEM_EXTENSION(docmala::OutputPlugin, HtmlOutput, "html", 1, "Write document to a HTML file", EXTENSION_SYSTEM_NO_USER_DATA )
