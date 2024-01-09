#pragma once

#include "Method.hpp"
// #include "../core/core.hpp"

class Delete :public Method {
	public:
		static bool	remove_file(std::string &path);
		static void create_response(Response &response);

};