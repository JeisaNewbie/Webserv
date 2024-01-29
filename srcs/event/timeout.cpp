#include "../core/core.hpp"

Timeout::Timeout(void) : saved_time(time(NULL)) {}

Timeout::Timeout(const Timeout& src) { *this = src; }

Timeout::~Timeout(void) {}

Timeout& Timeout::operator =(const Timeout& src) {
	if (this != &src) {
		saved_time = src.saved_time;
	}
	return *this;
}

void Timeout::setSavedTime(void) { time(&saved_time); }

bool Timeout::checkTimeout(void) {
	if (difftime(time(NULL), saved_time) >= TIME_OUT)
		return true;
	return false;
}