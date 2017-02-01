#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <extension_system/ExtensionSystem.hpp>

#include "Parameter.h"
#include "DocumentPart.h"
#include "Error.h"

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
        bool readText(char startCharacter);
        bool readParameterList(ParameterList &parameters, char blockEnd);
        bool readBlock(std::string &block);



        class File {
        public:
            File( ) {
            }

            explicit File( const std::string &fileName ) {
                _file.open(fileName);
                _fileName = fileName;
            }

            bool isOpen() const {
                return _file.is_open();
            }

            bool isEoF() const {
                return _file.eof() || _file.fail();
            }

            std::string fileName() const {
                return _fileName;
            }

            char getch() {
                char c = '\0';
                _file.get(c);
                if( c == '\n' ) {
                    _line++;
                    _column = 0;
                } else if( c == '\r' ) {
                    return getch();
                } else if( c == '.' ) {
                    auto pos = _file.tellg();
                    auto line = _line;
                    auto column = _column;

                    if( getch() == '.' && getch() == '.' && getch() == '\n' )
                    {
                        while( true ) {
                            c=getch();
                            if( c != ' ' && c != '\t' )
                                break;
                        }
                    } else {
                        _line = line;
                        _column = column;
                        _file.seekg(pos);
                    }
                } else {
                    _column++;
                }
                return c;
            }

            FileLocation location() const {
                return FileLocation {_line, _column, _fileName };
            }

        private:
            int _line = 1;
            int _column = 0;
            std::string _fileName;
            std::ifstream _file;
        };

        /**
         * A document consists of many document parts
         * All of these parts are stored in this variable
         */
        std::vector<DocumentPart> _document;
        File _file;
        std::vector<Error> _errors;
        extension_system::ExtensionSystem _pluginLoader;
        std::string _outputDir;
    };
}
