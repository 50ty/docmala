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
#pragma once

#include "FileLocation.h"
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include <string>
#include <vector>

namespace docmala {

namespace document_part {

struct Text;
struct FormatedText;
struct Caption;
struct Headline;
struct Image;
struct List;
struct Anchor;
struct Link;
struct GeneratedDocument;
struct Code;
struct Table;
struct Paragraph;

using Variant = boost::variant<Text, FormatedText, Caption, Headline, Image, List, Anchor, Link, GeneratedDocument, Code, Table, Paragraph>;

struct VisualElement {
    VisualElement(const FileLocation& location)
        : location(location) {}
    FileLocation location;
};

struct Table : public VisualElement {
    Table(const FileLocation& location)
        : VisualElement(location) {}

    struct Cell {
        std::vector<document_part::Variant> content;

        size_t columnSpan     = 0;
        size_t rowSpan        = 0;
        bool   isHeading      = false;
        bool   isHiddenBySpan = false;
    };

    size_t                         columns = 0;
    size_t                         rows    = 0;
    std::vector<std::vector<Cell>> cells;
};

struct Anchor {
    std::string  name;
    FileLocation location;
};

struct Link {
    enum class Type { Web, IntraFile, InterFile };

    std::string  data;
    std::string  text;
    Type         type;
    FileLocation location;
};

struct Paragraph {};

struct Code : public VisualElement {
    Code(const FileLocation& location)
        : VisualElement(location) {}
    std::string code;
    std::string type;
};

struct FormatedText {
    FormatedText() {}
    FormatedText(const std::string& text)
        : text(text) {}

    std::string text;
    bool        bold       = false;
    bool        italic     = false;
    bool        monospaced = false;
    bool        stroked    = false;
    bool        underlined = false;
};

struct GeneratedDocument : public VisualElement {
    GeneratedDocument(const FileLocation& location)
        : VisualElement(location) {}
    std::vector<document_part::Variant> document;
};

struct Text : public VisualElement {
    Text(const FileLocation& location = FileLocation())
        : VisualElement(location) {}

    std::vector<document_part::Variant> text;
};

struct Headline : public Text {
    Headline()
        : Text() {}
    Headline(const Text& text, int level)
        : Text(text)
        , level(level) {}

    int level = 0;
};

struct List {
    enum class Type { Points, Dashes, Numbered };

    List() {}
    List(const Text& text, Type type)
        : entries({{text, type, {}}}) {}
    struct Entry {
        Text               text;
        Type               type = Type::Points;
        std::vector<Entry> entries;
    };
    std::vector<Entry> entries;
};

struct Caption : public Text {
    Caption() = default;
    Caption(const Text& text)
        : Text(text) {}
};

struct Image : public Text {
    Image() = default;
    Image(const std::string& format, const std::string& fileExtension, const std::string& data, const Text& text)
        : Text(text)
        , format(format)
        , fileExtension(fileExtension)
        , data(data) {}
    std::string format;
    std::string fileExtension;
    std::string data;
};

} // namespace DocumentPart

} // namespace docmala
