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
	if (cycle.getWorkerConnections() != 0)
		throw Exception(CONF_DUP_DIRCTV);
	int n = stoi(tokens[1]);
	if (n <= 0) //최대 개수 제한은 몇개로?
		throw Exception(CONF_INVALID_DIRCTV_VALUE);
	cycle.setWorkerConnections(n);
}

// K, M, G 단위 사용 가능
void mainClientMaxBodySize(Cycle& cycle, std::string tokens[]) {
	if (cycle.getClientMaxBodySize() != 0)
		throw Exception(CONF_DUP_DIRCTV);
		
	int		n = stoi(tokens[1]);
	char	c = tokens[1].back();

	if (c == 'K')
		n *= KILO_BYTE;
	else if (c == 'M')
		n *= (KILO_BYTE * KILO_BYTE);
	else if (c == 'G')
		n *= (KILO_BYTE * KILO_BYTE * KILO_BYTE);

	if (n <= 0) //제한 몇으로?
		throw Exception(CONF_INVALID_DIRCTV_VALUE);
	cycle.setClientMaxBodySize(n);
}

void mainUriLimitLength(Cycle& cycle, std::string tokens[]) {
	if (cycle.getUriLimitLength() != 0)
		throw Exception(CONF_DUP_DIRCTV);
	int n = stoi(tokens[1]);
	if (n <= 0) //제한 몇으로?
		throw Exception(CONF_INVALID_DIRCTV_VALUE);
	cycle.setUriLimitLength(n);
}

void mainRoot(Cycle& cycle, std::string tokens[]) {
	if (cycle.getMainRoot().length() != 0)
		throw Exception(CONF_DUP_DIRCTV);
	cycle.setMainRoot(tokens[1]);
}

void serverListen(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();
	if (server.getPort() != 0)
		throw Exception(CONF_DUP_DIRCTV);
	int n = stoi(tokens[1]);
	if (n < 0 || 65535 < n) // 포트 범위?
		throw Exception(CONF_INVALID_DIRCTV_VALUE);
	server.setPort(n);
}

void serverName(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();
	if (server.getDomain().length() != 0)
		throw Exception(CONF_DUP_DIRCTV);
	if (tokens[1].find('/') != std::string::npos)
		throw Exception(CONF_INVALID_DIRCTV_VALUE);
	server.setDomain(tokens[1]);
}

void locationRoot(Cycle& cycle, std::string tokens[]) {
	Location&	location = cycle.getServerList().back().getLocationList().back();
	int			location_type = location.getLocationType();
	if (location.getSubRoot().length() != 0)
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
