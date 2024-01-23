#include "../core/core.hpp"

Location::Location(std::string _location_path)	\
	: location_path(_location_path), allowed_method(0), autoindex(-1) {}

Location::Location(const Location& src) { *this = src; }

Location::~Location(void) {}

Location& Location::operator =(const Location& src) {
	if (this != &src) {
		location_path = src.location_path;
		sub_root = src.sub_root;
		allowed_method = src.allowed_method;
		autoindex = src.autoindex;
		index = src.index;
	}
	return (*this);
}

void						Location::setSubRoot(std::string _sub_root) { sub_root = _sub_root; }
void						Location::setAllowedMethod(int _allowed_method) { allowed_method = _allowed_method; }
void						Location::setAutoIndex(int _autoindex) { autoindex = _autoindex; }

const std::string&			Location::getLocationPath(void) const { return location_path;}
const std::string&			Location::getSubRoot(void) const { return sub_root; }
int							Location::getAllowedMethod(void) const { return allowed_method; }
int							Location::getAutoIndex(void) const { return autoindex; }
std::vector<std::string>&	Location::getIndex(void) { return index; }

