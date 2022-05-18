/*
  TCP SERVER GROUP CHAT
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// Socket libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // Socket addresses
#include <unistd.h> // Close function

typedef struct Message {
  char group_name[30];
  char sender[30];
  char sent_at[30];
  char message[160];
} Message;

typedef struct User {
  char username[30];
  char password[30];
} User;

typedef struct Group {
  char name[30];
  char members[10][30];
  int messages_no;
  Message messages[100];
} Group;

// Global variables
User users[20];
Message messages[100];
Group groups[20];
int users_no = 0, messages_no = 0, groups_no = 0;
int server_socket, client_socket;
struct sockaddr_in server_address, client_address;
char request[2048], response[2048];

// Function protoypes
void load_data();
void send_response();
void login(char *username, char *password);
void signup(char *username, char *password);
void load_group_messages();
void update_users();
void update_messages();
void update_groups();
void group_screen(char *username, char *group_name);
void join_group(char *username, char *group_name);
void leave_group(char *username, char *group_name);
void group_list(char *username);
void send_message(char *username, char *group_name, char *message);
char *get_time();
void create_group(char *group_name);

int main() {
  int i, j;
  char *token;
  const char s[2] = "\n";
  char temp1[30], temp2[30], buffer[160];

  // Load data
  load_data();
  load_group_messages();

  // Create a socket
  server_socket = socket(AF_INET, SOCK_DGRAM, 0);

  // Specifying address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_port = htons(9003); // Port 9002 to correct byte order
  server_address.sin_addr.s_addr = INADDR_ANY; // Any interface on local machine

  // Bind socket to port (Because we are receiing data)
  if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    printf("Failed to bind socket to port 9003.\n");
    exit(1);
  }
  printf("[+] Waiting for requests...\n");

  socklen_t client_addr_size = sizeof(client_address);

  // Iterative server
  while (1) {
    // Receive request
    recvfrom(server_socket, request, sizeof(request), MSG_WAITALL, (struct sockaddr *) &client_address, &client_addr_size);
    printf("[+] Incoming request. Processing...\n");
    // Get request type
    token = strtok(request, s);
    printf("Request type: %s\n", token);
    if (strcmp("/login", token) == 0) {
      strcpy(temp1, strtok(NULL, s));
      strcpy(temp2, strtok(NULL, s));
      login(temp1, temp2);
    } else if (strcmp("/signup", token) == 0) {
      strcpy(temp1, strtok(NULL, s));
      strcpy(temp2, strtok(NULL, s));
      signup(temp1, temp2);
    } else if (strcmp("/groupinfo", token) == 0) {
      strcpy(temp1, strtok(NULL, s));
      strcpy(temp2, strtok(NULL, s));
      group_screen(temp1, temp2);
    } else if (strcmp("/grouplist", token) == 0) {
      strcpy(temp1, strtok(NULL, s));
      group_list(temp1);
    } else if (strcmp("/joingroup", token) == 0) {
      strcpy(temp1, strtok(NULL, s));
      strcpy(temp2, strtok(NULL, s));
      join_group(temp1, temp2);
    } else if (strcmp("/leavegroup", token) == 0) {
      strcpy(temp1, strtok(NULL, s));
      strcpy(temp2, strtok(NULL, s));
      leave_group(temp1, temp2);
    } else if (strcmp("/creategroup", token) == 0) {
      strcpy(temp1, strtok(NULL, s));
      create_group(temp1);
    } else if (strcmp("/message", token) == 0) {
      strcpy(temp1, strtok(NULL, s));
      strcpy(temp2, strtok(NULL, s));
      strcpy(buffer, strtok(NULL, s));
      send_message(temp1, temp2, buffer);
    } else {
      strcpy(request, "Unrecognized request");
      send_response();
    }
  }

  return 0;
}

void load_data() {
  /* 
    1. Load user data (username, password) from users.txt
    2. Load messages data (group_name, sender, sent_at, message) from messages.txt
    3. Load groups data (name, members) from groups.txt
  */
  // Load user data
  FILE *fp;
  char buffer[200];
  int line_no = 0, item_no = 0, i = 0;
  const char s[2] = ",";
  char *token;
  fp = fopen("users.txt", "r");
  if (fp == NULL) {
    printf("[Error] The file users.txt could not be found.");
    exit(1);
  }
  while (fgets(buffer, 200, fp)) {
    if (isspace(*buffer))
      continue;
    buffer[strcspn(buffer, "\n")] = 0;
    if (line_no == 0) {
      strcpy(users[item_no].username, buffer);	
      line_no++;
    } else if (line_no == 1) {
      strcpy(users[item_no].password, buffer);
      line_no = 0;
      item_no++;
    }
  }
  users_no = item_no;
  fclose(fp);

  // Load messages
  line_no = 0;
  item_no = 0;
  fp = fopen("messages.txt", "r");
  if (fp == NULL) {
    printf("[Error] The file messages.txt could not be found.");
    exit(1);
  }
  while (fgets(buffer, 50, fp)) {
    if (isspace(*buffer))
      continue;
    buffer[strcspn(buffer, "\n")] = 0;
    if (line_no == 0) {
      strcpy(messages[item_no].group_name, buffer);
      line_no++;
    } else if (line_no == 1) {
      strcpy(messages[item_no].sender, buffer);
      line_no++;
    } else if (line_no == 2) {
      strcpy(messages[item_no].sent_at, buffer);	
      line_no++;
    } else if (line_no == 3) {
      strcpy(messages[item_no].message, buffer);
      line_no = 0;
      item_no++;
    }
  }
  messages_no = item_no;
  fclose(fp);

  // Load groups
  line_no = 0;
  item_no = 0;
  fp = fopen("groups.txt", "r");
  if (fp == NULL) {
    printf("[Error] The file groups.txt could not be found.");
    exit(1);
  }
  while (fgets(buffer, 50, fp)) {
    if (isspace(*buffer))
      continue;
    buffer[strcspn(buffer, "\n")] = 0;
    if (line_no == 0) {
      strcpy(groups[item_no].name, buffer);
      line_no++;
    } else if (line_no == 1) {
      // Get comma-separated member list
      token = strtok(buffer, s);

      for (i = 0; token != NULL; i++) {
        strcat(groups[item_no].members[i], token);
        token = strtok(NULL, s);
      }
      line_no = 0;
      item_no++;
    }
  }
  groups_no = item_no;
  fclose(fp);
}

