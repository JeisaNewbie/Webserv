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

	char sAddr[15] = "10.14.10.6";
	int sPort = 4000;

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

	char message[BUF_SIZE];
	// int recieve_size;

	while (1) {
		fgets(message, BUF_SIZE, stdin);

		if (!strcmp(message, "quit")) {
			send(client_socket, message, (int)strlen(message), 0);
			break;
		}

		send(client_socket, message, (int)strlen(message), 0);
		// recieve_size = recv(client_socket, message, sizeof(message) - 1, 0);
	}

	close(client_socket);

	return 0;
}
