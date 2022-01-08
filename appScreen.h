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

typedef struct {
    PreLoginContainer preLoginContainer;
    FindContainer findContainer;
    ShowResultContainer showResultContainer;
} ScreenApp;

typedef struct{
    ScreenApp *screenApp;
    int sockFd;
    char username[50];
} UserData;