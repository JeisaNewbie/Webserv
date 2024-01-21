#include "Cgi.hpp"

Cgi::Cgi()
{
	this->f_in = tmpfile();
	this->f_out = tmpfile();
	this->fd_file_in = fileno (this->f_in);
	this->fd_file_out = fileno (this->f_out);
}

Cgi::Cgi(const Cgi& ref)
{
	if (this != &ref)
		this->env = ref.env;
	return ;
}

Cgi::~Cgi()
{
	if (env_cgi != NULL)
	{
		for (size_t i = 0; env_cgi[i]; i++)
			delete []env_cgi[i];
		delete []env_cgi;
	}
}

Cgi& Cgi::operator=(const Cgi& ref)
{
	if (this != &ref)
		this->env = ref.env;
	return *this;
}

void		Cgi::set_env(Request &request, uintptr_t client_soket)
{
	set_body (request.get_message_body());

	int i = request.get_path().find(".cpp");
	std::string tmp = request.get_path().substr(0, i);
	set_name (tmp);

	env["REQUEST_METHOD"] = request.get_method();
	env["CLIENT_SOKET"] = to_string (client_soket);
	env["QUERY_STRING"] = request.get_query_value("postdata");
	env["CONTENT_LENGTH"] = request.get_header_field("content_length");
	env["REDIRECT_PATH"] = request.get_cycle_instance().getMainRoot() + request.get_header_field("redirect_path");

	try
	{
		env_cgi = get_char_arr_of_env();
	}
	catch(const std::exception& e)
	{
		request.set_cgi(false);
		throw INTERNAL_SERVER_ERROR;
	}

}

char**		Cgi::get_char_arr_of_env()
{
	std::map<std::string, std::string>::iterator	it = this->env.begin();
	std::map<std::string, std::string>::iterator	ite = this->env.end();
	char	**arr = new char *[this->env.size() + 1];

	arr[this->env.size()] = NULL;

	for (int i = 0; it != ite; it++, i++)
	{
		std::string	env_elem = it->first + "=" + it->second;
		arr[i] = new char [env_elem.size() + 1];
		strcpy (arr[i], env_elem.c_str());
	}

	return arr;
}

void	Cgi::set_fd()
{
	fputs (this->cgi_body.c_str(), this->f_in);
	fflush (this->f_in);
	fseek (this->f_in, 0, SEEK_SET);
}

void	Cgi::execute_cgi(Request &request, Cgi &cgi)
{

	cgi.set_fd();
	std::cout <<"before_fork\n";
	cgi.pid = fork();

	if (cgi.pid == -1)
	{
		request.set_cgi(false);
		throw INTERNAL_SERVER_ERROR;
	}

	if (cgi.pid == 0)
	{
		dup2 (cgi.fd_file_in, STDIN_FILENO);
		dup2 (cgi.fd_file_out, STDOUT_FILENO);
		execve (cgi.cgi_name.c_str(), NULL, cgi.env_cgi);
		write (STDOUT_FILENO, "Status: 500\r\n\r\n", 15); //client_soket에 write
		exit (1);
	}

	std::cout<<"after_fork\n";

	throw OK;
}

std::string	&Cgi::get_response_from_cgi()
{
	int	len = 1;
	int	status = 0;

	lseek (this->fd_file_out, 0, SEEK_SET);

	while (len)
	{
		len = read (this->fd_file_out, buf, CGI_BUFFER_SIZE - 1);
		cgi_body += buf;
	}

	std::cout<<"BEFORE_GET_CHILD_PROCESS\n";
	waitpid (this->pid, &status, WNOHANG);
	std::cout<<"AFTER_GET_CHILD_PROCESS\n";
	if (WIFEXITED(status) == 1)
	{
		fclose (this->f_in);
		fclose (this->f_out);
		close (this->fd_file_in);
		close (this->fd_file_out);
		cgi_body = "";
		return cgi_body;
	}

	fclose (this->f_in);
	fclose (this->f_out);
	close (this->fd_file_in);
	close (this->fd_file_out);

	return cgi_body;
}

void	Cgi::set_body (std::string &body) {this->cgi_body = body;}
void	Cgi::set_name (std::string &name) {this->cgi_name = name;}
int		Cgi::get_fd() {return this->fd_file_out;}