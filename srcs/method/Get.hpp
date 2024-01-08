#pragma once

#include "Method.hpp"
// #include "../core/core.hpp"

class Get : public Method {
	private:
		/* data */
	public:
		Get();
		Get(const Get& ref);
		~Get();

		Get& operator=(const Get& ref);

		static void	make_body(std::string &path);
};