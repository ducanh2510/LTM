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
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "transfer.h"
#define BUFF_SIZE 100

GtkWidget *label_alert;
char main_message[100] = "";

void freeVar();

void initApp(UserData *userData);

// ================ CREATE WINDOW =====================
GtkWidget *createWindow(const gint width, const gint height, const gchar *const title);
GtkWidget *create_login_box(GtkWidget *stack, UserData *userData);
GtkWidget *create_stack_box(GtkWidget **stack);
GtkWidget *create_login_grid(GtkWidget *stack, UserData *userData);
GtkWidget *create_register_grid(GtkWidget *stack, UserData *userData);
GtkWidget *create_find_box(GtkWidget *stack, UserData *userData);
GtkWidget *create_find_grid(GtkWidget *stack, UserData *userData);
GtkWidget *create_show_img_downloaded_grid(GtkWidget *stack, UserData *userData);
void create_home_window(UserData *userData);
void initPreLoginScreen(UserData *userData);
void create_show_img_grid(UserData *userData);
void load_css(void);

// ================  =====================
void clicked_clbk(GtkButton *button, GtkStack *stack);
void login_clbk(GtkButton *button, GtkStack *stack);
void main_clbk(GtkButton *button, GtkStack *stack);
void register_clbk(GtkButton *button, GtkStack *stack);
void back_clbk(GtkButton* button, GtkStack *stack);
void find_action_clbk(GtkButton* button, GtkStack *stack);
void show_action_clbk(GtkButton* button, GtkStack *stack);

// ================ THREAD ====================
void recv_msg_handler(UserData *userData);
void *SendFileToServer(int new_socket, char *fname);

// ================ EVENT CLICK =====================
void download_img(GtkButton *button, UserData *userData);
void quit_clbk(GtkButton *button, UserData *userData);
void enter_login(GtkButton *button, UserData *userData);
void enter_signUp(GtkButton *button, UserData *userData);
void find_img(GtkButton *button, UserData *userData);
void exitFind(GtkButton *button, UserData *userData);
void do_not_download(GtkButton *button, UserData *userData);
void quit_sys_clbk(GtkButton *button, UserData *userData);

// ================  =====================
int count_file_in_dir(char *dir_name);
int initSocket(char *ip_address, int port, UserData *userData);

// =========================================
void initApp(UserData *userData) {
    initPreLoginScreen(userData);
}

void freeVar() {

}

// ================ MAIN =====================
int main(int argc, char *argv[]) {

    UserData *userData = (UserData*)malloc(sizeof(UserData));
    userData->screenApp = (ScreenApp*)malloc(sizeof(ScreenApp));
    gtk_init(&argc, &argv);

    if ((userData->sockFd = initSocket("127.0.0.1", 8888, userData)) <= 0)
        return userData->sockFd;

    initApp(userData);
    return 0;
}

// =============================================

// ================ CREATE WINDOW =====================

void initPreLoginScreen(UserData *userData) {
    GtkWidget *window, *login_box, *login_grid, *register_grid, *stack_box, *stack;

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
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_OVER_DOWN_UP);
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
    GtkWidget *box, *login_button, *register_button, *close_button;

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
    g_signal_connect(close_button, "clicked", G_CALLBACK(quit_sys_clbk), userData);

    /// *** Return the Box
    return box;
}

// Dien login o day
GtkWidget *create_login_grid(GtkWidget *stack, UserData *userData) {
    GtkWidget *grid;
    GtkWidget *login_button, *back_button;
    GtkWidget *label_username, *entry_username;
    GtkWidget *label_password, *entry_password;

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
    GtkWidget *register_button, *back_button;
    GtkWidget *label_username, *entry_username;
    GtkWidget *label_password, *entry_password;

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

GtkWidget *create_find_box(GtkWidget *stack, UserData *userData) {
    GtkWidget *box, *findLabel, *find_button,  *exit_button, *logou_button, *show_img;
    GtkWidget *grid, *image;
    
    /// *** Create the Box
    grid = gtk_grid_new();
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    find_button = gtk_button_new_with_label("FIND");
    show_img = gtk_button_new_with_label("SHOW IMAGE");
    logou_button = gtk_button_new_with_label("LOG OUT");
    exit_button = gtk_button_new_with_label("EXIT");
    findLabel = gtk_label_new("WELCOME TO SYSTEM");

    GdkPixbuf *pb;

    pb = gdk_pixbuf_new_from_file("user.png", NULL);
    pb = gdk_pixbuf_scale_simple(pb, 150, 150, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(gdk_pixbuf_copy(pb));
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 1, 1, 1);
    
    gtk_grid_set_row_spacing((GtkGrid *)grid, 10);

    gtk_box_pack_start(GTK_BOX(box), findLabel, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_image_new_from_pixbuf(pb), 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), find_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), show_img, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), logou_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), exit_button, 0, 0, 0);

    g_signal_connect(find_button, "clicked", G_CALLBACK(find_action_clbk), stack);
    g_signal_connect(show_img, "clicked", G_CALLBACK(show_action_clbk), stack);
    g_signal_connect(logou_button, "clicked", G_CALLBACK(exitFind), userData);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(quit_clbk), userData);

    userData->screenApp->findContainer.window = box;

    return box;
}

