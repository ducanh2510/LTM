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

singleList groups, files, users;

// ========================================

// Ham gui thong diep cho client va check - OK
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length){
	int sendByte = 0;
	sendByte = send(sock, buff, length, 0);
	if(sendByte > 0){
		
	}else{
		close(sock);
		pthread_exit(0);
	}
}

// Ham nhan thong diep tu client va check - OK
int readWithCheck(int sock, char buff[BUFF_SIZE], int length){
	int recvByte = 0;
	recvByte = read(sock, buff, length);
	if(recvByte > 0){
		return recvByte;
	}else{
		close(sock);
		pthread_exit(0);
	}
}

// ========================================

// Doc file chua thong tin user roi luu vao danh sach lien ket userList - OK
void readUserFile(singleList* users){
	char username[50], password[50], group_name[50];
	int status;
	FILE * f = fopen("./storage/user.txt","r");

	if(f == NULL) {
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		char c = fgetc(f);
    	if (c != EOF){
			int res = fseek(f, -1, SEEK_CUR);
		}else {
        	break;
		}

		fgets(username, 50, f);
		username[strlen(username) -1 ] = '\0';

		fgets(password, 50, f);
		password[strlen(password) -1 ] = '\0';

		fscanf(f,"%d\n", &status);

		user_struct *user = (user_struct*)malloc(sizeof(user_struct));
		strcpy(user->user_name, username);
		strcpy(user->password, password);
		user->status = status;
		insertEnd(users, user);
	}
    fclose(f);

}

// ok mot nua
int checkExistence(int type, singleList list, char string[50]) {
	// type = 1 check user
	// type = 2 check group
	// type = 3 check file
	switch (type) {
	// Check user 
	case 1: {
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL) {
				i++;
				if(strcmp(((user_struct*)list.cur->element)->user_name,string) != 0) {
					list.cur = list.cur->next;
				}else {
					return 1;
				}
				
			}
			return 0; 
		}
		break;

	default:
		printf("Type chua hop le !! (1,2 or 3)\n");
		break;
	}
}

// ok mot nua
void* findByName(int type, singleList list, char string[50]) {
	// type = 1 find user
	// type = 2 find group
	// type = 3 find file
	switch (type)
	{
	case 1:
		// Check user
		{
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL)
			{
				i++;
				if(strcmp(((user_struct*)list.cur->element)->user_name,string) != 0)
				{
					list.cur = list.cur->next;
				}
				else {
					return list.cur->element;
				}
				
			}
			return NULL; 
		}
		break;
	default:
		printf("Type chua hop le !! (1,2 or 3)\n");
		break;
	}
}

