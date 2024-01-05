# include "core.hpp"

void mainWorkerProcesses(Cycle& cycle, std::string tokens[]) {
	if (cycle.getWorkerProcesses() != 0)
		setException(CONF_DIRECTIVE_OVERLAP);
	int n = stoi(tokens[1]); //stoi는 catch로 예외처리
	if (n <= 0) //최대 개수 제한은 몇개로?
		setException(CONF_INVALID_VALUE);
	cycle.setWorkerProcesses(n);
}

void mainWorkerConnections(Cycle& cycle, std::string tokens[]) {
	if (cycle.getWorkerConnections() != 0)
		setException(CONF_DIRECTIVE_OVERLAP);
	int n = stoi(tokens[1]);
	if (n <= 0) //최대 개수 제한은 몇개로?
		setException(CONF_INVALID_VALUE);
	cycle.setWorkerConnections(n);
}

void mainClientMaxBodySize(Cycle& cycle, std::string tokens[]) {
	if (cycle.getClientMaxBodySize() != 0)
		setException(CONF_DIRECTIVE_OVERLAP);
	int n = stoi(tokens[1]);
	if (n <= 0) //최대 개수 제한은 몇개로?
		setException(CONF_INVALID_VALUE);
	cycle.setClientMaxBodySize(n);
}

void serverListen(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();
	if (server.getPort() != 0)
		setException(CONF_DIRECTIVE_OVERLAP);
	int n = stoi(tokens[1]);
	if (n < 0 || 65535 < n) // 포트 범위?
		setException(CONF_INVALID_VALUE);
	server.setPort(n);
}

void serverName(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();
	if (server.getDomain().length() != 0)
		setException(CONF_DIRECTIVE_OVERLAP);
	server.setDomain(tokens[1]);
}

void serverErrorPage(Cycle& cycle, std::string tokens[]) {
	Server& server = cycle.getServerList().back();
	if (server.getErrorPage().length() != 0)
		setException(CONF_DIRECTIVE_OVERLAP);
	server.setErrorPage(tokens[1]);
}

void locationRoot(Cycle& cycle, std::string tokens[]) {
	Location& location = cycle.getServerList().back().getLocationList().back();
	if (location.getStaticPath().length() != 0)
		setException(CONF_DIRECTIVE_OVERLAP);
	location.setStaticPath(tokens[1]);
}

void locationCgi(Cycle& cycle, std::string tokens[]) {
	Location& location = cycle.getServerList().back().getLocationList().back();
	if (location.getCgiPath().length() != 0)
		setException(CONF_DIRECTIVE_OVERLAP);
	location.setCgiPath(tokens[1]);
}
