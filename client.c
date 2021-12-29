#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "./communication_code.h"

#define BUFF_SIZE 100
char user[100] = "";

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

//
int printAvailableElements(char str[1000], char available_elements[20][50]) {
	char *token;
	int number_of_available_elements = 0;
	/* get the first token */
	token = strtok(str, "+");

	/* walk through other tokens */
	while (token != NULL) {
		printf("%d. %s\n", number_of_available_elements + 1, token);
		strcpy(available_elements[number_of_available_elements], token);
		token = strtok(NULL, "+");
		number_of_available_elements++;
	}
	return number_of_available_elements;
}

int menu1();
int menu2();
int menu3(char group_name[50]);
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
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed \n");
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

// Menu cac chuc nang cua chuong trinh - co the bo
int menu2() {
	int choice, catch;
	char err[10];
	printf("\n\n");
	printf("========================= ACTION ========================\n");
	printf("1. Search Image\n");
	printf("2. Logout\n");
	printf("=========================================================\n");
	printf("=> Enter your choice: ");
	catch = scanf("%d", &choice);
	fflush(stdin);

	printf("\n\n");
	if (catch > 0)
		return choice;
	else {
		fgets(err, 10, stdin);
		err[strlen(err) - 1] = '\0';
		printf("\"%s\" is not allowed!\n", err);
		return -1;
	}
}

// Menu chuc nang chuong trinh 2 - co the bo
int menu3(char group_name[50]) {
	int choice, catch;
	char err[10];
	printf("\n\n");
	printf("========================== %s ========================\n", group_name);
	printf("1. Upload\n");
	printf("2. Download\n");
	printf("3. Delete file\n");
	printf("4. View all files\n");
	printf("5. Kick\n");
	printf("6. Back\n");
	printf("==========================================================\n");
	printf("=> Enter your choice: ");
	catch = scanf("%d", &choice);

	printf("\n\n");

	if (catch > 0)
		return choice;
	else {
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
		printf("Login failed!!\n");
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
		printf("File open error");
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	int n;
	char sendline[1024] = {0};
	send(new_socket, &size, sizeof(size), 0);
	while ((n = fread(sendline, sizeof(char), 1024, fp)) > 0) {
		if (n != 1024 && ferror(fp)) {
			perror("Read File Error");
			exit(1);
		}
		if (send(new_socket, sendline, n, 0) == -1) {
			perror("Can't send file");
			exit(1);
		}
		memset(sendline, 0, 1024);
	}
	fclose(fp);
}

// Ham xu li gui yeu cau tim kiem cua client cho server - not checked
void send_msg_handler(int *sock) {
	int sockfd = *sock;
	char buffer[100];

	while (1) {
		sendCode(sockfd, FIND_IMG_REQUEST);
		sendWithCheck(sockfd, user, strlen(user) + 1);
		printf("Please enter a name of file > ");
		scanf("%s", buffer);
		str_trim_lf(buffer, 100);
		sendWithCheck(sockfd, buffer, strlen(buffer) + 1);
		bzero(buffer, 100);
	}
}

// Ham xu li nhan yeu cau cua server phia client - not checked
void recv_msg_handler(int *sock) {
	int REQUEST;
	int sockfd = *sock;
	while (1) {
		char message[BUFF_SIZE] = {}, fileName[BUFF_SIZE] = {'\0'}; 
		int receive = readWithCheck(sockfd, message, BUFF_SIZE);
		if (receive > 0) {
			REQUEST = atoi(message);
			switch (REQUEST) {
			case FIND_IMG_IN_USERS:
				readWithCheck(sockfd, fileName, sizeof(fileName));
				char file_path[200];
				strcpy(file_path, "./");
				strcat(file_path, fileName);
				printf("PATH: %s\n", file_path);
				// Khong vao duoc file
				if (access(file_path, F_OK) != -1) {
					sendCode(sockfd, FILE_WAS_FOUND);
					sendWithCheck(sockfd, user, strlen(user) + 1);
					SendFileToServer(sockfd, file_path);
				}
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
			pthread_t recv_msg_thread;
			if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, &sock) != 0) {
				printf("ERROR: pthread\n");
				exit(EXIT_FAILURE);
			}

			send_msg_handler(&sock);
		}
		break;
	case 3:
		exit(1);
	default:
		break;
	}
}