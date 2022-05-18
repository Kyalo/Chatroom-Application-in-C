/*
  UDP CLIENT:
  Simple UDP client program to demo creating socket, sending/receiving data, and closing socket
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
  char response[256];
  char request[] = "Hello server";
  // Creating socket
  network_socket = socket(AF_INET, SOCK_DGRAM, 0);

  // Specifying address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_port = htons(9003); // Port 9002 to correct byte order
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost
  
  // Send a request
  if(sendto(network_socket, request, sizeof(request), 0, (const struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    printf("Failed to send request.\n");
    exit(1);
  }
  printf("[+] Request sent.\n");

  // Receive response
  recv(network_socket, &response, sizeof(response), 0);
  printf("[+] Server Response:\n%s\n", response);

  // Close socket
  close(network_socket);

  return 0;
}