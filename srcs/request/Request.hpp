#ifndef REQUEST_HPP
 #define REQUEST_HPP

#include <string>
#include <cstring>
#include <map>
#include <list>
#include <iostream>
#include "../utils/Status.hpp"
// #include "../method/Method.hpp"

#define	FAIL			1
#define INVALID_METHOD	5
#define INVALID_SCHEME	6
#define	INVALID_HEADER	5
#define CR	'\r'
#define LF	'\n'

class Request
{
private:
	std::string										request_msg;
	std::string										request_line;
	std::string										request_target;
	std::string										uri;
	int												port;
	std::string										query;
	std::string										protocol_version;
	std::string										method;
	std::list<std::string>							headers;
	std::map<std::string, std::string>				header;
	std::map<std::string, std::string>::iterator	header_end;
	std::map<std::string, std::string>				query_elements;
	std::string 									message_body;
	char											*request_start;
	char											*request_line_end;
	char											*method_end;
	size_t											pos;
	size_t											content_length;
	int												chunked;
	int												status_code;
	bool											cgi;
	void											parse_query_string(std::string &query);
	void											parse_query_key_and_value(std::string &query_element);
	void											parse_header_key_and_value(std::string &header_element);
	void											set_header_key_and_value(std::string &key, std::string &value);
	void											check_header_is_valid();
	void											check_host();
	void											check_transfer_encoding_and_content_length();
	void											check_transfer_encoding();
	void											check_content_length();
	void											check_te();
	void											check_content_encoding();
	void											check_uri_form();
	//////-------------utils--------------------------------------------------
	std::string										lower(const char *key, size_t end);
	void											remove_spf(std::string &value, size_t end);
	void											remove_spb(std::string &value, size_t end);
public:
	Request();
	Request(std::string &msg);
	~Request();
	int												process_request_parsing(std::string &request_msg);
	void											parse_request();
	void											parse_request_line();
	void											parse_header_fields();
	bool											get_cgi();
	int												get_status_code();
	std::string&									get_method();

	void check_members();
};

#endif