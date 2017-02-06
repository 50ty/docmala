#include "Docmala.h"
#include "DocmaPlugin.h"

using namespace docmala;



Docmala::Docmala()
{
    _pluginLoader.searchDirectory("./", false);
}

bool Docmala::parseFile(const std::string &fileName, const std::string &outputDir)
{
    _file = File(fileName);
    _outputDir = outputDir;

    enum class Mode {
        BeginOfLine
    } mode { Mode::BeginOfLine };

    while( !_file.isEoF() )
    {
        char c = _file.getch();

        if( mode == Mode::BeginOfLine ) {
            switch(c)
            {
            case ' ':
            case '\t':
                continue;
            case '\n':
                if( _document.size() > 0 && _document.back().type() != DocumentPart::Type::Paragraph ) {
                    _document.push_back(DocumentPart::newParagraph());
                }
                continue;
            case '=':
                readHeadLine();
                break;
            case '.':
                readCaption();
                break;
            case '[':
                readPlugin();
                break;
            default:
                readText(c);
            }
        }
    }
    return true;
}

bool Docmala::produceOutput(const std::string &pluginName)
{
    auto plugin = _pluginLoader.createExtension<OutputPlugin>(pluginName);

    if( plugin ) {
        ParameterList parameters;

        parameters.insert(std::make_pair("inputFile", Parameter{"inputFile", _file.fileName(), FileLocation()} ) );
        plugin->write(parameters, _document);
    } else {
        _errors.push_back(Error{FileLocation(), "Unable to load output plugin '" + pluginName + "'." });
        return false;
    }
    return true;
}

std::vector<std::string> Docmala::listOutputPlugins() const
{
    auto plugins = _pluginLoader.extensions<OutputPlugin>();
    std::vector<std::string> knownOutputPlugins;

    for( auto plugin: plugins ) {
        knownOutputPlugins.push_back(plugin.name() + "  -  " + plugin.description());
    }

    return knownOutputPlugins;
}

bool Docmala::readHeadLine()
{
    DocumentPart::Headline headline;
    headline.level = 1;

    while( !_file.isEoF() ) {
        char c = _file.getch();
        if( c == '\n' ) {
            _errors.push_back(Error{_file.location(), std::string("Headline contains no text and is skipped.") });
            return false;
        }

        if( c == '=' ) {
            headline.level++;
        } else {
            headline.text.push_back(c);
            readLine(headline.text);
            break;
        }
    }

    _document.push_back(DocumentPart(headline));
    return true;

}

bool Docmala::readCaption()
{
    std::string caption;
    if( !readLine(caption) )
        return false;
    _document.push_back(DocumentPart::newCaption(caption));
    return true;
}

bool Docmala::readLine(std::string &destination)
{
    while( !_file.isEoF() ) {
        char c = _file.getch();
        if( c == '\n' ) {
            break;
        }
        destination.push_back(c);
    }
    return true;
}

bool Docmala::readPlugin()
{
    enum class Mode {
        Begin,
        ParameterName,
        SkipUntilNewLine
    } mode { Mode::Begin };

    FileLocation nameBegin;
    std::string name;
    ParameterList parameters;
    parameters.insert(std::make_pair("outputDir", Parameter { "outputDir", _outputDir, FileLocation() }));
    parameters.insert(std::make_pair("inputFile", Parameter{"inputFile", _file.fileName(), FileLocation()} ) );

    bool skipWarningPrinted = false;

    while( !_file.isEoF() ) {
        char c = _file.getch();

        if( mode == Mode::Begin )
        {
            if( c == ' ' || c == '\t' ) {
                continue;
            } else {
                mode = Mode::ParameterName;
                nameBegin = _file.location();
            }
        }
        if( mode == Mode::ParameterName ) {
            if( c == ',' ) {
                if( !readParameterList(parameters, ']' ) ) {
                    return false;
                } else {
                    mode = Mode::SkipUntilNewLine;
                    continue;
                }
            } else if( c == ']' ) {
                mode = Mode::SkipUntilNewLine;
                continue;
            } else if( (c >= 'a' && c <= 'z') ||
                       (c >= 'A' && c <= 'Z') ||
                       (c >= '0' && c <= '9') ||
                       c == '_' || c == '-' ) {
                name.push_back(c);
            } else {
                _errors.push_back(Error{nameBegin, std::string("Error while parsing plugin name. A valid name consists of ['a'-'z', 'A'-'Z', '0'-'9', '_', '-'] but a '") + c +"' was found." });
            }
        } else if( mode == Mode::SkipUntilNewLine )
        {
            if( c == '\n' ) {
                break;
            } else {
                if( !skipWarningPrinted ) {
                    _errors.push_back(Error{_file.location(), std::string("Text found after ']' is skipped.") });
                    skipWarningPrinted = true;
                }
            }
        }
    }

    auto plugin = _pluginLoader.createExtension<DocumentPlugin>(name);
    if( !plugin ) {
        _errors.push_back(Error{nameBegin, std::string("Unable to load plugin with name: '") + name + "'." });
        return false;
    } else {
        if( plugin->blockProcessing() == DocumentPlugin::BlockProcessing::Required ||
            plugin->blockProcessing() == DocumentPlugin::BlockProcessing::Optional ) {
            std::string block;
            if( !readBlock(block) ) {
                return false;
            } else {
                plugin->process(parameters, nameBegin, _document, block);
            }
        } else {
            plugin->process(parameters, nameBegin, _document, "");
        }
    }

    return true;
}

