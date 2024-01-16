#include "Get.hpp"

void	Get::make_body(Response &response, std::string &path)
{
	std::ifstream		file;
	std::stringstream	buf;

	file.open (path.c_str(), std::ifstream::in);
	if (file.is_open() == false)
		std::cout <<"file_open_failed\n";
	buf << file.rdbuf();
	response.set_body (buf.str());
	// file.close();
	// std::cout <<response.get_body();
}

void Get::create_response(Response &response)
{
	response.set_header_field("Content-Length", to_string(response.get_body().size()));
}
