#include "location.hpp"

Location::Location(std::string _location_path) : location_path(_location_path) {}

Location::Location(const Location& src) {
	*this = src;
}

Location::~Location(void) {}

Location& Location::operator =(const Location& src) {
	if (this != &src) {
		location_path = src.location_path;
		sub_root = src.sub_root;
	}
	return (*this);
}

void Location::setStaticPath(std::string _sub_root) {
	sub_root = _sub_root;
}

const std::string& Location::getBlockPath(void) const {
	return location_path;
}

const std::string& Location::getStaticPath(void) const {
	return sub_root;
}
