
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*
 * Function to extract the path from the HTTP request.
 * It finds the first space, then the next space, and extracts the content between them.
 */
void extract_path(const char *request, char* path){
  char *start = strchr(request, ' ');
  start++;
  
  char* end = strchr(start, ' ');
  size_t len = end - start;
  strncpy(path,start,len);
  path[len] = '\0';
}

int main() {
   /*
   * what setbuf does is that default stdout prints the value when a 
   * \n line occured, otherwise it'll store it in buffer and print it
   * at the end. To solve this (more of like JS interpreter kinda work)
   * We have to provide stdout buffering as NULL by using setbuf. 
   *
   * Although stderr is unbuffered but to make sure it works fine. It's
   * good to make it's buffering as NULL also by providing it in setbuf.
   *
   * strerror = Strerror turns a error code into human readable format.
   * */
 
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);
  char request[1024];
	// Debugging log
	printf("Logs from your program will appear here!\n");
 
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;

	/*
	 * Creating a socket using socket() function
	 * server_fd stores the file descriptor for the server socket.
	 * AF_INET specifies IPv4, SOCK_STREAM specifies TCP.
	 * Returns -1 on failure.
	 */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	 /*
   * setsockopt is used for reusing the port which was recently closed. Basically when we stop a server on a
   * port, the os doesn't immedietly give us the permission to reuse the port. We have to make that start manually.
   *
   * To start it manually we first set the reuse as 1 (Why? because turning it on is 1 and off is 0)
   * then we use setsockopt function to make it reuse. 
   *
   * setsockopt(file descriptor where the socket is initialized, socket level or SOL_SOCKET, reuse address, reuse variable as an address, size of reuse
   * ) and if this value is 0 then it's an success.
   * */
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	  /*
   * sockaddr_in server_addr create a sockaddr_in(IPv4) structure datatype which has these values = 
   * struct sockaddr_in {
   *       sa_family_t     sin_family;     AF_INET 
   *       in_port_t       sin_port;       Port number 
   *       struct in_addr  sin_addr;       IPv4 address 
   * };
   *
   * sin_family = Address Family Ipv4
   * sin_port = host to network short (because computer and network store numbers differently) port number
   * sin_addr = because it is a type in_addr 
   * and in_addr = struct in_addr {
   *      in_addr_t s_addr;
   *     };
   * so the s_addr refers to the host to network long or basically anyone can connect to this server.
   *
   * bind(file descriptor of socket, we've to typecast the server_addr in sockaddr because it is accepted in bind function,
   * sizeof the server_address) if this is 0 then successful otherwise not.
   *
   * why typecasting not directly sockaddr? Because sockaddr has less properties inside it. which in sockaddr_in hasn't. So 
   * for limited info sharing we use sockaddr which is general purpose.
   * */
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
							 .sin_port = htons(4221),
							 .sin_addr = { htonl(INADDR_ANY) },
						};

  /*
   * Connection Backlog is the capacity of clients a server can handle,
   * listen(socket file descriptor, connection_backlog) if listen is 0 then it's working
   * */
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	/*
	 * Setting up the socket to listen for incoming connections.
	 */
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
  
  char *response_ok = "HTTP/1.1 200 OK\r\n\r\n";
  char *response_error = "HTTP/1.1 404 Not Found\r\n\r\n";
  
  /*
   * Accepting a client connection.
   */
  int client = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	char path[1024];
  if(client>0){
    memset(request, 0, 1024);
    read(client, request, 1024);
    
    /*
     * Checking if the request is "GET / ".
     */
    if(strncmp(request, "GET / ", 6) == 0){
     send(client, response_ok, strlen(response_ok), 0);
    } else {
      extract_path(request, path);
      
      char *token = strtok(path, "/");
      /*
       * If the request is for echo, extract the message.
       */
      if(strcmp(token,"echo")==0){
        token = strtok(NULL, "/");
        char response[1024];
        snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/plain\r\n"
               "Content-Length: %zu\r\n"
               "\r\n"
               "%s", strlen(token), token);
        send(client, response, sizeof(response), 0);
      }else {
        send(client, response_error, strlen(response_error), 0);
      }
    }
  }
	printf("Client connected\n");

	/*
	 * Closing the server socket.
	 */
	close(server_fd);

	return 0;
}

