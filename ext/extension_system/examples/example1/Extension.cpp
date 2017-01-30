/**
	@file
	@copyright
		Copyright Bernd Amend and Michael Adam 2014
		Distributed under the Boost Software License, Version 1.0.
		(See accompanying file LICENSE_1_0.txt or copy at
		http://www.boost.org/LICENSE_1_0.txt)
*/
#include <extension_system/Extension.hpp>
#include "Interface.hpp"
#include <iostream>

class Extension : public Interface1
{
public:
	virtual void test1() override {
		std::cout<<"Hello from Extension1\n";
	}
};

EXTENSION_SYSTEM_EXTENSION(Interface1, Extension, "Example1Extension", 100, "Example 1 extension", EXTENSION_SYSTEM_NO_USER_DATA)
