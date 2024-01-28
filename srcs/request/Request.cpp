
#include "Request.hpp"

Request::Request()
{
	this->pos = 0;
	this->chunked = false;
	this->port = 80;
	this->expect = false;
	this->cgi = false;
	this->redirect = false;
	this->autoindex = false;
	this->index = false;
	this->file_name = "";
	this->content_length = 0;
}

Request::~Request() {}

void	Request::reset_data()
{
	request_msg.clear();
	cycle = NULL;
	request_line.clear();
	uri.clear();
	origin_path.clear();
	redirect_path.clear();
	autoindex_path.clear();
	path.clear();
	file_name.clear();
	host_only.clear();
	query.clear();
	protocol_version.clear();
	method.clear();
	headers.clear();
	header.clear();
	query_elements.clear();
	message_body.clear();

	this->pos = 0;
	this->content_length = 0;
	this->port = 80;
	this->chunked = false;
	this->expect = false;
	this->cgi = false;
	this->redirect = false;
	this->autoindex = false;
	this->index = false;
	this->file_name = "";
}

void	Request::process_request_parsing(std::string &request_msg, Cycle &cycle)
{
	try
	{
		this->request_msg = request_msg;
		this->cycle = &cycle;
		// std::cout<< "parse_request\n";
		parse_request ();
		std::cout<< "parse_request_line\n";
		parse_request_line ();
		std::cout<< "parse_header_fields\n";
		parse_header_fields ();
		std::cout<< "check_members\n";
		// check_members();
		std::cout<< "check_header_is_valid\n";
		check_header_is_valid ();
		// std::cout << "matching_server\n";
		matching_server(); // port와 listen이 일치하는지 확인 &&  host와 server_name 일치 확인 -> location과 uri(path)와 일치하는지 확인 (만약 path가 absolute form으로 올경우 그중 path를 파싱해서 path 와 location 비교)
		// std::cout<<"finish_mathcing_server\n";
	}
	catch(int e)
	{
		this->status_code = e;
		this->request_msg = "";
		std::cout << "REQUEST_PARSING_DONE_AND STATUS_CODE: " << e << std::endl;
	}
}

void	Request::parse_request()
{
	std::cout << "PARSE_REQUEST_START\n";
	std::string &msg = this->request_msg;
	size_t delimeter = msg.find ("\r\n");
	this->pos = 0;

	if (delimeter == std::string::npos)
		throw BAD_REQUEST;

	if (delimeter == 0)
	{
		std::cout << "PARSE_REQUEST_DELIMETER_0\n";
		this->pos = 2;
		if (this->pos >= this->request_msg.size())
			throw BAD_REQUEST;
		delimeter = msg.find ("\r\n", this->pos);
		if (delimeter == std::string::npos)
			throw BAD_REQUEST;
		std::cout << "PARSE_REQUEST_DELIMETER_0_DONE\n";
	}

	std::cout << "PARSE_REQUEST_WHILE_READY\n";
	std::cout << "POS: " << this->pos << ", DELIMETER: " << delimeter << std::endl;
	this->request_line = msg.substr (this->pos, delimeter - this->pos);
	std::cout << "PARSE_REQUEST_WHILE_START\n";

	while (delimeter != std::string::npos)
	{
		this->pos = delimeter + 2;
		delimeter = msg.find ("\r\n", this->pos);

		if (this->pos == delimeter)
		{
			this->message_body = msg.substr (this->pos + 2);
			break ;
		}
		this->headers.push_back (msg.substr (this->pos, delimeter - this->pos + 2));
	}
	std::cout << "PARSE_REQUEST_END\n";
}

