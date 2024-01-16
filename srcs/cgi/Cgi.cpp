#include "Cgi.hpp"

Cgi::Cgi() {}

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

void		Cgi::set_env(Request &request) {}

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

std::string	Cgi::execute_cgi(std::string &cgi_name, std::string &body)
{
	char	**env;
	pid_t	pid;

	try
	{
		env = get_char_arr_of_env();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return "500";
	}

	this->f_in = tmpfile();
	this->f_out = tmpfile();

	this->fd_file_in = fileno (f_in);
	this->fd_file_out = fileno (f_out);

	write (fd_file_in, body.c_str(), body.size());
	fflush (f_in);
	fseek (f_in, 0, SEEK_SET);

	pid = fork();

	if (pid == -1)
	{
		for (size_t i = 0; env[i]; i++)
			delete []env[i];
		delete []env;
		return "500";
	}

	if (pid == 0)
	{
		dup2 (fd_file_in, STDIN_FILENO);
		dup2 (fd_file_out, STDOUT_FILENO);
		execve (cgi_name.c_str(), NULL, env);
		//write (CLIENT_SOKET, "500", 4); //client_soket에 write
		exit (0);
	}

	for (size_t i = 0; env[i] != NULL; i++)
		delete []env[i];
	delete []env;

	return "";
}

std::string	&Cgi::get_response_from_cgi()
{
	int	len = 1;

	waitpid (-1, NULL, 0);
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