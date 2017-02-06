#pragma once

#include <string>
#include <fstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include "FileLocation.h"

namespace docmala {

    class File {
    public:
        File( ) {
        }

        explicit File( const std::string &fileName );

        bool isOpen() const {
            return _file.is_open();
        }

        bool isEoF() const {
            return _fileIterator >= _file.end();
        }

        std::string fileName() const;

        char getch();

        char previous();
        char following();

        FileLocation location() const;

    private:
        char _getch();

        int _line = 1;
        int _column = 0;

        boost::iostreams::mapped_file::iterator _fileIterator;

        char _previous[2] = {0};
        std::string _fileName;
        boost::iostreams::mapped_file _file;
        //std::ifstream _file;
    };
}
