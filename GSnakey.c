/********************************************************************************
* GSnakey v0.7 - GSnakey.c                                                      *
*                                                                               *
* Copyright (C) 2011 Anil Motilal Mahtani Mirchandani(anil.mmm@gmail.com)       *
*                                                                               *
* License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html> *
* This is free software: you are free to change and redistribute it.            *
* There is NO WARRANTY, to the extent permitted by law.                         *
*                                                                               *
*********************************************************************************/

#include <cairo.h>
#include <math.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gdk/gdkkeysyms.h>

#define ROW     24
#define COL     51

#define HEIGHT  ROW*12
#define WIDTH   COL*12

#define IN_ROW  12
#define IN_COL  25

#define WALL    '#'
#define SPACE   '.'
#define FOOD    '@'
#define RIGHT   'R'
#define LEFT    'L'
#define UP      'U'
#define DOWN    'D'
#define TAIL    'T'
#define STOP    'S'

#define invalid(x) ((x) == RIGHT || \
                    (x) == LEFT || \
                    (x) == UP || \
                    (x) == DOWN || \
                    (x) == TAIL || \
                    (x) == WALL)

#define issnake(x) ((x) == RIGHT || \
                    (x) == LEFT || \
                    (x) == UP || \
                    (x) == DOWN || \
                    (x) == TAIL)

char *applogo = "................................"
                "................................"
                "................................"
                "................................"
                "................................"
                "............***********........."
                "...........*###########*........"
                ".........*###############*......"
                ".......*####**********####*....."
                "......*####*..........**##*....."
                ".....*####*.............**......"
                ".....*####*....................."
                ".....*####**...................."
                "......*#####**.................."
                ".......**#####****.............."
                ".........*########***..........."
                "..........****#######**........."
                "..............***######**......."
                "......**.........***#####*......"
                ".....*##*...........**####*....."
                ".....*###**...........*####*...."
                ".....*#####*..........*####*...."
                "......**####*.........*####*...."
                "........*####********#####*....."
                ".........**############***......"
                "...........**########**........."
                ".............********..........."
                "................................"
                "................................"
                "................................"
                "................................"
                "................................";
                
int sr = IN_ROW;
int sc = IN_COL;
int remain = 0;

int tlapse = 400;
int step = 10;

char grid[ROW*COL];

int ibuf = 0;
int nbuf = 0;

char dirbuf[32];
int paused = 0;
int ended = 0;

int points = 0;
int keysig = -1;
int timesig = -1;
int init = 0;

char mapname[13] = "maps/map.000";
GtkWidget *darea, *window, *pointsbar, *statusbar;

void init_game();

enum
{
      COL_PIXBUF,
      NUM_COLS
};

static void
put_pixel (GdkPixbuf *pixbuf, int x, int y, guchar red, guchar green, guchar blue)
{
      int rowstride, n_channels;
      guchar *pixels, *p;
      
      n_channels = gdk_pixbuf_get_n_channels (pixbuf);
      
      rowstride = gdk_pixbuf_get_rowstride (pixbuf);
      pixels = gdk_pixbuf_get_pixels (pixbuf);
      p = pixels + y * rowstride + x * n_channels;
      
      p[0] = red;
      p[1] = green;
      p[2] = blue;
}

void draw_square(GdkPixbuf *pixbuf, int x, int y, guchar red, guchar green, guchar blue)
{
    int i, j;

    for(i = x; i < x + 2; i++){
        for(j = y; j < y + 2;  j++){
            put_pixel(pixbuf, i, j, red, green, blue);
        }
    }
}

GtkTreeModel *create_and_fill_model (void)
{
    int i, j, k, count = 0;
    char pixgrid[COL*ROW];
    char name[13];
    DIR *dh;
    struct dirent *file;
    GtkListStore *list_store;
    GdkPixbuf *pic;
    GtkTreeIter iter;

    list_store = gtk_list_store_new (NUM_COLS, GDK_TYPE_PIXBUF);

    dh = opendir("maps");

    while((file = readdir(dh))) {
        if (strlen(file->d_name) == 7) {
            if(strncmp(file->d_name, "map.", 4) == 0) {
                count++;
            }
        }
    }

    closedir(dh);
    
    for (k = 0; k < count; k++) {
        pic = gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,(COL + 1)*2,(ROW + 1)*2);
        sprintf(name, "maps/map.%03d", k);
        load_map(name, &pixgrid);
        
        for (i = 0; i < (ROW + 1)*2; i++) {
            put_pixel(pic, 0, i, 0, 0, 0);
            put_pixel(pic, COL*2 + 1, i, 0, 0, 0);
        }
        
        for (i = 0; i < (COL + 1)*2; i++) {
            put_pixel(pic, i, 0, 0, 0, 0);
            put_pixel(pic, i, ROW*2 + 1, 0, 0, 0);
        }
        
        for (i = 0; i < ROW; i++) {
            for (j = 0; j < COL; j++) {
                if(pixgrid[i*COL+j] == WALL) {
                    draw_square(pic, j*2 + 1, i*2 + 1, 255, 0, 0);
                } else {
                    draw_square(pic, j*2 + 1, i*2 + 1, 255, 255, 255);
                }
            }
        }
        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter, COL_PIXBUF, pic, -1);
    }
    
    return GTK_TREE_MODEL (list_store);
}


