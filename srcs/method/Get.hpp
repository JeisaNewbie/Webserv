#pragma once

#include "Method.hpp"
// #include "../core/core.hpp"

class Get : public Method {
	private:
		/* data */
	public:
		static void create_response(Response &response);
		static void	make_body(Response &response, std::string &path);
};