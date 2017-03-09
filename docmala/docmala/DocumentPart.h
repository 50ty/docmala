#pragma once

#include <string>
#include <vector>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include "FileLocation.h"


namespace docmala {

class DocumentPart {
public:
    enum class Type {
        Invalid,
        Custom,
        Headline,
        Text,
        FormatedText,
        Paragraph,
        Image,
        Caption,
        List,
        Anchor,
        Link,
        GeneratedDocument,
        Code,
        Table
    };

    struct VisualElement {
        VisualElement(int lineNumber)
            : line(lineNumber)
        {}
        int line;
    };

    struct KeyValuePair {
        std::string key;
        std::string value;
    };

    struct Table {
        Table() {}

        struct Cell {
            std::vector<DocumentPart> content;

            size_t columnSpan = 0;
            size_t rowSpan = 0;
            bool isHeading = false;
            bool isHiddenBySpan = false;
        };

        size_t columns = 0;
        size_t rows = 0;
        std::vector< std::vector<Cell> > cells;
    };

    struct Anchor {
        std::string name;
        FileLocation location;
    };

    struct Link {
        enum class Type {
            Web,
            IntraFile,
            InterFile
        };

        std::string data;
        std::string text;
        Type type;
        FileLocation location;
    };

    struct Paragraph {

    };

    struct Code : public VisualElement {
        Code(int lineNumber) : VisualElement(lineNumber) {}
        std::string code;
        std::string type;
    };

    struct FormatedText {
        FormatedText() {}
        FormatedText(const std::string &text)
            : text(text)
        {}

        std::string text;
        bool bold = false;
        bool italic = false;
        bool monospaced = false;
        bool stroked = false;
        bool underlined = false;
    };

    struct GeneratedDocument : public VisualElement {
        GeneratedDocument( int lineNumber ) : VisualElement(lineNumber) {}
        std::vector<DocumentPart> document;
    };

    struct Text : public VisualElement {
        Text(int lineNumber = -1) : VisualElement(lineNumber) {}

        std::vector<DocumentPart> text;
    };

    struct Headline : public Text {
        Headline() : Text(-1) {}
        Headline(const Text& text, int level)
            : Text(text)
            , level(level)
        {}

        int level = 0;
    };

    struct List {
        enum class Type {
            Points,
            Dashes,
            Numbered
        };

        List() { }
        List(const std::vector<Text> &entries, Type type, int level )
            : entries(entries)
            , type(type)
            , level(level)
        {}


        std::vector<Text> entries;
        Type type = Type::Points;
        int level = 0;
    };

    struct Caption : public Text {
        Caption() = default;
        Caption(const Text& text)
            : Text(text)
        {}
    };

    struct Image : public Text {
        Image() = default;
        Image(const std::string& format, const std::string& fileExtension, const std::string& data, const Text& text)
            : Text(text)
            , format(format)
            , fileExtension(fileExtension)
            , data(data)
        {}
        std::string format;
        std::string fileExtension;
        std::string data;
    };

    DocumentPart() = default;

    DocumentPart( const Paragraph & )
        : _type( Type::Paragraph )
    { }

    DocumentPart( const Headline &headline )
        : _type( Type::Headline )
        , _data( headline )
    { }

    DocumentPart( const Text &text )
        : _type( Type::Text )
        , _data( text )
    { }

    DocumentPart( const FormatedText &text )
        : _type( Type::FormatedText )
        , _data( text )
    { }

    DocumentPart( const Caption &caption )
        : _type( Type::Caption )
        , _data( caption )
    { }

    DocumentPart( const Image &image )
        : _type( Type::Image )
        , _data( image )
    { }

    DocumentPart( const List &list )
        : _type( Type::List )
        , _data( list )
    { }

    DocumentPart( const Anchor &anchor )
        : _type( Type::Anchor )
        , _data( anchor )
    { }

    DocumentPart( const Link &link )
        : _type( Type::Link )
        , _data( link )
    { }

    DocumentPart( const GeneratedDocument &generated )
        : _type( Type::GeneratedDocument )
        , _data( generated )
    { }

    DocumentPart( const Code &code )
        : _type( Type::Code )
        , _data( code )
    { }

    DocumentPart( const Table &table )
        : _type( Type::Table )
        , _data( table )
    { }

    Type type() const { return _type; }

    const Text* text() const {
        return boost::get<Text>(&_data);
    }

    Text* text() {
        return boost::get<Text>(&_data);
    }

    const FormatedText* formatedText() const {
        return boost::get<FormatedText>(&_data);
    }

    const Headline* headline() const {
        return boost::get<Headline>(&_data);
    }

    const Image* image() const {
        return boost::get<Image>(&_data);
    }

    const Caption* caption() const {
        return boost::get<Caption>(&_data);
    }

    const List* list() const {
        return boost::get<List>(&_data);
    }

    List* list() {
        return boost::get<List>(&_data);
    }

    const Anchor* anchor() const {
        return boost::get<Anchor>(&_data);
    }

    const Link* link() const {
        return boost::get<Link>(&_data);
    }

    Link* link() {
        return boost::get<Link>(&_data);
    }

    const GeneratedDocument* generatedDocument() const {
        return boost::get<GeneratedDocument>(&_data);
    }
    GeneratedDocument* generatedDocument() {
        return boost::get<GeneratedDocument>(&_data);
    }

    const Code* code() const {
        return boost::get<Code>(&_data);
    }

    const Table* table() const {
        return boost::get<Table>(&_data);
    }

private:
    template< class T >
    DocumentPart(Type type, const T &data)
        : _type(type)
        , _data(data)
    { }

    DocumentPart(Type type)
        : _type(type)
    { }

    Type _type = Type::Invalid;
    boost::variant<Text, FormatedText, Caption, Headline, Image, List, Anchor, Link, GeneratedDocument, Code, Table> _data;
};

}
