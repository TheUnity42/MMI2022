#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 6888

int handle_client_connect(int client_fd, char* request);

int main() {
	// make server struct
	struct sockaddr_in server = {0};
	struct sockaddr_in client = {0};

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	// open server socket descriptor
	int sfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sfd < 0) {
		perror("Error Creating Socket");
		return -1;
	}

	if(bind(sfd, (struct sockaddr*) &server, sizeof(server))) {
		perror("Error Binding to Socket Descriptor");
		return -1;
	}

	// listen for input
	if(listen(sfd, 0) < 0) {
		perror("Unable to Set Passive State on Socket");
		return -1;
	}

	socklen_t client_addr_size = sizeof(client);
	int cfd = accept(sfd, (struct sockaddr*) &client, &client_addr_size);

	if(cfd < 0) {
		perror("Error Connecting to Client Descriptor");
		return -1;
	}

	handle_client_connect(cfd, NULL);

	// respond
	const char* response = "HTTP/1.1 200 OK\n";
	int res_len = strlen(response);

	if(send(cfd, response, res_len, 0) < 0) {
		perror("Unable to Write Repsonse to Client");
		return -1;
	}

	// clean and exit
	close(cfd);
	close(sfd);

	return 0;
}

int handle_client_connect(int client_fd, char* request) {
	char* msg_data = (char*) malloc(1024);

	// print client connection
	ssize_t msg_len = recv(client_fd, msg_data, 1024, 0);

	if(msg_len < 0) {
		free(msg_data);
		perror("Error Reading Connection Message");
		return -1;
	}

	*(msg_data + msg_len) = '\0';
	printf("Recieved Connection Request:\n%s", msg_data);
	
	free(msg_data);
	return msg_len;
}
