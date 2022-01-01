
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
// #include <pthread.h>
// #include "./communication_code.h"

#define BUFF_SIZE 100
#define REQUEST_SIZE 1024
#define MAX_CLIENTS 10

void printRequest(char *request);
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length);
int readWithCheck(int sock, char buff[BUFF_SIZE], int length);
void *SendFileToServer(int new_socket, char fname[50]);
int receiveUploadedFile(int sock, char filePath[100]);
void str_trim_lf(char *arr, int length);
void sendCode(int sock, int code);
void clearBuff();

// NOI DUNG HAM DUOI DAY

void printRequest(char *request){
	printf("REQUEST: %s\n", request);
}
//Gui thong diep, khong duoc thong bao loi
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
// Ham gui file cho server - OK
void *SendFile(int new_socket, char fname[50]) {
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

// Ham gui tin hieu tuong ung cho server - OK
void sendCode(int sock, int code) {
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	sendWithCheck(sock, codeStr, strlen(codeStr) + 1);
}

void str_trim_lf(char *arr, int length) {
	int i;
	for (i = 0; i < length; i++) {
		if (arr[i] == '\n') {
			arr[i] = '\0';
			break;
		}
	}
}

void clearBuff() {
	char c;
	while ((c = getchar()) != '\n' && c != EOF) {
	}
}
