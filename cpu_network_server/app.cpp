#include <arpa/inet.h>		// ntohs()
#include <stdio.h>		// printf(), perror()
#include <stdlib.h>		// atoi()
#include <string.h>		// strlen()
#include <sys/socket.h>		// socket(), connect(), send(), recv()
#include <unistd.h>		// close()
#include "fs_client.h"

#include "helpers.h"		// make_client_sockaddr()

static const int MAX_MESSAGE_SIZE = 256;

/**
 * Sends a string message to the server.
 *
 * Parameters:
 *		hostname: 	Remote hostname of the server.
 *		port: 		Remote port of the server.
 * 		message: 	The message to send, as a C-string.
 * Returns:
 *		0 on success, -1 on failure.
 */
int send_message(const char *hostname, int port) {
	// (1) Create a socket

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// (2) Create a sockaddr_in to specify remote host and port
	struct sockaddr_in addr;
	if (make_client_sockaddr(&addr, hostname, port) == -1) {
		return -1;
	}

	// (3) Connect to remote server
    if (connect(sockfd, (sockaddr *) &addr, sizeof(addr)) == -1) {
    perror("Error connecting stream socket");
    return -1;
    }
    // (4) Send message to remote server
    const char malformed_request[200] = "FS_WRITEBLOCK 1 /dir";
    if (send(sockfd, malformed_request, sizeof(malformed_request), 0) == -1) {
    perror("Error sending on stream socket");
    return -1;
    }

	// (5) Close connection
	close(sockfd);

	return 0;
}

int main(int argc, const char **argv) {
	// Parse command line arguments
	if (argc != 3) {
		printf("Usage: ./client hostname port_num message\n");
		return 1;
	}
	const char *hostname = argv[1];
	int port = atoi(argv[2]);

    fs_clientinit(hostname, port);

	if (send_message(hostname, port) == -1) {
		return 1;
	}

	return 0;
}