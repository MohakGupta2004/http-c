#include<stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
int main(){
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
  setbuf(stderr, NULL);
  setbuf(stdout, NULL);

  /*
   * server_fd means server file descriptor which is used for making sure that the socket is working
   * or not.
   * client_addr_len means client address length = 
   * client_addr = This holds the properties of the client address. 
   * AF_INET = Address family IPv4. 
   * SOCK_STREAM = For TCP connection.
   *
   * server_fd often returns -1 as an error.
   * */ 
  int server_fd, client_addr_len;
  struct sockaddr_in client_addr;
  server_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if(server_fd == -1){
    printf("Socket connection failed. %s", strerror(errno));
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
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))<0){
    printf("Process failed, %s", strerror(errno));
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

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(4221);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))!=0){
    printf("Binding failed, %s", strerror(errno)); 
    return 1;
  }

  /*
   * Connection Backlog is the capacity of clients a server can handle,
   * listen(socket file descriptor, connection_backlog) if listen is 0 then it's working
   * */
  int connection_backlog = 5;
  if(listen(server_fd, connection_backlog) != 0){
    printf("Connection Full, %s", strerror(errno));
    return 1;
  }

  /*
   * Just like the bind function accept function accepts file descriptor, general purpose client address and address value
   * of the size of the client_addr(because it's struct).
   * */
  client_addr_len = sizeof(client_addr);
  if(accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)<0){
    printf("Can't able to accept Client, %s\n", strerror(errno));
    return 1;
  }

  printf("Client Connected\n"); 
  close(server_fd);
}