// not check
void convertSimpleFilesToString(singleList simple_file, char str[1000]){
	str[0] = '\0';
	simple_file.cur = simple_file.root;
	while(simple_file.cur != NULL)
  	{
		strcat(str, ((simple_file_struct*)simple_file.cur->element)->file_name);
		if(simple_file.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
    	simple_file.cur = simple_file.cur->next;
  	}
}

// not check 
void convertSimpleUsersToString(singleList simple_user, char str[1000]){
	str[0] = '\0';
	simple_user.cur = simple_user.root;
	while(simple_user.cur != NULL)
  	{
		strcat(str, ((simple_user_struct*)simple_user.cur->element)->user_name);
		if(simple_user.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
    	simple_user.cur = simple_user.cur->next;
  	}
}

// Gui tin hieu CODE tuong ung cho client - OK
void sendCode(int sock, int code){
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	printf("-->Response: %s\n", codeStr);
	sendWithCheck(sock , codeStr , strlen(codeStr) + 1); 
}

// not check
singleList getFilesOwns(singleList files, char username[50]){
	singleList files_owns;
	createSingleList(&files_owns);
	files.cur = files.root;
	while (files.cur != NULL)
	{
		if(strcmp(  ((file_struct*)files.cur->element)->owner, username) == 0){
			simple_file_struct *file_element = (simple_file_struct*) malloc(sizeof(simple_file_struct));
			strcpy(file_element->file_name, ((file_struct*)files.cur->element)->name);
			insertEnd(&files_owns, file_element);
		}
		files.cur = files.cur->next;
	}
	return files_owns;
}

// not check
int updateDownloadedTimes(singleList files, char file_name[50]){
	files.cur = files.root;
	while (files.cur != NULL)
	{
		if( strcmp( ((file_struct*)files.cur->element)->name, file_name) == 0 ){
			((file_struct*)files.cur->element)->downloaded_times += 1;
		}
		files.cur = files.cur->next;
	}
}

// Dang ky checked - OK
void signUp(int sock, singleList *users){
	char buff[BUFF_SIZE], username[50], password[50];
	int size;
	sendCode(sock, REGISTER_SUCCESS);

	while(1){
		size = readWithCheck(sock, buff, BUFF_SIZE);

		strcpy(username, buff);
		username[strlen(username) - 1] = '\0';
		if(username[strlen(username) - 2] == '\n') {
			username[strlen(username) - 2] = '\0';
		}
		printf("username: \'%s\'\n", username);
		if(checkExistence(1, *users, username) == 1) {
			sendCode(sock, EXISTENCE_USERNAME);
		}else {
			sendCode(sock, REGISTER_SUCCESS);
			break;
		}
	}

	readWithCheck(sock, buff, BUFF_SIZE);
	buff[strlen(buff) - 1] = '\0';
	if(buff[strlen(buff) - 2] == '\n'){
		buff[strlen(buff) - 2] = '\0';
	}
	printf("password: %s\n", buff);
	
	strcpy(password, buff);
	user_struct *user = (user_struct*)malloc(sizeof(user_struct));
	strcpy(user->user_name, username);
	strcpy(user->password, password);
	user->status = 1;
	insertEnd(users, user);
	sendCode(sock, REGISTER_SUCCESS);
}

// Dang nhap checked - OK
int signIn(int sock, singleList users, user_struct **loginUser){
	char buff[BUFF_SIZE], username[50], password[50];

	sendCode(sock, LOGIN_SUCCESS);

	while(1){
		readWithCheck(sock, buff, BUFF_SIZE);
		buff[strlen(buff) - 1] = '\0';
		printf("username: %s\n", buff);

		strcpy(username, buff);
		if(checkExistence(1, users, username) == 1){
			sendCode(sock, LOGIN_SUCCESS);
			break;
		}else{
			sendCode(sock, NON_EXISTENCE_USERNAME);
		}
	}
	readWithCheck(sock, buff, BUFF_SIZE);
	buff[strlen(buff) - 1] = '\0';
	printf("password: %s\n", buff);
	strcpy(password, buff);

	*loginUser = (user_struct*)(findByName(1, users, username));
	if(strcmp((*loginUser)->password, password) == 0){
		sendCode(sock, LOGIN_SUCCESS);
		return 1;
	}
	sendCode(sock, INCORRECT_PASSWORD);
	return 0;
}

// Gui toi cac client khac tru nguoi gui - not check
void send_message(int sock, char *s, singleList list, char username[50]) {
	list.cur = list.root;
	while (list.cur != NULL) {
		if(strcmp(((user_struct*)list.cur->element)->user_name, username) != 0) {
			sendWithCheck(sock, s, strlen(s) + 1);
			list.cur = list.cur->next;
		}else {
			continue;
		}
		
	}
}

// Ham nhan file va ghi file vao thu muc chua - OK
int receiveUploadedFile(int sock, char filePath[100]){
	int bytesReceived = 0;
	char recvBuff[1024], fname[100], path[100];
	FILE *fp;
	printf("Receiving file...\n");
	fp = fopen(filePath, "wb"); 
	if(NULL == fp) {
		printf("Error opening file\n");
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
            perror("Receive File Error");
            exit(1);
        }
		// if(n + total > sizeFileRecv) {
		// 	n = sizeFileRecv - total;
		// }
        if (fwrite(buff, sizeof(char), n, fp) != n) {
            perror("Write File Error");
            exit(1);
        }
		total += n;
        memset(buff, 0, 1024);
		if(total >= sizeFileRecv) {
			break;
		}
    }
	printf("\nFile OK....Completed\n");
	printf("TOTAL RECV: %d\n", total);
	fclose(fp);
	return 1;
}

// Ham xu li luong - not check
void *handleThread(void *my_sock){
	int new_socket = *((int *)my_sock);
	int REQUEST;
	char buff[BUFF_SIZE];
	char username[BUFF_SIZE] = {};
	user_struct *loginUser = NULL;

	while(1){
		readWithCheck( new_socket , buff, 100);
		REQUEST = atoi(buff);
			switch (REQUEST) {
				case REGISTER_REQUEST:
					printf("REGISTER_REQUEST\n");
					signUp(new_socket, &users);
					saveUsers(users);
					break;
				case LOGIN_REQUEST:
					// nhan username va password
					printf("LOGIN_REQUEST\n");
					if(signIn(new_socket, users, &loginUser) == 1){
						while(REQUEST != LOGOUT_REQUEST){
							readWithCheck( new_socket , buff, BUFF_SIZE);
							REQUEST = atoi(buff);
							switch (REQUEST) {
							case FIND_IMG_REQUEST:
								readWithCheck(new_socket, buff, sizeof(buff));
								strcpy(username, buff);
								readWithCheck(new_socket, buff, sizeof(buff));
								if(strstr(buff, "exit")) {

								}else {
									// gui yeu cau toi cac may con lai
									send_message(new_socket, buff, users, username);
									readWithCheck(new_socket, buff, sizeof(buff));
									char file_path[BUFF_SIZE];
									file_path[0] = '\0';
									strcpy(file_path, "./files/");
									strcat(file_path, buff);
									strcat(file_path, ".jpg");
									receiveUploadedFile(new_socket, file_path);
									// memset(buff, 0, strlen(buff) + 1);
								}
								break;
							case LOGOUT_REQUEST: //request code: 14
								printf("LOGOUT_REQUEST\n");
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
	if(argc == 1){
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
	address.sin_port = htons( port ); 
	
	// Forcefully attaching socket to the port 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)  { 
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
	while(1){
		pthread_t tid; 

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		printf("New request from sockfd = %d.\n",new_socket);	
        pthread_create(&tid, NULL, &handleThread, &new_socket);
    }
	close(server_fd);
	return 0; 
} 