void	Request::parse_request_line()
{
	std::string::iterator it = this->request_line.begin();
	std::string::iterator ite = this->request_line.end();
	std::string::iterator method_start;
	std::string::iterator method_end;
	std::string::iterator path_start;
	std::string::iterator path_end;
	std::string::iterator uri_start;
	std::string::iterator uri_end;
	std::string::iterator query_start;
	std::string::iterator query_end;
	char	ch;
	std::cout << "PARSE_REQUEST_LINE_START\n";
	enum {
		start = 0,
		method,
		spaces_before_uri,
		scheme_h,
		scheme_ht,
		scheme_htt,
		scheme_http,
		scheme_slash,
		scheme_slash_slash,
		uri,
		uri_path,
		query,
		query_parsing,
		http,
		http_H,
		http_HT,
		http_HTT,
		http_HTTP,
		major_digit,
		dot,
		minor_digit,
		almost_done,
		done
	} state;

	state = start;

	for (size_t pos = 0; it != ite; it++)
	{
		ch = *it;
		switch (state)
		{
		case start:

			if (ch != 'G' && ch != 'P' && ch != 'D')
				throw BAD_REQUEST;

			state = method;
			method_start = it;
			pos = it - method_start;
			break;

		case method:

			if (ch == ' ')
			{
				method_end = it;
				state = spaces_before_uri;
				pos = it + 1 - method_start;

				switch (method_end - method_start)
				{
				case 3:

					if (this->request_line.find ("GET") != std::string::npos)
					{
						this->method = "GET";
						break;
					}

					else
						throw BAD_REQUEST;

				case 4:

					if (this->request_line.find ("POST") != std::string::npos)
					{
						this->method = "POST";
						break;
					}

					else
						throw BAD_REQUEST;

				case 6:

					if (this->request_line.find ("DELETE") != std::string::npos)
					{
						this->method = "DELETE";
						break;
					}

					else
						throw BAD_REQUEST;

				default:
					throw BAD_REQUEST;
				}
			}

			if (!('A' <= ch && ch <= 'Z') && ch != ' ')
				throw BAD_REQUEST;

			break;

		case spaces_before_uri:

			if (ch == '/')
			{
				uri_start = it;
				path_start = it;
				pos = uri_start + 1 - method_start;
				state = uri_path;
				break;
			}

			if (ch == 'h')
			{
				state = scheme_h;
				break;
			}

			if (ch == ' ')
				break;
			else
				throw BAD_REQUEST;

			break;

		case scheme_h:

			switch (ch)
			{
			case 't':
				state = scheme_ht;
				break;

			default:
				throw BAD_REQUEST;
			}

			break;

		case scheme_ht:

			switch (ch)
			{
			case 't':
				state = scheme_htt;
				break;

			default:
				throw BAD_REQUEST;
			}

			break;

		case scheme_htt:

			switch (ch)
			{
			case 'p':
				state = scheme_http;
				break;

			default:
				throw BAD_REQUEST;
			}

			break;

		case scheme_http:

			switch (ch)
			{
			case ':':
				state = scheme_slash;
				break;

			default:
				throw BAD_REQUEST;
			}

			break;


		case scheme_slash:

			if (ch == '/')
			{
				state = scheme_slash_slash;
				break;
			}
			else
				throw BAD_REQUEST;

			break;

		case scheme_slash_slash:

			if (ch == '/')
			{
				if (*(it + 1) == '/')
					throw BAD_REQUEST;
				state = uri;
				uri_start = it + 1;
				pos = uri_start - method_start;
				break;
			}
			else
				throw BAD_REQUEST;

			break;

		case uri:

			if (('a' <= ch && ch <= 'z') || ch == '.')
				break;

			if (ch == '/')
			{
				path_start = it;
				state = uri_path;
				break;
			}

		case uri_path:

			if (('a' <= ch && ch <= 'z') || ch == '.')
				break;

			if (ch == '?')
			{
				path_end = it;
				query_start = it + 1;
				state = query;
				this->path = this->request_line.substr (path_start - method_start, path_end - path_start);
				pos = uri_end + 1 - method_start;
				break;
			}

			if (ch == ' ')
			{
				uri_end = it;
				state = http;
				this->path = this->request_line.substr (path_start - method_start, uri_end - path_start);
				std::cout <<"PARSING_PATH: " << path << std::endl;
				this->uri = this->request_line.substr (uri_start - method_start, uri_end - uri_start);
				check_uri_form();
				if (this->uri.size() > cycle->getUriLimitLength())
					throw URI_TOO_LONG;
				pos = uri_end + 1 - method_start;
				break;
			}

			break;

		case query:

			if (ch == ' ')
			{
				query_end = it;
				this->query = this->request_line.substr (query_start - method_start, query_end - query_start);
				this->uri = this->request_line.substr (uri_start - method_start, query_end - uri_start);
				check_uri_form();
				if (this->uri.size() > cycle->getUriLimitLength())
					throw URI_TOO_LONG;
				state = query_parsing;
			}

			break;

		case query_parsing:

			parse_query_string(this->query);
			state = http;

		case http:

			switch (ch)
			{
			case 'H':
				state = http_H;
				break;

			default:
				throw BAD_REQUEST;
			}

			break ;

		case http_H:

			switch (ch)
			{
			case 'T':
				state = http_HT;
				break;

			default:
				throw BAD_REQUEST;
			}

			break ;

		case http_HT:

			switch (ch)
			{
			case 'T':
				state = http_HTT;
				break;

			default:
				throw BAD_REQUEST;
			}

			break ;

		case http_HTT:

			switch (ch)
			{
			case 'P':
				state = http_HTTP;
				break;

			default:
				throw BAD_REQUEST;
			}

			break ;

		case http_HTTP:

			switch (ch)
			{
			case '/':
				state = major_digit;
				break;

			default:
				throw BAD_REQUEST;
			}

			break ;

		case major_digit:

			if (!('0' <= ch && ch <= '9'))
				throw BAD_REQUEST; //invalid parsing

			if (ch != '1')
				throw HTTP_VERSION_NOT_SUPPORTED; //invalid version

			state = dot;
			break ;

		case dot:

			if (ch == '.')
			{
				state = minor_digit;
				break ;
			}

			throw BAD_REQUEST; //invalid parsing

		case minor_digit:

			switch (ch)
			{
			case '0':
				this->protocol_version = "1.0";
				state = almost_done;
				break;

			case '1':
				this->protocol_version = "1.1";
				state = almost_done;
				break;

			default:
				if ('0' <= ch && ch <= '9')
					throw HTTP_VERSION_NOT_SUPPORTED; //invalid version

				if (!('0' <= ch && ch <= '9'))
					throw BAD_REQUEST; //invalid parsing
			}

			break ;

		case almost_done:

			switch (ch)
			{
			case CR:
				state = done;
				break;

			case LF:
				return ;

			case ' ':
				break;

			default:
				throw BAD_REQUEST; //invalid parsing
			}

			break ;

		case done:
			switch (ch)
			{
			case LF:
				return ;

			default:
				throw BAD_REQUEST; //invalid parsing
			}
		}
	}
}