void load_group_messages() {
  /* Load messages into the group structs they belong to */
  int i, j;
  for (i = 0; i < groups_no; i++) {
    groups[i].messages_no = 0;
    for (j = 0; j < messages_no; j++) {
      if (strcmp(messages[j].group_name, groups[i].name) == 0) {
        groups[i].messages[groups[i].messages_no] = messages[j];
        groups[i].messages_no++;
      }
    }
  } 
}

void update_users() {
  /* Update user records in users.txt */
  FILE *fp;
  int i = 0;
  fp = fopen("users.txt", "w");
  for (i = 0; i < users_no; i++) {
    fprintf(fp, "%s\n", users[i].username);
    fprintf(fp, "%s\n\n", users[i].password);
  }
  fclose(fp);
}

void update_messages() {
  /* Update messages records in users.txt */
  FILE *fp;
  int i = 0;
  fp = fopen("messages.txt", "w");
  for (i = 0; i < messages_no; i++) {
    fprintf(fp, "%s\n", messages[i].group_name);
    fprintf(fp, "%s\n", messages[i].sender);
    fprintf(fp, "%s\n", messages[i].sent_at);
    fprintf(fp, "%s\n\n", messages[i].message);
  }
  fclose(fp);
}

void update_groups() {
  /* Update group records in users.txt */
  FILE *fp;
  int i, j;
  fp = fopen("groups.txt", "w");
  for (i = 0; i < groups_no; i++) {
    fprintf(fp, "%s\n,", groups[i].name);
    for (j = 0; j < 10; j++) {
      if (strlen(groups[i].members[j]) > 0) {
        if (j == 0)
          fprintf(fp, "%s", groups[i].members[j]);
        else
          fprintf(fp, ",%s", groups[i].members[j]);
      }
    }
    fprintf(fp, "\n\n");
  }
  fclose(fp);
}

void send_response() {
  // Sending response
  sendto(server_socket, response, sizeof(response), 0, (struct sockaddr *) &client_address, sizeof(client_address));
}

void login(char *username, char *password) {
  /* Check for matching username, password pair */
  int i;
  for (i = 0; i < users_no; i++) {
    if ((strcmp(users[i].username, username) == 0) && (strcmp(users[i].password, password) == 0)) {
      snprintf(response, sizeof(response), "OK\n%s", username);
      send_response();
      return;
    }
  }
  snprintf(response, sizeof(response), "FAIL");
  send_response();
  return;
}

void signup(char *username, char *password) {
  int i;
  // Check if username is available
  for (i = 0; i < users_no; i++) {
    if (strcmp(users[i].username, username) == 0) {
      snprintf(response, sizeof(response), "FAIL");
      send_response();
      return;
    }
  }
  strcpy(users[users_no].username, username);
  strcpy(users[users_no].password, password);
  users_no++;
  update_users();
  snprintf(response, sizeof(response), "OK\n%s", username);
  send_response();
  return;
}

void group_screen(char *username, char *group_name) {
  int i, j, flag, group_index = -1;
  char buffer[256];
  // Check if group exists
  for (i = 0; i < groups_no; i++) {
    if (strcmp(groups[i].name, group_name) == 0) {
      group_index = i;
      break;
    }
  }
  if (group_index == -1) {
    snprintf(response, sizeof(response), "FAIL");
    send_response();
    return;
  }
  // Check if current user is a member
  for (i = 0; i < 10; i++) {
    if (strcmp(groups[group_index].members[i], username) == 0) {
      snprintf(response, sizeof(response), "OK\nIs member\n");
      for (j = 0; j < 10; j++) {
        if (strlen(groups[group_index].members[j]) > 0) {
          if (j > 0)
            strcat(response, ", ");
          strcat(response, groups[group_index].members[j]);
        }
      }
      strcat(response, "\n");
      for (j = 0; j < groups[group_index].messages_no; j++) {
        // Output formatted message to buffer
        snprintf(buffer, sizeof(buffer), "[%s] %s > %s\n", groups[group_index].messages[j].sender, groups[group_index].messages[j].sent_at, groups[group_index].messages[j].message);
        // Apend to response
        strcat(response, buffer);
      }
      send_response();
      return;
    }
  }
  snprintf(response, sizeof(response), "OK\nNot a member");
  send_response();
}

