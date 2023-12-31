#include "../core/core.hpp"

const int Conf::main_cmd_max = 5;
const int Conf::srv_cmd_max = 2;
const int Conf::loc_cmd_max = 1;

static void parseMain(Cycle &cycle, Conf &conf, std::ifstream& file);
static void parseServer(Cycle &cycle, Conf &conf, std::ifstream& file);
static void parseLocation(Cycle& cycle, Conf &conf, std::ifstream& file, const std::string& location_path);
static void callCmd(Cycle &cycle, Conf &conf, int location, \
						std::string *tokens, int token_cnt);
static int tokenizer(char *str, std::string *tokens);
static int checkConfLocation(std::string str[]);
static void checkGetlineError(std::ifstream& file);
static void checkLocationType(std::string location_path, int &location_type);
static void checkUseCgi(Cycle &cycle, std::string type);

Conf::Conf(void) {
	main_cmd = NULL;
	srv_cmd = NULL;
	loc_cmd = NULL;
}

Conf::Conf(std::string _name) {
	setFile(_name);
	setCmd();
}

Conf::Conf(const Conf& src) {
	*this = src;
}

Conf::~Conf(void) {
	if (file.is_open() == TRUE)
		file.close();

	if (main_cmd)
		delete[](main_cmd);
	if (srv_cmd)
		delete[](srv_cmd);
	if (loc_cmd)
		delete[](loc_cmd);
}

Conf& Conf::operator =(const Conf& src) {
	if (this != &src) {
		file_name = src.file_name;
		if (file.is_open() == TRUE)
			file.close();
		file.open(file_name);
		if (file.is_open() == FALSE)
			setException(CONF_OPEN_FAIL);

		if (main_cmd)
			delete[](main_cmd);
		main_cmd = new Cmd[main_cmd_max]();
		for (int i = 0; i < main_cmd_max; i++)
			main_cmd[i] = src.main_cmd[i];

		if (srv_cmd)
			delete[](srv_cmd);
		srv_cmd = new Cmd[srv_cmd_max]();
		for (int i = 0; i < srv_cmd_max; i++)
			srv_cmd[i] = src.srv_cmd[i]; 

		if (loc_cmd)
			delete[](loc_cmd);
		loc_cmd = new Cmd[loc_cmd_max]();
		for (int i = 0; i < loc_cmd_max; i++)
			loc_cmd[i] = src.loc_cmd[i];
	}
	return (*this);
}

void Conf::setFile(std::string _name) {
	file_name = _name;
	file.open(file_name);
	if (file.is_open() == FALSE)
		setException(CONF_OPEN_FAIL);
}

std::ifstream& Conf::getFile(void) {
	return file;
}

const std::ifstream& Conf::getFileConst(void) const {
	return file;
}

void Conf::setCmd(void) {
	main_cmd = new Cmd[main_cmd_max]();
	srv_cmd = new Cmd[srv_cmd_max]();
	loc_cmd = new Cmd[loc_cmd_max]();

	main_cmd[0] = Cmd("worker_processes", CMD_TAKE1, mainWorkerProcesses);
	main_cmd[1] = Cmd("worker_connections", CMD_TAKE1, mainWorkerConnections);
	main_cmd[2] = Cmd("client_max_body_size", CMD_TAKE1, mainClientMaxBodySize);
	main_cmd[3] = Cmd("uri_limit_length", CMD_TAKE1, mainUriLimitLength);
	main_cmd[4] = Cmd("root", CMD_TAKE1, mainRoot);

	srv_cmd[0] = Cmd("listen", CMD_TAKE1, serverListen);
	srv_cmd[1] = Cmd("server_name", CMD_TAKE1, serverName);

	loc_cmd[0] = Cmd("root", CMD_TAKE1, locationRoot);
}

const Cmd* Conf::getCmdListConst(int loc_type) const {
	if (loc_type == CONF_MAIN)
		return main_cmd;
	if (loc_type == CONF_SRV)
		return srv_cmd;
	if (loc_type == CONF_LOC)
		return loc_cmd;
	return NULL;
}

int Conf::getCmdMaxConst(int loc_type) const {
	if (loc_type == CONF_MAIN)
		return main_cmd_max;
	if (loc_type == CONF_SRV)
		return srv_cmd_max;
	if (loc_type == CONF_LOC)
		return loc_cmd_max;
	return -1;
}

void setConf(Conf &conf, int argc, char *file_name) {
	if (argc != 1 && argc != 2)
		setException(PROG_INVALID_ARG_CNT);
	if (argc == 1)
		conf = Conf(DEFAULT_FILE);
	else
		conf = Conf(file_name);
}

void parseConf(Cycle &cycle, Conf &conf) {
	char			buf[BUF_SIZE];
	std::string		tokens[TOKEN_SIZE];
	int				token_cnt;
	std::ifstream&	file = conf.getFile();

	while (file.getline(buf, sizeof(buf))) {
		if (buf[0] == '\0')
			continue;

		for (int i = 0; i < TOKEN_SIZE; i++)
			tokens[i] = '\0';

		token_cnt = tokenizer(buf, tokens);
		if (token_cnt != 2 || tokens[1] != "{")
			setException(CONF_INVALID_FORM);

		if (checkConfLocation(tokens) == CONF_MAIN)
			parseMain(cycle, conf, file);
		else // CONF_SRV
			parseServer(cycle, conf, file);
	}
	checkGetlineError(file);
}

