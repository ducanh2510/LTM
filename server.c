#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include "communication_code.h"
#include "linked_list.h"
#include "transfer.h"
#include <time.h>
#include <pthread.h>

int count_send = 0;
int count_write = 0;
char list_clients_img[REQUEST_SIZE] = "";
char main_name[BUFF_SIZE] = "";

singleList groups, files, users;

client_t *clients[MAX_CLIENTS];

int num_client = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Them cac client da dang nhap thanh cong vao mang - OK
void queue_add(client_t *cl) {
	pthread_mutex_lock(&clients_mutex);
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (!clients[i]) {
			clients[i] = cl;
			num_client++;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

// Delete client da dang xuat khoi mang - OK
void queue_delete(char *username) {
	pthread_mutex_lock(&clients_mutex);
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (clients[i] && strcmp(clients[i]->name, username) == 0) {
			clients[i] = NULL;
			num_client--;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

// In ra danh sach client dang ket noi - OK
void print_queue() {
	printf("[+]List clients: \n");
	for (int i = 0; i < num_client; i++) {
		printf("%s\n", clients[i]->name);
	}
}

// Doc file chua thong tin user roi luu vao danh sach lien ket userList - OK
void readUserFile(singleList *users) {
	char username[50], password[50], group_name[50];
	int status;
	FILE *f = fopen("./storage/user.txt", "r");

	if (f == NULL) {
		perror("[-]Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		char c = fgetc(f);
		if (c != EOF) {
			int res = fseek(f, -1, SEEK_CUR);
		}else {
			break;
		}

		fgets(username, 50, f);
		username[strlen(username) - 1] = '\0';

		fgets(password, 50, f);
		password[strlen(password) - 1] = '\0';

		fscanf(f, "%d\n", &status);

		user_struct *user = (user_struct *)malloc(sizeof(user_struct));
		strcpy(user->user_name, username);
		strcpy(user->password, password);
		user->status = status;
		insertEnd(users, user);
	}
	fclose(f);
}

// Kiem tra xem username da ton tai chua - OK
int checkExistence(int type, singleList list, char *string) {
	switch (type) {
	// Check user
	case 1: {
		int i = 0;
		list.cur = list.root;
		while (list.cur != NULL) {
			i++;
			if (strcmp(((user_struct *)list.cur->element)->user_name, string) != 0) {
				list.cur = list.cur->next;
			}else {
				return 1;
			}
		}
		return 0;
	}
	break;

	default:
		printf("[-]Type chua hop le !! (1,2 or 3)\n");
		break;
	}
}

// Tim kiem user theo username - OK
void *findByName(int type, singleList list, char string[50]) {
	switch (type) {
	case 1: {
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL) {
				i++;
				if (strcmp(((user_struct *)list.cur->element)->user_name, string) != 0) {
					list.cur = list.cur->next;
				}else {
					return list.cur->element;
				}
			}
			return NULL;
		}
		break;
	default:
		printf("[-]Type chua hop le !! (1,2 or 3)\n");
		break;
	}
}

// Dang ky checked - OK
void signUp(int sock, singleList *users, char *name, char *pass) {
	char buff[BUFF_SIZE];
	int size;
	printf("USERNAME: \'%s\'\n", name);
	if (checkExistence(1, *users, name) == 1) {
		sendCode(sock, EXISTENCE_USERNAME);
	}else {
		user_struct *user = (user_struct *)malloc(sizeof(user_struct));
		strcpy(user->user_name, name);
		strcpy(user->password, pass);
		user->status = 1;
		insertEnd(users, user);
		sendCode(sock, REGISTER_SUCCESS);
	}
}

// Dang nhap checked - OK
int signIn(int sock, singleList users, user_struct **loginUser, char *name, char *pass) {
	if (checkExistence(1, users, name) == 1) {
		*loginUser = (user_struct *)(findByName(1, users, name));
		if (strcmp((*loginUser)->password, pass) == 0) {
			sendCode(sock, LOGIN_SUCCESS);
			client_t *cli = (client_t *)malloc(sizeof(client_t));
			strcpy(cli->name, name);
			cli->sockfd = sock;
			cli->uid = num_client;
			queue_add(cli);
			return 1;
		}else {
			sendCode(sock, LOGIN_FAILED);
			return 0;
		}
	}else {
		sendCode(sock, LOGIN_FAILED);
		return 0;
	}
}

// Gui toi cac client khac tru nguoi gui - OK
void send_message(char name[100], char *nameFile) {
	char send_request[REQUEST_SIZE];
	for (int i = 0; i < num_client; i++) {
		if (strcmp(name, clients[i]->name) != 0) {
			sprintf(send_request, "%d*%s", FIND_IMG_IN_USERS, nameFile);
			printf("->SEND TO %s - RECV FROM %s - %s - %s \n", clients[i]->name, name, nameFile, send_request);
			send(clients[i]->sockfd, send_request, sizeof(send_request), 0);
			memset(send_request, '\0', strlen(send_request) + 1);
		}
	}
}

// Gui list danh sach anh cho nguoi yeu cau tim kiem - OK
void send_message_to_sender(char *list_imgs) {
	char send_request[REQUEST_SIZE];
	for (int i = 0; i < num_client; i++) {
		if (strcmp(main_name, clients[i]->name) == 0) {
			sprintf(send_request, "%d*%s", SEND_IMGS_TO_USER, list_imgs);
			send(clients[i]->sockfd, send_request, sizeof(send_request), 0);
			memset(send_request, '\0', strlen(send_request) + 1);
			break;
		}
	}
}

// Gui code khong tim thay anh - OK
void send_code_img_not_found(){ 
	for (int i = 0; i < num_client; i++) {
		if (strcmp(main_name, clients[i]->name) == 0) {
			sendCode(clients[i]->sockfd, NO_IMG_FOUND);
			break;
		}
	}
}

// Ham gui file cho server - OK
void *SendFileToClient(int new_socket, char fname[50]) {
	SendFile(new_socket, fname);
}

// Nhan file gui len tu client - OK
void receiveUploadedFileServer(int sock, char filePath[100]){
	if(receiveUploadedFile(sock, filePath)) count_write++;
	else return;
}

// Xoa file anh trong thu muc tam thoi - OK
void deleteImgsFile(char list_clients_img_copy[1024]) {
	char *deleteName;
	deleteName = strtok(list_clients_img_copy, "*");
	while(deleteName != NULL) {
		char path[1024];
		sprintf(path, "./files/%s.jpg", deleteName);
		if(remove(path) == 0){
			printf("[+] DELETED FILE SUCCESS: %s\n", path);
		}else{
			printf("[+] DELETED FILE FAILED: %s\n", path);
		}
		deleteName = strtok(NULL, "*");
	}
}

// Ham xu li luong - OK
void *handleThread(void *my_sock) {
	int new_socket = *((int *)my_sock);
	int REQUEST;
	char buff[1024];

	char username[BUFF_SIZE] = {};
	char *name, *pass;
	user_struct *loginUser = NULL;

	while (1) {
		readWithCheck(new_socket, buff, 1024);
		char *opcode = strtok(buff, "*");
		REQUEST = atoi(buff);
		switch (REQUEST) {
		case REGISTER_REQUEST:
			name = strtok(NULL, "*");
			pass = strtok(NULL, "*");
			printf("[+]REGISTER_REQUEST\n");
			signUp(new_socket, &users, name, pass);
			saveUsers(users);
			memset(name, '\0', strlen(name) + 1);
			memset(pass, '\0', strlen(pass) + 1);
			break;
		case LOGIN_REQUEST:
			// nhan username va password
			printf("[+]LOGIN_REQUEST\n");
			name = strtok(NULL, "*");
			pass = strtok(NULL, "*");
			if (signIn(new_socket, users, &loginUser, name, pass) == 1) {
				while (REQUEST != LOGOUT_REQUEST) {
					char *username;
					char *filename;
					char *choosen_user;
					readWithCheck(new_socket, buff, REQUEST_SIZE);
					printRequest(buff);
					char *opcode;
					opcode = strtok(buff, "*");
					REQUEST = atoi(opcode);
					switch (REQUEST) {
					case FIND_IMG_REQUEST:
						username = strtok(NULL, "*");
						strcpy(main_name, username);
						filename = strtok(NULL, "*");
						// gui yeu cau toi cac may con lai
						send_message(username, filename);
						count_send = num_client - 1;
						printf("[+]SEND TO ALL : %s\n", buff);
						memset(username, '\0', strlen(username) + 1);
						break;
					case FILE_WAS_FOUND:
						username = strtok(NULL, "*");
						printf("[+]FOUND FROM %s\n", username);  
						char file_path[BUFF_SIZE];
						str_trim_lf(username, strlen(username));
						sprintf(file_path, "./files/%s.jpg", username);
						username[strlen(username)] = '\0';
						pthread_mutex_lock(&clients_mutex);
						strcat(list_clients_img, username);
						strcat(list_clients_img, "*");
						pthread_mutex_unlock(&clients_mutex);
						receiveUploadedFileServer(new_socket, file_path);
						memset(file_path, '\0', strlen(file_path) + 1);
						if(count_send == count_write) {
							send_message_to_sender(list_clients_img);
							count_send = count_write = 0;
						}
						break;
					case CHOOSEN_USER:
						choosen_user = strtok(NULL, "*");
						sprintf(file_path, "./files/%s.jpg", choosen_user);
						SendFileToClient(new_socket, file_path);
						memset(main_name, '\0', strlen(main_name) + 1);
						char list_clients_img_copy[1024];
						strcpy(list_clients_img_copy, list_clients_img);
						deleteImgsFile(list_clients_img_copy);
						memset(list_clients_img, '\0', strlen(list_clients_img) + 1);
						memset(list_clients_img_copy, '\0', strlen(list_clients_img_copy) + 1);
						printf("===============COMPLETE===============\n");
						break;
					case FILE_WAS_NOT_FOUND:
						count_send--;
						if(count_send == 0) {
							send_code_img_not_found();
						}
						break;
					case NOT_CHOOSEN:
						memset(main_name, '\0', strlen(main_name) + 1);
						deleteImgsFile(list_clients_img);
						memset(list_clients_img, '\0', strlen(list_clients_img) + 1);
						break;
					case LOGOUT_REQUEST: // request code: 14
						printf("[+]LOGOUT_REQUEST\n");
						username = strtok(NULL, "*");
						queue_delete(username);
						sendCode(new_socket, LOGOUT_SUCCESS);
						memset(username, '\0', strlen(username) + 1);
						loginUser = NULL;
						break;
					default:
						break;
					}
				}
			}
			break;
		default:
			break;
		}
	}
	close(new_socket);
}

//==============MAIN==============
int main(int argc, char *argv[]) {
	if (argc == 1) {
		printf("Please input port number\n");
		return 0;
	}
	char *port_number = argv[1];
	int port = atoi(port_number);
	int opt = 1;
	int server_fd, new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("[-]Socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("[-]Setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	// Forcefully attaching socket to the port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("[-]Bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("[-]Listen");
		exit(EXIT_FAILURE);
	}

	char buff[100];
	createSingleList(&users);
	readUserFile(&users);
	while (1) {
		pthread_t tid;

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
			perror("[-]Accept");
			exit(EXIT_FAILURE);
		}
		printf("New request from sockfd = %d.\n", new_socket);
		pthread_create(&tid, NULL, &handleThread, &new_socket);
	}
	close(server_fd);
	return 0;
}