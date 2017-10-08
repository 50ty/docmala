/**
    @file
    @copyright
        Copyright (C) 2017 Michael Adam
        Copyright (C) 2017 Bernd Amend
        Copyright (C) 2017 Stefan Rommel

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <docmala/DocmaPlugin.h>
#include <docmala/Docmala.h>
#include <docmala/File.h>
#include <extension_system/Extension.hpp>
#include <memory>
#include <sstream>

using namespace docmala;

class TablePlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    BlockProcessing blockProcessing() const override;
    bool process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) override;

    enum class ReadCellResult { CellContent, NextRow, HeadlinesAbove, RowHeadlinesOnLeft, EndOfTable, SpanModifier };

    std::vector<Error> lastErrors() const override {
        return _errors;
    }

    ReadCellResult              readNextCell(std::string& cellContent);
    std::unique_ptr<MemoryFile> _file;
    std::vector<Error>          _errors;

    void addCell(size_t& currentCol, size_t currentRow, const DocumentPart::Table::Cell& cell, DocumentPart::Table& table);
    void ensureTableSize(DocumentPart::Table& table, size_t cols, size_t rows);

    void adjustTableSize(DocumentPart::Table& table);
};

DocumentPlugin::BlockProcessing TablePlugin::blockProcessing() const {
    return BlockProcessing::Optional;
}

void TablePlugin::ensureTableSize(DocumentPart::Table& table, size_t cols, size_t rows) {
    if (table.columns < cols) {
        table.columns = cols;
        for (auto& row : table.cells) {
            row.resize(table.columns, DocumentPart::Table::Cell());
        }
    }

    if (table.rows < rows) {
        table.rows = rows;
        while (table.cells.size() < table.rows) {
            table.cells.emplace_back();
            table.cells.back().resize(table.columns, DocumentPart::Table::Cell());
        }
    }
}

void TablePlugin::addCell(size_t& currentCol, const size_t currentRow, const DocumentPart::Table::Cell& cell, DocumentPart::Table& table) {
    ensureTableSize(table, currentCol + cell.columnSpan + 1, currentRow + cell.rowSpan + 1);

    if (table.cells[currentRow][currentCol].isHiddenBySpan) {
        auto row = table.cells[currentRow];
        bool ok  = false;
        for (; currentCol < row.size(); currentCol++) {
            if (!row[currentCol].isHiddenBySpan) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            currentCol += 1;
            ensureTableSize(table, currentCol + cell.columnSpan + 1, currentRow + cell.rowSpan + 1);
        }
    }

    table.cells[currentRow][currentCol] = cell;

    for (auto y = currentRow; y <= currentRow + cell.rowSpan; y++) {
        for (auto x = currentCol; x <= currentCol + cell.columnSpan; x++) {
            if (x == currentCol && y == currentRow) {
                continue;
            }
            table.cells[y][x].isHiddenBySpan = true;
        }
    }

    currentCol += cell.columnSpan + 1;
}

void TablePlugin::adjustTableSize(DocumentPart::Table& table) {
    auto cols = table.cells.back().size();
    // if the current row is longer than the prevously seen ones
    if (table.columns < cols) {
        table.columns = table.columns > cols ? table.columns : cols;
        if (!table.cells.empty()) {
            // fill rows with empty cells
            for (auto& row : table.cells) {
                while (row.size() < table.columns) {
                    row.emplace_back();
                }
            }
        }
    }
}

bool TablePlugin::process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) {
    _errors.clear();
    _file = std::make_unique<MemoryFile>(block, location.fileName);

    DocumentPart::Table table(location);
    table.cells.emplace_back();

    std::string               spanModifier;
    DocumentPart::Table::Cell hiddenCellPrototype;
    hiddenCellPrototype.isHiddenBySpan = true;
    size_t currentRow                  = 0;
    size_t currentCol                  = 0;

    while (true) {
        std::string cellContent;
        auto        readCellResult = readNextCell(cellContent);

        if (readCellResult == ReadCellResult::EndOfTable) {
            break;
        }
        if (readCellResult == ReadCellResult::CellContent) {
            FileLocation tableLocation = _file->location();
            tableLocation.line += location.line + 1;
            tableLocation.column -= cellContent.length();
            std::unique_ptr<IFile> file(new MemoryFile(cellContent, tableLocation));

            DocumentPart::Text text;
            Docmala::readText(file.get(), _errors, '\0', text);
            //            DocumentPart::FormatedText ft;
            //            ft.text = cellContent;
            //            text.text.push_back(ft);
            DocumentPart::Table::Cell cell;
            cell.content.emplace_back(text);

            if (!spanModifier.empty()) {
                std::istringstream stream(spanModifier);
                size_t             colSpan   = 0;
                char               separator = 0;
                size_t             rowSpan   = 0;
                stream >> colSpan;
                stream >> separator;
                stream >> rowSpan;

                cell.columnSpan = colSpan;
                cell.rowSpan    = rowSpan;
            }

            addCell(currentCol, currentRow, cell, table);
        } else if (readCellResult == ReadCellResult::NextRow) {
            currentRow++;
            currentCol = 0;
        } else if (readCellResult == ReadCellResult::HeadlinesAbove) {
            for (auto& row : table.cells) {
                for (auto& cell : row) {
                    cell.isHeading = true;
                }
            }
        } else if (readCellResult == ReadCellResult::RowHeadlinesOnLeft) {
            for (size_t i = 0; i < currentCol; i++) {
                table.cells[currentRow][i].isHeading = true;
            }
        } else if (readCellResult == ReadCellResult::SpanModifier) {
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

TablePlugin::ReadCellResult TablePlugin::readNextCell(std::string& cellContent) {
    bool firstChar = true;
    while (!_file->isEoF()) {
        char c = _file->getch();

        if ((c == '|' && _file->previous() != '|')) {
            return ReadCellResult::CellContent;
        }

        if (c == '|' && _file->previous() == '|') {
            return ReadCellResult::RowHeadlinesOnLeft;
        }

        if (firstChar && c == '+') {
            cellContent.push_back(c);
            while (!_file->isEoF()) {
                c = _file->getch();
                if (c == ':' || (c >= '0' && c <= '9')) {
                    cellContent.push_back(c);
                    continue;
                }
                if (c == ' ' || c == '\t') {
                    return ReadCellResult::SpanModifier;
                }
                break;
            }
        }

        if (_file->previous() == '\n' && c == '=') {
            while (!_file->isEoF()) {
                char c = _file->getch();
                if (c == '=') {
                    continue;
                }
                if (c == '\n') {
                    return ReadCellResult::HeadlinesAbove;
                }
                _errors.emplace_back(_file->location(),
                                     std::string("Invalid character in headline separator: '") + c + "'. Only '=' and newline is allowed.");
                return ReadCellResult::HeadlinesAbove;
            }
            return ReadCellResult::EndOfTable;
        }

        if (c == '\n') {
            return ReadCellResult::NextRow;
        }

        cellContent.push_back(c);
        if (_file->following() == '\n') {
            return ReadCellResult::CellContent;
        }
        firstChar = false;
    }
    return ReadCellResult::EndOfTable;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, TablePlugin, "table", 1, "Creates a table from the subsequent block", EXTENSION_SYSTEM_NO_USER_DATA)
