#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include "./request_code.h"

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define MAX_LINE 4096
#define BUFF_SIZE 100

static _Atomic unsigned int cli_count = 0;
static int uid = 10;
static int count_send = 0;  //??
static int count_write = 0; //??

typedef struct
{ // Them moi
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[32];
} client_t;
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

void sendCode(int sock, int code)
{ // chuyen int code sang str code roi gui
    char codeStr[10];
    sprintf(codeStr, "%d", code);
    sendWithCheck(sock, codeStr, strlen(codeStr) + 1, 0);
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

// Mang cac client
client_t *clients[MAX_CLIENTS];

// Mang ten cac client
char result[MAX_CLIENTS][30];

// Tong so client
int result_num = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Them moi

void str_overwrite_stdout()
{
    printf("\r%s", "> ");
    fflush(stdout);
}

// xu li dau enter
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

// In ra địa chỉ của client
void print_client_addr(struct sockaddr_in addr)
{
    printf("%d.%d.%d.%d",
           addr.sin_addr.s_addr & 0xff,
           (addr.sin_addr.s_addr & 0xff00) >> 8,
           (addr.sin_addr.s_addr & 0xff0000) >> 16,
           (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

// Them client vao mang cac client, mang nay chua cac client dang ket noi
void queue_add(client_t *cl)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (!clients[i])
        {
            clients[i] = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// Kiem tra xem name co trong danh sach client dang ket noi khong
int isInListClients(char *s)
{
    for (int i = 0; i < cli_count; i++)
    {
        if (strstr(clients[i]->name, s) != NULL)
        {
            return 1;
        }
    }
    return 0;
}

// in danh sach ket qua tra ve cho client
void print_result()
{
    printf("->List Result: \n");
    for (int i = 0; i <= result_num; i++)
        printf("%s\n", result[i]);
}

// Them tem may vao danh sach ket qua tra ve
void result_add(char *s)
{
    pthread_mutex_lock(&clients_mutex);

    strcpy(result[result_num], s);
    result_num++;

    pthread_mutex_unlock(&clients_mutex);
}

// xoa client ket noi/ khi client ngat ket noi
// xoa kieu nay on khong nhi??
void queue_remove(int uid)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// Ghi vao file anh tren server
void writefile(int sockfd, FILE *fp)
{
    printf("[+]write file begin\n");
    ssize_t n;
    int total = 0, sizeFileRecv;
    char buff[MAX_LINE] = {0};
    recv(sockfd, &sizeFileRecv, sizeof(sizeFileRecv), 0);
    printf("Send %d\n", sizeFileRecv);
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0)
    {
        if (n + total > sizeFileRecv)
        {
            n = sizeFileRecv - total;
        }
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }
        if (fwrite(buff, 1, n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        total += n;
        memset(buff, 0, MAX_LINE);
        if (total == sizeFileRecv)
            break;
    }
    printf("[+] Write file done!!");
    fclose(fp);
}


// Gui cho tat ca client khac tru ME
void send_message(char *s, int uid)
{
    pthread_mutex_lock(&clients_mutex);
    count_send = 0;
    for (int i = 0; i < cli_count; ++i)
    {
        if (clients[i]->uid == uid)
            printf("->send from %s to\n", clients[i]->name);
    }
    printf("clients: %d", cli_count);
    for (int i = 0; i < cli_count; ++i)
    {
        if (clients[i])
        {
            if (clients[i]->uid != uid && count_send < cli_count)
            {
                if (send(clients[i]->sockfd, s, strlen(s), 0) < 0)
                {
                    perror("ERROR: write to descriptor failed");
                    break;
                }
                count_send++;
                printf("[+]%s\n", clients[i]->name);
            }
        }
    }
    printf("[+]send all done!!\n");

    pthread_mutex_unlock(&clients_mutex);
}
// Gui s cho client ma yeu cau
void send_message_to_sender(char *s, int uid)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < cli_count; ++i)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {
                if (send(clients[i]->sockfd, s, strlen(s), 0) < 0)
                {
                    perror("ERROR: write to descriptor failed");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

/* Handle all communication with the client */
// Moi client ket noi vao server se xu li nhung cong viec sau
void *handle_client(void *arg)
{
    char buff_out[BUFFER_SZ];
    char name[32]; // Ten client
    int leave_flag = 0;
    cli_count++; // Tong so client
    client_t *cli = (client_t *)arg;
    char req[4];

    // Nhận tên , lưu tên
    recv(cli->sockfd, req, sizeof(req), 0);
    printf("->code: %s\n", req); // 111
    if (recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) < 2 || strlen(name) >= 32 - 1)
    {
        printf("Didn't enter the name.\n");
        leave_flag = 1;
    }
    else
    {
        strcpy(cli->name, name);
        sprintf(buff_out, "%s has joined\n", cli->name);
        printf("%s", buff_out);
    }

    bzero(buff_out, BUFFER_SZ);
    char image_to_client[1024];

    while (1)
    {
        char filename[50];
        char req_str[10];
        int REQUEST;
        if (leave_flag)
        {
            break; // neu thay tin hieu roi thi roi
        }

        recv(cli->sockfd, req_str, sizeof(req_str), 0);
        REQUEST = atoi(req_str);

        switch (REQUEST)
        {
        case SEND_FILE_NAME:
            printf("opcode: %d from %s\n", REQUEST, cli->name);
            // xu li ten file gui len de tim kiem
            buff_out[0] = '\0';
            if (recv(cli->sockfd, buff_out, sizeof(buff_out), 0) > 0)
            {
                strcpy(filename, buff_out);
                printf("->find file: %s all users\n", filename);
            }
            char send_name_sucsess[4];
            strcpy(send_name_sucsess, "115");
            send_message(send_name_sucsess, cli->uid);
            send_message(buff_out, cli->uid);
            if (strlen(buff_out) > 0)
            {

                memset(buff_out, 0, sizeof(buff_out));
            }

            break;
        case SEND_FOUND_USERNAME:
            printf("opcode: %d from %s\n", REQUEST, cli->name);
            memset(buff_out, 0, sizeof(buff_out));
            // printf("->find file: %s all users\n", filename);
            buff_out[0] = '\0';
            // xu li ten usser ma co file can tim kiem
            recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
            char file_path[200] = "./buff/";
            // strcat(file_path, filename);
            strcat(file_path, "_");
            strcat(file_path, buff_out);
            strcat(file_path, ".jpg");
            printf("->filename: %s\n", file_path);
            // tao string chua cac client chua mang ve client
            strcpy(image_to_client, "_");
            strcat(image_to_client, buff_out);
            strcat(image_to_client, ".jpg");
            strcat(image_to_client, ";");
            FILE *fp = fopen(file_path, "ab");
            // ghi vao file
            writefile(cli->sockfd, fp);
            // receiveUploadedFile(cli->sockfd, fp);
            // printf("%d\t-\t%d\n", count_write, count_send);
            str_trim_lf(buff_out, strlen(buff_out));
            memset(buff_out, 0, sizeof(buff_out));

            break;
        case 110: // k xulinua
            // xuli luong tai file len
            break;
        case 111: // da xu li roi
        // xu li ten user dang nhap
        default:
            break;
        }

        bzero(buff_out, BUFFER_SZ);
    }

    /* Delete client from queue and yield thread */
    close(cli->sockfd);
    queue_remove(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
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
    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    /* Ignore pipe signals */
    signal(SIGPIPE, SIG_IGN);

    if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&option, sizeof(option)) < 0)
    {
        perror("ERROR: setsockopt failed");
        return EXIT_FAILURE;
    }

    /* Bind */
    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    /* Listen */
    if (listen(listenfd, 10) < 0)
    {
        perror("ERROR: Socket listening failed");
        return EXIT_FAILURE;
    }

    printf("===  WELCOME TO THE SHARED APPLICATION ===\n");
    while (1)
    {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &clilen);

        /* Check if max clients is reached */
        if ((cli_count + 1) == MAX_CLIENTS)
        {
            printf("Max clients reached. Rejected: ");
            print_client_addr(cli_addr);
            printf(":%d\n", cli_addr.sin_port);
            close(connfd);
            continue;
        }

        /* Client settings */
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;

        /* Add client to the queue and fork thread */
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void *)cli);

        /* Reduce CPU usage */
        sleep(1);
    }

    return EXIT_SUCCESS;
}
