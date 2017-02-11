#include "File.h"

using namespace docmala;



//File::File(const std::string &fileName) {
//    _file.open(fileName);
//    _fileName = fileName;
//    _fileIterator = _file.begin();
//}

//char File::getch() {
//    char c = _getch();
//    _previous[0] = _previous[1];
//    _previous[1] = c;

//    if( c == '.' ) {
//        auto pos = _fileIterator;
//        auto line = _line;
//        auto column = _column;

//        if( _getch() == '.' && _getch() == '.' && _getch() == '\n' )
//        {
//            while( true ) {
//                c = _getch();
//                if( c != ' ' && c != '\t' )
//                    break;
//            }
//        } else {
//            _line = line;
//            _column = column;
//            _fileIterator = pos;
//        }
//    }
//    return c;
//}

//char File::previous()
//{
//    return _previous[0];
//}

//char File::following()
//{
//    auto pos = _fileIterator;
//    auto line = _line;
//    auto column = _column;

//    auto next = getch();

//    _line = line;
//    _column = column;
//    _fileIterator = pos;

//    return next;
//}

//FileLocation File::location() const {
//    return FileLocation {_line, _column, _fileName };
//}

//char File::_getch()
//{
//    char c = *_fileIterator;
//    _fileIterator++;
//    if( c == '\n' ) {
//        _line++;
//        _column = 0;
//    } else if( c == '\r' ) {
//        return _getch();
//    } else {
//        _column++;
//    }
//    return c;
//}

IFile::~IFile() {}



MemoryFile::MemoryFile(const std::string &data, const std::string &fileName)
    : _data(data)
    , _fileName(fileName)
    , _position(_data.begin())
{

}

bool MemoryFile::isOpen() const {
    return !_data.empty();
}

bool MemoryFile::isEoF() const
{
    return _position >= _data.end();
}

char MemoryFile::getch()
{
    char c = _getch();
    _previous[0] = _previous[1];
    _previous[1] = c;

    if( c == '.' ) {
        auto pos = _position;
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
            _position = pos;
        }
    }
    return c;
}

char MemoryFile::previous()
{
    return _previous[0];
}

char MemoryFile::following()
{
    auto pos = _position;
    auto line = _line;
    auto column = _column;

    auto next = getch();

    _line = line;
    _column = column;
    _position = pos;

    return next;
}

FileLocation MemoryFile::location() const
{
    return FileLocation {_line, _column, _fileName };
}

std::string MemoryFile::fileName() const
{
    return _fileName;
}

MemoryFile::MemoryFile()
{

}

char MemoryFile::_getch()
{
    char c = *_position;
    _position++;
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

File::File(const std::string &fileName)
{
    _fileName = fileName;

    std::ifstream in(fileName, std::ios::in | std::ios::binary);
    if (in)
    {
        in.seekg(0, std::ios::end);
        _data.resize( static_cast<std::string::size_type>(in.tellg()));
        in.seekg(0, std::ios::beg);
        in.read(&_data[0], static_cast<std::streamsize>(_data.size()));
        in.close();
        _position = _data.begin();
    }
}
