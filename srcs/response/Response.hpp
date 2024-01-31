#pragma once

#include <string>
#include <cstring>
#include <map>
#include <list>
#include <iostream>

class Response {
	private:
		std::string							header_line;
		std::map<std::string, std::string>	header;
		std::string 						headers;
		std::string							body;
		std::string							response_msg;
		bool								body_flag;
	public:
		Response();
		Response(const Response& ref);
		~Response();

		std::string &get_response_message();
		std::string	&get_header_line ();
		std::string	&get_header_field ();
		std::string	get_header_field (const char *key);
		std::string	&get_body();
		void	set_body(std::string body);
		void	set_header_line (int status_code);
		void	set_header_field (const std::string &key, const std::string &value);
		void	set_header_field (const std::string &key_value);
		void	assemble_message();
		void	reset_data();
};