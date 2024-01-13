#pragma once

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/file.h>

#define	_FILE	1000
#define	_DIR	1001

int	check_path_property(std::string &path);