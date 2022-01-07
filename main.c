#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "./view/include/initScreen.h"

int initSocket(char *ip_address, int port, UserData *userData) {
	int sock = 0;
	struct sockaddr_in serv_addr;

	// Try catch false when connecting
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n [-]Socket creation error \n");
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
    userData->sockFd = sock;
    return userData->sockFd;
}

int main(int argc, char *argv[]) {

    UserData userData;
    ScreenApp screenApp;
    gtk_init(&argc, &argv);

    if ((userData.sockFd = initSocket("127.0.0.1", 5500, &userData)) <= 0)
        return userData.sockFd;

    userData.screenApp = &screenApp;
    initApp(&userData);
    // gtk_widget_show(screenApp.preLoginContainer.window);
    // gtk_main();
    return 0;
}