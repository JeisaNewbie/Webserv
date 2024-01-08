#pragma once

#include <string>
#include <cstring>
#include <map>
#include <list>
#include <iostream>
// #include "../core/core.hpp"

class Response {
	private:
		std::string	header_line;
		std::map<std::string, std::string>	header;
	public:
		Response();
		Response(const Response& ref);
		~Response();
		Response& operator=(const Response& ref);
		void	set_header_line (int status_code);
};