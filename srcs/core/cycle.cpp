#include "core.hpp"

Cycle::Cycle(void) {}

Cycle::Cycle(const char** _envp) \
	: envp(_envp), use_cgi(FALSE), worker_processes(0), \
		worker_connections(0), client_max_body_size(0), \
		uri_limit_length(0)  {}

Cycle::Cycle(const Cycle& src) {
	*this = src;
}

Cycle::~Cycle(void) {}

Cycle& Cycle::operator =(const Cycle& src) {
	if (this != &src) {
		worker_processes = src.worker_processes;
		worker_connections = src.worker_connections;
		client_max_body_size = src.client_max_body_size;
		uri_limit_length = src.uri_limit_length;
		main_root = src.main_root;
		server_list = src.server_list;
		worker_list = src.worker_list;
	}
	return (*this);
}

const char** Cycle::getEnvp(void) const {
	return envp;
}

void Cycle::setUseCgi(bool state) {
	use_cgi = state;
}

bool Cycle::getUseCgi(void) const {
	return use_cgi;
}

void Cycle::setWorkerProcesses(u_int32_t n) {
	worker_processes = n;
}

int Cycle::getWorkerProcesses(void) const {
	return worker_processes;
}

void Cycle::setWorkerConnections(u_int32_t n) {
	worker_connections = n;
}

int Cycle::getWorkerConnections(void) const {
	return worker_connections;
}

void Cycle::setClientMaxBodySize(u_int32_t n) {
	client_max_body_size = n;
}

int Cycle::getClientMaxBodySize(void) const {
	return client_max_body_size;
}

void Cycle::setUriLimitLength(u_int32_t n) {
	uri_limit_length = n;
}

int Cycle::getUriLimitLength(void) const {
	return uri_limit_length;
}

void Cycle::setMainRoot(std::string _path) {
	main_root = _path;
}

const std::string& Cycle::getMainRoot(void) const {
	return main_root;
}

std::list<Server>& Cycle::getServerList(void){
	return server_list;
}

const std::list<Server>& Cycle::getServerListConst(void) const{
	return server_list;
}

void Cycle::setWorkerList(int idx, pid_t pid) {
	worker_list[idx] = pid;
}

worker_array Cycle::getWorkerList(void) {
	return worker_list;
}
