#include "Get.hpp"

void	Get::make_body(Response &response, std::string &path)
{
	std::ifstream		file;
	std::stringstream	buf;

	file.open (path.c_str(), std::ifstream::in);
	if (file.is_open() == false)
		;
	buf << file.rdbuf();
	response.set_body (buf.str());
	file.close();
}

void create_response(Response &response)
{
	response.set_header_field("Content-Length", std::stringstream (response.get_body().size()).str());
}