void	Request::parse_query_string(std::string &query)
{
	size_t delimeter = query.find ('&');
	size_t pos = 0;

	this->header["query_string"] = query;
	if (delimeter == std::string::npos)
	{
		parse_query_key_and_value (query);
		return;
	}

	while (delimeter != std::string::npos)
	{
		std::string tmp = query.substr (pos, delimeter - pos);
		parse_query_key_and_value (tmp);
		pos = delimeter + 1;
		delimeter = query.find ('&', pos);
	}
}

void	Request::parse_query_key_and_value(std::string &query_element)
{
	size_t pos = query_element.find('=');
	if (pos == std::string::npos)
		throw BAD_REQUEST;
	this->query_elements[query_element.substr (0, pos)] = query_element.substr (pos + 1, query_element.size() - pos - 1);
}

void	Request::parse_header_fields()
{
	std::list<std::string>::iterator it = this->headers.begin();
	std::list<std::string>::iterator ite = this->headers.end();

	for (; it != ite; it++)
		parse_header_key_and_value (*it);

	if (header.find("connection") == this->header.end())
		set_header_key_and_value ("connection", "keep-alive\r\n");
	this->header_end = this->header.end ();
	return ;
}

void	Request::parse_header_key_and_value(std::string &header_element)
{
	size_t 	pos = header_element.find (":");
	size_t	end =  header_element.size() - pos - 1;
	size_t	crlf = header_element.find ("\r\n");

	if (pos == std::string::npos || crlf == std::string::npos)
		throw BAD_REQUEST;

	// std::cout<<"HEADER_FIELD: "<<header_element<<std::endl;
	std::string	key = lower (header_element.substr (0, pos).c_str(), pos);
	std::string	value = header_element.substr (pos + 1, end);
	// std::cout<<"HEADER_KEY_AND_VALUE: "<<key<<", "<<value<<std::endl;
	remove_spf(value, end);
	remove_spb (value, value.find ("\r\n"));
	set_header_key_and_value (key, value);
}

