#include "Client.hpp"

Client::Client(){}
Client::Client(const Client& ref) {}
Client::~Client() {}

// Client&	Client::operator=(const Client& ref) {}

void	Client::init_client(uintptr_t **cgi_fd_arr, uintptr_t client_soket)
{
	this->cgi_fd_arr = cgi_fd_arr;
	set_client_soket(client_soket);
	set_cgi_fd_arr(client_soket);
}

void	Client::do_parse(std::string &request_msg, Cycle &cycle)
{
	get_request_instance().process_request_parsing(request_msg, cycle);
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

	this->get_cgi_instance().set_env(request, get_client_soket());
	std::cout<<"END_SET_PROPERTY_FOR_CGI\n";
	// Cgi::execute_cgi(request, get_cgi_instance());
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
			get_response_instance().set_body(body.substr (pos + 2,  body.find ("\r\n", pos + 2) + 2));
			break ;
		}

		get_response_instance().set_header_field(body.substr (pos, delimeter - pos));
		// std::cout<<"SECOND"<<std::endl;
		pos = delimeter + 2;
	}
	// std::cout<<"AFTER_BODY_FIND"<<std::endl;
	set_status_code(std::atoi(get_response_instance().get_header_field("Status_code").c_str()));
}

void	Client::assemble_response()
{
	response.set_header_line (get_status_code());
	// response.set_header_field ("key", "value");
	// status_code에 따라 body 수정 필요
	if (get_status_code() > BAD_REQUEST)
	{
		std::ifstream		error (request.get_cycle_instance().getMainRoot() + "/serve/error/" + to_string(get_status_code()) + ".html");
		std::stringstream	ss;

		ss << error.rdbuf();
		response.set_body(ss.str());
		std::cout <<"RESPONSE_ERROR: "<<response.get_body()<<std::endl;
		ss.str("");
		error.close();
	}
	if (get_request_instance().get_redirect() == true)
		get_response_instance().set_header_field("Location", get_request_instance().get_redirect_path());
	response.assemble_message ();
}

//------------------------getter && setter---------------------------
Phase		Client::get_current_phase() {return this->phase;}
Request& 	Client::get_request_instance () {return this->request;}
Response&	Client::get_response_instance () {return this->response;}
Cgi&		Client::get_cgi_instance() {return this->cgi;}
int			Client::get_status_code() {return get_request_instance().get_status_code();}
bool		Client::get_cgi() {return this->get_request_instance().get_cgi();}
bool		Client::get_chunked() {return this->get_request_instance().get_chunked();}
void		Client::set_phase (Phase state) {this->phase = state;}
void		Client::set_status_code(int status_code) {get_request_instance().set_status_code(status_code);}
void		Client::set_cgi (bool flag) {get_request_instance().set_cgi(flag);}
uintptr_t	Client::get_client_soket() {return this->client_soket;}
void		Client::set_client_soket(uintptr_t client_soket) {this->client_soket = client_soket;}
void		Client::set_cgi_fd_arr(uintptr_t client_soket) {*(this->cgi_fd_arr[get_cgi_instance().get_fd()]) = client_soket;}
void		Client::set_port(uint32_t port) {get_request_instance().set_port(port);}