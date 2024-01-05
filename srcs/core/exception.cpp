#include "core.hpp"

Exception::Exception(int error_code) {
	if (error_code == PROG_INVALID_ARG_CNT)
		message = "Program has 1 or 2 arguments";
	else if (error_code == CONF_OPEN_FAIL)
		message = "Open configuration file is failed";
	else if (error_code == CONF_READ_FAIL)
		message = "Read configuration file is failed";
	else if (error_code == CONF_DIRECTIVE_OVERLAP)
		message = "Configuration directive is overlaped";
	else if (error_code == CONF_INVALID_ARG_CNT)
		message = "Number of directive argument is not correct";
	else if (error_code == CONF_INVALID_VALUE)
		message = "Configuraion value is invalid";
	else if (error_code == CONF_INVALID_FORM)
		message = "Configuration form is invalid";
	else if (error_code == CONF_INVALID_LOC)
		message = "Configuration location is invalid";
	else if (error_code == CONF_INVALID_DIRECTIVE)
		message = "Configuration directive is not matched";
	else if (error_code == CONF_LACK_DIRECTIVE)
		message = "Configuration directive is lacked";
	else if (error_code == CONF_TOKENIZE_FAIL)
		message = "Tokenize configure command is failed";
	else if (error_code == WORK_OPEN_FAIL)
		message = "Open log file is failed";
	else if (error_code == WORK_CREATE_KQ_FAIL)
		message = "Create kqueue is failed";
	else if (error_code == WORK_CREATE_SOCKET_FAIL)
		message = "Create socket is failed";
	else if (error_code == EVENT_BIND_FAIL)
		message = "Function bind is failed";
	else if (error_code == EVENT_LISTEN_FAIL)
		message = "Function listen failed";
	else if (error_code == EVENT_ERROR_FLAG)
		message = "Event flag set to error";
	else if (error_code == EVENT_ACCEPT_FAIL)
		message = "Function accept is failed";
	else if (error_code == EVENT_RECV_FAIL)
		message = "Function recv is failed";
	else if (error_code == EVENT_SEND_FAIL)
		message = "Function send is failed";
	else
		message = "Error code is not defined";
}

const char*	Exception::what() const {
	return message.c_str();
}

void setException(int _error_code) {
	error_code = _error_code;
	std::cout << error_code << "\n";
	throw Exception(error_code);
}

void drivenEventException(std::ofstream &error_log, int _error_code, uintptr_t client_fd) {
	error_code = _error_code;
	std::string	tmp;

	if (client_fd != 0) {
		tmp = std::to_string(client_fd);
		error_log.write(tmp.c_str(), tmp.length());
		error_log.write(": ", 2);
	}
	tmp = std::string(Exception(error_code).what());
	error_log.write(tmp.c_str(), tmp.length());
	error_log.write("\n\n", 2);
}