bool isFormatSpecifier(char c)
{
    return c == '_' || c == '*' || c == '-';
}

bool Docmala::readText(char startCharacter)
{
    char c = startCharacter;
    DocumentPart::Text text;

    while( true ) {
        if( isFormatSpecifier(c) ) {
            if((isWhitespace(_file.previous()) || isFormatSpecifier(_file.previous()))
                && !isWhitespace(_file.next()) ) {
                if( !text.text.empty() ) {
                    _document.push_back(DocumentPart{text} );
                    text.text.clear();
                }
                switch(c) {
                case '_':
                    text.bold = true;
                    break;
                case '*':
                    text.italic = true;
                    break;
                case '-':
                    text.crossedOut = true;
                    break;
                }
            } else if( (!isWhitespace(_file.previous()) )
                && ( isWhitespace(_file.next()) || isFormatSpecifier(_file.next()))) {
                if( !text.text.empty() ) {
                    _document.push_back(DocumentPart{text} );
                    text.text.clear();
                }
                switch(c) {
                case '_':
                    text.bold = false;
                    break;
                case '*':
                    text.italic = false;
                    break;
                case '-':
                    text.crossedOut = false;
                    break;
                }
            }

        } else if( c == '\n' ) {
            if( !text.text.empty() )
                _document.push_back(DocumentPart{text} );
            return true;
        } else {
            text.text.push_back(c);
        }

        if( _file.isEoF() )
            break;

        c = _file.getch();
    }

    // end of file is no error, when parsing text
    if( !text.text.empty() )
        _document.push_back(DocumentPart{text});
    return true;
}