static void destroy (GtkWidget *window, gpointer data)
{
    gtk_main_quit ();
}
char inverse(char move)
{
    switch(move) {
    case RIGHT:
        return LEFT;
    case LEFT:
        return RIGHT;
    case UP:
        return DOWN;
    case DOWN:
        return UP;
    }
}

void draw_grid (GtkWidget *widget)
{
    cairo_t *cr;
    int i, j;

    cr = gdk_cairo_create (widget->window);
    
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, WIDTH, HEIGHT);
    cairo_fill(cr);

    for (i = 0; i < ROW; i++) {
        for (j = 0; j < COL; j++) {
            switch (grid[COL*i + j]) {
            case WALL:
                cairo_set_source_rgb(cr, 1, 0, 0);
                break;
            case FOOD:
                cairo_set_source_rgb(cr, 0, 1, 0);
                break;
            case RIGHT:
            case LEFT:
            case UP:
            case DOWN:
            case TAIL:
                cairo_set_source_rgb(cr, 0, 0, 0);
                break;
            case SPACE:
                continue;
            }
            
            cairo_rectangle(cr, j*12+ 1, i*12+ 1, 10, 10);
            cairo_fill(cr);
        }
    }
    
    cairo_destroy(cr);
}

static gboolean key_press (GtkWidget *widget, 
                           GdkEventKey *event,
                           GtkWidget *data)
{
    int id;
    char move;

    switch(event->keyval) {
    case GDK_Left:
        move = LEFT;
        break;
    case GDK_Up:
        move = UP;
        break;
    case GDK_Right:
        move = RIGHT;
        break;
    case GDK_Down:
        move = DOWN;
        break;
    case GDK_space:
        paused = !paused;
        
        if (ended != 1) {
            id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Info");
            gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
            
            if (paused == 1) {
                gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, "Paused");
            }
        }
        goto kp_exit;
    case GDK_Escape:
        init_game();
        gtk_widget_queue_draw(widget);
    default:
        goto kp_exit;
    }

    if (paused == 1 || ended == 1) {
        goto kp_exit;
    }
    
    if (init == 0) {
        if (move == UP) {
            sr -= 2;
            grid[sr*COL + sc] = DOWN;
            grid[(sr + 1)*COL + sc] = DOWN;
            grid[(sr + 2)*COL + sc] = TAIL;
        }
        init = 1;
    }
    
    if (nbuf < 32) {
        if (nbuf > 0) {
            if (dirbuf[(ibuf + nbuf + 31) % 32] == move ||
                dirbuf[(ibuf + nbuf + 31) % 32] == inverse(move)) {
                goto kp_exit;
            }
        }else {
            if (grid[sr*COL + sc] == move) {
                goto kp_exit;
            }
        }
        dirbuf[(ibuf + nbuf) % 32] = move;
        nbuf++;
    } 
    
kp_exit:
    return FALSE;
}

static gboolean expose (GtkWidget *widget,
                        GdkEventExpose *event,
                        gpointer data)
{
    draw_grid(widget);
    return FALSE;
}

void new_food()
{
    int ar, ac;
    
    do {
        ar = rand()%ROW;
        ac = rand()%COL;
    } while(grid[ar*COL + ac] != SPACE);
    
    grid[ar*COL + ac] = FOOD;
}

int update_snake(int ar, int ac)
{
    int n;
    switch(grid[ar*COL + ac]) {
    case UP:
        n = update_snake((ar + ROW - 1)%ROW, ac);
        break;
    case DOWN:
        n = update_snake((ar + 1)%ROW, ac);
        break;
    case RIGHT:
        n = update_snake(ar, (ac + 1)%COL);
        break;
    case LEFT:
        n = update_snake(ar, (ac + COL - 1)%COL);
        break;
    case TAIL:
        if (remain > 0) {
            remain--;
        } else {
            grid[ar*COL + ac] = SPACE;
            return 1;
        }
        break;
    }
    
    if (n == 1) {
        grid[ar*COL + ac] = TAIL;
    }
    
    return 0;
}

