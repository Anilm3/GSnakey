/********************************************************************************
* GSnakey v0.2 - GSnakey.c                                                      *
*                                                                               *
* Copyright (C) 2010 Anil Motilal Mahtani Mirchandani(anil.mmm@gmail.com)       *
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gdk/gdkkeysyms.h>

#define ROW		24
#define COL		51

#define HEIGHT	ROW*12
#define WIDTH	COL*12

#define IN_ROW	12
#define IN_COL	25

#define invalid(x) ((x) == SR || \
					(x) == SL || \
					(x) == SU || \
					(x) == SD || \
					(x) == ST || \
					(x) == WALL)
					
#define issnake(x) ((x) == SR || \
					(x) == SL || \
					(x) == SU || \
					(x) == SD || \
					(x) == ST)
enum object_t {
	WALL  = '#',
	SPACE = '.',
	FOOD  = '@',
	SR    = 'R',
	SL    = 'L',
	SU    = 'U',
	SD    = 'D',
	ST    = 'T'
};

enum dir_t {STOP, RIGHT, LEFT, UP, DOWN};

int sr = IN_ROW;
int sc = IN_COL;
int remain = 0;

int tlapse = 400;
int step = 10;

char grid[ROW*COL];

int ibuf = 0;
int nbuf = 0;

int dirbuf[32];

int points = 0;
int keysig;

GtkWidget *window, *statusbar;

static void destroy (GtkWidget *window, gpointer data)
{
	gtk_main_quit ();
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
			case SR:
			case SL:
			case SU:
			case SD:
			case ST:
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

static gboolean
key_press (GtkWidget *widget, 
		   GdkEventKey *event,
		   GtkWidget *data)
{
	int i, k;

	switch(event->keyval) {
	case GDK_Left:
		if (nbuf == 32) {
			dirbuf[ibuf] = LEFT;
			ibuf = (ibuf + 1) % 32;
		} else {
			if (nbuf > 0) {
				if (dirbuf[(ibuf + nbuf + 31) % 32] == LEFT ||
					dirbuf[(ibuf + nbuf + 31) % 32] == RIGHT) {
					break;
				}
			}else {
				if (grid[sr*COL + sc] == SL) {
					break;
				}
			}
			dirbuf[(ibuf + nbuf) % 32] = LEFT;
			nbuf++;
		} 
		break;
	case GDK_Up:
		if (nbuf == 32) {
			dirbuf[ibuf] = UP;
			ibuf = (ibuf + 1) % 32;
		} else {
			if (nbuf > 0) {
				if (dirbuf[(ibuf + nbuf + 31) % 32] == UP ||
					dirbuf[(ibuf + nbuf + 31) % 32] == DOWN) {
					break;
				}
			} else {
				if (grid[sr*COL + sc] == SU) {
					break;
				}
			}
			dirbuf[(ibuf + nbuf) % 32] = UP;
			nbuf++;
		}
		break;
	case GDK_Right:
		if (nbuf == 32) {
			dirbuf[ibuf] = RIGHT;
			ibuf = (ibuf + 1) % 32;
		} else {
			if (nbuf > 0) {
				if (dirbuf[(ibuf + nbuf + 31) % 32] == RIGHT ||
					dirbuf[(ibuf + nbuf + 31) % 32] == LEFT) {
					break;
				}
			} else {
				if (grid[sr*COL + sc] == SR) {
					break;
				}
			}
			dirbuf[(ibuf + nbuf) % 32] = RIGHT;
			nbuf++;
		}
		break;
	case GDK_Down:
		if (nbuf == 32) {
			dirbuf[ibuf] = DOWN;
			ibuf = (ibuf + 1) % 32;
		} else {
			if (nbuf > 0) {
				if (dirbuf[(ibuf + nbuf + 31) % 32] == DOWN ||
					dirbuf[(ibuf + nbuf + 31) % 32] == UP) {
					break;
				}
			} else {
				if (grid[sr*COL + sc] == SD) {
					break;
				}
			}
			dirbuf[(ibuf + nbuf) % 32] = DOWN;
			nbuf++;
		}
		break;
	}
	return FALSE;
}

static gboolean
on_expose_event (GtkWidget *widget,
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
	case SU:
		n = update_snake((ar + ROW - 1)%ROW, ac);
		break;
	case SD:
		n = update_snake((ar + 1)%ROW, ac);
		break;
	case SR:
		n = update_snake(ar, (ac + 1)%COL);
		break;
	case SL:
		n = update_snake(ar, (ac + COL - 1)%COL);
		break;
	case ST:
		if (remain > 0) {
			remain--;
		} else {
			grid[ar*COL + ac] = SPACE;
			return 1;
		}
		break;
	}
	
	if (n == 1) {
		grid[ar*COL + ac] = ST;
	}
	
	return 0;
}

static gboolean
advance (GtkWidget *widget)
{
	int srow, scol;
	
	if (nbuf > 0) {
		nbuf--;
	}
	
	switch (dirbuf[ibuf]) {
	case UP:
		if (grid[sr*COL + sc] == SU) {
			break;
		}
		
		srow = (sr + ROW - 1) % ROW;
		if (invalid(grid[srow*COL + sc])) {
			int id;
			dirbuf[ibuf] = STOP;
			id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
			char string[30];
			sprintf(string, "Game Over : %d Points", points);
			gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
			gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, string);
			g_signal_handler_disconnect(window, keysig);
		} else {
			sr = srow;
			
			if (grid[sr*COL + sc] == FOOD) {
				int id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
				char string[20];
				points += 10;
				sprintf(string, "%d Points", points);
				gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
				gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, string);
				tlapse = ((tlapse-step) < 10 ? 10 : tlapse-step);
				remain = 2;
				new_food();
			}
			
			grid[sr*COL + sc] = SD;
			update_snake(sr, sc);
		}
		
		break;
	case DOWN:
		if (grid[sr*COL + sc] == SD) {
			break;
		}
		
		srow = (sr + ROW + 1) % ROW;
		if (invalid(grid[srow*COL + sc])) {
			int id;
			dirbuf[ibuf] = STOP;
			id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
			char string[30];
			sprintf(string, "Game Over : %d Points", points);
			gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
			gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, string);
			g_signal_handler_disconnect(window, keysig);
		} else {
			sr = srow;
			
			if (grid[sr*COL + sc] == FOOD) {
				int id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
				char string[20];
				points += 10;
				sprintf(string, "%d Points", points);
				gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
				gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, string);
				tlapse = ((tlapse-step) < 10 ? 10 : tlapse-step);
				remain = 2;
				new_food();
			}
			
			grid[sr*COL + sc] = SU;
			update_snake(sr, sc);
		}
		break;
	case RIGHT:
		if (grid[sr*COL + sc] == SR) {
			break;
		}
		
		scol = (sc + COL + 1) % COL;
		if (invalid(grid[sr*COL + scol])) {
			int id;
			dirbuf[ibuf] = STOP;
			id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
			char string[30];
			sprintf(string, "Game Over : %d Points", points);
			gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
			gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, string);
			g_signal_handler_disconnect(window, keysig);
		} else {
			sc = scol;
			
			if (grid[sr*COL + sc] == FOOD) {
				int id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
				char string[20];
				points += 10;
				sprintf(string, "%d Points", points);
				gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
				gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, string);
				tlapse = ((tlapse-step) < 10 ? 10 : tlapse-step);
				remain = 2;
				new_food();
			}
			
			grid[sr*COL + sc] = SL;
			update_snake(sr, sc);
		}
		break;
	case LEFT:
		if (grid[sr*COL + sc] == SL) {
			break;
		}
		
		scol = (sc + COL - 1) % COL;
		if (invalid(grid[sr*COL + scol])) {
			int id;
			dirbuf[ibuf] = STOP;
			id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
			char string[30];
			sprintf(string, "Game Over : %d Points", points);
			gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
			gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, string);
			g_signal_handler_disconnect(window, keysig);
			return FALSE;
		} else {
			sc = scol;
			
			if (grid[sr*COL + sc] == FOOD) {
				int id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
				char string[20];
				points += 10;
				sprintf(string, "%d Points", points);
				gtk_statusbar_pop (GTK_STATUSBAR(statusbar), id);
				gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, string);
				tlapse = ((tlapse-step) < 10 ? 10 : tlapse-step);
				remain = 2;
				new_food();
			}
			
			grid[sr*COL + sc] = SR;
			update_snake(sr, sc);
		}
		break;
	}
	
	if (nbuf > 0) {
		ibuf = (ibuf + 1) % 32;
	}

	gtk_widget_queue_draw(widget);
	g_timeout_add(tlapse, (GSourceFunc) advance, widget);
	
	return FALSE;
}

int load_map(const char *name)
{
	char c;
	int fd, n, i;
	
	fd = open(name, O_RDONLY);
	
	if (fd == -1) {
		printf("Error: No map\n");
		return -1;
	}
	
	for (i = 0; i < ROW; i++) {
		n = read(fd, &grid[i*COL], COL*sizeof(char));
		
		if (n != COL*sizeof(char)) {
			close(fd);
			printf("Error: Invalid map\n");
			return -1;
		}
		
		n = read(fd, &c, sizeof(char));
		
		if (n != sizeof(char)) {
			close(fd);
			printf("Error: Invalid map\n");
			return -1;
		}
	}
	
	close(fd);
	
	return 0;
}

int main (int argc, char *argv[])
{
	int id;
	GtkWidget *darea, *vbox;
	
	srand(time(0));
	
	memset(&grid, SPACE, sizeof(grid)/sizeof(char));
	memset(&dirbuf, STOP, sizeof(dirbuf)/sizeof(int));
	
	if (load_map("map1") == -1) {
		exit(EXIT_SUCCESS);
	}
	
	new_food();
	
	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
/*	gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT); */
	gtk_window_set_resizable(GTK_WINDOW (window), FALSE);

	darea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(darea), WIDTH, HEIGHT);

	statusbar = gtk_statusbar_new ();

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), darea);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), statusbar);
	
	gtk_container_add(GTK_CONTAINER (window), vbox);
	
	g_signal_connect(darea, "expose-event",
					 G_CALLBACK(on_expose_event), darea);
	keysig = g_signal_connect(window, "key_press_event",
					 G_CALLBACK(key_press), NULL);
	g_signal_connect(window, "destroy",
					 G_CALLBACK(destroy), NULL);

	id = gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "Points");
	gtk_statusbar_push (GTK_STATUSBAR(statusbar), id, "0 Points");
				
	gtk_widget_show_all(window);
	
	g_timeout_add(tlapse, (GSourceFunc) advance, (gpointer) darea);
	
	gtk_main();

	return 0;
}
