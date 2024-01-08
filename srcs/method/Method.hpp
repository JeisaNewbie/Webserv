#pragma once

// #include "../core/core.hpp"
#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../utils/Utils.hpp"

class Method
{
protected:
	std::string start_line;
	std::string *headers;
	std::string body;

public:
	virtual ~Method();
	static int	find_index(Request &request);
	void create_response(Response &response);
	enum {
	GET,
	POST,
	DELETE
	};
};
