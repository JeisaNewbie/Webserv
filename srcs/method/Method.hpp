#pragma once

#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../utils/Utils.hpp"
#include <fstream>
#include <sstream>

class Method
{
protected:
	std::string start_line;
	std::string *headers;
	std::string body;

public:
	void create_response(Response &response);
};