void	Request::check_header_is_valid()
{
	std::cout<< "check_host\n";
	check_host();
	std::cout<< "check_transfer_encoding_and_content_length\n";
	check_transfer_encoding_and_content_length();
	std::cout<< "check_te\n";
	check_te();
	std::cout<< "check_content_encoding\n";
	check_content_encoding();
	std::cout<<"check_body_limits\n";
	// check_header_limits(); // config로 설정한 서버의 header limits size를 넘으면 return 413
	check_body_limits();
	check_expect();
}
void	Request::check_expect()
{
	if (header.find("expect") == header_end)
		return;

	if (protocol_version == "1.0")
		return;

	if (header["expect"] != "100-continue")
		throw EXPECTION_FAILED;

	if (header.find ("content-length") == header_end)
	{
		if (header.find ("transfer-encoding") == header_end)
			throw BAD_REQUEST;
	}
	else if (header["content-length"] == "0\r\n")
		throw BAD_REQUEST;

	this->expect = true;
	throw CONTINUE;

}

void	Request::check_body_limits()
{
	// std::cout <<"CLIENT_MAX_BODY_SIZE: " << cycle->getClientMaxBodySize()<<std::endl;
	if (content_length > cycle->getClientMaxBodySize())
	{
		std::cout <<"CHECK_BODY_LIMITS_CONTENT_LENGTH: " << content_length << std::endl;
		throw REQUEST_ENTITY_TOO_LARGE;
	}
}

void	Request::check_host()
{
	if (this->header.find ("host") == this->header_end)
	{
		std::cout<<"no host header field\n";
		throw BAD_REQUEST;
	}

	std::string	&host = this->header["host"];
	size_t		end = host.size () - 2;
	size_t		port_num = 0;
	size_t		port_len = 0;
	bool		port_flag = false;

	for (size_t i = 0; i < end; i++)
	{
		if (host[i] == ':' && port_flag == false)
		{
			port_flag = true;
			this->host_only = host.substr (0, host.find (':'));
			if (i == end - 1)
				throw BAD_REQUEST;
			continue;
		}

		if (port_flag == true)
		{
			if (!('0' <= host[i] && host[i] <= '9'))
				throw BAD_REQUEST;
			if (port_len > 5)
				throw BAD_REQUEST;
			if (port_num > 65535)
				throw BAD_REQUEST;
			port_num = port_num * 10 + (host[i] - '0');
			std::cout<<"PORT_NUM: "<<port_num<<std::endl;
			port_len++;
			continue;
		}

		if (!std::isalnum (host[i]) && host[i] != '.' && host[i] != '-')
			throw BAD_REQUEST;
	}

	if (port_num != 0)
		this->port = port_num;
}

void	Request::check_transfer_encoding_and_content_length()
{
	if (this->header.find("transfer-encoding") != this->header_end \
		&& this->header.find("content-length") != this->header_end)
		throw BAD_REQUEST;
	else if (this->header.find("transfer-encoding") == this->header_end \
		&& this->header.find("content-length") == this->header_end)
		return ;
	else if (this->header.find("transfer-encoding") != this->header_end)
		check_transfer_encoding();
	else
		check_content_length();
}

void	Request::check_transfer_encoding()
{
	std::string	&transfer_encoding_value = this->header["transfer-encoding"];
	size_t		pos = transfer_encoding_value.find ("chunked\r\n");

	if (pos != 0)
		throw NOT_IMPLEMENTED; //chunked외에 다른 인코딩이 있다는 뜻. 자원되지 않는 인코딩은 501

	if (transfer_encoding_value.find ("\r\n", pos + 7) != 0)
	{
		this->header["connection"] = "close\r\n";
		throw BAD_REQUEST;
	}
	decode_chunked (this->message_body);
}