GtkWidget *create_find_grid(GtkWidget *stack, UserData *userData) {
    GtkWidget *fileNameLabel, *fileNameEntry;
    GtkWidget *findBtn, *backBtn, *show_temp_btn, *label_alert_img;
    GtkWidget *grid;

    grid = gtk_grid_new();
    fileNameLabel = gtk_label_new("File name");
    fileNameEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(fileNameEntry), "Please enter the name of file");

    backBtn = gtk_button_new_with_label("Back");
    label_alert_img = gtk_button_new_with_label("NOT FOUND IMAGE");
    findBtn = gtk_button_new_with_label("Find");
    show_temp_btn = gtk_button_new_with_label("DOWNLOAD IMG");

    // gtk_grid_attach_next_to(GTK_GRID(grid), label_alert_img, NULL, GTK_POS_RIGHT, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(grid), fileNameLabel, NULL, GTK_POS_BOTTOM, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(grid), fileNameEntry, NULL, GTK_POS_BOTTOM, 1, 2);
    gtk_grid_attach_next_to(GTK_GRID(grid), findBtn, NULL, GTK_POS_BOTTOM, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(grid), show_temp_btn, NULL, GTK_POS_BOTTOM, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(grid), backBtn, NULL, GTK_POS_BOTTOM, 1, 1);

    userData->screenApp->findContainer.fileNameEntry = fileNameEntry;
    gtk_grid_set_row_spacing((GtkGrid *)grid, 10);

    g_signal_connect(findBtn, "clicked", G_CALLBACK(find_img), userData);
    g_signal_connect(backBtn, "clicked", G_CALLBACK(back_clbk), stack);
    g_signal_connect(show_temp_btn, "clicked", G_CALLBACK(create_show_img_grid), userData);

    return grid;
}

GtkWidget *create_show_img_downloaded_grid(GtkWidget *stack, UserData *userData) {
    GtkWidget *grid, *image, *image_button, *not_download, *back_btn;
    GdkPixbuf *pb;
    gtk_init(NULL, NULL);
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing((GtkGrid *)grid, 10);
    int k = 0, count_img = 0;
    char list_img_clients[1024] = "";
    back_btn = gtk_button_new_with_label("BACK");
    DIR *d;
    struct dirent *dir;
    d = opendir("./download_imgs/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
            strcat(list_img_clients, dir->d_name);
            strcat(list_img_clients, "*");
            count_img++;
          }
      }
      closedir(d);
    }
    printf("LIST IMGS: %s\tCOUNT: %d\n", list_img_clients, count_img);
    char *fileName = strtok(list_img_clients, "*");
    for (int i = 0; i <= count_img; i++) {
        if(i == count_img) {
            gtk_grid_attach(GTK_GRID(grid), back_btn, count_img, count_img, 1, 1);
            break;
        }
        gchar name[50];
        char file_path[200];
        sprintf(name, "%s%d", "Image", i);
        sprintf(file_path, "./download_imgs/%s", fileName);
        image_button = gtk_button_new_with_label(name);
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
    }
    g_signal_connect(back_btn, "clicked", G_CALLBACK(back_clbk), stack);
    return grid;
}

void create_home_window(UserData *userData) {
    GtkWidget *window, *box, *stack_box, *stack;
    GtkWidget *findLabel, *find_box, *find_grid, *show_grid;
    

    window = createWindow(600, 400, "SHARE IMAGE APPPICATION");
    stack_box = create_stack_box(&stack);
    gtk_container_add(GTK_CONTAINER(window), stack_box);

    find_box = create_find_box(stack, userData);
    find_grid = create_find_grid(stack, userData);
    load_css();
    show_grid = create_show_img_downloaded_grid(stack, userData);

    gtk_stack_add_named(GTK_STACK(stack), find_box, "Main_find");
    gtk_stack_add_named(GTK_STACK(stack), find_grid, "Find");
    gtk_stack_add_named(GTK_STACK(stack), show_grid, "Show");

    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_OVER_DOWN_UP);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 1000);
    gtk_stack_set_interpolate_size(GTK_STACK(stack), TRUE);

    userData->screenApp->findContainer.window = window;

    gtk_window_set_deletable((GtkWindow*)window, FALSE);
    gtk_widget_show_all(window);
}

