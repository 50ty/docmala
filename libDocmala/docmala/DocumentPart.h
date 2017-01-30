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
        Image
    };

    struct KeyValuePair {
        std::string key;
        std::string value;
    };

    struct Text {
        Text() = default;
        Text(const std::string& text)
            : text(text)
        { }
        std::string text;
    };

    struct Headline : public Text {
        int level = 0;
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

    DocumentPart( const Headline &headline )
        : _type( Type::Headline )
        , _data( headline )
    { }

    static DocumentPart newParagraph() {
        return DocumentPart(Type::Paragraph);
    }

    static DocumentPart newText(const std::string &text) {
        return DocumentPart( Type::Text, Text {text});
    }

    static DocumentPart newImage(const std::string &format, const std::string &data) {
        return DocumentPart( Type::Image, Image {format, data, ""});
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
    boost::variant<Text, Headline, Image> _data;
};

}
