#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "./request_code.h"

#define LENGTH 4096
#define MAX_CLIENTS 100
#define BUFF_SIZE 100

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option)
{
    int sendByte = 0;
    sendByte = send(sock, buff, length, option);
    if (sendByte > 0)
    {
    }
    else
    {
        close(sock);
        pthread_exit(0);
    }
}

void sendCode(int sock, int code){// chuyen int code sang str code roi gui
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	sendWithCheck(sock , codeStr , strlen(codeStr) + 1 , 0 ); 
}


int readWithCheck(int sock, char buff[BUFF_SIZE], int length)
{
    int recvByte = 0;
    recvByte = read(sock, buff, length);
    if (recvByte > 0)
    {
        return recvByte;
    }
    else
    {
        close(sock);
        exit(0);
    }
}

// Tao dau nhay nhap noi dung
void str_overwrite_stdout()
{
    printf("Please enter a name of file > ");
    fflush(stdout);
}

// Xu li dau enter
void str_trim_lf(char *arr, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        if (arr[i] == '\n')
        {
            arr[i] = '\0';
            break;
        }
    }
}

// Ctrl + C va Exit
void catch_ctrl_c_and_exit(int sig)
{
    flag = 1;
}

// Ham gui file anh
void sendfile(char *filepath, int sockfd)
{
    int n;
    FILE *fp = fopen(filepath,"rb");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    send(sockfd, &size, sizeof(size), 0);
    char sendline[LENGTH] = {0};
    while ((n = fread(sendline, sizeof(char), LENGTH, fp)) > 0)
    {
        if (n != LENGTH && ferror(fp))
        {
            perror("Read File Error");
            exit(1);
        }
        if (send(sockfd, sendline, n, 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, LENGTH);
    }

}


// Ham gui tin
void send_msg_handler()
{
    char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};

    while (1)
    {
        str_overwrite_stdout();
        fgets(message, LENGTH, stdin);
        str_trim_lf(message, LENGTH);
        printf("%s\n", message);

        if (strcmp(message, "exit") == 0)
        {
            break;
        }
        else
        {
            // gui ten file o day
            sendCode(sockfd, SEND_FILE_NAME);
            send(sockfd, message, sizeof(message), 0);
        
        }
        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
    }
    catch_ctrl_c_and_exit(2);
}

// Ham xu li nhan tin de gui yeu cau tai anh
void request_download_image_handler(char *s)
{
    char *image_request[MAX_CLIENTS];
    image_request[0] = strtok(s, ";");
    int i = 1;
    while (image_request[i] != NULL)
    {
        image_request[i] = strtok(NULL, s);
        i++;
    }
    printf("Moi chon anh tai ve\n");
    for (int j = 0; j < i - 1; j++)
    {
        printf("%d\t%s\n", j + 1, image_request[j]);
    }
    int option;
    scanf("%d", &option);
    send(sockfd, image_request[option], strlen(image_request[option]), 0); // chua can
}

// Ham nhan tin
void recv_msg_handler()
{
    while (1)
    {
        char message[LENGTH] = {'\0'};
        char res[10];
        int receive = recv(sockfd, res, sizeof(res), 0); // nhận thông điệp tìm kiếm từ server
        if (receive > 0)
        {
            switch (atoi(res))
            {
            case SEND_NAME_SUCCESS:
                /* code */
                if (recv(sockfd, message, sizeof(message), 0) > 0)
                {
                    char file_path[100] = "./";
                    strcat(file_path, message);
                    sendCode(sockfd, SEND_FOUND_USERNAME);
                        send(sockfd, name, strlen(name), 0);   
                        // SendFileToServer(file_path, sockfd);
                        // FILE *fp = fopen(file_path,"rb");
                        sendfile(file_path,sockfd);
                }

                break;

            default:
                break;
            }
        };

    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    signal(SIGINT, catch_ctrl_c_and_exit);

    printf("Please enter your name: ");
    fgets(name, 32, stdin);
    str_trim_lf(name, strlen(name));

    if (strlen(name) > 32 || strlen(name) < 2)
    {
        printf("Name must be less than 30 and more than 2 characters.\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    /* Socket settings */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1)
    {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

    // Send name
    // char send_name_req[4];
    // strcpy(send_name_req, "111");
    // send(sockfd, send_name_req, sizeof(send_name_req), 0);
    sendCode(sockfd, SEND_USERNAME);
    send(sockfd, name, 32, 0); // ok
    printf("=== WELCOME TO THE SHARED APPLICATION ===\n");

    // Luồng gửi dữ liệu
    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *)send_msg_handler, NULL) != 0)
    {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }
    // Luồng nhận thông tin
    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
    {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        if (flag)
        {
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);
    return EXIT_SUCCESS;
}