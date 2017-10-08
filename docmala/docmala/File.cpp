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
#include "File.h"

using namespace docmala;
IFile::~IFile() {}

MemoryFile::MemoryFile(const std::string& data, const std::string& fileName)
    : _data(data)
    , _fileName(fileName)
    , _position(_data.begin()) {}

MemoryFile::MemoryFile(const std::string& data, const FileLocation& baseLocation)
    : _data(data)
    , _fileName(baseLocation.fileName)
    , _position(_data.begin())
    , _line(baseLocation.line)
    , _column(baseLocation.column) {}

bool MemoryFile::isOpen() const {
    return !_data.empty();
}

bool MemoryFile::isEoF() const {
    return _position >= _data.end();
}

char MemoryFile::getch() {
    _previous[0] = _previous[1];
    char c       = _getch();
    _previous[1] = c;
    return c;
}

char MemoryFile::previous() {
    return _previous[0];
}

char MemoryFile::following() {
    auto pos        = _position;
    auto line       = _line;
    auto column     = _column;
    char previous[] = {_previous[0], _previous[1]};
    auto next       = getch();

    _line        = line;
    _column      = column;
    _position    = pos;
    _previous[0] = previous[0];
    _previous[1] = previous[1];

    return next;
}

FileLocation MemoryFile::location() const {
    return FileLocation{_line, _column, _fileName};
}

std::string MemoryFile::fileName() const {
    return _fileName;
}

MemoryFile::MemoryFile()
    : _position(_data.end()) {}

char MemoryFile::_getch() {
    char c = *_position;
    _position++;
    if (c == '\r') {
        return _getch();
    } else if (previous() == '\n') {
        _line++;
        _column = 0;
    } else {
        _column++;
    }
    return c;
}

File::File(const std::string& fileName) {
    if (fileName.empty() || fileName.back() == '\\' || fileName.back() == '/')
        return;

    _fileName = fileName;

    std::ifstream in(fileName, std::ios::in | std::ios::binary);
    if (in.is_open()) {
        in.seekg(0, std::ios::end);
        _data.resize(static_cast<std::string::size_type>(in.tellg()));
        in.seekg(0, std::ios::beg);
        in.read(&_data[0], static_cast<std::streamsize>(_data.size()));
        in.close();
        _position = _data.begin();
    }
}