void join_group(char *username, char *group_name) {
  int i, j, index = -1;
  for (i = 0; i < groups_no; i++) {
    if (strcmp(groups[i].name, group_name) == 0) {
      index = i;
      break;
    }
  }
  if (index == -1) {
    snprintf(response, sizeof(response), "FAIL\nGroup not found");
    send_response();
    return;
  }
  for (j = 0; j < 10; j++) {
    if (strlen(groups[index].members[j]) == 0) {
      strcpy(groups[index].members[j], username);
      update_groups();
      snprintf(response, sizeof(response), "OK");
      send_response();
      return;
    }
  }
  snprintf(response, sizeof(response), "FAIL\nGroup is full");
  send_response();
}

void leave_group(char *username, char *group_name) {
  int i, j, index = -1;
  for (i = 0; i < groups_no; i++) {
    if (strcmp(groups[i].name, group_name) == 0) {
      index = i;
      break;
    }
  }
  if (index == -1) {
    snprintf(response, sizeof(response), "FAIL\nGroup not found");
    send_response();
    return;
  }
  // Find current user and delete
  for (i = 0; i < 10; i++) { // Iterate thru members
    if (strcmp(groups[index].members[i], username) == 0) {
      for (j = i; j < 9; j++) { // Delete by shifting
        strcpy(groups[index].members[j], groups[index].members[j + 1]);
      }
      strcpy(groups[index].members[9], "");
      update_groups();
      snprintf(response, sizeof(response), "OK");
      send_response();
      return;
    }
  }
  snprintf(response, sizeof(response), "FAIL\nYou are not a member of the group.");
  send_response();
  return;
}

void group_list(char *username) {
  int i, j, flag;
  char buffer[256];
  strcpy(response, "OK\n[+] Joined Groups\n");
  for (i = 0; i < groups_no; i++) {
    for (j = 0; j < 10; j++) {
      if (strcmp(groups[i].members[j], username) == 0) {
        snprintf(buffer, sizeof(buffer), "> %s\n", groups[i].name);
        strcat(response, buffer);
        break;
      }
    }
  }
  strcat(response, "\n[+] Unjoined Groups\n");
  for (i = 0; i < groups_no; i++) {
    flag = 0;
    // Check if the logged in user is a member
    for (j = 0; j < 10; j++) {
      if (strcmp(groups[i].members[j], username) == 0) {
        flag = 1;
      }
    }
    if (flag == 0) {
      snprintf(buffer, sizeof(buffer), "> %s\n", groups[i].name);
      strcat(response, buffer);
    }
  }
  send_response();
}

void send_message(char *username, char *group_name, char *message) {
  int i, j, index = -1;
  for (i = 0; i < groups_no; i++) {
    if (strcmp(groups[i].name, group_name) == 0) {
      index = i;
      break;
    }
  }
  if (index == -1) {
    snprintf(response, sizeof(response), "FAIL\nGroup not found");
    send_response();
    return;
  }
  // Construct message
  strcpy(messages[messages_no].group_name, group_name);
  strcpy(messages[messages_no].message, message);
  strcpy(messages[messages_no].sender, username);
  strcpy(messages[messages_no].sent_at, get_time());
  messages_no++;

  // Save to group
  strcpy(groups[i].messages[groups[i].messages_no].group_name, group_name);
  strcpy(groups[i].messages[groups[i].messages_no].message, message);
  strcpy(groups[i].messages[groups[i].messages_no].sender, username);
  strcpy(groups[i].messages[groups[i].messages_no].sent_at, get_time());
  groups[i].messages_no++;

  update_messages();
  snprintf(response, sizeof(response), "OK");
  send_response();
}

char *get_time() {
  char *pts; /* pointer to time string */
	time_t now; /* current time */
	char *ctime();
	(void) time(&now);
  char *result = ctime(&now);
  if (result[strlen(result)-1] == '\n' )
    result[strlen(result)-1] = 0;
  return result;
}

void create_group(char *group_name) {
  int i;
  for (i = 0; i < groups_no; i++) {
    if (strcmp(groups[i].name, group_name) == 0) {
      snprintf(response, sizeof(response), "FAIL\nGroup already exists.");
      send_response();
      return;
    }
  }
  strcpy(groups[groups_no].name, group_name);
  groups_no++;
  update_groups();
  snprintf(response, sizeof(response), "OK");
  send_response();
}