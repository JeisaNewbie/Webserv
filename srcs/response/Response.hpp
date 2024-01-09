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
		std::string	body;
		std::string	response_msg;
		bool	body_flag;
	public:
		Response();
		Response(const Response& ref);
		~Response();

		Response& operator=(const Response& ref);


		std::string	&get_header_line ();
		std::string	get_header_field ();
		std::string	&get_body();
		void	set_header_line (int status_code);
		void	set_header_field (const std::string &key, const std::string &value);
		void	assemble_message();
};