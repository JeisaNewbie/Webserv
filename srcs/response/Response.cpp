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

Response::Response() {}
Response::Response(const Response& ref) {}
Response::~Response() {}

Response& Response::operator=(const Response& ref) {}

void	Response::set_header_line (int status_code)
{
	header_line = "HTTP/1.1 ";
	header_line += status_line[status_code].code;
	header_line += " ";
	header_line += status_line[status_code].text;
	header_line += "\r\n";
}
