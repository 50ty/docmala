#pragma once

#include "docmala_global.h"
#include <string>
#include <fstream>
#include "FileLocation.h"

namespace docmala {

    class DOCMALA_API IFile {
    public:

        virtual ~IFile();
        virtual bool isOpen() const = 0;
        virtual bool isEoF() const = 0;

        virtual char getch() = 0;
        virtual char previous() = 0;
        virtual char following() = 0;

        virtual FileLocation location() const = 0;
        virtual std::string fileName() const = 0;

    };

//    class File : public IFile {
//    public:
//        File( ) {
//        }

//        explicit File( const std::string &fileName );

//        bool isOpen() const override {
//            return _file.is_open();
//        }

//        bool isEoF() const override {
//            return _fileIterator >= _file.end();
//        }

//        char getch() override;

//        char previous() override;
//        char following() override;

//        FileLocation location() const override;

//    private:
//        char _getch();

//        int _line = 1;
//        int _column = 0;

//        boost::iostreams::mapped_file::iterator _fileIterator;

//        char _previous[2] = {0};
//        std::string _fileName;
//        boost::iostreams::mapped_file _file;
//    };

    class DOCMALA_API MemoryFile : public IFile {


    public:
        MemoryFile(const std::string &data, const std::string &fileName = "");
        bool isOpen() const override;

        bool isEoF() const override;
        char getch() override;
        char previous() override;
        char following() override;
        FileLocation location() const override;
        std::string fileName() const override;

    protected:
        MemoryFile();
        std::string _data;
        std::string _fileName;
        std::string::iterator _position;
    private:
        char _getch();

        int _line = 1;
        int _column = 0;
        char _previous[2] = {0};

    };

    class DOCMALA_API File : public MemoryFile {
    public:

        explicit File( const std::string &fileName );
    };
}
