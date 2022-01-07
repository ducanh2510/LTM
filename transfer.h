#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include "colorCode.h"

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
ssize_t getpasswd (char **pw, size_t sz, int mask, FILE *fp);

// NOI DUNG HAM DUOI DAY

// In ra thong tin thong diep request - OK
void printRequest(char *request){
	printf(FG_GREEN "[+]REQUEST: %s\n" NORMAL, request);
}

//Gui thong diep, khong duoc thong bao loi - OK
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int sendByte = 0;
	sendByte = send(sock, buff, length, 0);
	if (sendByte > 0) {
	}else {
		printf("[-]Connection is interrupted\n");
		exit(0);
	}
}

// Nhan thong diep tu server va check - OK
int readWithCheck(int sock, char buff[BUFF_SIZE], int length) {
	int recvByte = 0;
	recvByte = recv(sock, buff, length, 0);
	if (recvByte > 0) {
		return recvByte;
	}
}

// Ham gui file cho server - OK
void *SendFile(int new_socket, char *fname) {
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
	printf(FG_GREEN "[+]Receiving file..." NORMAL "\n");
	fp = fopen(filePath, "wb");
	if (NULL == fp) {
		printf("[-]Error opening file\n");
		return -1;
	}
	int sizeFileRecv;
	recv(sock, &sizeFileRecv, sizeof(sizeFileRecv), 0);
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
	printf(FG_GREEN "[+]File OK....Completed" NORMAL "\n");
	printf(FG_GREEN "[+]TOTAL RECV: %d\n" NORMAL, total);
	fclose(fp);
	return 1;
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

void clearBuff() {
	char c;
	while ((c = getchar()) != '\n' && c != EOF) {
	}
}

ssize_t getpasswd (char **pw, size_t sz, int mask, FILE *fp) {
    if (!pw || !sz || !fp) return -1; 
#ifdef MAXPW
    if (sz > MAXPW) sz = MAXPW;
#endif

    if (*pw == NULL) {      
        void *tmp = realloc (*pw, sz * sizeof **pw);
        if (!tmp)
            return -1;
        memset (tmp, 0, sz);  
        *pw =  (char*) tmp;
    }

    size_t idx = 0; 
    int c = 0;

    struct termios old_kbd_mode;   
    struct termios new_kbd_mode;

    if (tcgetattr (0, &old_kbd_mode)) { 
        fprintf (stderr, "%s() error: tcgetattr failed.\n", __func__);
        return -1;
    }   /* copy old to new */
    memcpy (&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    if (tcsetattr (0, TCSANOW, &new_kbd_mode)) {
        fprintf (stderr, "%s() error: tcsetattr failed.\n", __func__);
        return -1;
    }

    while (((c = fgetc (fp)) != '\n' && c != EOF && idx < sz - 1) ||
            (idx == sz - 1 && c == 127))
    {
        if (c != 127) {
            if (31 < mask && mask < 127)  
                fputc (mask, stdout);
            (*pw)[idx++] = c;
        }
        else if (idx > 0) {   
            if (31 < mask && mask < 127) {
                fputc (0x8, stdout);
                fputc (' ', stdout);
                fputc (0x8, stdout);
            }
            (*pw)[--idx] = 0;
        }
    }
    (*pw)[idx] = 0; 

    if (tcsetattr (0, TCSANOW, &old_kbd_mode)) {
        fprintf (stderr, "%s() error: tcsetattr failed.\n", __func__);
        return -1;
    }

    if (idx == sz - 1 && c != '\n')
        fprintf (stderr, " (%s() warning: truncated at %zu chars.)\n",
                __func__, sz - 1);

    return idx; 
}