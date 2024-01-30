#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>
#include <fstream>
#include <sstream>

#define	_FILE	1000
#define	_DIR	1001

int	check_path_property(std::string path);
void set_error_page(std::string path, std::string &content);

template <class T>
std::string	to_string(T num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}