void eat_food()
{
    int id;
    char string[30];
    
    points += 10;
    
    tlapse = ((tlapse-step) < 10 ? 10 : tlapse-step);
    remain = 2;
    
    sprintf(string, "Points : %d", points);
    
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR(pointsbar), "Points");
    gtk_statusbar_pop (GTK_STATUSBAR(pointsbar), id);
    gtk_statusbar_push (GTK_STATUSBAR(pointsbar), id, string);
    
    new_food();
}

void game_over()
{
    int id;
    char string[30];
    
    dirbuf[ibuf] = STOP;
    
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR(pointsbar), "Points");
    
    sprintf(string, "Points : %d", points);
    
    gtk_statusbar_pop (GTK_STATUSBAR(pointsbar), id);
    gtk_statusbar_push (GTK_STATUSBAR(pointsbar), id, string);
    
    
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Info");
    gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
    gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, "Game Over");
    
    timesig = -1;
    ended = 1;
}

static gboolean advance (GtkWidget *widget)
{
    int srow = sr;
    int scol = sc;
    
    if (paused == 1 || ended == 1) {
        goto pre_exit;
    }
    
    if (nbuf > 0) {
        nbuf--;
    }
    
    switch (dirbuf[ibuf]) {
    case UP:
        srow = (sr + ROW - 1) % ROW;
        break;
    case DOWN:
        srow = (sr + ROW + 1) % ROW;
        break;
    case RIGHT:
        scol = (sc + COL + 1) % COL;
        break;
    case LEFT:
        scol = (sc + COL - 1) % COL;
        break;
    default:
        goto pre_exit;
    }

    if (invalid(grid[srow*COL + scol])) {
        game_over();
        goto a_exit;
    } else {
        sc = scol;
        sr = srow;
        
        if (grid[sr*COL + sc] == FOOD) {
            eat_food();
        }
        
        grid[sr*COL + sc] = inverse(dirbuf[ibuf]);
        update_snake(sr, sc);
    }
        
    if (nbuf > 0) {
        ibuf = (ibuf + 1) % 32;
    }

    gtk_widget_queue_draw(widget);
    
pre_exit:
    timesig = g_timeout_add(tlapse, (GSourceFunc) advance, (gpointer) darea);
    
a_exit:
    return FALSE;
}

int load_map(const char *name, char *gridp)
{
    char c;
    int fd, n, i;
    
    fd = open(name, O_RDONLY);
    
    if (fd == -1) {
        return -1;
    }
    
    for (i = 0; i < ROW; i++) {
        n = read(fd, &gridp[i*COL], COL*sizeof(char));
        
        if (n != COL*sizeof(char)) {
            close(fd);
            return -1;
        }
        
        n = read(fd, &c, sizeof(char));
        
        if (n != sizeof(char)) {
            close(fd);
            return -1;
        }
    }
    
    close(fd);
    
    return 0;
}

void init_game()
{
    int id;
    
    srand(time(0));
    
    sr = IN_ROW;
    sc = IN_COL;
    remain = 0;
    tlapse = 400;
    step = 10;
    ibuf = 0;
    nbuf = 0;
    points = 0;
    paused = 0;
    ended = 0;
    init = 0;
    
    memset(&grid, SPACE, sizeof(grid)/sizeof(char));
    memset(&dirbuf, STOP, sizeof(dirbuf)/sizeof(int));
    
    if (load_map(mapname, grid) == -1) {
        memset(&grid, SPACE, sizeof(grid)/sizeof(char));
    }
    
    grid[(sr - 2)*COL + sc] = TAIL;
    grid[(sr - 1)*COL + sc] = UP;
    grid[sr*COL + sc] = UP;
    
    new_food();
    
    if (keysig == -1) {
        keysig = g_signal_connect(window, "key_press_event",
                                  G_CALLBACK(key_press), NULL);
    }
    
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR(pointsbar), "Points");
    gtk_statusbar_pop (GTK_STATUSBAR(pointsbar), id);
    gtk_statusbar_push (GTK_STATUSBAR(pointsbar), id, "Points : 0");

    id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Info");
    gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
    
    if (timesig == -1) {
        timesig = g_timeout_add(tlapse, (GSourceFunc) advance, (gpointer) darea);
    }

    gtk_widget_queue_draw(darea);
}

