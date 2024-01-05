#include "cmd.hpp"

Cmd::Cmd(void) {}

Cmd::Cmd(std::string _name, int _arg_cnt, handler_t _handler) :	\
	name(_name), arg_cnt(_arg_cnt), handler(_handler) {}

Cmd::Cmd(const Cmd& src) {
	*this = src;
}

Cmd::~Cmd(void) {}

Cmd& Cmd::operator =(const Cmd& src) {
	if (this != &src) {
		name = src.name;
		arg_cnt = src.arg_cnt;
		handler = src.handler;
	}
	return (*this);
}

const std::string& Cmd::getName(void) const {
	return name;
}

int Cmd::getArgCnt(void) const {
	return arg_cnt;
}

const handler_t& Cmd::getHandler(void) const {
	return handler;
}
