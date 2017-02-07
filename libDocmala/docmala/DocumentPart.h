#pragma once

#include <string>
#include <vector>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>


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
        List
    };

    struct KeyValuePair {
        std::string key;
        std::string value;
    };

    struct Paragraph {

    };

    struct FormatedText {
        FormatedText() {}

        std::string text;
        bool bold = false;
        bool italic = false;
        bool crossedOut = false;
    };

    struct Text {
        Text() {}

        std::vector<FormatedText> text;
    };

    struct Headline : public Text {
        Headline() {}
        Headline(const Text& text, int level)
            : Text(text)
            , level(level)
        {}

        int level = 0;
    };

    struct List : public Text {
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
    boost::variant<Text, Caption, Headline, Image, List> _data;
};

}
