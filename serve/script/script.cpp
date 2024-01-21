#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <ctime>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

static void			handleGetMethod(const std::string directory_path, const std::string get_data);
static void			handlePostMethod(const std::string directory_path, const std::string post_data);
static std::string	getEnvString(const char* str);
static std::string	createUniqueFileName();

int main() {
	std::string			request_method = getEnvString("REQUEST_METHOD");
	const std::string	data = getEnvString("QUERY_STRING");
	const std::string	directory_path = getEnvString("REDIRECT_PATH");

	if (request_method == "GET")
		handleGetMethod(directory_path, data);
	else if (request_method == "POST")
		handlePostMethod(directory_path, data);

	return 0;
}

static void handleGetMethod(const std::string directory_path, const std::string get_data) {
	std::ifstream	file(directory_path + get_data);
	std::string		line;

	if (file.is_open() == false)
		return ; //error

	std::cout << "Content-Type: text/html\r\n";
	std::cout << "Status_code: 201\r\n";
	std::cout << "\r\n";

	std::cout << "<!DOCTYPE html>\n";
	std::cout << "<html lang=\"en\">\n";
	std::cout << "<head>\n";
	std::cout << "	<meta charset=\"UTF-8\">\n";
	std::cout << "	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	std::cout << "	<title>" << get_data << "</title>\n";
	std::cout << "</head>\n";
	std::cout << "<body>\n";
	std::cout << "	<h1>";
	
	while (std::getline(file, line))
		std::cout << line;

	std::cout << "</h1>\n";
	std::cout << "</body>\n";
	std::cout << "</html>\n";
	std::cout << "0\r\n"; // 맞나?

}

static void handlePostMethod(const std::string directory_path, const std::string post_data) {
	std::cout << "Content-Type: text/plain\r\n";

	// content 길이 제한 둘까??
	if (post_data.length() > 0) {
		
		std::string		new_file_name = createUniqueFileName();
		std::ofstream	new_file(std::string(directory_path) + "/" + new_file_name);
		// std::ofstream	new_file;

		// new_file.open((std::string(directory_path) + "/" + new_file_name).c_str());
		new_file << post_data;
		new_file.close();

		//response 작성
		std::cout << "Status_code: 201\r\n";
	}
}

static std::string getEnvString(const char* str) {
	const char*	tmp = getenv(str);
	
	if (tmp == NULL)
		// error

	return std::string(tmp);
}

static std::string createUniqueFileName() {
	time_t		now = time(nullptr);
	struct tm*	timeinfo = localtime(&now);

	char buffer[80];
	strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeinfo);

	return std::string(buffer) + ".txt";
}
