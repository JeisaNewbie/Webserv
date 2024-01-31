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
// #include "../method/Method.hpp"
#include "../method/Get.hpp"
#include "../method/Delete.hpp"
#include "../utils/Utils.hpp"
#include "../cgi/Cgi.hpp"
# include "../event/timeout.hpp"

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
		Timeout		timeout;
		uintptr_t	client_soket;
		bool		read_fail;
	public:
		Phase phase;
		Client();
		Client(const Client& ref);
		~Client();

		// Client& operator=(const Client& ref);

		void		do_parse(Cycle &cycle);
		void		set_property_for_cgi(Request &request);
		void		do_method_without_cgi(Request &request);
		void		parse_cgi_response(Cgi &cgi);
		void		assemble_response();
		void		init_client(uintptr_t client_soket);
		void		reset_data();

		Request		&get_request_instance ();
		Response	&get_response_instance ();
		Cgi			&get_cgi_instance();
		Timeout		&get_timeout_instance();
		Phase		get_current_phase();
		int			get_status_code();
		bool		get_cgi();
		bool		get_cgi_fork_status ();
		bool		get_chunked();
		bool		get_expect();
		bool		get_read_fail();
		void		set_read_fail(bool flag);
		uintptr_t	get_client_soket();
		uintptr_t*	get_client_soket_ptr();
		void		set_phase (Phase state);
		void		set_status_code(int status_code);
		void		set_cgi (bool flag);
		void		set_client_soket(uintptr_t client_soket);
		void		set_cgi_fork_status (bool status);
		void		set_port(size_t port);

		ssize_t		body_length;
};