#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <docmala/Docmala.h>
#include <docmala/File.h>
#include <sstream>

using namespace docmala;

class TablePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block) override;

    enum class ReadCellResult {
        CellContent,
        NextRow,
        HeadlinesAbove,
        RowHeadlinesOnLeft,
        EndOfTable,
        SpanModifier
    };

    ReadCellResult readNextCell(std::string &cellContent);
    std::unique_ptr<MemoryFile> _file;
    std::vector<Error> _errors;
};


DocumentPlugin::BlockProcessing TablePlugin::blockProcessing() const {
    return BlockProcessing::Optional;
}

bool TablePlugin::process(const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block)
{
    _file.reset( new MemoryFile(block, location.fileName) );

    DocumentPart::Table table;
    table.cells.push_back( std::vector<DocumentPart::Table::Cell>() );

    std::string spanModifier;

    while (true) {
        std::string cellContent;
        auto readCellResult = readNextCell(cellContent);

        if( readCellResult == ReadCellResult::EndOfTable ) {
            break;
        } else if( readCellResult == ReadCellResult::CellContent ) {
            DocumentPart::Text text;
            DocumentPart::FormatedText ft;
            ft.text = cellContent;
            text.text.push_back(ft);
            DocumentPart::Table::Cell cell;
            cell.content.push_back(text);

            if( !spanModifier.empty() ) {
                std::istringstream stream(spanModifier);
                int colSpan = 0;
                char separator = 0;
                int rowSpan = 0;
                stream >> colSpan;
                stream >> separator;
                stream >> rowSpan;

                cell.columnSpan = colSpan+1;
                cell.rowSpan = rowSpan+1;
            }
            table.cells.back().push_back(cell);
        } else if( readCellResult == ReadCellResult::NextRow ) {
            table.cells.push_back( std::vector<DocumentPart::Table::Cell>() );
        } else if( readCellResult == ReadCellResult::HeadlinesAbove ) {
            for( auto &row : table.cells ) {
                for( auto &cell : row ) {
                    cell.isHeading = true;
                }
            }
        } else if( readCellResult == ReadCellResult::RowHeadlinesOnLeft ) {
            for( auto &cell : table.cells.back() ) {
                cell.isHeading = true;
            }
        } else if( readCellResult == ReadCellResult::SpanModifier ) {
            spanModifier = cellContent;
            continue;
        }
        spanModifier.clear();
    }

    document.addPart(table);
    // change error line numbers and file name
    (void)block;
    (void)location;
    (void)document;
    (void)parameters;
    return true;
}

TablePlugin::ReadCellResult TablePlugin::readNextCell(std::string &cellContent)
{
    bool firstChar = true;
    while( !_file->isEoF() )
    {
        char c = _file->getch();
        if( (c == '|' && _file->previous() != '|') ) {
            return ReadCellResult::CellContent;
        }

        if (c == '|' && _file->previous() == '|') {
            return ReadCellResult::RowHeadlinesOnLeft;
        }

        if( firstChar  && c == '+' ) {
            cellContent.push_back(c);
            while( !_file->isEoF() ) {
                c = _file->getch();
                if( c == ':' || (c >= '0' && c <= '9') ) {
                    cellContent.push_back(c);
                    continue;
                } else if( c == ' ' || c == '\t' ) {
                    return ReadCellResult::SpanModifier;
                } else {
                    break;
                }
            }
        }

        if (_file->previous() == '\n' && c == '=' ) {
            while( !_file->isEoF() ) {
                char c = _file->getch();
                if( c == '=' ) {
                    continue;
                } else if( c == '\n' ) {
                    return ReadCellResult::HeadlinesAbove;
                } else {
                    _errors.push_back({_file->location(), std::string("Invalid character in headline separator: '") + c + "'. Only '=' and newline is allowed."});
                    return ReadCellResult::HeadlinesAbove;
                }
            }
            return ReadCellResult::EndOfTable;
        }

        if( c == '\n' ) {
            return ReadCellResult::NextRow;
        }

        cellContent.push_back(c);
        if( _file->following() == '\n' ) {
            return ReadCellResult::CellContent;
        }
        firstChar = false;
    }
    return ReadCellResult::EndOfTable;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, TablePlugin, "table", 1, "Creates a table from the subsequent block", EXTENSION_SYSTEM_NO_USER_DATA )
