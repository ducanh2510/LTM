#include <gtk/gtk.h>

typedef struct{
    GtkWidget *window;
    GtkWidget *label_alert;
    GtkWidget *login_user_name;
    GtkWidget *login_password;
    GtkWidget *register_user_name;
    GtkWidget *register_password;
} PreLoginContainer;

typedef struct{
    GtkWidget *window;
    GtkWidget *fileNameLabel;
    GtkWidget *fileNameEntry;
    GtkWidget *box;
} FindContainer;

typedef struct{
    GtkWidget *window;
} ShowResultContainer;

typedef struct{
    GtkWidget *window;
} ShowResourcesContainer;

typedef struct {
    PreLoginContainer preLoginContainer;
    FindContainer findContainer;
    ShowResultContainer showResultContainer;
    ShowResourcesContainer showResourcesContainer;
} ScreenApp;

typedef struct{
    ScreenApp *screenApp;
    GtkWidget *stack_box;
    int sockFd;
    char username[50];
} UserData;