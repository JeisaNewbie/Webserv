#include "Response.hpp"
#include "../utils/Status.hpp"

Status	status_line[] = {
	{"200", "OK"},
	{"201", "Created"},
	{"202", "Accepted"},
	{NULL, NULL},
	{"204", "No Content"},
	{NULL, NULL},
	{NULL, NULL},
	{"301", "Moved Permanently"},
	{"302", "Found"},
	{"303", "See Other"},
	{"304", "Not Modified"},
	{NULL, NULL},
	{NULL, NULL},
	{"307", "Temporary Redirect"},
	{"308", "Permanent Redirect"},
	{"400", "Bad Request"},
	{"401", "Unathorized"},
	{NULL, NULL},
	{"403", "Forbidden"},
	{"404", "Not Found"},
	{"405", "Method Not Allowed"},
	{"406", "Not Acceptable"},
	{NULL, NULL},
	{"408", "Request Timeout"},
	{"409", "Conflict"},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{"413", "Content Too Large"},
	{"414", "URI Too Long"},
	{"415", "Unsupported Media Type"},
	{NULL, NULL},
	{"417", "Expectation Failed"},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{NULL, NULL},
	{"500", "Internal Server Error"},
	{"501", "Not Implemented"},
	{"502", "Bad Gateway"},
	{"503", "Service Unavailable"},
	{"504", "Gateway Timeout"},
	{"505", "HTTP Version Not Supported"}
};

Response::Response()
{
	body_flag = false;
}

Response::Response(const Response& ref) {}
Response::~Response() {}

Response& Response::operator=(const Response& ref) {}

void	Response::assemble_message()
{
	response_msg = get_header_line();
	response_msg += get_header_field();
	response_msg += "\r\n";
	if (body_flag == true)
		response_msg += get_body();
}

std::string	&Response::get_header_line () {return this->header_line;}

std::string	Response::get_header_field ()
{
	std::map<std::string, std::string>::iterator	it = header.begin();
	std::map<std::string, std::string>::iterator	ite = header.end();
	std::string	header_fields;

	for (; it != ite; it++)
		header_fields += it->first + ": " + it->second + "\r\n";

	return header_fields;
}

std::string	&Response::get_body() {return this->body;}

void	Response::set_header_line (int status_code)
{
	header_line = "HTTP/1.1 ";
	header_line += status_line[status_code].code;
	header_line += " ";
	header_line += status_line[status_code].text;
	header_line += "\r\n";
}

void	Response::set_header_field (const std::string &key, const std::string &value)
{
	header[key] = value;
}