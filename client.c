#include <stdio.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "appScreen.h"
#include "communication_code.h"
#include "transfer.h"
#define BUFF_SIZE 100

GtkWidget *label_alert;
char list_img_clients[1024] = "";
static int send_done = 0;
char main_message[100] = "";

void initApp(UserData *userData);

GtkWidget *createWindow(const gint width, const gint height, const gchar *const title);
GtkWidget *create_login_box(GtkWidget *stack, UserData *userData);
GtkWidget *create_stack_box(GtkWidget **stack);
GtkWidget *create_login_grid(GtkWidget *stack, UserData *userData);
GtkWidget *create_register_grid(GtkWidget *stack, UserData *userData);

void create_find_window(UserData *userData);
void create_show_img_grid(UserData *userData);

void login_clbk(GtkButton *button, GtkStack *stack);
void main_clbk(GtkButton *button, GtkStack *stack);
void register_clbk(GtkButton *button, GtkStack *stack);

void recv_msg_handler(UserData *userData);
void *SendFileToServer(int new_socket, char *fname);

void clicked_clbk(GtkButton *button, GtkStack *stack);
void download_img(GtkButton *button, UserData *userData);
void load_css(void);
void quit_clbk(GtkButton *button, UserData *userData);
void enter_login(GtkButton *button, UserData *userData);
void enter_signUp(GtkButton *button, UserData *userData);
int count_file_in_dir(char *dir_name);

void find_img(GtkButton *button, UserData *userData);
void exitFind(GtkButton *button, UserData *userData);

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

    UserData *userData = (UserData*)malloc(sizeof(UserData));
    ScreenApp screenApp;
    gtk_init(&argc, &argv);

    if ((userData->sockFd = initSocket("127.0.0.1", 5500, userData)) <= 0)
        return userData->sockFd;

    userData->screenApp = &screenApp;
    initApp(userData);
    return 0;
}

// =============================================

void initPreLoginScreen(UserData *userData) {

    GtkWidget *window;
    GtkWidget *login_box;  // 3 nutg
    GtkWidget *login_grid; //
    GtkWidget *register_grid;

    GtkWidget *stack_box;
    GtkWidget *stack;

    // gtk_init(NULL, NULL);
    load_css();

    /// *** Create a Window
    window = createWindow(600, 400, "SHARE IMAGE APPPICATION");

    /// *** Create the Stack Box
    stack_box = create_stack_box(&stack);
    gtk_container_add(GTK_CONTAINER(window), stack_box);

    /// ***
    login_box = create_login_box(stack, userData);
    login_grid = create_login_grid(stack, userData);
    register_grid = create_register_grid(stack, userData);

    /// ***
    gtk_stack_add_named(GTK_STACK(stack), login_box, "Main");
    gtk_stack_add_named(GTK_STACK(stack), login_grid, "Login");
    gtk_stack_add_named(GTK_STACK(stack), register_grid, "Register");

    /// ***
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 1000);
    gtk_stack_set_interpolate_size(GTK_STACK(stack), TRUE);
    userData->screenApp->preLoginContainer.window = window;

    /// ***
    gtk_widget_show_all(window);
    gtk_widget_hide(userData->screenApp->preLoginContainer.label_alert);
    gtk_window_set_deletable((GtkWindow*)window, FALSE);

    gtk_main();
}

// Load css - OK
void load_css(void) {
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;
    /// ***
    const gchar *css_style_file = "style.css";
    GFile *css_fp = g_file_new_for_path(css_style_file);
    GError *error = 0;
    /// ***
    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    /// ***
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_file(provider, css_fp, &error);
    /// ***
}

// Tao cua so moi
GtkWidget *createWindow(const gint width, const gint height, const gchar *const title) {
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    g_signal_connect(window, "destroy", gtk_main_quit, window);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    gtk_container_set_border_width(GTK_CONTAINER(window), 50);
    return window;
}

