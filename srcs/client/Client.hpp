#pragma once

#include <string>
#include <cstring>
#include <map>
#include <list>
#include <iostream>
#include "../utils/Status.hpp"
#include "../request/Request.hpp"
#include "../response/Response.hpp"

#define	FILE	1000
#define	DIR		1001

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
	public:
		Phase phase;
		Client();
		Client(const Client& ref);
		~Client();

		Client& operator=(const Client& ref);

		Request		&get_request_instance ();
		Response	&get_response_instance ();
		void		set_phase (Phase state);
		Phase		get_current_phase();
		int			get_status_code();
		bool		get_cgi();
		void		do_parse(std::string &request_msg);
		void		do_method();
		void		do_method_with_cgi();
		void		do_method_without_cgi();
		void		assemble_response();
		int			check_path_property();

};