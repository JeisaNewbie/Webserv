#include "../core/core.hpp"

Exception::Exception(int _costom_error) {
	system_error = errno;
	costom_error = _costom_error;
	setMessage(costom_error);
}

Exception::Exception(int _costom_error, std::string _problem) {
	system_error = errno;
	costom_error = _costom_error;
	problem = _problem;
	setMessage(costom_error);
}

Exception::Exception(const Exception& src) { *this = src; }

Exception::~Exception(void) {}

Exception& Exception::operator =(const Exception& src) {
	if (this != &src)
		message = src.message;
	return *this;
}

void	Exception::setMessage(int costom_error) {
	switch (costom_error) {
	case PROG_INVALID_ARG_CNT:
		message = "Program has 1 or 2 arguments";
		break;
	
	case PROG_FAIL_FUNC:
		message = "Failed to standard function";
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

	case CONF_DUP_SRV_BLOCK:
		message = "Server blocks is duplicated";
		break;

	case CONF_DUP_LOC_BLOCK:
		message = "Location blocks is duplicated";
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

	case EVENT_FAIL_CREATE_KQ:
		message = "Failed to create kqueue";
		break;

	case EVENT_FAIL_CREATE_SOCKET:
		message = "Failed to create socket";
		break;

	case EVENT_FAIL_BIND:
		message = "Bind function failed";
		break;

	case EVENT_FAIL_LISTEN:
		message = "Listen function failed";
		break;

	case EVENT_FAIL_FCNTL:
		message = "Fcntl function failed";
		break;

	case EVENT_FAIL_ACCEPT:
		message = "Accept fucntion failed";
		break;

	case EVENT_FAIL_KEVENT:
		message = "Kevent function failed";
		break;

	case EVENT_FAIL_RECV:
		message = "Recv function failed";
		break;

	case EVENT_FAIL_SEND:
		message = "Send function failed";
		break;

	case EVENT_CONNECT_FULL:
		message = "Connection limit is reached";
		break;

	default:
		message = "Error code is not defined";
		break;
	}
}

int					Exception::getSystemError(void) const { return system_error; }
int 				Exception::getCostomError(void) const { return costom_error; }
const std::string&	Exception::getProblemStr(void) const { return problem; }

const char*			Exception::what(void) const { return message.c_str(); }

int mainException(Exception& e) {
	std::cerr << "error code [" << e.getCostomError() << "] : " \
				<< e.what() << std::endl;
	if (e.getSystemError() != 0)
		std::cerr << ": " << strerror(e.getSystemError()) << std::endl;
	if (e.getProblemStr().length())
		std::cerr << "=> " << e.getProblemStr() << "\n";
	return e.getCostomError();
}

void eventException(int _costom_error, uintptr_t client_fd) {
	if (client_fd != 0)
		std::cout << client_fd << ": ";
	std::cout << Exception(_costom_error).what() << "\n";
}
