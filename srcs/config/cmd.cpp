#include "../core/core.hpp"

Cmd::Cmd(void) {}

Cmd::Cmd(std::string _name, int _arg_cnt, handler_t _handler) \
	: name(_name), arg_cnt(_arg_cnt), handler(_handler) {}

Cmd::Cmd(const Cmd& src) { *this = src; }

Cmd::~Cmd(void) {}

Cmd& Cmd::operator =(const Cmd& src) {
	if (this != &src) {
		name = src.name;
		arg_cnt = src.arg_cnt;
		handler = src.handler;
	}
	return (*this);
}

const std::string&	Cmd::getName(void) const { return name; }
int					Cmd::getArgCnt(void) const { return arg_cnt; }
const handler_t&	Cmd::getHandler(void) const { return handler; }

void mainWorkerConnections(Cycle& cycle, std::string tokens[]) {
	if (cycle.getWorkerConnections() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	int	n = std::strtol(tokens[1].c_str(), NULL, 10);
	if (n <= 0 || MAX_FD < n)
		throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);

	cycle.setWorkerConnections(n);
}

// K, M, G 단위 사용 가능
void mainClientMaxBodySize(Cycle& cycle, std::string tokens[]) {
	if (cycle.getClientMaxBodySize() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	long long	n = std::strtol (tokens[1].c_str(), NULL, 10);
	char		c = tokens[1].back();

	if (c == 'K')
		n *= KILO_BYTE;
	else if (c == 'M')
		n *= MEGA_BYTE;
	else if (c == 'G')
		n *= GIGA_BYTE;

	if (n <= 0 || MAX_BODY < n)
		throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);

	cycle.setClientMaxBodySize(n);
}

void mainRoot(Cycle& cycle, std::string tokens[]) {
	if (cycle.getMainRoot().length() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	if (tokens[1].back() == '/')
		throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);

	cycle.setMainRoot(tokens[1]);
}

void mainDefaultErrorRoot(Cycle& cycle, std::string tokens[]) {
	if (cycle.getDefaultErrorRoot().length() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	if (tokens[1][0] == '/' || tokens[1].back() == '/')
		throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);

	cycle.setDefaultErrorRoot(tokens[1]);
}

// registered port 범위만 허용
void serverListen(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();

	if (server.getPort() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	int n = std::strtol(tokens[1].c_str(), NULL, 10);
	if ((n != 80 && n < 1024) || 65535 < n)
		throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);

	server.setPort(n);
}

void serverName(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();

	if (server.getDomain().length() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	if (tokens[1].find('/') != std::string::npos)
		throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);

	server.setDomain(tokens[1]);
}

void locationRoot(Cycle& cycle, std::string tokens[]) {
	Location&	location = cycle.getServerList().back().getLocationList().back();

	if (location.getSubRoot().length() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	if (tokens[1][0] != '/' || tokens[1].back() == '/')
		throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);

	location.setSubRoot(tokens[1]);
}

void locationAllowedMethod(Cycle& cycle, std::string tokens[]) {
	Location&	location = cycle.getServerList().back().getLocationList().back();

	if (location.getAllowedMethod() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	int	res = 0;
	for (int i = 1; tokens[i].length(); i++) {
		if (tokens[i] == "GET")
			res |= METHOD_GET;
		else if (tokens[i] == "POST")
			res |= METHOD_POST;
		else if (tokens[i] == "DELETE")
			res |= METHOD_DELETE;
		else
			throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);
	}

	location.setAllowedMethod(res);
}

void locationAutoIndex(Cycle& cycle, std::string tokens[]) {
	Location&	location = cycle.getServerList().back().getLocationList().back();

	if (location.getAutoIndex() != -1)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	if (tokens[1] == "on")
		location.setAutoIndex(TRUE);
	else if (tokens[1] == "off")
		location.setAutoIndex(FALSE);
	else
		throw Exception(CONF_INVALID_DIRCTV_VALUE, tokens[1]);
}

void locationIndex(Cycle& cycle, std::string tokens[]) {
	Location&					location = cycle.getServerList().back().getLocationList().back();
	std::vector<std::string>&	index = location.getIndex();

	if (location.getIndex().size() != VALID)
		throw Exception(CONF_DUP_DIRCTV, tokens[0]);

	for (int i = 1; tokens[i].length(); i++)
		index.push_back(tokens[i]);
}
