#pragma once

#include <string>
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
        Caption
    };

    struct KeyValuePair {
        std::string key;
        std::string value;
    };

    struct Text {
        Text() {}

        Text(const std::string& text)
            : text(text)
            , bold(false)
            , italic(false)
        { }

        std::string text;
        bool bold = false;
        bool italic = false;
        bool crossedOut = false;
    };

    struct Headline : public Text {
        Headline() {}
        Headline(const std::string& text, int level)
            : Text(text)
            , level(level)
        {}

        int level = 0;
    };

    struct Caption : public Text {
        Caption() = default;
        Caption(const std::string& text)
            : Text(text)
        {}
    };

    struct Image : public Text {
        Image() = default;
        Image(const std::string& format, const std::string& data, const std::string& text)
            : Text(text)
            , format(format)
            , data(data)
        {}
        std::string format;
        std::string data;
    };

    DocumentPart() = default;

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

    static DocumentPart newParagraph() {
        return DocumentPart(Type::Paragraph);
    }

    static DocumentPart newText(const std::string &text) {
        return DocumentPart( Type::Text, Text {text});
    }

    static DocumentPart newCaption(const std::string &caption) {
        return DocumentPart( Type::Caption, Caption {caption});
    }

    static DocumentPart newImage(const std::string &format, const std::string &data) {
        return DocumentPart( Type::Image, Image {format, data, ""});
    }

    static DocumentPart newHeadline(const std::string &text, int level) {
        return DocumentPart( Type::Text, Headline {text, level});
    }

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
    boost::variant<Text, Caption, Headline, Image> _data;
};

}
