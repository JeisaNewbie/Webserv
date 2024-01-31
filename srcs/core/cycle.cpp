#include "core.hpp"

Cycle::Cycle(const char** _envp) : envp(_envp), worker_connections(0),	\
		client_max_body_size(0), uri_limit_length(0) {}

Cycle::Cycle(const Cycle& src) { *this = src; }

Cycle::~Cycle(void) {}

Cycle& Cycle::operator =(const Cycle& src) {
	if (this != &src) {
		worker_connections = src.worker_connections;
		client_max_body_size = src.client_max_body_size;
		uri_limit_length = src.uri_limit_length;
		main_root = src.main_root;
		default_error_root = src.default_error_root;
		
		server_list = src.server_list;
	}
	return (*this);
}

void					Cycle::setWorkerConnections(u_int32_t n) { worker_connections = n; }
void					Cycle::setClientMaxBodySize(size_t n) { client_max_body_size = n; }
void					Cycle::setUriLimitLength(size_t n) { uri_limit_length = n; }
void					Cycle::setMainRoot(std::string _path) { main_root = _path; }
void					Cycle::setDefaultErrorRoot(std::string _path) { default_error_root = _path; }

const char**			Cycle::getEnvp(void) const { return envp; }
int						Cycle::getWorkerConnections(void) const { return worker_connections; }
int						Cycle::getClientMaxBodySize(void) const { return client_max_body_size; }
int						Cycle::getUriLimitLength(void) const { return uri_limit_length; }
const std::string&		Cycle::getMainRoot(void) const { return main_root; }
const std::string&		Cycle::getDefaultErrorRoot(void) const { return default_error_root; }
std::list<Server>&		Cycle::getServerList(void) { return server_list; }
