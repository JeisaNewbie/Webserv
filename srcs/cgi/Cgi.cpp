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

Cgi::~Cgi() {}

Cgi& Cgi::operator=(const Cgi& ref)
{
	if (this != &ref)
		this->env = ref.env;
	return *this;
}

void		Cgi::set_env(Request &request, uintptr_t client_soket)
{
	set_body (request.get_message_body());
	set_name (request.get_path());
	env["REQUEST_METHOD"] = request.get_method();
	env["CLIENT_SOKET"] = to_string (client_soket);
}

char**		Cgi::get_char_arr_of_env()
{
	std::map<std::string, std::string>::iterator	it = this->env.begin();
	std::map<std::string, std::string>::iterator	ite = this->env.end();
	char	**arr = new char *[this->env.size() + 1];

	arr[this->env.size()] = NULL;

	for (int i = 0; it != ite; it++)
	{
		std::string	env_elem = it->first + "=" + it->second;
		arr[i] = new char [env_elem.size() + 1];
		strcpy (arr[i], env_elem.c_str());
	}

	return arr;
}

void	Cgi::execute_cgi(Request &request, Cgi &cgi)
{
	char	**env;
	pid_t	pid;

	try
	{
		env = cgi.get_char_arr_of_env();
	}
	catch(const std::exception& e)
	{
		request.set_cgi(false);
		throw INTERNAL_SERVER_ERROR;
	}

	// write (cgi.fd_file_in, body.c_str(), body.size());
	fputs (cgi.cgi_body.c_str(), cgi.f_in);
	fflush (cgi.f_in);
	fseek (cgi.f_in, 0, SEEK_SET);

	pid = fork();

	if (pid == -1)
	{
		for (size_t i = 0; env[i]; i++)
			delete []env[i];
		delete []env;
		request.set_cgi(false);
		throw INTERNAL_SERVER_ERROR;
	}

	if (pid == 0)
	{
		dup2 (cgi.fd_file_in, STDIN_FILENO);
		dup2 (cgi.fd_file_out, STDOUT_FILENO);
		execve (cgi.cgi_name.c_str(), NULL, env);
		//write (CLIENT_SOKET, "CGI", 4); //client_soket에 write
		exit (1);
	}

	for (size_t i = 0; env[i] != NULL; i++)
		delete []env[i];
	delete []env;

	return "";
}

std::string	&Cgi::get_response_from_cgi()
{
	int	len = 1;
	int	status = 0;

	waitpid (-1, &status, 0);
	if (WIFEXITED(status) == 1)
	{
		fclose (this->f_in);
		fclose (this->f_out);
		close (this->fd_file_in);
		close (this->fd_file_out);
		cgi_body = "";
		return cgi_body;
	}

	lseek (this->fd_file_out, 0, SEEK_SET);

	while (len)
	{
		len = read (this->fd_file_out, buf, CGI_BUFFER_SIZE - 1);
		cgi_body += buf;
	}

	fclose (this->f_in);
	fclose (this->f_out);
	close (this->fd_file_in);
	close (this->fd_file_out);

	return cgi_body;
}

void	Cgi::set_body (std::string &body) {this->cgi_body = body;}
void	Cgi::set_name (std::string &name) {this->cgi_name = name;}