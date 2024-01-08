#include "server.hpp"

Server::Server(void) : port(0) {}

Server::Server(const Server& src) {
	*this = src;
}

Server::~Server(void) {}

Server& Server::operator =(const Server& src) {
	if (this != &src) {
		port = src.port;
		domain = src.domain;
		location_list = src.location_list;
	}
	return (*this);
}

void Server::setPort(uint32_t _port){
	port = _port;
}

void Server::setDomain(std::string _domain){
	domain = _domain;
}

uint32_t Server::getPort(void) const{
	return port;
}

const std::string& Server::getDomain(void) const{
	return domain;
}

std::list<Location>& Server::getLocationList(void){
	return location_list;
}

const std::list<Location>& Server::getLocationListConst(void) const{
	return location_list;
}
