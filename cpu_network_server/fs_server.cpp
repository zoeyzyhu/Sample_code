#include <arpa/inet.h>		// htons()
#include <stdio.h>		// printf(), perror()
#include <stdlib.h>		// atoi()
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), send(), recv()
#include <unistd.h>		// close()
#include <iostream>
#include <regex>


#include "helpers.h"		// make_server_sockaddr(), get_port_number() reference from lab's solution !!!
#include "fs_server.h"
#include "file_system.h"

static const size_t MAX_MESSAGE_SIZE = FS_BLOCKSIZE + 512;

/**
 * Receives a string message from the client and prints it to stdout.
 *
 * Parameters:
 * 		connectionfd: 	File descriptor for a socket connection
 * 				(e.g. the one returned by accept())
 * Returns:
 *		0 on success, -1 on failure.
 */
int handle_connection(int connectionfd) {

	// (1) Receive message from client.

	char msg[MAX_MESSAGE_SIZE];
	memset(msg, 0, sizeof(msg));

	// Call recv() enough times to consume all the data the client sends.
	size_t recvd = 0;
	ssize_t block_rval = 0;
	ssize_t rval;
	std::string request_message;
	std::string requestType = "";
	std::string userName = "";
	std::string pathname = "";
	std::string rest = "";
	unsigned i = 0;
	bool nullFlag = true;
	bool rawRequest = false;
	do {
		// Receive as many additional bytes as we can in one call to recv()
		// (while not exceeding MAX_MESSAGE_SIZE bytes in total).
		rval = recv(connectionfd, msg + recvd, FS_BLOCKSIZE, 0);
		if (rval == -1) {
			close(connectionfd);
			return 0;
		}
		if (rawRequest == false) {
			for (unsigned ind = 0; ind < rval; ind++) {
				if (msg[recvd] == '\0') {
					rawRequest = true;
					while (i < request_message.size() && request_message[i] != ' ') {
						requestType += request_message[i];
						i++;
					}
					i++;
					if (!(std::regex_match(requestType, std::regex("(FS_READBLOCK|FS_WRITEBLOCK|FS_CREATE|FS_DELETE)")))) {
						close(connectionfd);
						return 0;
					}
					while (i < request_message.size() && request_message[i] != ' ') {
						userName += request_message[i];
						i++;
					}
					i++;
					if (userName.size() < 1 || userName.size() > FS_MAXUSERNAME) {
						close(connectionfd);
						return 0;
					}
					while (i < request_message.size() && request_message[i] != ' ') {
						pathname += request_message[i];
						i++;
					}
					i++;
					if (!(std::regex_match(pathname, std::regex("(/[^/]+)+"))) || pathname.size() > FS_MAXPATHNAME) {
						close(connectionfd);
						return 0;
					}
					unsigned count = 0;
					for(unsigned i=1; i < pathname.size() + 1; i++){
						if(pathname[i] != '/' && i != pathname.size()){
							count += 1;
						}
						else{
							if (count > FS_MAXFILENAME) {
								close(connectionfd);
								return 0;
							} else {
								count = 0;
							}
						}
					}

					if (requestType != "FS_DELETE") {
						while (i < request_message.size()) {
							rest += request_message[i];
							i++;
						}

					}

					if (requestType != "FS_WRITEBLOCK") {
						nullFlag = false;
					} else {
						block_rval = rval - ind;
					}
					recvd += block_rval;
					break;
				} else {
					request_message += msg[recvd];
				}
				recvd += 1;
			}
		} else {
			block_rval += rval;
		}
	} while (nullFlag && block_rval < FS_BLOCKSIZE && recvd < MAX_MESSAGE_SIZE);  // recv() returns 0 when client closes

	if (recvd >= MAX_MESSAGE_SIZE) 
	{
		close(connectionfd);
		return 0;
	}

	Request currRequest;
	memset(currRequest.pathname, 0, sizeof(currRequest.pathname));
	memset(currRequest.username, 0, sizeof(currRequest.username));
	strcpy(currRequest.pathname, pathname.c_str());
	strcpy(currRequest.username, userName.c_str());
	unsigned responseLength = request_message.size() + 1;


	if (requestType == "FS_DELETE") {
		// DO SOMETHING

		if (i != request_message.size() + 1) {
			close(connectionfd);
			return 0;
		}

		try
		{
		 	/*if (fs_delete(currRequest) != 1) {
				close(connectionfd);
				return 0;

			}*/
            filesystem_delete(currRequest);
		}
		catch (const std::runtime_error& error)
		{
			close(connectionfd);
			return 0;
		}
	}
	if (requestType == "FS_CREATE"){
		// DO SOMETHING
		if (rest == "f" || rest == "d") {
			currRequest.type = rest == "f" ? 'f' : 'd';
			try
			{	
				/*if (fs_create(currRequest) != 1) {
					close(connectionfd);
					return 0;

				}*/
                filesystem_create(currRequest);
			}
			catch (const std::runtime_error& error)
			{
				close(connectionfd);
				return 0;
			}

			
		} else {
			close(connectionfd);
			return 0;

		}
	}




	if (requestType == "FS_READBLOCK") {
		if (!(std::regex_match(rest, std::regex("[1-9]+[0-9]*|0")))) {
			close(connectionfd);
			return 0;
		}
		// DO SOMETHING
		currRequest.block = atoi(rest.c_str());
		if (currRequest.block >= FS_MAXFILEBLOCKS) {
			close(connectionfd);
			return 0;
		}
		try
		{
		 	filesystem_read(currRequest);
		}
		catch (const std::runtime_error& error)
		{
			close(connectionfd);
			return 0;
		}
	}

	if (requestType == "FS_WRITEBLOCK") {
		if (!(std::regex_match(rest, std::regex("[1-9]+[0-9]*|0")))) {
			close(connectionfd);
			return 0;
		}
		// DO SOMETHING
		currRequest.block = atoi(rest.c_str());
		if (currRequest.block >= FS_MAXFILEBLOCKS) {
			close(connectionfd);
			return 0;
		}
		memcpy(currRequest.data, msg + request_message.size() + 1, FS_BLOCKSIZE);
		try
		{
		 	filesystem_write(currRequest);
		}
		catch (const std::runtime_error& error)
		{
			close(connectionfd);
			return 0;
		}
	}
	
	char rMsg[MAX_MESSAGE_SIZE];
	memset(rMsg, 0, sizeof(rMsg));
	memcpy(rMsg, request_message.c_str(), responseLength);
	unsigned needToSend = responseLength;
	if (requestType == "FS_READBLOCK") {
		memcpy(rMsg + responseLength, currRequest.data, FS_BLOCKSIZE);
		needToSend += FS_BLOCKSIZE;

	}

	size_t sent = 0;
	do {
		ssize_t n = send(connectionfd, rMsg + sent, needToSend - sent, MSG_NOSIGNAL);
		if (n == -1) {
			close(connectionfd);
			return 0;
		}
		sent += n;
	} while (sent < needToSend);

	close(connectionfd);

	return 0;
}

