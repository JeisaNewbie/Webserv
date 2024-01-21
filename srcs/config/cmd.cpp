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
		throw Exception(CONF_DUP_DIRCTV);

	int	n = stoi(tokens[1]);
	if (n <= 0 || MAX_FD < n)
		throw Exception(CONF_INVALID_DIRCTV_VALUE);

	cycle.setWorkerConnections(n);
}

// K, M, G 단위 사용 가능
void mainClientMaxBodySize(Cycle& cycle, std::string tokens[]) {
	if (cycle.getClientMaxBodySize() != VALID)
		throw Exception(CONF_DUP_DIRCTV);
		
	long long	n = stoll(tokens[1]);
	char		c = tokens[1].back();

	if (c == 'K')
		n *= KILO_BYTE;
	else if (c == 'M')
		n *= MEGA_BYTE;
	else if (c == 'G')
		n *= GIGA_BYTE;

	if (n <= 0 || MAX_BODY < n)
		throw Exception(CONF_INVALID_DIRCTV_VALUE);

	cycle.setClientMaxBodySize(n);
}

void mainUriLimitLength(Cycle& cycle, std::string tokens[]) {
	if (cycle.getUriLimitLength() != VALID)
		throw Exception(CONF_DUP_DIRCTV);

	int	n = stoi(tokens[1]);
	if (n <= 0) //제한 몇으로?
		throw Exception(CONF_INVALID_DIRCTV_VALUE);

	cycle.setUriLimitLength(n);
}

void mainRoot(Cycle& cycle, std::string tokens[]) {
	if (cycle.getMainRoot().length() != VALID)
		throw Exception(CONF_DUP_DIRCTV);

	cycle.setMainRoot(tokens[1]);
}

// registered port 범위만 허용
void serverListen(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();

	if (server.getPort() != VALID)
		throw Exception(CONF_DUP_DIRCTV);

	int n = stoi(tokens[1]);
	if (n < 1024 || 65535 < n)
		throw Exception(CONF_INVALID_DIRCTV_VALUE);

	server.setPort(n);
}

void serverName(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();

	if (server.getDomain().length() != VALID)
		throw Exception(CONF_DUP_DIRCTV);

	if (tokens[1].find('/') != std::string::npos)
		throw Exception(CONF_INVALID_DIRCTV_VALUE);

	server.setDomain(tokens[1]);
}

void locationRoot(Cycle& cycle, std::string tokens[]) {
	Location&	location = cycle.getServerList().back().getLocationList().back();
	int			location_type = location.getLocationType();

	if (location.getSubRoot().length() != VALID)
		throw Exception(CONF_DUP_DIRCTV);
	if (location_type == LOC_DEFAULT \
		&& tokens[1].back() == '/')
		throw Exception(CONF_INVALID_DIRCTV_VALUE);
	if ((location_type == LOC_ERROR \
		|| location_type == LOC_CGI) \
		&& tokens[1].back() != '/')
		throw Exception(CONF_INVALID_DIRCTV_VALUE);

	location.setSubRoot(tokens[1]);
}

void locationAllowedMethod(Cycle& cycle, std::string tokens[]) {
	Location&	location = cycle.getServerList().back().getLocationList().back();

	if (location.getAllowedMethod() != VALID)
		throw Exception(CONF_DUP_DIRCTV);

	int	res = 0;
	for (int i = 1; tokens[i].length(); i++) {
		if (tokens[i] == "GET")
			res |= METHOD_GET;
		else if (tokens[i] == "POST")
			res |= METHOD_POST;
		else if (tokens[i] == "DELETE")
			res |= METHOD_DELETE;
		else
			throw Exception(CONF_INVALID_DIRCTV_VALUE);
	}
	
	location.setAllowedMethod(res);
}