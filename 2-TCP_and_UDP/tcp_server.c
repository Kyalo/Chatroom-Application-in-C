/*
  TCP SERVER:
  Simple TCP server program to demo creating socket, binding to an address, listening for connections, accepting connections, sending/receiving data, and closing socket
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Socket libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // Socket addresses
#include <arpa/inet.h>
#include <unistd.h> // Close function

int main() {
  int server_socket;
  char response[256] = "Hello, I'm the TCP server. If you are getting this, the connection was successful", request[256];

  // Create a socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // Specifying address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_port = htons(9002); // Port 9002 to correct byte order
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

  // Bind socket to address
  bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

  // Listen for connections
  listen(server_socket, 10); // Queue capacity of 10
  printf("[+] Server is listening...\n");

  // Accept incoming client connections
  int client_socket;
  client_socket = accept(server_socket, NULL, NULL);
  printf("[+] Incoming connection.\n");

  // Receive request
  recv(client_socket, &request, sizeof(request), 0);

  // Processing request
  printf("[+] Request: %s\n", request);

  // Send response
  send(client_socket, response, sizeof(response), 0);
  printf("[+] Sending response...\n");

  // Close connection
  close(server_socket);
}