void change_map(GtkIconView *iconview, 
                GtkTreePath *path, 
                gpointer user_data)
{
    int *vec = gtk_tree_path_get_indices (path);
    
    if (vec != NULL) {
        sprintf(mapname, "maps/map.%03d", vec[0]);
        init_game();
    }
}

void show_about(GtkWidget *widget, 
                GtkWidget *whatever, 
                gpointer user_data)
{
    GtkWidget *dialog;
    GdkPixbuf *logo;

    const gchar *authors[] = {"Anil Motilal Mahtani Mirchandani", NULL};

    const gchar *documenters[] = {"Anil Motilal Mahtani Mirchandani", NULL};
    
    const char *lic = "Copyright (C) 2011 Anil Motilal Mahtani Mirchandani(anil.mmm@gmail.com)\n\n"
                      "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>"
                      "This is free software: you are free to change and redistribute it."
                      "There is NO WARRANTY, to the extent permitted by law.";
                      
    dialog = gtk_about_dialog_new ();

    /* Set application data that will be displayed in the main dialog. */
    gtk_about_dialog_set_name (GTK_ABOUT_DIALOG (dialog), "GSnakey");
    gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), "0.7");
    gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog), 
                                    "(C) 2011 Anil M. Mahtani Mirchandani");
    gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (dialog), 
                                   "Snake game based on Google Old Snakey!");

    /* Set the license text, which is usually loaded from a file. Also, set the
    * web site address and label. */
    gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (dialog), lic);
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog), 
                                  "https://github.com/Anilm3/GSnakey/");
    gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (dialog), 
                                        "https://github.com/Anilm3/GSnakey/");

    /* Set the application authors, documenters and translators. */
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (dialog), authors);
    gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (dialog), documenters);
  
    gtk_about_dialog_set_wrap_license (GTK_ABOUT_DIALOG (dialog), TRUE);
    
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}


void show_keys(GtkWidget *widget, 
               GtkWidget *whatever, 
               gpointer user_data)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new (GTK_WINDOW(window),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_INFO,
                                     GTK_BUTTONS_OK,
                                     "Up\t\t: Up Arrow\n"
                                     "Down\t: Down Arrow\n"
                                     "Right\t: Right Arrow\n"
                                     "Left\t\t: Left Arrow\n"
                                     "Pause\t: Space\n"
                                     "Reset\t: Esc");
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

static gboolean 
statusbar_hint (GtkMenuItem *menuitem, 
                GdkEventProximity *event, 
                gpointer *user_data)
{
  gchar *hint;
  guint id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "MenuItemHints");

  if (event->type == GDK_ENTER_NOTIFY)
  {
    hint = (gchar*) g_object_get_data (G_OBJECT (menuitem), "menuhint");
    gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, hint);
  }
  else if (event->type == GDK_LEAVE_NOTIFY)
    gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
  
  return FALSE;
}

