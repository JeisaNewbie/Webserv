#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <ctime>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <exception>

static void			handleGetMethod(const std::string directory_path, const std::string get_data);
static void			handlePostMethod(const std::string directory_path, const std::string post_data);
static void			handleSignal(int);
static std::string	getEnvString(const char* str);
static std::string	createUniqueFileName();

int main() {

	std::cerr << "SCRIPT_START\n";

	signal(SIGTERM, handleSignal);

	try {
		// while (1) {}
		std::string			data_post;
		std::string			request_method = getEnvString("REQUEST_METHOD");
		// std::string			data_post = getEnvString("QUERY_STRING_POST");
		const std::string	data_get = getEnvString("QUERY_STRING_GET");
		const std::string	directory_path = getEnvString("REDIRECT_PATH");

		while (std::getline(std::cin, data_post)) {}
		// if (line.size() != 0)
		// 	data_post = line;
		if (request_method == "GET")
			handleGetMethod(directory_path, data_get);
		else if (request_method == "POST")
			handlePostMethod(directory_path, data_post);
	}
	catch(const std::exception& e) {
		std::cerr << "Script exception caught: " << e.what() << std::endl;
		return -1;
	}

	std::cerr << "SCRIPT_DONE\n";
	return 0;
}

static void handleGetMethod(const std::string directory_path, const std::string get_data) {
	std::ifstream		file(directory_path + get_data);
	std::stringstream	ss;

	if (file.is_open() == false) {
		std::cout << "Status_code: 404\r\n";
		return ;
	}

	ss << file.rdbuf();

	std::cerr << "WRITE_TO_FD\n";
	std::cout << "Content-Type: text/plain\r\n";
	std::cout << "Status_code: 200\r\n";
	std::cout << "\r\n";
	std::cout << ss.str();
	std::cout << "\r\n";
}

static void handlePostMethod(const std::string directory_path, const std::string post_data) {
	std::cout << "Content-Type: text/plain\r\n";

	// content 길이 제한 둘까??
	if (post_data.length() > 0) {

		std::string		new_file_name = createUniqueFileName();
		std::ofstream	new_file(std::string(directory_path) + new_file_name);

		new_file << post_data;
		new_file.close();

		std::cout << "Status_code: 201\r\n";
	}
}

static void handleSignal(int signum) {
	// 파일 내용 삭제
	static_cast<void>(signum);
	std::cout << "Status_code: 500\r\n";
	exit(1);
}

static std::string getEnvString(const char* str) {
	const char*	tmp = getenv(str);

	if (tmp == NULL)
		throw std::runtime_error("get env");

	return std::string(tmp);
}

static std::string createUniqueFileName() {
	time_t		now = time(nullptr);
	struct tm*	timeinfo = localtime(&now);

	char buffer[80];
	strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeinfo);

	return std::string(buffer) + ".txt";
}
