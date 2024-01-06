
#include "Request.hpp"

Request::Request() {}
Request::Request(std::string &msg)
: request_msg(msg)
{
	this->pos = 0;
}

Request::~Request()
{
}

int	Request::process_request_parsing(std::string &request_msg)
{
	this->request_msg = request_msg;

	try
	{
		// std::cout<< "parse_request\n";
		parse_request ();
		// std::cout<< "parse_request_line\n";
		parse_request_line ();
		// std::cout<< "parse_header_fields\n";
		parse_header_fields ();
		// std::cout<< "check_members\n";
		// check_members();
		// std::cout<< "check_header_is_valid\n";
		check_header_is_valid ();
	}
	catch(int e)
	{
		this->status_code = e;
		std::cout << e << std::endl;
		return FAIL;
	}
	return this->status_code = OK;
}

void	Request::parse_request()
{
	std::string &msg = this->request_msg;
	size_t delimeter = msg.find ("\r\n");

	if (delimeter == 0)
	{
		this->pos = 2;
		delimeter = msg.find ("\r\n", this->pos);
		if (delimeter == std::string::npos)
			throw BAD_REQUEST;
	}

	this->request_line = msg.substr (pos, delimeter - pos);

	while (delimeter != std::string::npos)
	{
		this->pos = delimeter + 2;
		delimeter = msg.find ("\r\n", this->pos);

		if (this->pos == delimeter)
		{
			this->message_body = msg.substr (pos + 2,  msg.find ("\r\n", this->pos + 2) + 2);
			break ;
		}
		this->headers.push_back (msg.substr (pos, delimeter - pos + 2));
	}
}

void	Request::parse_request_line()
{
	std::string::iterator it = this->request_line.begin();
	std::string::iterator ite = this->request_line.end();
	std::string::iterator method_start;
	std::string::iterator method_end;
	std::string::iterator uri_start;
	std::string::iterator uri_end;
	std::string::iterator query_start;
	std::string::iterator query_end;
	char	ch;

	enum {
		start = 0,
		method,
		uri,
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
				uri_start = it + 1;
				state = uri;
				pos = uri_start - method_start;

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

		case uri: //scheme[http], authority[userinfo@]host[:port], path, query

			if (ch == '?')
			{
				uri_end = it;
				query_start = it + 1;
				state = query;
				this->uri = this->request_line.substr (pos, uri_end - uri_start);
				pos = uri_end + 1 - method_start;
				break;
			}

			if (ch == ' ')
			{
				uri_end = it;
				state = http;
				this->uri = this->request_line.substr (pos, uri_end - uri_start);
				this->request_target = this->uri;
				pos = uri_end + 1 - method_start;
				break;
			}

			check_uri_form();
			break;

		case query:

			if (ch == ' ')
			{
				query_end = it;
				this->query = this->request_line.substr (pos, query_end - query_start);
				this->request_target = this->request_line.substr (uri_start - method_start, query_end - uri_start); //check_request_target_length
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

	std::string	key = lower (header_element.substr (0, pos).c_str(), pos);
	std::string	value = header_element.substr (pos + 1, end);

	remove_spf(value, end);
	remove_spb (value, value.find ("\r\n"));
	set_header_key_and_value (key, value);
}

void	Request::set_header_key_and_value(std::string &key, std::string &value)
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


void	Request::check_header_is_valid()
{
	// std::cout<< "check_host\n";
	check_host();
	// std::cout<< "check_transfer_encoding_and_content_length\n";
	check_transfer_encoding_and_content_length();
	// std::cout<< "check_te\n";
	check_te();
	// std::cout<< "check_content_encoding\n";
	check_content_encoding();
	// check_header_limits(); // config로 설정한 서버의 header limits size를 넘으면 return 413
	// check_body_limits(); //client의 body size를 넘으면 reutnr 413
	// check_request_target_limits(); //request-target이 config파일의 서버 uri보다 길면 return 414
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
			port_num = port_num * 10 + host[i] - '0';
			port_len++;
			continue;
		}

		if (!std::isalnum (host[i]) && host[i] != '.' && host[i] != '-')
			throw BAD_REQUEST;
	}
	// if (port_num != this->port)
	// 	throw BAD_REQUEST;
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
		this->header["connection"] = "close";
		throw BAD_REQUEST;
	}

	// decode_chunked();
}

void	Request::check_content_length()
{
	// std::cout<< "check_content_length\n";
	char	*endptr;
	this->content_length = std::strtol(this->header["content-length"].c_str(), &endptr, 10);
	// std::cout<< "check_content_length_1\n";
	if (strlen(endptr) > 1 && strncmp (endptr, "\r\n", 2) != 0)
		throw BAD_REQUEST;
	// std::cout<< "check_content_length_2\n";
	if (this->content_length > 2147483647)
		throw REQUEST_ENTITY_TOO_LARGE;
	// std::cout<< "check_content_length_3\n";
	if (this->header["content-length"][0] == '0' && this->header["content-length"].size() > 1)
		throw BAD_REQUEST;
	// std::cout<< "check_content_length_4\n";
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

void Request::check_members()
{

	std::cout << "--------------------------------------------------------------\n";
	std::cout << "Request_line: " << this->request_line << std::endl;
	std::cout << std::endl;
	std::cout << "Request_method: " << this->method << std::endl;
	std::cout << std::endl;
	std::cout << "Request_target: " << this->request_target << std::endl;
	std::cout << std::endl;
	std::cout << "Request_uri: " << this->uri << std::endl;
	std::cout << std::endl;
	std::cout << "Request_query: " << this->query << std::endl;
	std::cout << std::endl;
	for (std::map<std::string, std::string>::iterator it = this->query_elements.begin(); it != this->query_elements.end(); it++)
		std::cout << "Request_query_element[ " << it->first << " ]: " << it->second << std::endl<<std::endl;

	std::cout<< "---------------Request_headers-----------------------------------------------\n";

	for (std::list<std::string>::iterator it = this->headers.begin(); it != this->headers.end(); it++)
		std::cout << *it << std::endl;

	std::cout<< "---------------Request_headers_after_parsing-----------------------------------------------\n";

	for (std::map<std::string, std::string>::iterator it = this->header.begin(); it != this->header_end; it++)
		std::cout << it->first << " | " << it->second << std::endl;

	std::cout << "--------------------------------------------------------------\n";
	std::cout << "Request_body: " << this->message_body << std::endl;
	std::cout<< "--------------------------------------------------------------\n";
}

int	Request::get_status_code() {return this->status_code;}
std::string& Request::get_method() {return this->method;}
bool Request::get_cgi() {return this->cgi;}

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