void	Request::decode_chunked(std::string &msg) // "0\r\n 없으면 무조건 chunkded error"
{
	std::string	chunk = msg;
	size_t		chunk_size = 0;
	size_t		delimeter = 0;
	size_t		pos = 0;

	if (msg.find ("0\r\n") == std::string::npos)
		throw BAD_REQUEST;
	this->message_body = "";

	while (1)
	{
		delimeter = chunk.find("\r\n", pos);
		if (delimeter == std::string::npos)
			break;
		chunk_size = std::strtol (chunk.substr(pos, delimeter - pos).c_str(), NULL, 16);
		this->content_length += chunk_size;
		if (chunk_size == 0)
		{
			set_chunked (true);
			throw OK;
		}
		this->message_body += chunk.substr(delimeter + 2, chunk_size);
		pos = (delimeter + 2) + (chunk_size + 2);
	}
}

void	Request::check_content_length()
{
	std::cout<< "check_content_length\n";
	char	*endptr;
	this->content_length = std::strtol(this->header["content-length"].c_str(), &endptr, 10);
	std::cout<< "check_content_length_1\n";
	if (strlen(endptr) > 1 && strncmp (endptr, "\r\n", 2) != 0)
		throw BAD_REQUEST;
	std::cout<< "check_content_length_2\n";
	if (this->content_length > 2147483647)
		throw REQUEST_ENTITY_TOO_LARGE;
	std::cout<< "check_content_length_3\n";
	if (this->header["content-length"][0] == '0' && this->header["content-length"].substr(0, this->header["content-length"].find("\r\n")).size() > 1)
		throw BAD_REQUEST;
	std::cout<< "check_content_length_4\n";
	if (this->content_length < this->message_body.size() || this->content_length > this->message_body.size())
		throw BAD_REQUEST;

}

void	Request::check_te()
{
	if (this->header.find ("te") == this->header_end)
		return;
	if (this->header["te"].find ("chunked\r\n") == 0)
		return;
	throw NOT_IMPLEMENTED;
}

void	Request::check_content_encoding()
{
	if (this->header.find ("content-encoding") != this->header_end)
	{
		if (this->header["content-encoding"].empty())
			return ;
		throw UNSUPPORTED_MEDIA_TYPE;
	}
}

void	Request::check_uri_form()
{
	size_t	pos = this->uri.find("//");

	if (pos == 0)
		pos = this->uri.find("//", 1);

	if (pos != std::string::npos)
		throw NOT_FOUND;
}

void	Request::matching_absolute_path()
{
	std::string	tmp_path = cycle->getMainRoot() + path;
	// std::cout<<"MATCHING_ABSOLUTE_PATH: " << tmp_path << std::endl;
	int			path_property = check_path_property(tmp_path);

	origin_path = path;

	if (path_property == _FILE || path_property == _DIR)
	{
		path = tmp_path;
		throw OK;
	}
}

void	Request::check_is_cgi()
{
	set_header_key_and_value("redirect_path", "/serve/redirect/");
	if (origin_path.find(".cpp") != std::string::npos)
	{
		if (origin_path.find("/script.cpp") == std::string::npos)
			throw NOT_FOUND;

		if (method == "DELETE")
		{
			path = cycle->getMainRoot() + get_header_field("redirect_path") + get_query_value("deletedata");
			throw OK;
		}
		path = cycle->getMainRoot() + "/serve/script/script.cgi";
		this->set_cgi (true);
		throw OK;
	}
}

std::string Request::check_index(std::list<Location>::iterator it)
{
	std::vector<std::string>::iterator	it_v = it->getIndex().begin();
	std::vector<std::string>::iterator	ite_v = it->getIndex().end();
	std::string							path_index;

	for (; it_v != ite_v; it_v++)
	{
		path_index = cycle->getMainRoot() + it->getSubRoot() + "/" + *it_v;
		// std::cout<<"PATH_INDEX: "<< path_index << std::endl;
		if (check_path_property(path_index) == _FILE)
		{
			return  "/" + *it_v;
		}
	}

	return "";
}

