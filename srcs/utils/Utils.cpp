#include "Utils.hpp"

int	check_path_property(std::string &path)
{
	struct stat file_status;

	if (stat (path.c_str(), &file_status) != 0)
		return -1;

	if (S_ISREG(file_status.st_mode))
		return FILE;

	if (S_ISDIR(file_status.st_mode))
		return DIR;

	return -1;
}