void create_show_img_grid(UserData *userData) {
    GtkWidget *grid, *window;
    window = createWindow(500, 400, "SHARE IMAGE APPPICATION");
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing((GtkGrid *)grid, 10);
    int k = 0, m = 0, count_img = count_file_in_dir("./temporary_image/");
    char list_img_clients[1024] = "";
    DIR *d;
    struct dirent *dir;
    d = opendir("./temporary_image/");
    if (d) {
      while ((dir = readdir(d)) != NULL) {
          if(dir->d_type == DT_REG) {
            strcat(list_img_clients, dir->d_name);
            strcat(list_img_clients, "*");
          }
      }
      closedir(d);
    }
    char *fileName = strtok(list_img_clients, "*");
    GtkWidget *not_download = gtk_button_new_with_label("I DON'T WANT DOWNLOAD IMG");
    for (int i = 0; i <= count_img; i++) {
        if( i == count_img) {
            gtk_grid_attach(GTK_GRID(grid), not_download, count_img, count_img / 3 + 2, 1, 1);
            break;
        }
        GdkPixbuf *pb;
        GtkWidget *image;
        GtkWidget *image_button;
        gchar *name;
        char file_path[200];
        image_button = gtk_button_new_with_label(fileName);
        sprintf(file_path, "./temporary_image/%s", fileName);
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
        g_signal_connect(not_download, "clicked", G_CALLBACK(do_not_download), userData);
    }  
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_window_set_deletable((GtkWindow*)window, FALSE);
    userData->screenApp->showResultContainer.window = window;
    gtk_widget_show_all(window);
}

// ================ END CREATE WINDOW =====================

// ================ SWITCH ROUTER =====================

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

void register_clbk(GtkButton *button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Register", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void back_clbk(GtkButton* button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Main_find", GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void find_action_clbk(GtkButton* button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Find", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void show_action_clbk(GtkButton* button, GtkStack *stack) {
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Show", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

// ================ END SWITCH ROUTER =====================

// ================ THREAD =====================

void recv_msg_handler(UserData *userData) {
    int REQUEST;
	int sockfd = userData->sockFd;
	char recvReq[1024];
	char sendReq[1024];
	char *fileName;
	while (1) {
		char message[BUFF_SIZE] = {}; 
		int receive = readWithCheck(sockfd, recvReq, REQUEST_SIZE);
        printf("RECV MESSAGE: %s\n", recvReq);
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
				memset(file_path, '\0', strlen(file_path) + 1);
				break;
            case SEND_DONE: 
                printf("Count file : %d\n", count_file_in_dir("./temporary_image/"));
                break;
            case SEND_IMGS_TO_USER:
                fileName = strtok(NULL, "*");
                sprintf(file_path, "./temporary_image/%s.jpg", fileName);
                receiveUploadedFile(sockfd, file_path);
                memset(fileName, '\0', strlen(fileName) + 1);
                break;
			case NO_IMG_FOUND:
				printf(FG_RED "No images found!!!\n" ); 
				break;
			default:
				break;
			}
		}else if (receive == 0) {
		}
	}
}

void *SendFileToServer(int new_socket, char fname[50]) {
	SendFile(new_socket, fname);
}

// ================ END THREAD=====================

// ================ EVENT CLICK =====================

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
        printf("[+]Login success!!!\n");
        create_home_window(userData);
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

void quit_clbk(GtkButton *button, UserData *userData) {
    g_print("GoodBye\n");
    char send_request[1024];
    sprintf(send_request, "%d*%s", LOGOUT_REQUEST, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    readWithCheck(userData->sockFd, send_request, sizeof(send_request));
    if(atoi(send_request) == LOGOUT_SUCCESS) {
        sprintf(send_request, "%d*%s", EXIT_SYS, userData->username);
        sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
        memset(send_request, '\0', strlen(send_request) + 1);
        gtk_main_quit();
        exit(EXIT_SUCCESS);
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
    gtk_widget_show(userData->screenApp->findContainer.window);
    gtk_entry_set_text((GtkEntry*)userData->screenApp->findContainer.fileNameEntry, "");
}

void do_not_download(GtkButton *button, UserData *userData) {
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
    gtk_entry_set_text((GtkEntry*)userData->screenApp->findContainer.fileNameEntry, "");
}

void find_img(GtkButton *button, UserData *userData) {
    char send_request[1024];
    const gchar *message = gtk_entry_get_text((GtkEntry*)userData->screenApp->findContainer.fileNameEntry);
    if(strlen(message) > 0) {
        sprintf(send_request, "%d*%s*%s", FIND_IMG_REQUEST, userData->username, message);
        strcpy(main_message, message);
	    sendWithCheck(userData->sockFd, send_request, strlen(send_request) + 1);
        memset(send_request, '\0', strlen(send_request) + 1);
        gtk_entry_set_text((GtkEntry*)userData->screenApp->findContainer.fileNameEntry, "");
    }
}

void quit_sys_clbk(GtkButton *button, UserData *userData) {
    g_print("GoodBye!!!\n");
    char send_request[1024];
    sprintf(send_request, "%d*%s", EXIT_SYS, userData->username);
    sendWithCheck(userData->sockFd, send_request, sizeof(send_request));
    memset(send_request, '\0', strlen(send_request) + 1);
    gtk_main_quit();
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

// ================ END EVENT CLICK =====================

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