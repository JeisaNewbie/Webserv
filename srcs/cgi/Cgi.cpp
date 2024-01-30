#include "Cgi.hpp"

Cgi::Cgi()
{
	this->fork_status = false;
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
	int i = request.get_path().find(".cpp");
	std::string tmp = request.get_path().substr(0, i);
	set_name (tmp);

	env["REQUEST_METHOD"] = request.get_method();
	env["QUERY_STRING_POST"] = request.get_query_value("postdata");
	env["QUERY_STRING_GET"] = request.get_query_value("getdata");
	env["CONTENT_LENGTH"] = request.get_header_field("content_length");
	env["REDIRECT_PATH"] = request.get_cycle_instance().getMainRoot() + request.get_header_field("redirect_path");

	try
	{
		env_cgi = get_char_arr_of_env();
	}
	catch(const std::exception& e)
	{
		request.set_cgi(false);
		std::cout<<"CGI_ENV_FAIL\n";
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
	this->f_in = tmpfile();
	this->f_out = tmpfile();
	this->fd_file_in = fileno (this->f_in);
	this->fd_file_out = fileno (this->f_out);
	fputs (this->cgi_body.c_str(), this->f_in);
	fflush (this->f_in);
	fseek (this->f_in, 0, SEEK_SET);
	this->cgi_body = "";
}

void	Cgi::execute_cgi(Request &request, Cgi &cgi) // cgi 무한루프 발생시 강제 종료 설정(timeout등)
{
	std::cout <<"BEFORE_FORK\n";
	std::cout <<"CGI_PATH: "<< cgi.cgi_name<<std::endl;
	std::cout <<"REDIRECT_PATH: "<<cgi.env["REDIRECT_PATH"]<<std::endl;
	cgi.pid = fork();

	if (cgi.pid == -1)
	{
		request.set_cgi(false);
		request.set_status_code(INTERNAL_SERVER_ERROR);
		return ;
	}

	if (cgi.pid == 0)
	{
		dup2 (cgi.fd_file_in, STDIN_FILENO);
		dup2 (cgi.fd_file_out, STDOUT_FILENO);
		execve (cgi.cgi_name.c_str(), NULL, cgi.env_cgi);
		write (STDOUT_FILENO, "Status: 500\r\n\r\n", 16);
		exit (1);
	}

	std::cout<<"AFTER_FORK\n";
}

std::string	&Cgi::get_response_from_cgi()
{
	int	len = 1;
	int	status = 0;

	std::cout << "WAIT_PID_START\n";
	waitpid (this->pid, &status, 0);
	lseek (this->fd_file_out, 0, SEEK_SET);
	std::cout << "WAIT_PID_DONE\n";
	std::memset(buf, 0, sizeof(buf));
	while (len)
	{
		len = read (this->fd_file_out, buf, CGI_BUFFER_SIZE - 1);
		if (len == 0)
			break;
		cgi_body += buf;
		std::memset(buf, 0, sizeof(buf));
		// std::cout<<len<<std::endl;
		// std::cout<<cgi_body<<std::endl;
	}

	// std::cout<<"BEFORE_GET_CHILD_PROCESS\n";
	// std::cout<<"AFTER_GET_CHILD_PROCESS\n";
	std::cout << "STATUS_CODE: " << WEXITSTATUS(status) << "\n";
	if (WEXITSTATUS(status))
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

	std::cout << "CGI_BODY: " << cgi_body << "\n";
	return cgi_body;
}

void	Cgi::set_body (std::string &body) {this->cgi_body = body;}
void	Cgi::set_name (std::string &name) {this->cgi_name = name;}
int		Cgi::get_fd() {return this->fd_file_out;}
pid_t	Cgi::get_pid() {return this->pid;}
void	Cgi::set_cgi_fork_status(bool status) {this->fork_status = status;}
bool	Cgi::get_cgi_fork_status() {return this->fork_status;}