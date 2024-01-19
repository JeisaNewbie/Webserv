#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <ctime>
#include <string>
#include <sys/socket.h>

std::string createUniqueFileName() {
    time_t		now = time(nullptr);
    struct tm*	timeinfo = localtime(&now);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeinfo);

    return std::string(buffer) + ".txt";
}

int main() {
    const std::string	request_method(getenv("REQUEST_METHOD"));

    if (request_method == "POST") {

		// 여기서 try catch 하면 어떻게 되지? 오빠가 만든 to_string 가져올까?
        
        // const std::string	content_length_str = getenv("CONTENT_LENGTH");
        // unsigned long		content_length = stoi(content_length_str);
        const std::string	post_data(getenv("QUERY_STRING"));

        std::cout << "Content-Type: text/plain\n\n";

        // content 길이 제한 ??
        if (post_data.length() > 0) {

    		const std::string	directory_path(getenv("REDIRECT_PATH"));
            std::string			new_file_name = createUniqueFileName();
            std::ofstream		new_file;

            new_file.open((std::string(directory_path) + "/" + new_file_name).c_str());
            new_file << post_data;
            new_file.close();

			//response 작성
            std::cout << "Status_code: 201\r\n";
            // std::cout << "Content_length: \r\n";

			const std::string	client_soket_str(getenv("CLIENT_SOCKET"));
			int					client_soket = stoi(client_soket_str);

			send(client_soket, "CGI", 4, 0);
        }
    }

    return 0;
}
