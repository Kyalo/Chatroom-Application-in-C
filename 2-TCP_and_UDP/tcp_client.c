/*
  TCP CLIENT:
  Simple TCP client program to demo creating socket, connecting, sending/receiving data, and closing socket
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
  int network_socket;
  char request[] = "Identify server";
  char response[256];

  // Creating socket
  network_socket = socket(AF_INET, SOCK_STREAM, 0);

  // Specifying address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_port = htons(9002); // Port 9002 to correct byte order
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost
  
  // Connect to a socket
  if (connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    printf("Error connecting to server.\n");
    exit(1);
  }
  printf("[+] Connected to the server successfully.\n");

  // Sending request
  send(network_socket, request, sizeof(request), 0);
  printf("[+] Sending request...\n");

  // Receive response
  recv(network_socket, &response, sizeof(response), 0);
  printf("[+] Server Response:\n%s\n", response);

  // Close connection
  close(network_socket);

  return 0;
}