void	Request::matching_server()
{
	std::list<Server>			&servers = cycle->getServerList();
	std::list<Server>::iterator it = servers.begin();
	std::list<Server>::iterator ite = servers.end();
	std::string 				&host = this->host_only;

	// std::cout <<"PATH: " << path << std::endl;

	if (path != "/")
		matching_absolute_path();

	matched_server = cycle->getServerList().begin();
	check_is_cgi();

	// std::cout<<"BEFORE_MATCHING_SERVER\n";
	for (; it != ite; it++)
	{
		// std::cout <<"PORT_MATCHED\n";
		if (port != it->getPort())
			continue;
		// std::cout << "SERVER IS " << it->getDomain() << std::endl;
		if (host != it->getDomain())
			continue;
		// std::cout <<"SERVER_MATCHED\n";
		// std::cout<<"SERVER: " << it->getDomain() << std::endl;
		matched_server = it;
		break;
	}
	// std::cout<<"BEFORE_MATCHING_ROUTE\n";
	matching_route(matched_server->getLocationList().begin(), matched_server->getLocationList().end());
	if (check_allowed_method () == false)
		throw METHOD_NOT_ALLOWED;

	if (matched_location->getAutoIndex() == true)
	{
		this->autoindex = true;
		this->autoindex_path = cycle->getMainRoot() + matched_location->getSubRoot();
		if (*autoindex_path.rbegin() != '/')
			autoindex_path += '/';
		// std::cout<<"AUTOINDEX_PATH: " << this->autoindex_path << std::endl;
		throw OK;
	}
	else if (matched_location->getIndex().size() != 0)
	{
		// std::cout<<"BEFORE_SET_REDIRECT\n";
		this->file_name = check_index(matched_location);
		if (this->file_name != "")
			set_redirect(cycle->getMainRoot(), matched_location->getSubRoot(), file_name);
	}
	throw NOT_FOUND;
}

bool	Request::check_allowed_method()
{
	std::string &method = this->method;

	if (method == "GET")
		return (matched_location->getAllowedMethod() & METHOD_GET);

	if (method == "POST")
		return (matched_location->getAllowedMethod() & METHOD_POST);

	return (matched_location->getAllowedMethod() & METHOD_DELETE);
}

void	Request::set_redirect(std::string main_root, std::string sub_root, std::string file)
{
	this->redirect = true;
	this->redirect_path = sub_root + file;
	std::cout <<"REDIRECT: " << this->redirect_path << std::endl;
	throw FOUND;
}

void	Request::matching_route(std::list<Location>::iterator it, std::list<Location>::iterator ite)
{
	std::list<Location>::iterator					it_begin = it;
	std::map<size_t, std::list<Location>::iterator>	depth_map;
	size_t											depth = 0;
	int												i = 0;
	std::string										sub_r;
	std::string										sub_d = path;

	if (*(sub_d.rbegin()) == '/' && sub_d.size() > 1)
		sub_d = sub_d.substr (0, sub_d.size() - 1);

	for (; it != ite; it++)
	{
		try {
			std::cout<<"MATCHING_ROUTE_ING\n";
			sub_r = it->getLocationPath();
			matching_sub_route(sub_r, sub_d, &depth);
		}
		catch (size_t e){
			matched_location = it;
			return ;
		}
		depth_map[depth] = it;
		std::cout << "MATCHING_ROUTE_DEPTH_MAP: " << depth << std::endl;
		depth = 0;
	}
	std::cout<<"MATCHING_ROUTE_DONE\n";
	matched_location = it_begin;
	if (depth_map.rbegin()->first != 0)
		matched_location = depth_map.rbegin()->second;
}

size_t	Request::matching_sub_route(std::string route, std::string dest, size_t *depth)
{
	if (route == "" && dest == "")
		throw std::string::npos;

	// if ((route == dest) && depth == 0)
	// 	throw std::string::npos;

	if (route == dest)
	{
		std::cout<<"route == sub_\n";
		(*depth)++;
		if (route.find ('/', 1) != std::string::npos && dest.find ('/', 1) != std::string::npos)
			matching_sub_route (route.substr (route.find ('/', 1)), dest.substr (dest.find ('/', 1)), depth);
		return *depth;
	}

	if (route != "" && dest != "")
	{
		std::cout<<"route != "" && dest != ""\n";
		return 0;
	}

	return *depth;
}

