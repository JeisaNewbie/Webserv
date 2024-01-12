#include "../core/core.hpp"

Exception::Exception(int error_code) {
	switch (error_code) {
	case PROG_INVALID_ARG_CNT:
		message = "Program has 1 or 2 arguments";
		break;
	
	case CONF_FAIL_OPEN:
		message = "Failed to open configure file";
		break;

	case CONF_FAIL_READ:
		message = "Failed to read configure file";
		break;

	case CONF_DUP_DIRCTV:
		message = "Directives is duplicated";
		break;

	case CONF_INVALID_BLOCK_FORM:
		message = "Form of configure block is invalid";
		break;

	case CONF_INVALID_BLOCK_LOC:
		message = "Position of block is incorrect";
		break;

	case CONF_INVALID_LOC_PATH:
		message = "Path of location block is invalid";
		break;

	case CONF_INVALID_DIRCTV:
		message = "Directive is not matched";
		break;

	case CONF_INVALID_DIRCTV_ARG_CNT:
		message = "Number of directive arguments is incorrect";
		break;

	case CONF_INVALID_DIRCTV_VALUE:
		message = "Value of directive is invalid";
		break;

	case CONF_INVALID_CGI:
		message = "Type of cgi is invalid";
		break;

	case CONF_LACK_DIRCTV:
		message = "Directive is incomplete";
		break;

	case CONF_FAIL_TOKENIZE:
		message = "Failed to tokenize configure command";
		break;

	case WORK_FAIL_OPEN:
		message = "Failed to open log file";
		break;

	case WORK_FAIL_CREATE_KQ:
		message = "Failed to create kqueue";
		break;

	case WORK_FAIL_CREATE_SOCKET:
		message = "Failed to create socket";
		break;

	case EVENT_FAIL_BIND:
		message = "Bind function failed";
		break;

	case EVENT_FAIL_LISTEN:
		message = "Listen function failed";
		break;

	case EVENT_FAIL_ACCEPT:
		message = "Accept fucntion failed";
		break;

	case EVENT_FAIL_RECV:
		message = "Recv function failed";
		break;

	case EVENT_FAIL_SEND:
		message = "Send function failed";
		break;

	case EVENT_SET_ERROR_FLAG:
		message = "Event flag is set to error";
		break;

	default:
		message = "Error code is not defined";
		break;
	}
}

Exception::Exception(const Exception& src) {
	*this = src;
}

Exception::~Exception(void) {}

Exception& Exception::operator =(const Exception& src) {
	if (this != &src)
		message = src.message;
	return *this;
}

const char*	Exception::what() const {
	return message.c_str();
}

void setException(int _error_code) {
	error_code = _error_code;
	throw Exception(error_code);
}

void handleWorkerException(std::ofstream& error_log, int _error_code) {
	std::string	tmp;
	Exception	e(_error_code);

	tmp = std::string(e.what());
	error_log.write(tmp.c_str(), tmp.length());
	error_log.write("\n: ", 1);

	tmp = strerror(errno);
	error_log.write(tmp.c_str(), tmp.length());
	error_log.write("\n\n", 2);
	error_log.flush();
	throw e;
}

void handleEventException(std::ofstream& error_log, int _error_code, uintptr_t client_fd) {
	std::string	tmp;
	error_code = _error_code;

	if (client_fd != 0) {
		tmp = std::to_string(client_fd);
		error_log.write(tmp.c_str(), tmp.length());
		error_log.write(": ", 2);
	}
	tmp = std::string(Exception(error_code).what());
	error_log.write(tmp.c_str(), tmp.length());
	error_log.write("\n\n", 2);
	error_log.flush();
}
