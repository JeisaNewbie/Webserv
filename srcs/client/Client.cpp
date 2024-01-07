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
	if (get_cgi() == true && check_path_property() == FILE)
		do_method_with_cgi();
	else
		do_method_without_cgi();
}

void	Client::do_method_with_cgi() {}

void	Client::do_method_without_cgi() {}

void	Client::assemble_response() {}


//------------------------getter && setter---------------------------
void	Client::set_phase (Phase state) {this->phase = state;}
Phase	Client::get_current_phase() {return this->phase;}
Request& Client::get_request_instance () {return this->request;}
Response& Client::get_response_instance () {return this->response;}
int		Client::get_status_code() {return get_request_instance().get_status_code();}
bool	Client::get_cgi() {return this->get_request_instance().get_cgi();}
bool	Client::get_chunked() {return this->get_request_instance().get_chunked();}
int		Client::check_path_property() {};
