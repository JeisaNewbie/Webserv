#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 1024

#define FAKE_RESPONSE "HTTP/1.1 200 OK\\r\nContent-Length:5\r\n\r\n\
\
hello"

using namespace std;

int main() {
	int server_socket, client_socket;
	sockaddr_in server_addr, client_addr;
	socklen_t client_addr_size;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
		return 0;

	memset(&server_addr, 0, sizeof(sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4000);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(server_socket, (sockaddr*)&server_addr, sizeof(sockaddr_in)) == -1)
		return 0;
	if(listen(server_socket, 0) == -1)
		return 0;
	char message[BUF_SIZE];
	int recieve_size;
	while(1){
		client_addr_size = sizeof(sockaddr_in);
		client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
		if(client_socket == -1)
			return 0;
		
		while(1){
			recieve_size = recv(client_socket, message, sizeof(message) - 1, 0);
			if(recieve_size == -1)
				return 0;
			message[recieve_size] = '\0';
			if(!strcmp(message, "exit") || !strcmp(message, "quit")){
				close(client_socket);
				break;
			}

			cout << message << endl;

			send(client_socket, FAKE_RESPONSE, (int)strlen(FAKE_RESPONSE), 0);
		}
		if(!strcmp(message, "quit")){
			close(server_socket);
			break;
		}
	}
	return 0;
}