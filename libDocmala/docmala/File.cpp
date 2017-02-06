#include "File.h"

using namespace docmala;



File::File(const std::string &fileName) {
    _file.open(fileName);
    _fileName = fileName;
    _fileIterator = _file.begin();
}

std::string File::fileName() const {
    return _fileName;
}

char File::getch() {
    char c = _getch();
    _previous[0] = _previous[1];
    _previous[1] = c;

    if( c == '.' ) {
        auto pos = _fileIterator;
        auto line = _line;
        auto column = _column;

        if( _getch() == '.' && _getch() == '.' && _getch() == '\n' )
        {
            while( true ) {
                c = _getch();
                if( c != ' ' && c != '\t' )
                    break;
            }
        } else {
            _line = line;
            _column = column;
            _fileIterator = pos;
        }
    }
    return c;
}

char File::previous()
{
    return _previous[0];
}

char File::next()
{
    auto pos = _fileIterator;
    auto line = _line;
    auto column = _column;

    auto next = getch();

    _line = line;
    _column = column;
    _fileIterator = pos;

    return next;
}

FileLocation File::location() const {
    return FileLocation {_line, _column, _fileName };
}

char File::_getch()
{
    char c = *_fileIterator;
    _fileIterator++;
    if( c == '\n' ) {
        _line++;
        _column = 0;
    } else if( c == '\r' ) {
        return _getch();
    } else {
        _column++;
    }
    return c;
}
