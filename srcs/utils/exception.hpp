#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

# include <iostream>

enum costom_error_type {
	PROG_INVALID_ARG_CNT = 1,
	PROG_FAIL_FUNC,

	CONF_FAIL_OPEN,
	CONF_FAIL_READ,
	CONF_DUP_DIRCTV,
	CONF_DUP_SRV_BLOCK,
	CONF_DUP_LOC_BLOCK,
	CONF_INVALID_BLOCK_FORM,
	CONF_INVALID_BLOCK_LOC,
	CONF_INVALID_LOC_PATH,
	CONF_INVALID_DIRCTV,
	CONF_INVALID_DIRCTV_ARG_CNT,
	CONF_INVALID_DIRCTV_VALUE,
	CONF_INVALID_CGI,
	CONF_LACK_DIRCTV,
	CONF_FAIL_TOKENIZE,

	EVENT_FAIL_CREATE_KQ,
	EVENT_FAIL_CREATE_SOCKET,
	EVENT_FAIL_BIND,
	EVENT_FAIL_LISTEN,
	EVENT_FAIL_FCNTL,
	EVENT_FAIL_ACCEPT,
	EVENT_FAIL_KEVENT,
	EVENT_FAIL_RECV,
	EVENT_FAIL_SEND,
	EVENT_CONNECT_FULL,
};

class Exception {
	public:
		Exception(int _costom_error);
		Exception(int _costom_error, std::string _problem);
		Exception(const Exception& src);
		~Exception(void);

		Exception& operator =(const Exception& src);
		
		void				setMessage(int costom_error);

		int					getSystemError(void) const;
		int					getCostomError(void) const;
		const std::string&	getProblemStr(void) const;

		const char*			what(void) const;

	private:
		Exception(void);

		int			system_error;
		int			costom_error;
		std::string	message;
		std::string	problem;
};

int		mainException(Exception& e);
void	eventException(int _costom_error, uintptr_t client_fd);

#endif