bool Docmala::readParameterList(ParameterList &parameters, char blockEnd)
{
    enum class Mode {
        ParameterName,
        EndOrEquals,
        ParameterValue,
        EndOrNext
    } mode { Mode::ParameterName };

    enum class ValueMode {
        Normal,
        Extended
    } valueMode { ValueMode::Normal };

    Parameter parameter;

    while( !_file.isEoF() ) {
        char c = _file.getch();
        if( c == '\n' ) {
            _errors.push_back(Error{_file.location(), std::string("Error while parsing parameters. A valid parameter definition was expected but a 'newline' was found.") });
            return false;
        }
        if( mode == Mode::ParameterName ) {
            if( c == ' ' || c == '\t' )  {
                if( parameter.key.empty() ) { // skip leading white spaces
                    continue;
                } else {
                    mode = Mode::EndOrEquals;
                    continue;
                }
            } else if( (c >= 'a' && c <= 'z') ||
                       (c >= 'A' && c <= 'Z') ||
                       (c >= '0' && c <= '9') ||
                        c == '_' || c == '-' ) {
                parameter.key.push_back(c);
            } else if( c == '=' ) {
                if( parameter.key.empty() ) {
                    _errors.push_back(Error{_file.location(), std::string("Error while parsing parameter name. A name consisting of ['a'-'z', 'A'-'Z', '0'-'9', '_', '-'] was expected but a '") + c +"' was found." });
                    return false;
                } else {
                    mode = Mode::ParameterValue;
                    continue;
                }
            } else if ( c == ',' ) {
                if( parameter.key.empty() ) {
                    _errors.push_back(Error{_file.location(), std::string("Error while parsing parameter name. A name consisting of ['a'-'z', 'A'-'Z', '0'-'9', '_', '-'] was expected but a '") + c +"' was found." });
                    return false;
                } else {
                    parameters.insert( std::make_pair(parameter.key, parameter) );
                    parameter = Parameter();
                    mode=Mode::ParameterName;
                    continue;
                }
            } else if ( c == blockEnd ) {
                if( parameter.key.empty() ) {
                    _errors.push_back(Error{_file.location(), std::string("Error while parsing parameter name. A valid name consisting of ['a'-'z', 'A'-'Z', '0'-'9', '_', '-'] was expected but a ']' was found.") });
                    return false;
                } else {
                    parameters.insert( std::make_pair(parameter.key, parameter) );
                    parameter = Parameter();
                    return true;
                }
            } else {
                _errors.push_back(Error{_file.location(), std::string("Error while parsing parameter name. A valid name consisting of [a'-'z', 'A'-'Z', '0'-'9', '_', '-'] or a '") + blockEnd + "' was expected but a '" + c +"' was found." });
                return false;
            }
        } else if( mode == Mode::EndOrEquals ) {
            if( c == ' ' || c == '\t' )  {
                continue;
            } else if( c == '=' ) {
                mode = Mode::ParameterValue;
                continue;
            } else if( c == blockEnd ) {
                parameters.insert( std::make_pair(parameter.key, parameter) );
                parameter = Parameter();
                return true;
            } else {
                _errors.push_back(Error{_file.location(), std::string("Error while parsing parameter. A '=' or '") + blockEnd + "' was expected but a '" + c +"' was found." });
                return false;
            }
        } else if( mode == Mode::EndOrNext ) {
            if( c == ' ' || c == '\t' )  {
                continue;
            } else if( c == ',' ) {
                mode = Mode::ParameterName;
                parameters.insert( std::make_pair(parameter.key, parameter) );
                parameter = Parameter();
                continue;
            } else if( c == blockEnd ) {
                parameters.insert( std::make_pair(parameter.key, parameter) );
                parameter = Parameter();
                return true;
            } else {
                _errors.push_back(Error{_file.location(), std::string("Error while parsing parameter. A '=' or '") + blockEnd + "' was expected but a '" + c +"' was found." });
                return false;
            }
        } else if( mode == Mode::ParameterValue ) {
            if( parameter.value.empty() && (c == ' ' || c == '\t') )  {
                continue;
            } else if( parameter.value.empty() && valueMode == ValueMode::Normal ) {
                if( c == '"' ) {
                    valueMode = ValueMode::Extended;
                    continue;
                } else if( (c >= 'a' && c <= 'z') ||
                           (c >= 'A' && c <= 'Z') ||
                           (c >= '0' && c <= '9') ) {
                    valueMode = ValueMode::Normal;
                    parameter.value.push_back(c);
                    continue;
                } else {
                    _errors.push_back(Error{_file.location(), std::string("Error while parsing parameter value. " )
                                                               + "A valid value has to start with ['a'-'z', 'A'-'Z', '0'-'9'] or "
                                                               + " has to be placed in '\"' but a " + c +"' was found." });
                    return false;
                }
            } else {
                if( valueMode == ValueMode::Extended ) {
                    if( c == '"' ) {
                        if( parameter.value.back() == '\\' ) {
                            parameter.value.back() = '"';
                            continue;
                        } else {
                            mode = Mode::EndOrNext;
                            continue;
                        }
                    } else if( c == '\n' ) {
                        _errors.push_back(Error{_file.location(), std::string("Error while parsing parameter value. A '\"' was expected but a 'newline' was found.") });
                        return false;
                    } else {
                        parameter.value.push_back(c);
                        continue;
                    }
                } else if( valueMode == ValueMode::Normal ) {
                    if( c == ' ' || c == '\t' || c == ',' || c == blockEnd )  {
                        mode = Mode::ParameterName;
                        parameters.insert( std::make_pair(parameter.key, parameter) );
                        parameter = Parameter();
                        if( c == blockEnd ) {
                            return true;
                        } else {
                            continue;
                        }
                    } else {
                        parameter.value.push_back(c);
                        continue;
                    }
                }
            }
        }
    }
    _errors.push_back(Error{_file.location(), std::string("Error while parsing parameters. A valid parameter definition was expected but an 'end of file' was found.") });
    return false;
}

bool Docmala::readBlock(std::string &block)
{
    const std::string delimiter = "----";

    enum class Mode {
        Delimiter,
        Text
    } mode { Mode::Delimiter };


    std::string potentialDelimiter;
    bool searchingEnd = false;

    while( !_file.isEoF() ) {
        char c = _file.getch();

        if( mode == Mode::Delimiter ) {
            if( c == '-' ) {
                potentialDelimiter.push_back(c);
                continue;
            } else {
                if( potentialDelimiter.substr(0, 4) == "----" ) {
                    if( c == '\n' ) {
                        potentialDelimiter.clear();
                        if( searchingEnd ) {
                            return true;
                        }
                        searchingEnd = true;
                        continue;
                    } else {
                        _errors.push_back(Error{_file.location(), std::string("Error while reading block. A valid delimiter may only contain '-' but a '") + c + "' was found." });
                        return false;
                    }
                } else {
                    if( !searchingEnd ) {
                        _errors.push_back(Error{_file.location(), std::string("Error while reading block. A valid delimiter ('") + delimiter + "') was expected but a '" + c + "' was found." });
                        return false;
                    } else {
                        block.append(potentialDelimiter);
                        block.push_back(c);
                        potentialDelimiter.clear();
                        mode = Mode::Text;
                        continue;
                    }
                }
            }
        } else if( mode == Mode::Text ) {
            if( c == '\n' ) {
                mode = Mode::Delimiter;
            }
            block.push_back(c);
        }
    }
    _errors.push_back(Error{_file.location(), std::string("Error while parsing block. A valid block definition was expected but an 'end of file' was found.") });
    return false;
}

bool Docmala::isWhitespace(char c) const
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\0';
}
