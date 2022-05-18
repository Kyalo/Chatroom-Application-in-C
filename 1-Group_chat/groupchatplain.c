#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

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
char logged_in_user[30];
char separator[] = "---------------------------------------------------\n";

// Function protoypes
void load_data();
int login(char *username, char *password);
int signup(char *username, char *password);
void load_group_messages();
void update_users();
void update_messages();
void update_groups();
void list_all_groups();
void list_joined_groups();
void list_unjoined_groups();
int chat_screen(char *group_name);
int leave_group(int index);
int join_group(int index);
char *get_time();

/* MAIN */
int main() {
  int i, j, flag;
  char username[30], password[30], temp[30], choice_str[4];
  load_data();
  load_group_messages();
  printf("--------Chat Application--------\n");
  start:
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
      if (login(username, password)) {
        // If invalid login
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
      if (signup(username, password)) {
        // If signup error
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
      for (i = 0; i < groups_no; i++) {
        if (strcmp(groups[i].name, temp) == 0) {
          printf("\nMatch found. Opening %s chat screen...\n", temp);
          if (chat_screen(temp)) {
            // If user exits chat screen
            goto Mainmenu;
          };
        }
      }
      printf("No matches found\n");
      goto search;
      break;
    case 2:
      groups_list:
      printf(separator);
      printf("----- GROUPS LIST ----\nSelect group to open\n\n");
      printf("[+] Joined Groups\n");
      list_joined_groups();
      printf("\n[+] Unjoined Groups\n");
      list_unjoined_groups();
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
      for (i = 0; i < groups_no; i++) {
        if (strcmp(groups[i].name, temp) == 0) {
          printf("Group already exists.\n");
          goto create;
        }
      }
      strcpy(groups[groups_no].name, temp);
      groups_no++;
      update_groups();
      printf("The group '%s' has been created successfully\n", temp);
      if (chat_screen(temp)) {
        // If user exits chatscreen
        goto Mainmenu;
      }
      break;
    default:
      printf("Invalid choice.\n");
      goto Mainmenu;
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

int login(char *username, char *password) {
  /* Check for matching username, password pair */
  int i;
  for (i = 0; i < users_no; i++) {
    if ((strcmp(users[i].username, username) == 0) && (strcmp(users[i].password, password) == 0)) {
      strcpy(logged_in_user, username);
      printf("\nLogin successful. Welcome %s!\n", logged_in_user);
      return 0;
    }
  }
  printf("\nInvalid credentials. Please try again.\n");
  return 1;
}

int signup(char *username, char *password) {
  int i;
  // Check if username is available
  for (i = 0; i < users_no; i++) {
    if (strcmp(users[i].username, username) == 0) {
      printf("\nUsername is already taken.\n");
      return 1;
    }
  }
  strcpy(users[users_no].username, username);
  strcpy(users[users_no].password, password);
  users_no++;
  update_users();
  strcpy(logged_in_user, username);
  printf("User created successfully. Welcome %s!\n", logged_in_user);
  return 0;
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

void list_all_groups() {
  int i;
  for (i = 0; i < groups_no; i++) {
    printf("> %s\n", groups[i].name);
  }
}

void list_joined_groups() {
  int i, j;
  for (i = 0; i < groups_no; i++) {
    for (j = 0; j < 10; j++) {
      if (strcmp(groups[i].members[j], logged_in_user) == 0)
        printf("> %s\n", groups[i].name);
    }
  }
}

void list_unjoined_groups() {
  int i, j, flag;
  for (i = 0; i < groups_no; i++) {
    flag = 0;
    // Check if the logged in user is a member
    for (j = 0; j < 10; j++) {
      if (strcmp(groups[i].members[j], logged_in_user) == 0) {
        flag = 1;
      }
    }
    if (flag == 0)
      printf("> %s\n", groups[i].name);
  }
}

int chat_screen(char *group_name) {
  int i, j, flag;
  char message[160];
  // Check if group exists
  for (i = 0; i < groups_no; i++) {
    if (strcmp(groups[i].name, group_name) == 0) {
      while (1) {
        chatscreen:
        printf(separator);
        printf("---- Chat screen: Group '%s' ----\n\n", groups[i].name);
        // Check if current user is a member
        flag = 0;
        for (j = 0; j < 10; j++) {
          if (strcmp(groups[i].members[j], logged_in_user) == 0) {
            flag = 1;
          }
        }
        // If user is a member, they can send messages or leave
        if (flag == 1) {
          printf("Group members: ");
          for (j = 0; j < 10; j++) {
            if (strlen(groups[i].members[j]) > 0) {
              if (j == 0)
                printf("%s", groups[i].members[j]);
              else
                printf(", %s", groups[i].members[j]);
            }
          }
          printf("\n\n");
          for (j = 0; j < groups[i].messages_no; j++) {
            printf("[%s] %s > %s\n", groups[i].messages[j].sender, groups[i].messages[j].sent_at, groups[i].messages[j].message);
          }
          printf("\nEnter message (0 to go back, /exit to leave group): ");
          scanf(" %[^\n]s", message);
          if (strcmp(message, "0") == 0) {
            return 1;
          } else if (strcmp(message, "/exit") == 0) {
            if (!leave_group(i)) {
              // Left group
              return 1;
            }
          }
          else {
            // Construct message
            strcpy(messages[messages_no].group_name, group_name);
            strcpy(messages[messages_no].message, message);
            strcpy(messages[messages_no].sender, logged_in_user);
            strcpy(messages[messages_no].sent_at, get_time());
            messages_no++;

            // Save to group
            strcpy(groups[i].messages[groups[i].messages_no].group_name, group_name);
            strcpy(groups[i].messages[groups[i].messages_no].message, message);
            strcpy(groups[i].messages[groups[i].messages_no].sender, logged_in_user);
            strcpy(groups[i].messages[groups[i].messages_no].sent_at, get_time());
            groups[i].messages_no++;

            update_messages();
            goto chatscreen;
          }
        }
        else {
          join:
          printf("You are not a member of this group.\nJoin group? (y for yes, n to go back): ");
          scanf(" %[^\n]s", message);
          if (strcmp(message, "n") == 0)
            return 1;
          else if (strcmp(message, "y") == 0) {
            if (!join_group(i)) // If there is no error
              goto chatscreen;
            else
              return 1; // Error while joining
          } else {
            printf("Invalid choice.\n");
            goto join;
          }
        }
      }
    }
  }
  printf("Group not found.\n");
  return 1;
}

int leave_group(int index) {
  int i, j;
  // Find current user and delete
  for (i = 0; i < 10; i++) { // Iterate thru members
    if (strcmp(groups[index].members[i], logged_in_user) == 0) {
      for (j = i; j < 9; j++) { // Delete by shifting
        strcpy(groups[index].members[j], groups[index].members[j + 1]);
      }
      strcpy(groups[index].members[9], "");
      update_groups();
      printf("You have left the group successfully.\n");
      return 0;
    }
  }
  printf("You are not a member of the group.\n");
  return 1;
}

int join_group(int index) {
  int j;
  for (j = 0; j < 10; j++) {
    if (strlen(groups[index].members[j]) == 0) {
      strcpy(groups[index].members[j], logged_in_user);
      update_groups();
      printf("Joined group successfully.\n");
      return 0;
    }
  }
  printf("Group is full.\n");
  return 1;
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
