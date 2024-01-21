#include "../core/core.hpp"

Location::Location(int _location_type, std::string _location_path)	\
	: location_type(_location_type), location_path(_location_path),	\
		allowed_method(0) {}

Location::Location(const Location& src) { *this = src; }

Location::~Location(void) {}

Location& Location::operator =(const Location& src) {
	if (this != &src) {
		location_type = src.location_type;
		location_path = src.location_path;
		sub_root = src.sub_root;
	}
	return (*this);
}

void				Location::setSubRoot(std::string _sub_root) { sub_root = _sub_root; }
void				Location::setAllowedMethod(int _allowed_method) { allowed_method = _allowed_method; }

int					Location::getLocationType(void) const { return location_type; }
const std::string&	Location::getLocationPath(void) const { return location_path;}
const std::string&	Location::getSubRoot(void) const { return sub_root; }
int					Location::getAllowedMethod(void) const { return allowed_method; }
