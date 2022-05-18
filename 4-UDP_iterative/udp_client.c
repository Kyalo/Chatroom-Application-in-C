/*
  UDP CLIENT GROUP CHAT
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

// Global variables
char request[2048], response[2048], logged_in_user[30];
char separator[] = "---------------------------------------------------\n";
const char s[2] = "\n";
struct sockaddr_in server_address;

// Function prototypes
void send_request();
void set_server_address();
int chat_screen(char *group_name);

int main() {
  int i, j, flag;
  char username[30], password[30], temp[30], choice_str[4], message[160];
  char *token;
  set_server_address();
  printf("--------Chat Application--------\n");
  start:
  printf(separator);
  printf("Select an option to continue.\n\n1. Log in\n2. Sign Up\n\nChoice: ");
  scanf(" %[^\n]s", choice_str);
  switch (atoi(choice_str)) {
    case 1:
      login:	// login label
      printf(separator);
      printf("Enter your credentials below.\n\nUsername: ");
      scanf(" %[^\n]s", username);
      printf("Password: ");
      scanf(" %[^\n]s", password);
      snprintf(request, sizeof(request), "/login\n%s\n%s", username, password);
      send_request();
      // Process response
      token = strtok(response, s);
      if (strcmp("OK", token) == 0) {
        token = strtok(NULL, s);
        strcpy(logged_in_user, token);
        printf("Login successful. Welcome %s!\n", logged_in_user);
      }
      else {
        // If invalid login
        printf("\nInvalid credentials. Please try again.\n");
        printf(separator);
        goto start;
      }
      break;
    case 2:
      signup:
      printf(separator);
      printf("Enter new user details below.\n\nUsername: ");
      scanf(" %[^\n]s", username);
      printf("Password: ");
      scanf(" %[^\n]s", password);
      snprintf(request, sizeof(request), "/signup\n%s\n%s", username, password);
      send_request();
      // Process response
      token = strtok(response, s);
      if (strcmp("OK", token) == 0) {
        token = strtok(NULL, s);
        strcpy(logged_in_user, token);
        printf("User created successfully. Welcome %s!\n", logged_in_user);
      } else {
        // If signup error
        printf("\nUsername is already taken.\n");
        goto signup;
      }
      break;
    default:
      printf("Invalid choice.\n");
      goto start;
  }
  Mainmenu:
  printf(separator);
  printf("Select an option to continue.\n\n1. Search for a group\n2. Groups list\n3. Create new group\n\nChoice: ");
  scanf(" %[^\n]s", choice_str);
  switch (atoi(choice_str)) {
    case 1:
      search:
      printf(separator);
      printf("Group search\nEnter name (0 to go back): ");
      scanf(" %[^\n]s", temp);
      if (strcmp(temp, "0") == 0)
        goto Mainmenu;
      if (chat_screen(temp)) {
        goto Mainmenu;
      }
      break;
    case 2:
      groups_list:
      printf(separator);
      snprintf(request, sizeof(request), "/grouplist\n%s", logged_in_user);
      send_request();
      // Process response
      token = strtok(response, s);
      if (strcmp("OK", token) != 0) {
        printf("Error displaying groups list.\n");
        goto Mainmenu;
      }
      token = strtok(NULL, s);
      printf("----- GROUPS LIST ----\nSelect group to open\n\n");
      while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, s);
      }
      printf("\nGroup Name (0 to go back): ");
      scanf(" %[^\n]s", temp);
      if (strcmp(temp, "0") == 0)
        goto Mainmenu;
      if (chat_screen(temp)) {
        // If user exits chatscreen
        goto Mainmenu;
      }
      break;
    case 3:
      create:
      printf(separator);
      printf("Enter group name (0 to go back): ");
      scanf(" %[^\n]s", temp);
      if (strcmp(temp, "0") == 0)
        goto Mainmenu;
      snprintf(request, sizeof(request), "/creategroup\n%s", temp);
      send_request();
      // Process response
      token = strtok(response, s);
      if (strcmp("OK", token) == 0) {
        printf("The group '%s' has been created successfully\n", temp);
        if (chat_screen(temp)) {
          // If user exits chatscreen
          goto Mainmenu;
        }
      } else {
        printf("%s\n", strtok(NULL, s));
      }
      goto Mainmenu;
      break;
    default:
      printf("Invalid choice.\n");
      goto Mainmenu;
  }

  return 0;
}

void set_server_address() {
  // Specify address and port
  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_port = htons(9003); // Port 9002 to correct byte order
  server_address.sin_addr.s_addr = INADDR_ANY; // Any interface on local machine
}

void send_request() {
  // Create socket
  int network_socket = socket(AF_INET, SOCK_DGRAM, 0);
  // Sending request
  if(sendto(network_socket, request, sizeof(request), 0, (const struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    printf("Failed to send request.\n");
    exit(1);
  }
  // Receive response
  // Receive response
  recv(network_socket, &response, sizeof(response), 0);
  //printf("[+] Response: %s\n", response);
  close(network_socket);
}

int chat_screen(char *group_name) {
  int i, j, flag;
  char message[160], *token;
  chatscreen:
  snprintf(request, sizeof(request), "/groupinfo\n%s\n%s", logged_in_user, group_name);
  send_request();
  token = strtok(response, s);
  if (strcmp("FAIL", token) == 0) {
    printf("Group not found.\n");
    return 1;
  } else if (strcmp("OK", token) == 0) {
    token = strtok(NULL, s);
    printf("---- Chat screen: Group '%s' ----\n\n", group_name);
    if (strcmp("Not a member", token) == 0) {
      join:
      printf("You are not a member of this group.\nJoin group? (y for yes, n to go back): ");
      scanf(" %[^\n]s", message);
      if (strcmp(message, "n") == 0)
        return 1;
      else if (strcmp(message, "y") == 0) {
        snprintf(request, sizeof(request), "/joingroup\n%s\n%s", logged_in_user, group_name);
        send_request();
        // Process response
        token = strtok(response, s);
        if (strcmp("OK", token) == 0) {
          printf("Joined group successfully.\n");
        }
        else {
          printf("%s\n", strtok(NULL, s));
        }
        goto chatscreen;
      } else {
        printf("Invalid choice.\n");
        goto join;
      }
    } else {
      printf("\nGroup members: %s\n\n", strtok(NULL, s));
      token = strtok(NULL, s);
      while(token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, s);
      }
      printf("\nEnter message (0 to go back, /exit to leave group): ");
      scanf(" %[^\n]s", message);
      if (strcmp(message, "0") == 0) {
        return 1;
      } else if (strcmp(message, "/exit") == 0) {
        snprintf(request, sizeof(request), "/leavegroup\n%s\n%s", logged_in_user, group_name);
        send_request();
        // Process response
        token = strtok(response, s);
        if (strcmp("OK", token) == 0)
          printf("Left group successfully.\n");
        else
          printf("%s\n", strtok(NULL, s));
        return 1;
      } else {
        snprintf(request, sizeof(request), "/message\n%s\n%s\n%s", logged_in_user, group_name, message);
        send_request();
        // Process response
        token = strtok(response, s);
        if (strcmp("OK", token) == 0) {
          goto chatscreen;
        }
        else {
          printf("%s\n", strtok(NULL, s));
          return 1;
        }
      }
    }
  }
}
