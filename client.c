#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "./communication_code.h"

#define BUFF_SIZE 100
#define REQUEST_SIZE 1024
char user[100] = "";
int num_c = 0;
int recv_sig = 0;
char user_has_img[10][BUFF_SIZE];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

void printRequest(char *request){
	printf("REQUEST: %s\n", request);
}

// Gui thong diep toi server va check - OK
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int sendByte = 0;
	sendByte = send(sock, buff, length, 0);
	if (sendByte > 0) {
	}else {
		close(sock);
		printf("Connection is interrupted.n");
		exit(0);
	}
}

// Nhan thong diep tu server va check - OK
int readWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int recvByte = 0;
	recvByte = recv(sock, buff, length, 0);
	if (recvByte > 0) {
		return recvByte;
	}else {
		close(sock);
		exit(0);
	}
}

int menu1();
void navigation(int sock);
void signUp(int sock);
int signIn(int sock);
void sendCode(int sock, int code);
void clearBuff();
void str_trim_lf(char *arr, int length);

//=============== MAIN ==================
int main(int argc, char *argv[]) {
	pthread_t thread;
	if (argc != 3) {
		printf("Please input IP address and port number\n");
		return 0;
	}
	char *ip_address = argv[1];
	char *port_number = argv[2];
	int port = atoi(port_number);
	int sock = 0;
	struct sockaddr_in serv_addr;

	// Try catch false when connecting
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
		printf("\n[-]Invalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\n[-]Connection Failed \n");
		return -1;
	}

	// ============================Start to communicate with Server======================
	// ==================================================================================
	do {
		navigation(sock);
	} while (1);

	close(sock);
	return 0;
}

// Menu dang nhap hoac dang ky - OK
int menu1() {
	int choice, catch;
	char err[10];
	printf("\n\n");
	printf("====================UPLOAD FILE IMAGE SHARING===================\n");
	printf("1. Sign up\n");
	printf("2. Sign in\n");
	printf("3. Exit\n");
	printf("==========================================================\n");
	printf("=> Enter your choice: ");
	catch = scanf("%d", &choice);

	printf("\n\n");

	if (catch > 0)
		return choice;
	else
	{
		fgets(err, 10, stdin);
		err[strlen(err) - 1] = '\0';
		printf("\"%s\" is not allowed!\n", err);
		return -1;
	}
}

// Chuc nang dang ky - OK
void signUp(int sock) {
	char username[50], password[50], buff[BUFF_SIZE];

	sendCode(sock, REGISTER_REQUEST);
	readWithCheck(sock, buff, BUFF_SIZE);
	printf("========================= SIGNUP ========================\n");

	clearBuff();
	while (1) {
		printf("Enter username: ");
		fgets(username, 50, stdin);
		while (strlen(username) <= 0 || username[0] == '\n') {
			printf("Username is empty!!!!!!!\n");
			printf("Enter username: ");
			scanf("%s", username);
			str_trim_lf(username, 50);
		}
		sendWithCheck(sock, username, sizeof(username));

		readWithCheck(sock, buff, BUFF_SIZE);
		if (atoi(buff) == EXISTENCE_USERNAME) {
			printf("Username is not available!!\n");
		}else {
			break;
		}
	};

	printf("Enter password: ");
	fgets(password, 50, stdin);
	while (strlen(password) <= 0 || password[0] == '\n') {
		printf("Password is empty!!!!!!!!!\n");
		printf("Enter password: ");
		fgets(password, 50, stdin);
	}
	sendWithCheck(sock, password, sizeof(password));

	readWithCheck(sock, buff, BUFF_SIZE);
	if (atoi(buff) == REGISTER_SUCCESS) {
		printf("Dang ki tai khoan thanh cong\n");
	}
}

// Chuc nang dang nhap - OK
int signIn(int sock) {
	char username[50], password[50], buff[BUFF_SIZE];
	sendCode(sock, LOGIN_REQUEST);
	readWithCheck(sock, buff, BUFF_SIZE);
	printf("========================= SIGNIN ========================\n");

	clearBuff();
	while (1){
		printf("Enter username: ");
		fgets(username, 50, stdin);
		while (strlen(username) <= 0 || username[0] == '\n'){
			username[0] = '\0';
			printf("Username is empty!!!!\n");
			printf("Enter username: ");
			fgets(username, 50, stdin);
		}

		sendWithCheck(sock, username, sizeof(username));

		readWithCheck(sock, buff, BUFF_SIZE);
		if (atoi(buff) == NON_EXISTENCE_USERNAME) {
			printf("Username is not available!!\n");
		}else {
			break;
		}
	}

	printf("Enter password: ");
	fgets(password, 50, stdin);
	while (strlen(password) <= 0 || password[0] == '\n'){
		printf("Password is empty!!!!!\n");
		printf("Enter password: ");
		fgets(password, 50, stdin);
	}
	sendWithCheck(sock, password, sizeof(password) + 1);
	readWithCheck(sock, buff, BUFF_SIZE);
	if (atoi(buff) != LOGIN_SUCCESS) {
		printf("[-]Login failed!!\n");
		return 0;
	}else {
		strcpy(user, username);
		return 1;
	}
}

// Ham gui tin hieu tuong ung cho server - OK
void sendCode(int sock, int code) {
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	sendWithCheck(sock, codeStr, strlen(codeStr) + 1);
}

