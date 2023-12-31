#include "Client.hpp"

Client::Client(){}
Client::Client(const Client& ref) {}
Client::~Client() {}

Client&	Client::operator=(const Client& ref) {}

void	Client::do_parse(std::string &request_msg, Cycle &cycle)
{
	get_request_instance().process_request_parsing(request_msg, cycle);
}

void	Client::do_method()
{
	int		path_property = check_path_property(get_request_instance().get_path());
	bool	cgi = get_cgi();

	if (cgi == true && path_property == FILE)
		do_method_with_cgi();
	else if (cgi == false && (path_property == FILE | path_property == DIR))
		do_method_without_cgi(path_property);
	else
		return set_status_code(NOT_FOUND);
}

void	Client::do_method_with_cgi() {}

void	Client::do_method_without_cgi(int path_property)
{
	std::string	&method = get_request_instance().get_method();

	if (method == "POST")
		throw ;

	if (method == "GET")
	{
		if (path_property == FILE)
			Get::make_body(get_request_instance().get_path());
		// else
		// 	Get::
	}

	if (method == "DELETE")
	{
		if (path_property == DIR)
			throw NOT_FOUND;
	}
}

void	Client::assemble_response() {}

//------------------------getter && setter---------------------------
Phase	Client::get_current_phase() {return this->phase;}
Request& Client::get_request_instance () {return this->request;}
Response& Client::get_response_instance () {return this->response;}
int		Client::get_status_code() {return get_request_instance().get_status_code();}
bool	Client::get_cgi() {return this->get_request_instance().get_cgi();}
bool	Client::get_chunked() {return this->get_request_instance().get_chunked();}
void	Client::set_phase (Phase state) {this->phase = state;}
void	Client::set_status_code(int status_code) {get_request_instance().set_status_code(status_code);}
