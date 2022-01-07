#include <gtk/gtk.h>
#include <pthread.h>
GtkWidget *window;
GtkWidget *find_window;

GtkWidget *createWindow(const gint width, const gint height, const gchar *const title);
GtkWidget *create_login_box(GtkWidget *stack);
GtkWidget *create_stack_box(GtkWidget **stack);
GtkWidget *create_login_grid(GtkWidget *stack);
GtkWidget *create_register_grid(GtkWidget *stack);

GtkWidget *create_find_window();

void login_clbk(GtkButton *button, GtkStack *stack);
void main_clbk(GtkButton *button, GtkStack *stack);
void register_clbk(GtkButton *button, GtkStack *stack);

void clicked_clbk(GtkButton *button, GtkStack *stack);
void load_css(void);
void quit_clbk(void);
void enter_login(GtkButton *button, GtkWidget *entry_username);

void myThread(int *k)
{
    int i = 50;
    while(i>0){
        printf("%d ", i--);
    }
    printf("\ndone thread!!! %d \n", *k);
}

int main(void)
{

    GtkWidget *login_box;
    GtkWidget *login_grid;
    GtkWidget *register_grid;

    GtkWidget *stack_box;
    GtkWidget *stack;

    /// ***
    gtk_init(NULL, NULL);
    load_css();

    /// *** Create a Window
    window = createWindow(300, 300, "SHARE IMAGE APPPICATION");
    find_window = create_find_window();

    /// *** Create the Stack Box
    stack_box = create_stack_box(&stack);
    gtk_container_add(GTK_CONTAINER(window), stack_box);

    /// ***
    login_box = create_login_box(stack);
    login_grid = create_login_grid(stack);
    register_grid = create_register_grid(stack);

    /// ***
    gtk_stack_add_named(GTK_STACK(stack), login_box, "Main");
    gtk_stack_add_named(GTK_STACK(stack), login_grid, "Login");
    gtk_stack_add_named(GTK_STACK(stack), register_grid, "Register");

    /// ***
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 1000);
    gtk_stack_set_interpolate_size(GTK_STACK(stack), TRUE);

    /// ***
    gtk_widget_show_all(window);
    gtk_main();
}

void load_css(void)
{
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

GtkWidget *createWindow(const gint width, const gint height, const gchar *const title)
{
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    g_signal_connect(window, "destroy", gtk_main_quit, window);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    gtk_container_set_border_width(GTK_CONTAINER(window), 50);
    return window;
}

// tao 3 box co 3 nut dau tien
GtkWidget *create_login_box(GtkWidget *stack)
{
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
    g_signal_connect(close_button, "clicked", G_CALLBACK(quit_clbk), NULL);

    /// *** Return the Box
    return box;
}

GtkWidget *create_login_grid(GtkWidget *stack) // dien login o day
{
    GtkWidget *grid;
    GtkWidget *login_button;
    GtkWidget *back_button;

    GtkWidget *label_username;
    GtkWidget *entry_username;

    GtkWidget *label_password;
    GtkWidget *entry_password;

    GtkWidget *label_alert;

    /// *** Create the Grid
    grid = gtk_grid_new();

    /// ***
    label_username = gtk_label_new("Username:");
    label_password = gtk_label_new("Password:");
    label_alert = gtk_label_new("User name or Password incorrect!!!");
    

    /// ***
    entry_username = gtk_entry_new();
    entry_password = gtk_entry_new();

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
    gtk_grid_set_row_spacing((GtkGrid*) grid, 10);

        /// ***
        g_signal_connect(back_button, "clicked", G_CALLBACK(main_clbk), stack);
        g_signal_connect(login_button, "clicked", G_CALLBACK(enter_login), entry_username);

    /// ***
    return grid;
}

GtkWidget *create_register_grid(GtkWidget *stack)
{
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

    /// ***
    return grid;
}
// tao stack
GtkWidget *create_stack_box(GtkWidget **stack)
{
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

GtkWidget *create_find_window()
{
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Find IMG");
    g_signal_connect(window, "destroy", gtk_main_quit, window);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    gtk_container_set_border_width(GTK_CONTAINER(window), 50);
    return window;
}

void main_clbk(GtkButton *button, GtkStack *stack)
{
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Main", GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void login_clbk(GtkButton *button, GtkStack *stack)
{
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Login", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void enter_login(GtkButton *button, GtkWidget *entry_username)
{
    const gchar *userNameData = gtk_entry_get_text((GtkEntry*) entry_username); 
    //   const gchar *passData = gtk_entry_get_text(GTK_ENTRY(entry_password)); 
    pthread_t my_thread;
            int k =3;

			if (pthread_create(&my_thread, NULL, (void *)myThread, &k) != 0) {
				printf("[-]ERROR: pthread\n");
				exit(EXIT_FAILURE);
			}
			
      printf("%s\n", userNameData);
    gtk_widget_hide(window);
    gtk_widget_show_all(find_window);


}

void register_clbk(GtkButton *button, GtkStack *stack)
{
    g_return_if_fail(GTK_IS_BUTTON(button));
    g_return_if_fail(GTK_IS_STACK(stack));

    gtk_stack_set_visible_child_full(stack, "Register", GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    g_print("Switching to %s.\n", gtk_stack_get_visible_child_name(stack));
}

void quit_clbk(void)
{
    g_print("GoodBye\n");
    gtk_main_quit();
}