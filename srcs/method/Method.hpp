#pragma once

#include <iostream>
#include <string>



class Method
{
protected:
	std::string start_line;
	std::string *headers;
	std::string body;
	Method();
	~Method();
public:
	enum {
	GET,
	POST,
	DELETE
	};
};

Method::Method()
{
}

Method::~Method()
{
}