// Tao 3 box co 3 nut dau tien
GtkWidget *create_login_box(GtkWidget *stack, UserData *userData) {
    GtkWidget *box;
    GtkWidget *login_button;
    GtkWidget *register_button;
    GtkWidget *close_button;

    /// *** Create the Box
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    /// *** Create the Buttons
    login_button = gtk_button_new_with_label("Login");
    register_button = gtk_button_new_with_label("Register");
    close_button = gtk_button_new_with_label("EXIT");

    /// *** Add them to the Box
    gtk_box_pack_start(GTK_BOX(box), login_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), register_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), close_button, 0, 0, 0);

    /// ***
    g_signal_connect(login_button, "clicked", G_CALLBACK(login_clbk), stack);
    g_signal_connect(register_button, "clicked", G_CALLBACK(register_clbk), stack);
    g_signal_connect(close_button, "clicked", G_CALLBACK(quit_clbk), userData);

    /// *** Return the Box
    return box;
}

// Dien login o day
GtkWidget *create_login_grid(GtkWidget *stack, UserData *userData) {
    GtkWidget *grid;
    GtkWidget *login_button;
    GtkWidget *back_button;

    GtkWidget *label_username;
    GtkWidget *entry_username;

    GtkWidget *label_password;
    GtkWidget *entry_password;

    /// *** Create the Grid
    grid = gtk_grid_new();

    /// ***
    label_username = gtk_label_new("Username:");
    label_password = gtk_label_new("Password:");
    label_alert = gtk_label_new("User name or Password incorrect!!!");

    /// ***
    entry_username = gtk_entry_new();
    userData->screenApp->preLoginContainer.login_user_name = entry_username;
    entry_password = gtk_entry_new();
    gtk_entry_set_visibility((GtkEntry *)entry_password, FALSE);
    userData->screenApp->preLoginContainer.login_password = entry_password;
    userData->screenApp->preLoginContainer.label_alert = label_alert;

    /// ***
    login_button = gtk_button_new_with_label("Login");
    back_button = gtk_button_new_with_label("Go to Main");

    /// ***
    gtk_grid_attach(GTK_GRID(grid), label_username, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_username, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), back_button, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), login_button, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_alert, 0, 3, 1, 1);

    // gtk_widget_hide(label_alert);

    gtk_grid_set_row_spacing((GtkGrid *)grid, 10);
    /// ***
    g_signal_connect(back_button, "clicked", G_CALLBACK(main_clbk), stack);
    g_signal_connect(login_button, "clicked", G_CALLBACK(enter_login), userData);
    /// ***
    return grid;
}

GtkWidget *create_register_grid(GtkWidget *stack, UserData *userData) {
    GtkWidget *grid;
    GtkWidget *register_button;
    GtkWidget *back_button;

    GtkWidget *label_username;
    GtkWidget *entry_username;

    GtkWidget *label_password;
    GtkWidget *entry_password;

    /// *** Create the Grid
    grid = gtk_grid_new();

    /// ***
    label_username = gtk_label_new("Username: ");
    label_password = gtk_label_new("Password:");

    /// ***
    entry_username = gtk_entry_new();
    entry_password = gtk_entry_new();
    gtk_entry_set_visibility((GtkEntry *)entry_password, FALSE);
    userData->screenApp->preLoginContainer.register_user_name = entry_username;
    userData->screenApp->preLoginContainer.register_password = entry_password;
    userData->screenApp->preLoginContainer.label_alert = label_alert;

    /// ***
    register_button = gtk_button_new_with_label("Register");
    back_button = gtk_button_new_with_label("Back to Main");

    /// ***
    gtk_grid_attach(GTK_GRID(grid), label_username, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_username, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), back_button, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), register_button, 1, 6, 1, 1);

    /// ***
    g_signal_connect(back_button, "clicked", G_CALLBACK(main_clbk), stack);
    g_signal_connect(register_button, "clicked", G_CALLBACK(enter_signUp), userData);

    /// ***
    return grid;
}

