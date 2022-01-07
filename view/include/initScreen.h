#include "appScreen.h"
#include "../../communication_code.h"
#include "../../transfer.h"
#define BUFF_SIZE 100

GtkWidget *label_alert;

GtkWidget *createWindow(const gint width, const gint height, const gchar *const title);
GtkWidget *create_login_box(GtkWidget *stack, UserData *userData);
GtkWidget *create_stack_box(GtkWidget **stack);
GtkWidget *create_login_grid(GtkWidget *stack, UserData *userData);
GtkWidget *create_register_grid(GtkWidget *stack, UserData *userData);

void create_find_window(UserData *userData);

void login_clbk(GtkButton *button, GtkStack *stack);
void main_clbk(GtkButton *button, GtkStack *stack);
void register_clbk(GtkButton *button, GtkStack *stack);

void clicked_clbk(GtkButton *button, GtkStack *stack);
void load_css(void);
void quit_clbk(GtkButton *button, UserData *userData);
void enter_login(GtkButton *button, UserData *userData);
void enter_signUp(GtkButton *button, UserData *userData);

void find_img(GtkButton *button, UserData *userData);
void exitFind(GtkButton *button, UserData *userData);

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
    close_button = gtk_button_new_with_label("Close");

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
    exitBtn = gtk_button_new_with_label("Exit");

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_box_pack_start(GTK_BOX(box), fileNameLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), fileNameEntry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), findBtn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), exitBtn, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), box);
    userData->screenApp->findContainer.window = window;
    gtk_widget_show_all(window);
    g_signal_connect(findBtn,"clicked",G_CALLBACK(find_img),userData);
    g_signal_connect(exitBtn, "clicked", G_CALLBACK(exitFind), userData);
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
        printf("Name: %s\n", userData->username);
        gtk_widget_hide(userData->screenApp->preLoginContainer.window);
        create_find_window(userData);
        printf("[+]Login success!!!\n");
        return;
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

void find_img(GtkButton *button, UserData *userData) {
    // char send_request[1024];
    // sprintf(send_request, "%d*%s*%s", FIND_IMG_REQUEST, userData->username, userData->screenApp->findContainer.fileNameEntry);
	// sendWithCheck(userData->sockFd, send_request, strlen(send_request) + 1);
    // memset(send_request, '\0', strlen(send_request) + 1);
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