#include "Delete.hpp"

bool	Delete::remove_file(std::string &path)
{
	if (std::remove (path.c_str()) != 0)
		return false;
	return true;
}

void	Delete::create_response(Response &response)
{
	response.set_header_field ("Content-Length", "0");
}
