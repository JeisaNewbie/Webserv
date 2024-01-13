#include "Client.hpp"

Client::Client(){}
Client::Client(const Client& ref) {}
Client::~Client() {}

// Client&	Client::operator=(const Client& ref) {}

void	Client::do_parse(std::string &request_msg, Cycle &cycle)
{
	get_request_instance().process_request_parsing(request_msg, cycle);
}

void	Client::do_method()
{
	std::string	&path = get_request_instance().get_path();
	int			path_property = check_path_property(path);
	bool		cgi = get_cgi();

	try
	{
		if (cgi == true && path_property == _FILE)
			do_method_with_cgi();
		else if (cgi == false && (path_property == _FILE | path_property == _DIR))
			do_method_without_cgi(path, path_property);
		else
			throw NOT_FOUND;
	}
	catch(int e)
	{
		set_status_code(e);
	}

}

void	Client::do_method_with_cgi() {}

void	Client::do_method_without_cgi(std::string &path, int path_property)
{
	std::string	&method = get_request_instance().get_method();

	if (method == "POST")
		throw NO_CONTENT;

	if (method == "GET")
	{
		if (path_property == _FILE)
		{
			Get::make_body(response, path);
			Get::create_response(response);
			throw OK;
		}
		else
			throw NOT_FOUND;
	}

	if (method == "DELETE")
	{
		Delete::remove_file(path);
		Delete::create_response(response);
	}
}

void	Client::assemble_response()
{
	response.set_header_line (get_status_code());
	response.assemble_message ();
}

//------------------------getter && setter---------------------------
Phase	Client::get_current_phase() {return this->phase;}
Request& Client::get_request_instance () {return this->request;}
Response& Client::get_response_instance () {return this->response;}
int		Client::get_status_code() {return get_request_instance().get_status_code();}
bool	Client::get_cgi() {return this->get_request_instance().get_cgi();}
bool	Client::get_chunked() {return this->get_request_instance().get_chunked();}
void	Client::set_phase (Phase state) {this->phase = state;}
void	Client::set_status_code(int status_code) {get_request_instance().set_status_code(status_code);}