// Tao stack
GtkWidget *create_stack_box(GtkWidget **stack) {
    GtkWidget *box;

    /// *** Create the Box
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    /// *** Create a Stack
    *stack = gtk_stack_new();
    gtk_widget_set_halign(*stack, GTK_ALIGN_CENTER);
    gtk_box_set_center_widget(GTK_BOX(box), *stack);

    /// ***
    return box;
}

void create_find_window(UserData *userData) {

    GtkWidget *window;
    GtkWidget *fileNameLabel;
    GtkWidget *fileNameEntry;
    GtkWidget *box;
    GtkWidget *findBtn;
    GtkWidget *exitBtn;
    load_css();
    window = createWindow(500, 400, "SHARE IMAGE APPPICATION");
    fileNameLabel = gtk_label_new("File name");
    fileNameEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(fileNameEntry), "Please enter the name of file");

    userData->screenApp->findContainer.fileNameLabel = fileNameLabel;
    userData->screenApp->findContainer.fileNameEntry = fileNameEntry;

    findBtn = gtk_button_new_with_label("Find");
    exitBtn = gtk_button_new_with_label("LOG_OUT");

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_box_pack_start(GTK_BOX(box), fileNameLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), fileNameEntry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), findBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), exitBtn, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), box);
    userData->screenApp->findContainer.window = window;
    gtk_widget_show_all(window);
    gtk_window_set_deletable((GtkWindow*)window, FALSE);
    g_signal_connect(findBtn,"clicked",G_CALLBACK(find_img), userData);
    g_signal_connect(exitBtn, "clicked", G_CALLBACK(exitFind), userData);
}

int count_file_in_dir(char *dir_name) {
    int count = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir("./temporary_image/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
            count++;
          }
      }
      closedir(d);
    }
    return count;
}

void create_show_img_grid(UserData *userData) {
    GtkWidget *grid, *window;
    window = createWindow(500, 400, "SHARE IMAGE APPPICATION");
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing((GtkGrid *)grid, 10);
    int k = 0, m = 0, count_img = count_file_in_dir("./temporary_image/");
    char file_path[200];
    printf("LIST CREAT : %s\n", list_img_clients);
    char *fileName = strtok(list_img_clients, "*");
    GtkWidget *not_download = gtk_button_new_with_label("I DON'T WANT DOWNLOAD IMG");
    for (int i = 0; i <= count_img; i++) {
        if( i == count_img) {
            gtk_grid_attach(GTK_GRID(grid), not_download, count_img, count_img, 1, 1);
            break;
        }
        GdkPixbuf *pb;
        GtkWidget *image;
        GtkWidget *image_button;
        gchar name[50];
        sprintf(name, "%s", fileName);
        image_button = gtk_button_new_with_label(name);
        strcpy(file_path, "");
        sprintf(file_path, "./temporary_image/%s.jpg", fileName);
        pb = gdk_pixbuf_new_from_file(file_path, NULL);
        pb = gdk_pixbuf_scale_simple(pb, 100, 100, GDK_INTERP_BILINEAR);
        image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb);
        gtk_grid_attach(GTK_GRID(grid), image, k, (i / 3) * 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), image_button, k, (i / 3) * 2 + 1, 1, 1);
        if (k == 2)
            k = 0;
        else
            k++;
        
        fileName = strtok(NULL, "*");
        g_signal_connect(image_button, "clicked", G_CALLBACK(download_img), userData);
    }  
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_window_set_deletable((GtkWindow*)window, FALSE);
    userData->screenApp->showResultContainer.window = window;
    gtk_widget_show_all(window);
    memset(list_img_clients, '\0', strlen(list_img_clients) + 1);
}