void	Request::check_members()
{

	std::cout << "--------------------------------------------------------------\n";
	std::cout << "Request_line: " << this->request_line << std::endl;
	std::cout << std::endl;
	std::cout << "Request_method: " << this->method << std::endl;
	std::cout << std::endl;
	std::cout << "Request_uri: " << this->uri << std::endl;
	std::cout << std::endl;
	std::cout << "Request_path: " << this->path << std::endl;
	std::cout << std::endl;
	std::cout << "Request_query: " << this->query << std::endl;
	std::cout << std::endl;
	for (std::map<std::string, std::string>::iterator it = this->query_elements.begin(); it != this->query_elements.end(); it++)
		std::cout << "Request_query_element[ " << it->first << " ]: " << it->second << std::endl<<std::endl;

	std::cout<< "---------------Request_headers-----------------------------------------------\n";

	for (std::list<std::string>::iterator it = this->headers.begin(); it != this->headers.end(); it++)
		std::cout << *it << std::endl;

	std::cout<< "---------------Request_headers_after_parsing-----------------------------------------------\n";

	for (std::map<std::string, std::string>::iterator it = this->header.begin(); it != this->header.end(); it++)
		std::cout << it->first << " | " << it->second << std::endl;

	std::cout << "--------------------------------------------------------------\n";
	std::cout << "Request_body: " << this->message_body << std::endl;
	std::cout<< "--------------------------------------------------------------\n";
}

//-----------------------------getter && setter------------------------------
Cycle&			Request::get_cycle_instance() {return *(this->cycle);}
std::string&	Request::get_header_field(const char *key) {return this->header[key];}
std::string&	Request::get_query_value(const char *key) {return this->query_elements[key];}
int				Request::get_status_code() {return this->status_code;}
std::string&	Request::get_method() {return this->method;}
bool			Request::get_redirect() {return this->redirect;}
bool			Request::get_autoindex() {return this->autoindex;}
bool			Request::get_index() {return this->index;}
bool 			Request::get_cgi() {return this->cgi;}
bool 			Request::get_expect() {return this->expect;}
bool			Request::get_chunked() {return this->chunked;}
std::string&	Request::get_message_body() {return this->message_body;}
std::string&	Request::get_redirect_path() {return this->redirect_path;}
std::string&	Request::get_autoindex_path() {return this->autoindex_path;}
std::string&	Request::get_path() {return this->path;}
std::string&	Request::get_file_name() {return this->file_name;}
void 			Request::set_status_code(int status_code) {this->status_code = status_code;}
void			Request::set_cgi (bool flag) {this->cgi = flag;}
void			Request::set_chunked (bool flag) {this->chunked = flag;}
void			Request::set_header_key_and_value(const char *key, const char *value){this->header[key] = value;}
void			Request::set_port(uint32_t port) {this->port = port;}
void			Request::set_header_key_and_value(std::string &key, std::string &value)
{

	if (this->header.find(key) != this->header.end())
	{
		this->header[key].pop_back();
		this->header[key].pop_back();

		if (key == "host" || key == "content-length")
			throw BAD_REQUEST;

		if (key == "cookie")
		{
			this->header[key].append ("; " + value + "\r\n");
			return;
		}

		this->header[key].append(", " + value + "\r\n");
		return;
	}
	this->header.insert (std::pair<std::string, std::string>(key, value));
}
//----------------------------------------utils---------------------------------------

std::string	Request::lower(const char *key, size_t end)
{
	const char	lowcase[] =
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0" "0123456789\0\0\0\0\0\0"
		"\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
		"\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

	std::string	tmp_key("", end);

	for (size_t i = 0; i < end; i++)
	{
		if (lowcase[static_cast<unsigned int>(key[i])] == '\0')
			throw BAD_REQUEST; //INVALID_HEADER
		tmp_key.at(i) = lowcase[static_cast<unsigned int>(key[i])];
	}

	return tmp_key;
}

void	Request::remove_spf(std::string &value, size_t end)
{
	size_t	i = 0;

	while (1)
	{
		if (value[i] != ' ')
		{
			value.erase (0, i);
			return;
		}

		if (i == end)
			throw BAD_REQUEST;

		i++;
	}
}

void	Request::remove_spb(std::string &value, size_t end)
{
	size_t	i = end - 1;

	while (1)
	{
		if (value[i] != ' ')
		{
			value.erase (i + 1, end - i - 1);
			return ;
		}

		if (i-- == 0)
			throw BAD_REQUEST;
	}
}
