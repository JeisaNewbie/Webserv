#include "Client.hpp"

Client::Client()
{
	body_length = 0;
	read_fail = false;
}
Client::Client(const Client& ref) {}
Client::~Client() {}

void	Client::reset_data()
{
	std::cout<<"RESET_DATA_REQUEST\n";
	get_request_instance().reset_data();
	get_response_instance().reset_data();
	// this->cgi_fd_arr[get_cgi_instance().get_fd()] = 0;
}

void	Client::init_client(uintptr_t client_soket)
{
	set_client_soket(client_soket);
	get_timeout_instance().setSavedTime();
}

void	Client::do_parse(Cycle &cycle)
{
	get_request_instance().process_request_parsing(cycle);
}

void	Client::set_property_for_cgi(Request &request)
{
	std::cout<<"SET_PROPERTY_FOR_CGI\n";

	std::string &path = request.get_path();
	int			path_property = check_path_property (path);

	// std::cout<<"Path: "<<path<<std::endl;
	if (path_property == -1)
	{
		set_cgi(false);
		throw NOT_FOUND;
	}

	this->get_cgi_instance().set_body(request.get_message_body());
	std::cout << "SET_CGI_BODY: " << this->get_cgi_instance().get_body() << std::endl;
	this->get_cgi_instance().set_env(request, get_client_soket());
	std::cout << "GET_CLI_SOCKET: " << get_client_soket() << std::endl;
	this->get_cgi_instance().set_fd();
	std::cout<<"END_SET_PROPERTY_FOR_CGI\n";
}

void	Client::do_method_without_cgi(Request &request)
{
	// std::cout<<"METHOD_WITHOUT_CGI_START\n";
	// std::cout<<request.get_path()<<std::endl;

	//set_autoindex(); ->if no -> find_index(); -> if no -> 404.html;
	std::string path = request.get_path();
	int			path_property = check_path_property (path);

	// if (path_property == -1)
	// 	throw NOT_FOUND;

	std::string	&method = request.get_method();

	if (method == "GET")
	{
		if (path_property == _FILE)
		{
			Get::make_body(response, path);
			Get::create_response(response);
			throw OK;
		}
		else
		{
			if (request.get_autoindex() == false)
				throw NOT_FOUND;
			Get::set_autoindex(request, response);
			Get::create_response(response);
			throw OK;
		}
	}

	if (method == "POST")
		throw NO_CONTENT;

	if (method == "DELETE")
	{
		Delete::remove_file(path);
		Delete::create_response(response);
	}
}

void	Client::parse_cgi_response(Cgi &cgi)
{
	std::string body = cgi.get_response_from_cgi();
	size_t 		delimeter = 0;
	size_t		pos = 0;

	delimeter = body.find ("\r\n", pos);
	// std::cout<<"BODY_FIND_NO_ERR"<<std::endl;
	while (delimeter != std::string::npos)
	{
		delimeter = body.find ("\r\n", pos);
		if (delimeter == std::string::npos)
			break;
		// std::cout<<"FIRST"<<std::endl;

		if (pos == delimeter)
		{
			// std::cout<<"THIRD"<<std::endl;
			get_response_instance().set_body(body.substr (pos + 2, body.find ("\r\n", pos + 2)));
			std::cout << "parse_cgi_response_body: " << get_response_instance().get_body() << std::endl;
			break ;
		}

		get_response_instance().set_header_field(body.substr (pos, delimeter - pos));
		// std::cout<<"SECOND"<<std::endl;
		pos = delimeter + 2;
	}
	// std::cout<<"AFTER_BODY_FIND"<<std::endl;
	set_status_code(std::atoi(get_response_instance().get_header_field("Status_code").c_str()));
	std::cout << "parse_cgi_response_get_status_code: " << get_status_code() << std::endl;
	set_cgi(false);
}

void	Client::assemble_response()
{
	response.set_header_line (get_status_code());

	if (get_status_code() >= BAD_REQUEST)
	{
		std::string path = request.get_cycle_instance().getMainRoot() + "/serve/error/" + to_string(get_status_code()) + ".html";
		int file_status = check_path_property(path);

		if (file_status == _FILE)
			set_error_page (path, response.get_body());
		else
		{
			path = "/Users/ahkiler/Webserv/" + request.get_cycle_instance().getDefaultErrorRoot();
			set_error_page (path, response.get_body());
		}

	}
	std::cout <<"a\n";
	if (get_read_fail() == false)
		response.set_header_field ("Connection", get_request_instance().get_header_field("connection").substr (0, get_request_instance().get_header_field("connection").size () - 2)); // request parsing 없이 response 진행할수도 있음
	std::cout <<"b\n";
	if (response.get_header_field("Connection") == "keep-alive")
		response.set_header_field ("Keep-Alive", "timeout=50, max=1000");
	std::cout <<"c\n";
	response.set_header_field ("Access-Control-Allow-Origin", "*");
	response.set_header_field ("Content-Length", to_string(get_response_instance().get_body().size()));
	std::cout <<"dd\n";
	if (get_read_fail() == false && get_request_instance().get_redirect() == true)
		get_response_instance().set_header_field("Location", get_request_instance().get_redirect_path());
	std::cout <<"e\n";
	response.assemble_message ();
}

//------------------------getter && setter---------------------------
Phase		Client::get_current_phase() {return this->phase;}
Request& 	Client::get_request_instance () {return this->request;}
Response&	Client::get_response_instance () {return this->response;}
Cgi&		Client::get_cgi_instance() {return this->cgi;}
Timeout&	Client::get_timeout_instance() {return this->timeout;}
int			Client::get_status_code() {return get_request_instance().get_status_code();}
bool		Client::get_cgi() {return this->get_request_instance().get_cgi();}
bool		Client::get_cgi_fork_status () {return get_cgi_instance().get_cgi_fork_status();}
bool		Client::get_expect() {return this->get_request_instance().get_expect();}
bool		Client::get_chunked() {return this->get_request_instance().get_chunked();}
bool		Client::get_read_fail () {return this->read_fail;}
void		Client::set_phase (Phase state) {this->phase = state;}
void		Client::set_status_code(int status_code) {get_request_instance().set_status_code(status_code);}
void		Client::set_cgi (bool flag) {get_request_instance().set_cgi(flag);}
void		Client::set_read_fail (bool flag) {this->read_fail = flag;}
void		Client::set_cgi_fork_status (bool status) {get_cgi_instance().set_cgi_fork_status(status);}
uintptr_t	Client::get_client_soket() {return this->client_soket;}
uintptr_t*	Client::get_client_soket_ptr() {return &(this->client_soket);}
void		Client::set_client_soket(uintptr_t client_soket) {this->client_soket = client_soket;}
void		Client::set_port(uint32_t port) {get_request_instance().set_port(port);}