void main_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Main", GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void login_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Login", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void enter_login(GtkButton *button, UserData *userData) {

    const gchar *userNameData = gtk_entry_get_text((GtkEntry *)userData->screenApp->preLoginContainer.login_user_name);
    const gchar *pass = gtk_entry_get_text((GtkEntry *)userData->screenApp->preLoginContainer.login_password);
    char sign_in_request[1024];
    char buff[BUFF_SIZE];
    printf("[+] USERNAME: %s\t PASSWORD: %s\n", userNameData, pass);
    sprintf(sign_in_request, "%d*%s*%s", LOGIN_REQUEST, userNameData, pass);
    sendWithCheck(userData->sockFd, sign_in_request, sizeof(sign_in_request));
    readWithCheck(userData->sockFd, buff, BUFF_SIZE);
    if (atoi(buff) != LOGIN_SUCCESS) {
        gtk_widget_show(userData->screenApp->preLoginContainer.label_alert);
        printf("[-]Login failed!!\n");
        return;
    }else {
        strcpy(userData->username, userNameData);
        gtk_widget_hide(userData->screenApp->preLoginContainer.window);
        create_find_window(userData);
        printf("[+]Login success!!!\n");
        pthread_t recv_msg_thread;			
		if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, userData) != 0) {
			printf("[-]ERROR: pthread\n");
			exit(EXIT_FAILURE);
		}
    }
}

void enter_signUp(GtkButton *button, UserData *userData) {
    const gchar *userNameData = gtk_entry_get_text((GtkEntry *)userData->screenApp->preLoginContainer.register_user_name);
    const gchar *pass = gtk_entry_get_text((GtkEntry *)userData->screenApp->preLoginContainer.register_password);

    char sign_in_request[1024];
    char buff[BUFF_SIZE];
    printf("USERNAME: %s\t PASSWORD: %s\n", userNameData, pass);
    sprintf(sign_in_request, "%d*%s*%s", REGISTER_REQUEST, userNameData, pass);
    sendWithCheck(userData->sockFd, sign_in_request, sizeof(sign_in_request));
    readWithCheck(userData->sockFd, buff, BUFF_SIZE);
    if (atoi(buff) == REGISTER_SUCCESS) {
        printf(FG_GREEN "\n[+]Successful account registration!!!\n");
        return;
    }else if(atoi(buff) == EXISTENCE_USERNAME) {
        printf(FG_GREEN "\n[+]Account registration failed!!!\n");
        return;
    }
}

void register_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Register", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void quit_clbk(GtkButton *button, UserData *userData) {
    g_print("GoodBye\n");
    sendCode(userData->sockFd, EXIT_SYS);
    close(userData->sockFd);
    gtk_main_quit();
}

void recv_msg_handler(UserData *userData) {
    int REQUEST;
	int sockfd = userData->sockFd;
	char recvReq[1024];
	char sendReq[1024];
	char *fileName, *num_cl;
	while (1) {
		char message[BUFF_SIZE] = {}; 
		int receive = readWithCheck(sockfd, recvReq, REQUEST_SIZE);
		if(receive <= 0) {
            return;
        }
        char *opcode;
		opcode = strtok(recvReq, "*");
		if (receive > 0) {
			REQUEST = atoi(opcode);
			switch (REQUEST) {
			case FIND_IMG_IN_USERS:
				printf(FG_GREEN "\n[+]FIND_IMG_IN_USERS\n");
				fileName = strtok(NULL, "*");
				char file_path[200];
				strcpy(file_path, "./client_file/");
				strcat(file_path, fileName);
				printf(FG_GREEN"[+]FILENAME_TO_SEARCH : %s \n"NORMAL, fileName);
				// neu tim thay:
				if(access(file_path, F_OK) != -1) {
					sprintf(sendReq, "%d*%s", FILE_WAS_FOUND, userData->username);
					send(sockfd, sendReq, sizeof(sendReq), 0);
					printf(FG_GREEN"[+]FILE_WAS_FOUND\n"NORMAL);
					SendFileToServer(sockfd, file_path);
					printf(FG_GREEN"[+]SEND FILE DONE\n"NORMAL);
				}else {
					sprintf(sendReq, "%d*%s", FILE_WAS_NOT_FOUND, userData->username);
					send(sockfd, sendReq, sizeof(sendReq), 0);
					memset(sendReq, '\0', strlen(sendReq) + 1);
				}
				memset(recvReq, '\0', strlen(recvReq) + 1);
				memset(file_path, '\0', strlen(file_path) + 1);
				break;
            case SEND_IMGS_TO_USER:
                fileName = strtok(NULL, "*");
                sprintf(file_path, "./temporary_image/%s.jpg", fileName);
                receiveUploadedFile(sockfd, file_path);
                strcat(list_img_clients, fileName);
                strcat(list_img_clients, "*");
                break;
            case SEND_DONE: 
                printf("Count file : %d\n", count_file_in_dir("./temporary_image/"));
                send_done = 1;
                break;
			case NO_IMG_FOUND:
				memset(recvReq, '\0', strlen(recvReq) + 1);
				printf(FG_RED "No images found!!!\n" ); 
				break;
			default:
				break;
			}
		}else if (receive == 0) {
		}
	}
}

