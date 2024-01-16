#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/file.h>

#define	_FILE	1000
#define	_DIR	1001

int	check_path_property(std::string &path);

template <class T>
std::string	to_string(T num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}