#ifndef REQUEST_HPP
 #define REQUEST_HPP

#include <string>
#include <cstring>
#include <map>
#include <list>
#include <iostream>
// #include "../core/core.hpp"
#include "../core/cycle.hpp"
#include "../utils/Status.hpp"
#include "../utils/Utils.hpp"
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
	Cycle											*cycle;
	std::string										request_line;
	std::string										uri;
	std::string										origin_path;
	std::string										redirect_path;
	std::string										autoindex_path;
	std::string										path;
	std::string										file_name;
	uint32_t										port;
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
	bool											chunked;
	int												status_code;
	bool											cgi;
	bool											redirect;
	bool											autoindex;
	bool											index;
	std::list<Server>::iterator						matched_server;
	std::list<Location>::iterator					matched_location;
	void											parse_query_string(std::string &query);
	void											parse_query_key_and_value(std::string &query_element);
	void											parse_header_key_and_value(std::string &header_element);
	void											check_header_is_valid();
	void											matching_server();
	void											matching_absolute_path();
	void											matching_route(std::list<Location>::iterator it, std::list<Location>::iterator ite);
	size_t											matching_sub_route(std::string route, std::string dest, size_t *depth);
	void											check_body_limits();
	void											check_host();
	void											check_transfer_encoding_and_content_length();
	void											check_transfer_encoding();
	void											check_content_length();
	void											check_te();
	void											check_is_cgi();
	std::string										check_index(std::list<Location>::iterator it);
	void											check_content_encoding();
	bool											check_allowed_method();
	void											check_uri_form();
	void											decode_chunked(std::string &msg);
	//////-------------utils--------------------------------------------------
	std::string										lower(const char *key, size_t end);
	void											remove_spf(std::string &value, size_t end);
	void											remove_spb(std::string &value, size_t end);
public:
	Request();
	~Request();
	void											process_request_parsing(std::string &request_msg, Cycle &cycle);
	void											parse_request();
	void											parse_request_line();
	void											parse_header_fields();

	//-----------------------------getter && setter------------------------------
	Cycle&											get_cycle_instance();
	bool											get_cgi();
	bool											get_redirect();
	bool											get_autoindex();
	bool											get_index();
	int												get_status_code();
	std::string&									get_method();
	bool											get_chunked();
	std::string&									get_redirect_path();
	std::string&									get_autoindex_path();
	std::string&									get_path();
	std::string&									get_file_name();
	std::string&									get_message_body();
	std::string&									get_header_field(const char *key);
	std::string&									get_query_value(const char *key);
	void											set_status_code(int status_code);
	void											set_cgi (bool flag);
	void											set_chunked (bool flag);
	void											set_header_key_and_value(std::string &key, std::string &value);
	void											set_header_key_and_value(const char *key, const char *value);
	void											set_port(uint32_t port);
	void											set_redirect(std::string main_root, std::string sub_root, std::string file);
	void check_members();
};

#endif