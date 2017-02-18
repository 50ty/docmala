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
        Paragraph,
        Image,
        Caption,
        List,
        Anchor,
        GeneratedDocument,
        Code
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

    struct Anchor {
        std::string name;
        FileLocation location;
    };

    struct Paragraph {

    };

    struct Code {
        std::string code;
        std::string type;
    };

    struct FormatedText {
        FormatedText() {}

        std::string text;
        bool bold = false;
        bool italic = false;
        bool crossedOut = false;
    };

    struct GeneratedDocument : public VisualElement {
        GeneratedDocument( int lineNumber ) : VisualElement(lineNumber) {}
        std::vector<DocumentPart> document;
    };

    struct Text : public VisualElement {
        Text(int lineNumber = -1) : VisualElement(lineNumber) {}

        std::vector<FormatedText> text;
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
        Image(const std::string& format, const std::string& data, const Text& text)
            : Text(text)
            , format(format)
            , data(data)
        {}
        std::string format;
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

    DocumentPart( const GeneratedDocument &generated )
        : _type( Type::GeneratedDocument )
        , _data( generated )
    { }

    DocumentPart( const Code &code )
        : _type( Type::Code )
        , _data( code )
    { }

    Type type() const { return _type; }

    const Text* text() const {
        return boost::get<Text>(&_data);
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

    const GeneratedDocument* generatedDocument() const {
        return boost::get<GeneratedDocument>(&_data);
    }

    const Code* code() const {
        return boost::get<Code>(&_data);
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
    boost::variant<Text, Caption, Headline, Image, List, Anchor, GeneratedDocument, Code> _data;
};

}