// Xu li dau enter - OK
void str_trim_lf(char *arr, int length) {
	int i;
	for (i = 0; i < length; i++) {
		if (arr[i] == '\n') {
			arr[i] = '\0';
			break;
		}
	}
}

// Chua biet lam gi - not check
void clearBuff() {
	char c;
	while ((c = getchar()) != '\n' && c != EOF) {
	}
}

// Ham gui file cho server - OK
void *SendFileToServer(int new_socket, char fname[50]) {
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
	return 1;
}

int findByUsername(char username[100]) {
	for(int i = 0; i < num_c; i++) {
		if(strstr(user_has_img[i], username)) {
			return i;
		}
	}
}

// Ham xu li gui yeu cau tim kiem cua client cho server - not checked
void send_msg_handler(int *sock) {
	int sockfd = *sock;
	char buffer[100];
	char send_request[REQUEST_SIZE];
	while (1) {
		if(recv_sig == 0) {
			printf("Please enter a name of file > ");
			fflush(stdout);
			fgets(buffer, 100, stdin);
			str_trim_lf(buffer, 100);
			str_trim_lf(user, 100);
			if(strstr(buffer, "exit")) {
				break;
			}
			if(strlen(buffer) > 0) {
				sprintf(send_request, "%d*%s*%s", FIND_IMG_REQUEST, user, buffer);
				sendWithCheck(sockfd, send_request, strlen(send_request) + 1);
				bzero(buffer, 100);
				recv_sig = 1;
				memset(send_request, '\0', strlen(send_request) + 1);
			}
		}
	}
}

// Ham xu li nhan yeu cau cua server phia client - not checked
void recv_msg_handler(int *sock) {
	int REQUEST;
	int sockfd = *sock;
	char recvReq[REQUEST_SIZE];
	char sendReq[REQUEST_SIZE];
	int choose;
	char *fileName, *list_imgs;
	while (1) {
		char message[BUFF_SIZE] = {}; 
		int receive = readWithCheck(sockfd, recvReq, REQUEST_SIZE);
		printf("\n[+}FIND_IMG_IN_USERS : %s \n", recvReq);
		char *opcode;
		opcode = strtok(recvReq, "*");
		if (receive > 0) {
			REQUEST = atoi(opcode);
			switch (REQUEST) {
			case FIND_IMG_IN_USERS:
				fileName = strtok(NULL, "*");
				char file_path[200];
				strcpy(file_path, "./");
				strcat(file_path, fileName);
				printf("\n[+]FILENAME_TO_SEARCH : %s \n", fileName);
				// neu tim thay:
				if(access(file_path, F_OK) != -1) {
					sprintf(sendReq, "%d*%s", FILE_WAS_FOUND, user);
					send(sockfd, sendReq, sizeof(sendReq), 0);
					printf("[+]FILE_WAS_FOUND\n");
					SendFileToServer(sockfd, file_path);
					printf("[+]SEND FILE DONE\n");
				}else {
					sprintf(sendReq, "%d*%s", FILE_WAS_NOT_FOUND, user);
					send(sockfd, sendReq, sizeof(sendReq), 0);
					memset(sendReq, '\0', strlen(sendReq) + 1);
				}
				memset(file_path, '\0', strlen(file_path) + 1);
				printf("Please enter a name of file > ");
				fflush(stdout);
				break;
			case SEND_IMGS_TO_USER:
				list_imgs = strtok(NULL, "*");
				while(list_imgs != NULL) {
					strcpy(user_has_img[num_c], list_imgs);
					num_c++;
					list_imgs = strtok(NULL, "*");
				}
				printf("Danh sach ten client co anh\n");
				for(int j = 0; j < num_c; j++) {
					printf("%d\t%s\n", j + 1, user_has_img[j]);
				} 
				printf("Moi chon anh tai ve:\t");
				fflush(stdout);
				scanf("%d", &choose);
				fflush(stdout);
				printf("%d\n", choose);
				sprintf(sendReq, "%d*%s", CHOOSEN_USER, user_has_img[choose - 1]);
				printf("CHOOSEN USER: %s\n", sendReq);
				send(sockfd, sendReq, sizeof(sendReq), 0);
				sprintf(file_path, "./%s.jpg", user_has_img[choose - 1]);
				receiveUploadedFile(sockfd, file_path);
				memset(sendReq, '\0', strlen(sendReq) + 1);
				recv_sig = 0;
				break;
			default:
				break;
			}
		}else if (receive == 0) {
		}
	}
}

void navigation(int sock) {
	int opt1, opt2, opt3;
	char buffer[100];
	opt1 = menu1();

	switch (opt1) {
	case 1: // Dang ki
		signUp(sock);
		break;
	case 2: // Dang nhap
		if (signIn(sock) == 1) {

			printf("=== WELCOME TO THE SHARED IMAGE APPLICATION ===\n");
			pthread_t send_msg_thread;
			if (pthread_create(&send_msg_thread, NULL, (void *)send_msg_handler, &sock) != 0) {
				printf("[-]ERROR: pthread\n");
				exit(EXIT_FAILURE);
			}

			pthread_t recv_msg_thread;
			if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, &sock) != 0) {
				printf("[-]ERROR: pthread\n");
				exit(EXIT_FAILURE);
			}

			pthread_mutex_lock(&mutex);
			while(recv_sig == 1) {
				pthread_cond_wait(&cond, &mutex);
			}
			pthread_mutex_unlock(&mutex);
			pthread_join(recv_msg_thread, NULL);
		}
		break;
	case 3:
		exit(1);
	default:
		break;
	}
}