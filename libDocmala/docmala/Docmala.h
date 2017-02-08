#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <extension_system/ExtensionSystem.hpp>

#include "Parameter.h"
#include "DocumentPart.h"
#include "DocmaPlugin.h"
#include "Error.h"
#include "File.h"

namespace docmala {

    class Docmala {
    public:
        Docmala();

        bool parseFile( const std::string &fileName, const std::string &outputDir );

        bool produceOutput( const std::string &pluginName );

        std::vector<std::string> listOutputPlugins() const;

        std::vector<Error> errors() const {
            return _errors;
        }

        const std::vector<DocumentPart>& document() const {
            return _document;
        }

    private:

        bool readHeadLine();
        bool readCaption();
        bool readLine(std::string &destination);
        bool readPlugin();
        bool readText(char startCharacter, DocumentPart::Text &text);
        bool readParameterList(ParameterList &parameters, char blockEnd);
        bool readBlock(std::string &block);
        bool readList(DocumentPart::List::Type type);

        bool isWhitespace(char c) const;


        /**
         * A document consists of many document parts
         * All of these parts are stored in this variable
         */
        std::vector<DocumentPart> _document;
        File _file;
        std::vector<Error> _errors;
        extension_system::ExtensionSystem _pluginLoader;
        std::string _outputDir;
        std::map<std::string, std::shared_ptr<DocumentPlugin>> _loadedDocumentPlugins;
    };
}
