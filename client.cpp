#include <arpa/inet.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>

#define BUF_SIZE 1024

using namespace std;

int
	my_inet_addr(char *cAddr)
{
	int res = 0;

	string			 addr(cAddr);
	vector< string > addrs;

	istringstream str(addr);
	while (getline(str, addr, '.'))
		addrs.push_back(addr);

	int power = 0;
	for (int i = 3; i >= 0; i--) {
		int	   num = stoi(addrs[i]);
		string binary = bitset< 8 >(num).to_string();
		for (int j = binary.length() - 1; j >= 0; j--) {
			res += (binary[j] - '0') * pow(2, power);
			power++;
		}
	}
	return htonl(res);
}

int
	main()
{
	int			client_socket;
	sockaddr_in server_addr;

	char sAddr[15] = "127.0.0.1";
	int sPort = 80;

	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == -1)
		return 0;

	memset(&server_addr, 0, sizeof(sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(sPort);
	server_addr.sin_addr.s_addr = my_inet_addr(sAddr);

	if (connect(client_socket, (sockaddr *)&server_addr, sizeof(sockaddr))
		== -1)
		return 0;

	// char message[BUF_SIZE];
	char r_message[BUF_SIZE];
	int recieve_size;

	// while (1) {
		std::string str("POST /submit_form HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 28\r\nConnection: keep-alive\r\n\r\nusername=johndoe&password=1234\r\n");
		// while (1) {
		// 	fgets(message, BUF_SIZE, stdin);
		// 	std::string tmp(message);
		// 	if (tmp.find("stop") != std::string::npos)
		// 		break;
		// 	str = str.substr(0, str.length() - 1) += tmp;
		// 	str[str.length() - 1] = '\0';
		// }

		std::cout << "send message: " << str.length() << " " << str << "\n";
		send(client_socket, str.c_str(), str.length(), 0);

		recieve_size = recv(client_socket, r_message, sizeof(r_message) - 1, 0);
		if (recieve_size == -1)
			return -1;

		std::cout << "recieve message: " << r_message << "\n";
	// }

	close(client_socket);

	return 0;
}
