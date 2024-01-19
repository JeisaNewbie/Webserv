#pragma once

#include <string>
#include <cstring>
#include <map>
#include <list>
#include <iostream>
#include <sys/stat.h>
#include <sys/file.h>
#include "../request/Request.hpp"
#include "../response/Response.hpp"
// #include "../core/core.hpp"
// #include "../utils/Status.hpp"
#include "../core/cycle.hpp"
// #include "../method/Method.hpp"
#include "../method/Get.hpp"
#include "../method/Post.hpp"
#include "../method/Delete.hpp"
#include "../utils/Utils.hpp"
#include "../cgi/Cgi.hpp"

// #define	_FILE	1000
// #define	_DIR		1001

enum Phase {
	PARSING,
	METHOD,
	RESPONSE,
	DONE
};

class Client {
	private:
		Request		request;
		Response	response;
		Cgi			cgi;
		uintptr_t	client_soket;
	public:
		Phase phase;
		Client();
		Client(const Client& ref);
		~Client();

		// Client& operator=(const Client& ref);

		void		do_parse(std::string &request_msg, Cycle &cycle);
		void		do_method();
		void		do_method_with_cgi(Request &request);
		void		do_method_without_cgi(Request &request);
		void		parse_cgi_response(Cgi &cgi);
		void		assemble_response();

		Request		&get_request_instance ();
		Response	&get_response_instance ();
		Cgi			&get_cgi_instance();
		Phase		get_current_phase();
		int			get_status_code();
		bool		get_cgi();
		bool		get_chunked();
		uintptr_t	get_client_soket();
		void		set_phase (Phase state);
		void		set_status_code(int status_code);
		void		set_cgi (bool flag);
		void		set_client_soket(uintptr_t client_soket);


};