/**
 * Endlessly runs a server that listens for connections and serves
 * them _synchronously_.
 *
 * Parameters:
 *		port: 		The port on which to listen for incoming connections.
 *		queue_size: 	Size of the listen() queue
 * Returns:
 *		-1 on failure, does not return on success.
 */
int run_server(int port, int queue_size) {

	// (1) Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Error opening stream socket");
		return -1;
	}

	// (2) Set the "reuse port" socket option
	int yesval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1) {
		perror("Error setting socket options");
		return -1;
	}

	// (3) Create a sockaddr_in struct for the proper port and bind() to it.
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	if (make_server_sockaddr(&addr, port) == -1) {
		return -1;
	}

	// (3b) Bind to the port.
	if (bind(sockfd, (sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("Error binding stream socket");
		return -1;
	}

	// (3c) Detect which port was chosen.
	port = get_port_number(sockfd);
	cout_lock.lock();
	std::cout << "\n@@@ port " << port<< std::endl;
	cout_lock.unlock();

	// (4) Begin listening for incoming connections.
	listen(sockfd, queue_size);

	fs_init();

	// (5) Serve incoming connections one by one forever.
	while (true) {
		int connectionfd = accept(sockfd, 0, 0);
		if (connectionfd == -1) {
			perror("Error accepting connection");
			return -1;
		}

		std::thread t1(handle_connection, connectionfd);
		t1.detach();
	}
}

int main(int argc, const char **argv) {
	// Parse command line arguments
	int port;
	if (argc != 2) {
		port = 0;
	} else {
		port = atoi(argv[1]);
	}

	if (run_server(port, 30) == -1) {
		return 1;
	}
	return 0;
}
