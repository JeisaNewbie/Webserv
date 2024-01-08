#pragma once

// #include "Method.hpp"
#include "../core/core.hpp"

class Delete :public Method {
	private:
		/* data */
	public:
		Delete();
		Delete(const Delete& ref);
		~Delete();

		Delete& operator=(const Delete& ref);
};