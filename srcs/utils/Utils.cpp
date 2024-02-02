#include "Utils.hpp"

int	check_path_property(std::string path)
{
	struct stat file_status;

	if (stat (path.c_str(), &file_status) != 0)
		return -1;

	if (S_ISREG(file_status.st_mode))
		return _FILE;

	if (S_ISDIR(file_status.st_mode))
		return _DIR;

	return -1;
}

void set_error_page(std::string path, std::string &content)
{
	std::ifstream error (path);
	if (error.is_open() == false)
	{
		error.open("/Users/eunwolee/private/Webserv/serve/error/400.html");
		if (error.is_open() == false)
			return;
	}
	std::stringstream ss;
	ss << error.rdbuf();
	content = ss.str();
	ss.str("");
	error.close();
	return;
}