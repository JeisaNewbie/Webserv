#pragma once

// #include "Method.hpp"
#include "../core/core.hpp"

class Post : public Method {
	private:
		/* data */
	public:
		Post();
		Post(const Post& ref);
		~Post();

		Post& operator=(const Post& ref);
};