int main (int argc, char *argv[])
{
    int id, x, y;
    int rowstride, n_channels;
    guchar *pixels, *p;
    GtkAccelGroup *group;
    
    GtkWidget *hbox, *vbox, *vbox2, *table;
    GtkWidget *icon_view;
    GtkWidget *scrolled_window;
    GtkWidget *menubar;
    GtkWidget *file, *help;
    GtkWidget *keys, *about, *import, *open, *quit;
    GtkWidget *filemenu, *helpmenu;
    GdkColor color;
    GdkPixbuf *icon;
    
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "GSnakey");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW (window), FALSE);

    icon = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 32, 32);

    n_channels = gdk_pixbuf_get_n_channels (icon);
    
    for (x = 0; x < 32; x++) {
        for (y = 0; y < 32; y++) {


            rowstride = gdk_pixbuf_get_rowstride (icon);
            pixels = gdk_pixbuf_get_pixels (icon);
            p = pixels + y * rowstride + x * n_channels;
            
            p[0] = 255;
            p[1] = 0;
            p[2] = 0;
            p[3] = (applogo[y*32 + x] != '.' ? 
                    (applogo[y*32 + x] == '#' ? 255 : 127) : 0);
        }
    }
    
	gtk_window_set_icon (GTK_WINDOW(window), icon);
	
    darea = gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(darea), WIDTH, HEIGHT);

    statusbar = gtk_statusbar_new ();
    pointsbar = gtk_statusbar_new ();

    group = gtk_accel_group_new ();
    menubar = gtk_menu_bar_new ();
    
    file = gtk_menu_item_new_with_mnemonic ("_File");
    help = gtk_menu_item_new_with_mnemonic ("_Help");
    filemenu = gtk_menu_new ();
    helpmenu = gtk_menu_new ();

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (file), filemenu); 
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (help), helpmenu);

    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), file); 
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), help);

    quit = gtk_image_menu_item_new_with_label("Quit");
    gtk_menu_shell_append (GTK_MENU_SHELL (filemenu), quit);

    keys = gtk_image_menu_item_new_with_label("Keys");
    about = gtk_image_menu_item_new_with_label("About");
    gtk_menu_shell_append (GTK_MENU_SHELL (helpmenu), keys);
    gtk_menu_shell_append (GTK_MENU_SHELL (helpmenu), about);
    
    gtk_image_menu_item_set_accel_group(GTK_IMAGE_MENU_ITEM(quit), group);
    gtk_image_menu_item_set_accel_group(GTK_IMAGE_MENU_ITEM(keys), group);
    gtk_image_menu_item_set_accel_group(GTK_IMAGE_MENU_ITEM(about), group);
    
    g_signal_connect (G_OBJECT (quit), "enter-notify-event",
                      G_CALLBACK (statusbar_hint), statusbar);
    g_signal_connect (G_OBJECT (quit), "leave-notify-event",
                      G_CALLBACK (statusbar_hint), statusbar);
    g_signal_connect (G_OBJECT (keys), "enter-notify-event",
                      G_CALLBACK (statusbar_hint), statusbar);
    g_signal_connect (G_OBJECT (keys), "leave-notify-event",
                      G_CALLBACK (statusbar_hint), statusbar);
    g_signal_connect (G_OBJECT (about), "enter-notify-event",
                      G_CALLBACK (statusbar_hint), statusbar);
    g_signal_connect (G_OBJECT (about), "leave-notify-event",
                      G_CALLBACK (statusbar_hint), statusbar);
                    
    g_object_set_data (G_OBJECT (quit), "menuhint",
                       (gpointer) "Quit GSnakey, probably not what you're looking for.");
    g_object_set_data (G_OBJECT (keys), "menuhint",
                       (gpointer) "Show keyboard settings.");
    g_object_set_data (G_OBJECT (about), "menuhint",
                       (gpointer) "Show about message.");
                     
    gtk_widget_add_accelerator (quit, "activate", group, GDK_Q, 
                                GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (keys, "activate", group, GDK_K, 
                                GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                         GTK_SHADOW_IN);
                                       
    gtk_widget_set_size_request (scrolled_window, 138, HEIGHT);
    
    icon_view = gtk_icon_view_new_with_model (create_and_fill_model ());
    gtk_container_add (GTK_CONTAINER (scrolled_window), icon_view);
        
    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (icon_view), COL_PIXBUF);
    gtk_icon_view_set_column_spacing (GTK_ICON_VIEW (icon_view), 0);
    gtk_icon_view_set_row_spacing (GTK_ICON_VIEW (icon_view), 0);
    gtk_icon_view_set_margin (GTK_ICON_VIEW (icon_view), 0);
/*    gtk_icon_view_set_item_padding (GTK_ICON_VIEW (icon_view), 5);*/
    gtk_widget_set_can_focus (icon_view, FALSE);
    
    table = gtk_table_new (1, 6, TRUE);
    gtk_table_attach_defaults (GTK_TABLE (table), statusbar, 0, 5, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), pointsbar, 5, 6, 0, 1);
    
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start_defaults (GTK_BOX (vbox),  darea);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), table);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start_defaults (GTK_BOX (hbox), scrolled_window);
    gtk_box_pack_start_defaults (GTK_BOX (hbox), vbox);
    
    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start_defaults (GTK_BOX (vbox2), menubar);
    gtk_box_pack_start_defaults (GTK_BOX (vbox2), hbox);
    
    gtk_container_add(GTK_CONTAINER (window), vbox2);
    
    g_signal_connect (G_OBJECT (quit), "activate",
                G_CALLBACK (destroy), NULL);
    g_signal_connect (G_OBJECT (about), "activate",
                G_CALLBACK (show_about), NULL);
    g_signal_connect (G_OBJECT (keys), "activate",
                G_CALLBACK (show_keys), NULL);
    g_signal_connect (G_OBJECT (icon_view), "item-activated",
                G_CALLBACK (change_map), NULL);
                
    g_signal_connect(darea, "expose-event",
                     G_CALLBACK(expose), darea);
    g_signal_connect(window, "destroy",
                     G_CALLBACK(destroy), NULL);
    init_game();

    gtk_window_add_accel_group (GTK_WINDOW (window), group);
    
    gtk_widget_show_all(window);
    
    gtk_main();

    return 0;
}
