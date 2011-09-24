/********************************************************************************
* GSnakey v0.1 - GSnakey.c                                                      *
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

enum dir_t {STOP, RIGHT, LEFT, UP, DOWN, NONE};

int row = 0;
int col = 0;
int fr, fc, sr, sc;
int remain = 0;
int tlapse = 500;
int step = 10;
char *grid;

int ibuf = 0;
int nbuf = 0;

enum dir_t dirbuf[32];
enum dir_t direction = STOP;

static void destroy (GtkWidget *window, gpointer data)
{
	if(grid != NULL){
		free(grid);
	}
	
	gtk_main_quit ();
}

void draw_grid (GtkWidget *widget)
{
	cairo_t *cr;
	int i, j, width, height;

	cr = gdk_cairo_create (widget->window);
	
	width = widget->allocation.width;
	height = widget->allocation.height;
	
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	for (i = 0; i < row; i++) {
		for (j = 0; j < col; j++) {
			switch (grid[col*i + j]) {
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
				if (grid[sr*col + sc] == SL) {
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
				if (grid[sr*col + sc] == SU) {
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
				if (grid[sr*col + sc] == SR) {
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
				if (grid[sr*col + sc] == SD) {
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
		ar = rand()%row;
		ac = rand()%col;
	} while(grid[ar*col + ac] != SPACE);
	
	grid[ar*col + ac] = FOOD;
}

int update_snake(int ar, int ac)
{
	int n;
	switch(grid[ar*col + ac]) {
	case SU:
		n = update_snake((ar + row - 1)%row, ac);
		break;
	case SD:
		n = update_snake((ar + 1)%row, ac);
		break;
	case SR:
		n = update_snake(ar, (ac + 1)%col);
		break;
	case SL:
		n = update_snake(ar, (ac + col - 1)%col);
		break;
	case ST:
		if (remain > 0) {
			remain--;
		} else {
			grid[ar*col + ac] = SPACE;
			return 1;
		}
		break;
	}
	
	if (n == 1) {
		grid[ar*col + ac] = ST;
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
		if (grid[sr*col + sc] == SU) {
			break;
		}
		
		srow = (sr + row - 1) % row;
		if (invalid(grid[srow*col + sc])) {
			dirbuf[ibuf] = STOP;
			printf("END\n");
			exit(EXIT_SUCCESS);
		} else {
			sr = srow;
			
			if (grid[sr*col + sc] == FOOD) {
				tlapse = ((tlapse-step) < 50 ? 50 : tlapse-step);
				step = step + 5;
				remain = 2;
				new_food();
			}
			
			grid[sr*col + sc] = SD;
			update_snake(sr, sc);
		}
		
		break;
	case DOWN:
		if (grid[sr*col + sc] == SD) {
			break;
		}
		
		srow = (sr + row + 1) % row;
		if (invalid(grid[srow*col + sc])) {
			dirbuf[ibuf] = STOP;
			printf("END\n");
			exit(EXIT_SUCCESS);
		} else {
			sr = srow;
			
			if (grid[sr*col + sc] == FOOD) {
				tlapse = ((tlapse-step) < 50 ? 50 : tlapse-step);
				step = step + 5;
				remain = 2;
				new_food();
			}
			
			grid[sr*col + sc] = SU;
			update_snake(sr, sc);
		}
		break;
	case RIGHT:
		if (grid[sr*col + sc] == SR) {
			break;
		}
		
		scol = (sc + col + 1) % col;
		if (invalid(grid[sr*col + scol])) {
			dirbuf[ibuf] = STOP;
			printf("END\n");
			exit(EXIT_SUCCESS);
		} else {
			sc = scol;
			
			if (grid[sr*col + sc] == FOOD) {
				tlapse = ((tlapse-step) < 50 ? 50 : tlapse-step);
				step = step + 5;
				remain = 2;
				new_food();
			}
			
			grid[sr*col + sc] = SL;
			update_snake(sr, sc);
		}
		break;
	case LEFT:
		if (grid[sr*col + sc] == SL) {
			break;
		}
		
		scol = (sc + col - 1) % col;
		if (invalid(grid[sr*col + scol])) {
			dirbuf[ibuf] = STOP;
			printf("END\n");
			exit(EXIT_SUCCESS);
		} else {
			sc = scol;
			
			if (grid[sr*col + sc] == FOOD) {
				tlapse = ((tlapse-step) < 50 ? 50 : tlapse-step);
				step = step + 5;
				remain = 2;
				new_food();
			}
			
			grid[sr*col + sc] = SR;
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

int main (int argc, char *argv[])
{
	FILE *fd;
	int n, i, j;
	int width, height;
	int count = 0;
	GtkWidget *window;
	GtkWidget *darea;
	
	srand(time(0));
	
	fd = fopen("map1", "r");
	
	if (fd == NULL) {
		printf("Error: No map\n");
		exit(EXIT_SUCCESS);
	}
	
	n = fscanf(fd, "%d %d\n", &row, &col);
	
	if (n != 2 ||
		row < 10 || row > 100 ||
		col < 10 || col > 100) 
	{
		fclose(fd);
		printf("Error: Invalid map 1\n");
		exit(EXIT_SUCCESS);
	}
	
	grid = (char *) malloc(sizeof(char)*row*col);
	
	if (grid == NULL) {
		free(grid);
		fclose(fd);
		printf("Error: Memory error 2\n");
		exit(EXIT_SUCCESS);
	}
	
	n = fscanf(fd, "%d %d\n", &sr, &sc);

	if (n != 2){
		free(grid);
		fclose(fd);
		printf("Error: Invalid map 3\n");
		exit(EXIT_SUCCESS);
	}
	
	printf("%d %d\n", sr, sc);
	
	for (i = 0; i < row; i++) {
		for (j = 0; j < col; j++) {
			n = fscanf(fd, "%c", &grid[i*col+j]);
			
			if (n != 1) {
				free(grid);
				fclose(fd);
				printf("Error: Invalid map 4\n");
				exit(EXIT_SUCCESS);
			}
		}
		
		n = fscanf(fd, "\n");
		if (n != 0) {
			free(grid);
			fclose(fd);
			printf("Error: Invalid map 5\n");
			exit(EXIT_SUCCESS);
		}
	}

	new_food();
	
	fclose(fd);
	
	width = col*12;
	height = row*12;
	
	memset(&dirbuf, NONE, sizeof(dirbuf)/sizeof(enum dir_t));
	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), width, height); 
	gtk_window_set_resizable(GTK_WINDOW (window), FALSE);
	
	darea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(darea), width, height);

	gtk_container_add(GTK_CONTAINER (window), darea);
	
	g_signal_connect(darea, "expose-event",
					 G_CALLBACK(on_expose_event), darea);
	g_signal_connect(window, "key_press_event",
					 G_CALLBACK(key_press), NULL);
	g_signal_connect(window, "destroy",
					 G_CALLBACK(destroy), NULL);
					 
	gtk_widget_show_all(window);
	
	g_timeout_add(tlapse, (GSourceFunc) advance, (gpointer) darea);
	
	gtk_main();

	return 0;
}
