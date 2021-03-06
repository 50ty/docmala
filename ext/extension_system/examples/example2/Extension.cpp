/**
    @file
    @copyright
        Copyright Bernd Amend and Michael Adam 2014-2017
        Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt)
*/
#include "Interface.hpp"
#include <extension_system/Extension.hpp>
#include <iostream>

class Extension : public Interface2 {
public:
    void test1() override {
        std::cout << "Hello from Interface2 Extension\n";
    }
};

// clang-format off
// export extension and add user defined metadata
EXTENSION_SYSTEM_EXTENSION(Interface2, Extension, "Example2Extension", 100, "Example 2 extension",
                           EXTENSION_SYSTEM_DESCRIPTION_ENTRY("author", "Alice Bobbens")
                           EXTENSION_SYSTEM_DESCRIPTION_ENTRY("vendor", "42 inc.")
                           EXTENSION_SYSTEM_DESCRIPTION_ENTRY("target_product", "example2"))
