#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

void showFileList() {
	std::cout << "Content-Type: text/html\n\n";
	std::cout << "<!DOCTYPE html>\n";
	std::cout << "<html>\n";
	std::cout << "<head>\n";
	std::cout << "	<meta charset=\"utf-8\">\n";
	std::cout << "	<meta name=\"viewport\" content=\"width=device-width\">\n";
	std::cout << "	<title>User Posts With File List</title>\n";
	std::cout << "</head>\n";
	std::cout << "<body>\n";
	std::cout << "	<h1>Welcome to the User Posts Page</h1>\n";
	std::cout << "	<div id=\"postContainer\">\n";
	std::cout << "	</div>\n";
	std::cout << "	<textarea id=\"postText\" rows=\"4\" cols=\"50\" placeholder=\"Write your post here\"></textarea>\n";
	std::cout << "	<button id=\"postButton\" onclick=\"addPost()\">Add Post</button>\n";
	std::cout << "	<button id=\"deleteButton\" onclick=\"deletePost()\">Delete Post</button>\n";
	std::cout << "	<h1>File List</h1>\n";
	std::cout << "	<ul id=\"fileList\">\n";

	for (const auto & entry : fs::directory_iterator(".")) {
		std::cout << "	<li><a href=\"#\" onclick=\"getFileContent('" << entry.path().filename() << "')\">" << entry.path().filename() << "</a></li>\n";
	}

	std::cout << "	</ul>\n";
	std::cout << "	<script src=\"cgi.cpp\"></script>\n";
	std::cout << "</body>\n";
	std::cout << "</html>\n";
}

void createNewFile(const std::string& filename, const std::string& content) {
	std::ofstream file(filename);
	if (file.is_open()) {
		file << content;
		file.close();
	}
}

std::string getFileContent(const std::string& filename) {
	std::ifstream file(filename);
	return std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
}

int main() {
	std::string requestMethod = getenv("REQUEST_METHOD");

	if (requestMethod == "GET") {
		std::string query(getenv("QUERY_STRING"));
		if (query.length() > 9 && query.substr(0, 9) == "filename=") {
			std::string filename = query.substr(9);
			std::string fileContent = getFileContent(filename);
			std::cout << "Content-Type: text/html\n\n";
			std::cout << fileContent;
		}
		else {
			std::cout << "Content-Type: text/plain\n\n";
			std::cout << "Error: Invalid filename";
		}
	}
	else if (requestMethod == "POST") {
		std::string contentLengthString = getenv("CONTENT_LENGTH");
		int contentLength = 0;
		if (!contentLengthString.empty()) {
			contentLength = std::stoi(contentLengthString);
		}
		
		std::string input;
		for (int i = 0; i < contentLength; i++) {
			char c;
			std::cin.get(c);
			input += c;
		}
		
		createNewFile("newfile.txt", input);
		showFileList(); // Show the updated file list after creating a new file
	}
	else {
		showFileList();
	}
	
	return 0;
}
