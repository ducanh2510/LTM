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
#include <time.h>
#include <pthread.h>

#define BUFF_SIZE 100
#define MAX_CLIENTS 10
#define REQUEST_SIZE 1024

int count_send = 0;
int count_write = 0;
char list_clients_img[REQUEST_SIZE] = "";
char main_name[BUFF_SIZE] = "";

singleList groups, files, users;
typedef struct
{
	int sockfd;
	int uid;
	char name[100];
} client_t;

client_t *clients[MAX_CLIENTS];

int num_client = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void printRequest(char *request) {
	printf("[+]REQUEST: %s\n", request);
}

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

// In ra danh sach client dang ket noi - OK
void print_queue() {
	printf("[+]List clients: \n");
	for (int i = 0; i < num_client; i++) {
		printf("%s\n", clients[i]->name);
	}
}

// ========================================

// Ham gui thong diep cho client va check - OK
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int sendByte = 0;
	sendByte = send(sock, buff, length, 0);
	if (sendByte > 0) {
	}else {
		close(sock);
		pthread_exit(0);
	}
}

// Ham nhan thong diep tu client va check - OK
int readWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int recvByte = 0;
	recvByte = read(sock, buff, length);
	if (recvByte > 0)
	{
		return recvByte;
	}
	else
	{
		close(sock);
		pthread_exit(0);
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

// Gui tin hieu CODE tuong ung cho client - OK
void sendCode(int sock, int code) {
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	printf("-->Response: %s\n", codeStr);
	sendWithCheck(sock, codeStr, strlen(codeStr) + 1);
}

// Kiem tra xem username da ton tai chua - OK
int checkExistence(int type, singleList list, char string[50]) {
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
void signUp(int sock, singleList *users) {
	char buff[BUFF_SIZE], username[50], password[50];
	int size;
	sendCode(sock, REGISTER_SUCCESS);

	while (1) {
		size = readWithCheck(sock, buff, BUFF_SIZE);

		strcpy(username, buff);
		username[strlen(username) - 1] = '\0';
		if (username[strlen(username) - 2] == '\n') {
			username[strlen(username) - 2] = '\0';
		}
		printf("username: \'%s\'\n", username);
		if (checkExistence(1, *users, username) == 1) {
			sendCode(sock, EXISTENCE_USERNAME);
		}else {
			sendCode(sock, REGISTER_SUCCESS);
			break;
		}
	}

	readWithCheck(sock, buff, BUFF_SIZE);
	buff[strlen(buff) - 1] = '\0';
	if (buff[strlen(buff) - 2] == '\n') {
		buff[strlen(buff) - 2] = '\0';
	}
	printf("password: %s\n", buff);

	strcpy(password, buff);
	user_struct *user = (user_struct *)malloc(sizeof(user_struct));
	strcpy(user->user_name, username);
	strcpy(user->password, password);
	user->status = 1;
	insertEnd(users, user);
	sendCode(sock, REGISTER_SUCCESS);
}

// Dang nhap checked - OK
int signIn(int sock, singleList users, user_struct **loginUser) {
	char buff[BUFF_SIZE], username[50], password[50];
	sendCode(sock, LOGIN_SUCCESS);

	while (1) {
		readWithCheck(sock, buff, BUFF_SIZE);
		buff[strlen(buff) - 1] = '\0';
		printf("username: %s\n", buff);

		strcpy(username, buff);
		if (checkExistence(1, users, username) == 1) {
			sendCode(sock, LOGIN_SUCCESS);
			break;
		}else {
			sendCode(sock, NON_EXISTENCE_USERNAME);
		}
	}
	readWithCheck(sock, buff, BUFF_SIZE);
	buff[strlen(buff) - 1] = '\0';
	printf("password: %s\n", buff);
	strcpy(password, buff);

	*loginUser = (user_struct *)(findByName(1, users, username));
	if (strcmp((*loginUser)->password, password) == 0) {
		sendCode(sock, LOGIN_SUCCESS);
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		strcpy(cli->name, username);
		cli->sockfd = sock;
		cli->uid = num_client;
		queue_add(cli);
		return 1;
	}
	sendCode(sock, INCORRECT_PASSWORD);
	return 0;
}

// Gui toi cac client khac tru nguoi gui - OK
void send_message(char name[100], char *nameFile) {
	char send_request[REQUEST_SIZE];
	for (int i = 0; i < num_client; i++) {
		if (strcmp(name, clients[i]->name) != 0) {
			sprintf(send_request, "%d*%s", FIND_IMG_IN_USERS, nameFile);
			printf("->send to %s - %s - %s - %s \n", clients[i]->name, name, nameFile, send_request);
			send(clients[i]->sockfd, send_request, sizeof(send_request), 0);
			memset(send_request, '\0', strlen(send_request) + 1);
		}
	}
}

// Gui list danh sach anh cho nguoi yeu cau tim kiem
void send_message_to_sender(char *list_imgs) {
	char send_request[REQUEST_SIZE];
	for (int i = 0; i < num_client; i++) {
		if (strcmp(main_name, clients[i]->name) == 0) {
			sprintf(send_request, "%d*%s", SEND_IMGS_TO_USER, list_imgs);
			send(clients[i]->sockfd, send_request, sizeof(send_request), 0);
			printf("message Send: %s\n", send_request);
			memset(send_request, '\0', strlen(send_request) + 1);
			break;
		}
	}
}

// Ham gui file cho server - OK
void *SendFileToClient(int new_socket, char fname[50]) {
	FILE *fp = fopen(fname, "rb");
	if (fp == NULL) {
		printf("[-]File open error");
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	int n, total = 0;
	char sendline[1024] = {0};
	send(new_socket, &size, sizeof(size), 0);
	while ((n = fread(sendline, sizeof(char), 1024, fp)) > 0) {
		if (n != 1024 && ferror(fp)) {
			perror("[-]Read File Error");
			exit(1);
		}
		if (send(new_socket, sendline, n, 0) == -1) {
			perror("[-]Can't send file");
			exit(1);
		}
		total += n;
		memset(sendline, '\0', 1024);
		if(total >= size) {
			fclose(fp);
			break;
		}
	}
}

// Ham nhan file va ghi file vao thu muc chua - OK
int receiveUploadedFile(int sock, char filePath[100]) {
	int bytesReceived = 0;
	char recvBuff[1024], fname[100], path[100];
	FILE *fp;
	printf("[+]Receiving file...\n");
	fp = fopen(filePath, "wb");
	if (NULL == fp) {
		printf("[-]Error opening file\n");
		return -1;
	}
	int sizeFileRecv;
	recv(sock, &sizeFileRecv, sizeof(sizeFileRecv), 0);
	printf("%d\n", sizeFileRecv);
	ssize_t n;
	int total = 0;
	char buff[1024] = {0};
	while ((n = recv(sock, buff, 1024, 0)) > 0) {
		if (n == -1) {
			perror("[-]Receive File Error");
			exit(1);
		}
		if (fwrite(buff, sizeof(char), n, fp) != n) {
			perror("[-]Write File Error");
			exit(1);
		}
		total += n;
		memset(buff, '\0', 1024);
		if (total >= sizeFileRecv) {
			break;
		}
	}
	printf("\n[+]File OK....Completed\n");
	printf("[+]TOTAL RECV: %d\n", total);
	fclose(fp);
	count_write++;
	return 1;
}

// Ham xu li luong - not check
void *handleThread(void *my_sock) {
	int new_socket = *((int *)my_sock);
	int REQUEST;
	char buff[1024];

	char username[BUFF_SIZE] = {};
	user_struct *loginUser = NULL;

	while (1) {
		readWithCheck(new_socket, buff, 100);
		REQUEST = atoi(buff);
		switch (REQUEST) {
		case REGISTER_REQUEST:
			printf("[+]REGISTER_REQUEST\n");
			signUp(new_socket, &users);
			saveUsers(users);
			break;
		case LOGIN_REQUEST:
			// nhan username va password
			printf("[+]LOGIN_REQUEST\n");
			if (signIn(new_socket, users, &loginUser) == 1) {
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
						printf("user name: %s\n", username);
						filename = strtok(NULL, "*");
						printf("file name: %s\n", filename);
						// gui yeu cau toi cac may con lai
						send_message(username, filename);
						count_send = num_client - 1;
						printf("[+]SEND TO ALL : %s\n", buff);
						memset(username, '\0', sizeof(username) + 1);
						break;
					case FILE_WAS_FOUND:
						username = strtok(NULL, "*");
						printf("[+]FOUND FROM %s\n", username);  
						char file_path[BUFF_SIZE];
						username[strlen(username)-1] = '\0';
						sprintf(file_path, "./files/%s.jpg", username);
						username[strlen(username)] = '\0';
						pthread_mutex_lock(&clients_mutex);
						strcat(list_clients_img, username);
						strcat(list_clients_img, "*");
						pthread_mutex_unlock(&clients_mutex);
						receiveUploadedFile(new_socket, file_path);
						memset(file_path, '\0', strlen(file_path) + 1);
						if(count_send == count_write) {
							send_message_to_sender(list_clients_img);
							printf("LIST IMGS: %s\n", list_clients_img);
							// memset(list_clients_img, '\0', strlen(list_clients_img) + 1);
							memset(main_name, '\0', strlen(main_name) + 1);
							count_send = count_write = 0;
						}
						break;
					case CHOOSEN_USER:
						choosen_user = strtok(NULL, "*");
						printf("CHOOSEN USER: %s\n", choosen_user);
						sprintf(file_path, "./files/%s.jpg", choosen_user);
						SendFileToClient(new_socket, file_path);
						char list_clients_img_copy[1024];
						strcpy(list_clients_img_copy, list_clients_img);
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
						memset(list_clients_img, '\0', strlen(list_clients_img) + 1);
						break;
					case FILE_WAS_NOT_FOUND:
						count_send--;
						break;
					case LOGOUT_REQUEST: // request code: 14
						printf("[+]LOGOUT_REQUEST\n");
						loginUser = NULL;
						sendCode(new_socket, LOGOUT_SUCCESS);
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
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	// Forcefully attaching socket to the port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	char buff[100];
	createSingleList(&users);
	readUserFile(&users);
	while (1) {
		pthread_t tid;

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		printf("New request from sockfd = %d.\n", new_socket);
		pthread_create(&tid, NULL, &handleThread, &new_socket);
	}
	close(server_fd);
	return 0;
}