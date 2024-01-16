#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include "../request/Request.hpp"
#include "../utils/Utils.hpp"

#define	READ			0
#define	WRITE			1
#define	CGI_BUFFER_SIZE	1024

class Cgi {
	private:
		std::map<std::string, std::string>	env;
		std::string	cgi_name;
		std::string	cgi_body;
		char	buf[CGI_BUFFER_SIZE];
		FILE	*f_in;
		FILE	*f_out;
		int		fd_file_in;
		int		fd_file_out;

	public:
		Cgi();
		Cgi(const Cgi& ref);
		~Cgi();

		Cgi& operator=(const Cgi& ref);

		std::string&	get_response_from_cgi();
		void			set_env(Request &request, uintptr_t client_soket);
		char			**get_char_arr_of_env();
		static void		execute_cgi(Request &request, Cgi &cgi);
		void			set_body (std::string &body);
		void			set_name (std::string &name);
};