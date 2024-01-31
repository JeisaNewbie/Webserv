#pragma once

#include <dirent.h>
#include "Method.hpp"

class Get : public Method {
	private:
		/* data */
	public:
		static void create_response(Response &response);
		static bool	make_body(Response &response, std::string &path);
		static void	set_autoindex(Request &request, Response &response);
};