static void parseMain(Cycle &cycle, Conf &conf, std::ifstream& file) {
	char			buf[BUF_SIZE];
	std::string		tokens[TOKEN_SIZE];
	int				token_cnt;
	std::string		str_buf;

	while (file.getline(buf, sizeof(buf))) {
		str_buf = static_cast<std::string>(buf);

		if (str_buf.length() == 0)
			continue;
		if (str_buf == "}")
			break;
		
		token_cnt = tokenizer(buf, tokens);
		tokens[0] = &tokens[0][1]; //tab으로 시작하니까
		callCmd(cycle, conf, CONF_MAIN, tokens, token_cnt);
	}
	checkGetlineError(file);

	if (cycle.getWorkerProcesses() == 0 \
		|| cycle.getWorkerConnections() == 0 \
		|| cycle.getClientMaxBodySize() == 0 \
		|| cycle.getUriLimitLength() == 0 \
		|| cycle.getMainRoot() == "")
		setException(CONF_LACK_DIRECTIVE);
}

static void parseServer(Cycle &cycle, Conf &conf, std::ifstream& file) {
	char				buf[BUF_SIZE];
	std::string			tokens[TOKEN_SIZE];
	int					token_cnt;
	std::string			str_buf;
	std::list<Server>&	server_list = cycle.getServerList();

	server_list.push_back(Server()); //복사해서 추가함

	while (file.getline(buf, sizeof(buf))) {
		str_buf = static_cast<std::string>(buf);

		if (str_buf.length() == 0)
			continue;
		if (str_buf == "}")
			break;

		token_cnt = tokenizer(buf, tokens);
		tokens[0] = &tokens[0][1]; //tab으로 시작하니까

		if (tokens[0] == "location") {
			if (token_cnt != 3 || tokens[2] != "{")
				setException(CONF_INVALID_FORM);
			parseLocation(cycle, conf, file, tokens[1]);
			continue;
		}
		callCmd(cycle, conf, CONF_SRV, tokens, token_cnt);
	}
	checkGetlineError(file);

	if (server_list.back().getPort() == 0 || \
		server_list.back().getDomain() == "")
		setException(CONF_LACK_DIRECTIVE);
}

static void parseLocation(Cycle& cycle, Conf &conf, std::ifstream& file, const std::string& location_path) {
	char					buf[BUF_SIZE];
	std::string				tokens[TOKEN_SIZE];
	int						token_cnt, location_type;
	std::string				str_buf;
	std::list<Location>&	location_list = cycle.getServerList().back().getLocationList();

	checkLocationType(location_path, location_type);
	location_list.push_back(Location(location_type, location_path)); //복사해서 추가함
	if (location_type == LOC_CGI)
		checkUseCgi(cycle, location_path);

	while (file.getline(buf, sizeof(buf))) {
		str_buf = static_cast<std::string>(buf);

		if (str_buf.length() == 0)
			continue;
		if (str_buf == "\t}")
			break;

		token_cnt = tokenizer(buf, tokens);
		tokens[0] = &tokens[0][2]; //tab 2개로 시작하니까
		callCmd(cycle, conf, CONF_LOC, tokens, token_cnt);
	}
	checkGetlineError(file);

	if (location_list.back().getSubRoot() == "")
		setException(CONF_LACK_DIRECTIVE);
}

static void callCmd(Cycle &cycle, Conf &conf, int location, \
						std::string *tokens, int token_cnt) {
	handler_t	handler;
	int 		idx;
	const Cmd 	*cmd = conf.getCmdListConst(location);
	int			cmd_max = conf.getCmdMaxConst(location);

	for (idx = 0; idx < cmd_max; idx++) {
		if (cmd[idx].getName() == tokens[0]) {
			if (cmd[idx].getArgCnt() != token_cnt - 1)
				setException(CONF_INVALID_ARG_CNT);
			handler = cmd[idx].getHandler();
			handler(cycle, tokens);
			break;
		}
	}
	if (idx == cmd_max)
		setException(CONF_INVALID_DIRECTIVE);
}

static int tokenizer(char *str, std::string *tokens) {
	std::istringstream	istr(str);
	std::string			token;
	int					idx = 0;

	while (getline(istr, token, ' '))
		if (token.empty() == FALSE) // 공백이 두 개 이상 이어질 때, 공백과 공백 사이에 빈 토큰이 생성됨
			tokens[idx++] = token;

	if (istr.eof() == FALSE)
		setException(CONF_TOKENIZE_FAIL);
	return idx;
}

static int checkConfLocation(std::string str[]) {
	if (str[0] == "main")
		return CONF_MAIN;
	if (str[0] == "server")
		return CONF_SRV;
	setException(CONF_INVALID_LOC);
	return 0;
}

static void checkGetlineError(std::ifstream& file) {
	if (file.eof() != TRUE && (file.fail() == TRUE || file.bad() == TRUE))
		setException(CONF_READ_FAIL);
}

static void checkLocationType(std::string location_path, int &location_type) {
	if (location_path == "/")
		location_type = LOC_DEFAULT;
	else if (location_path == "/error")
		location_type = LOC_ERROR;
	else if (location_path[0] == '.')
		location_type = LOC_CGI;
	else
		setException(CONF_INVALID_LOC_TYPE);
}

static void checkUseCgi(Cycle &cycle, std::string type) {
	if (type == ".php")
			cycle.setUseCgi(TRUE);
	else
		setException(CONF_INVALID_CGI);
}