void download_img(GtkButton *button, UserData *userData) {
    const gchar *text = gtk_button_get_label(button);
    char file_path1[1024], file_path2[1024];
    sprintf(file_path1, "./download_imgs/%s_%s", text, main_message);
    sprintf(file_path2, "./temporary_image/%s.jpg", text);
    FILE *fp2 = fopen(file_path2, "r");
    FILE *fp1 = fopen(file_path1, "w");
    char buff[1024];
    while(fread(buff, sizeof(char), 1024, fp2) > 0) {
        fwrite(buff, sizeof(char), 1024, fp1);
    }
    fclose(fp1);
    fclose(fp2);
    DIR *d;
    struct dirent *dir;
    d = opendir("./temporary_image/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
              char file_path[1024];
              sprintf(file_path, "./temporary_image/%s", dir->d_name);
            if(remove(file_path) == 0){
		    	printf("[+] DELETED FILE SUCCESS: %s\n", file_path);
		    }else{
		    	printf("[+] DELETED FILE FAILED: %s\n", file_path);
		    }
          }
      }
      closedir(d);
    }
    gtk_widget_hide(userData->screenApp->showResultContainer.window);
    // userData->screenApp->showResultContainer.window = NULL;
    gtk_entry_set_text((GtkEntry*)userData->screenApp->findContainer.fileNameEntry, "");
}

void *SendFileToServer(int new_socket, char fname[50]) {
	SendFile(new_socket, fname);
}

void find_img(GtkButton *button, UserData *userData) {
    char send_request[1024];
    const gchar *message = gtk_entry_get_text((GtkEntry*)userData->screenApp->findContainer.fileNameEntry);
    sprintf(send_request, "%d*%s*%s", FIND_IMG_REQUEST, userData->username, message);
    strcpy(main_message, message);
	sendWithCheck(userData->sockFd, send_request, strlen(send_request) + 1);
    memset(send_request, '\0', strlen(send_request) + 1);
    while(1) { 
        if(send_done == 1) {
            printf("Da vao while\n");
            create_show_img_grid(userData);
            send_done = 0;
            break;
        }
    }
    // gtk_widget_hide(userData->screenApp->findContainer.window);
}

void exitFind(GtkButton *button, UserData *userData) {
    char send_request[1024];
    sprintf(send_request, "%d*%s", LOGOUT_REQUEST, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    readWithCheck(userData->sockFd, send_request, sizeof(send_request));
    if(atoi(send_request) == LOGOUT_SUCCESS) {
        gtk_widget_show(userData->screenApp->preLoginContainer.window);
        gtk_widget_hide(userData->screenApp->findContainer.window);
        gtk_entry_set_text((GtkEntry*)userData->screenApp->preLoginContainer.login_user_name, "");
        gtk_entry_set_text((GtkEntry*)userData->screenApp->preLoginContainer.login_password, "");
    }
}

void initApp(UserData *userData) {
    printf("init\n");
    initPreLoginScreen(userData);
}