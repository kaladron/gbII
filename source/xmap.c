/******* XMAP code    ******/
/* Version: 3.2.4 	   */
/* Date:    11/30/93	   */
/* Rich Fitzgerald	   */
/* fitz@rsj.harvard.edu	   */
/* Based on:	           */
/* Scott Anderson 4/13/91  */
/* VERSION 2.1.0           */
/* black and white xmap    */
/* See CHANGES.XMAP,xmap.h */
/***************************/

#include "gb.h"
#ifdef XMAP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include "xmap.h"
#include "vars.h"
#include "option.h"
#include "csp.h"

/* cyclical button labels */
static char *ShipLabel[3] = {"Show All",
			     "Ships Only",
			     "No Ships"};

static char *xmaplabel[4] = {"XMap Off  ",
			     "Maps Only ",
			     "Orbit Only",
			     "XMap On   "};

/* windows */
static widget *widget_list;

static mwin *map_win;
static sec_win *sector_win;
static but_win *remap_win;
static but_win *mapnow_win;
static but_win *anal_win;
static but_win *sur_win;
static but_win *rep_win;
static but_win *cdup_win;
static but_win *cduni_win;
static but_win *cdhere_win;
static but_win *cdhome_win;
static but_win *reverse_win;
static but_win *ships_win;
static but_win *zoom_win;
static pan_win *orb_panel;
static int reverse_orbit = 0;
static int show_ships = 0;

static but_win *mapon_win;

static but_win *last_but_win;
static but_win *graph_win;
static but_win *fert_win;
static but_win *res_win;
static but_win *eff_win;
static but_win *mob_win;
static but_win *mil_win;
static but_win *popn_win;
static but_win *mpopn_win;
static but_win *geog_win;

static pan_win *bottom_panel;
static pan_win *left_panel;
static pan_win *right_panel;
static in_win *input_win;

/* X interface structures */
Display *mdpy;
static GC sec_text_gc, but_text_gc, invert_gc, clear_gc;
static Font sec_font;
static Font button_font;

char xmap_map_command_scope[200];	/* temporary storage of map scope */
static char x_scope[200];		/* scope of displayed map/orbit */

/* to ensure floating scale factors; defaults in xmap.h */
static float orb_min_x = Orb_Default_min_x;
static float orb_max_x = Orb_Default_max_x;
static float orb_min_y = Orb_Default_min_y;
static float orb_max_y = Orb_Default_max_y;

static int is_xmap_survey;
static int normal_map = 1;
static int no_sector;
static int Xavailable;
static int xmap_expecting = 0;
static int is_zoom;
static int x_is_map;
static int x_is_orbit;
static int x_is_pressed;
static int x_graph_type;
static int x_pressed_x;
static int x_pressed_y;
static int x_prev_x;
static int x_prev_y;
static int x_is_redraw;
static int x_maxx;
static int x_maxy;
static int cursect_x = -1;
static int cursect_y = -1;
static int xmap_on;
static char x_map_data[MAXDATA];
static xOrb *xOrbit_head = NULL;
static xMap xcur_map = {0, 0, "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			999999, 999999, 999999, 999999, 999999, 999999, 999999,
			0, 0, 0, 0, (xSector *) NULL};

extern char *build_scope();
extern int  csp_send_request(int, char *);
extern int  process_key(char *, int);
extern int  send_gb(char *, int);
extern void add_history(char *);
extern void add_recall(char *, int);
extern void msg (char *, ...);
extern void plot_orbit(char *);
extern void plot_surface(char *);

char xmap_get_graph(int x, int y);
int  set_zoom(int x1, int y1, int x2, int y2);
void add_input(char the_char);
void add_new_widget(widget *w);
void clear(mwin *win);
void clear_input();
void do_input();
void empty_proc();
void input_update(in_win *win, int pos);
void MapWidget(widget w);
void process_xmap_survey(int num, char *s);
void remove_input();
void sector_display_stats(pan_win *win);
void xmap_deploy(int x, int y, int x2, int y2, char *u);
void xmap_mover(int x, int y, int x2, int y2, char *u);
void xmap_ping(xSector *q);
void xmap_plot_redraw();
void xmap_replot_orbit(xOrb *head);
void xmap_plot_continue();
widget *create_mwin(Display *dpy, widget *parent, int x, int y, int w, int h);
widget *create_panel(Display *dpy, widget *parent, int x, int y, int w, int h);
widget *create_button(Display *dpy, widget *parent, int x, int y, int w,
		      int h);
widget *create_input(Display *dpy, widget *parent, int x, int y, int w, int h);
widget *create_sector(Display *dpy, widget *parent, int x, int y, int w,
		      int h);

/* Window type declaration for widget allocation*/
widget_type mwin_type = {
    "Line drawing Window",
    create_mwin};
widget_type pan_win_type = {
    "Panel Window",
    create_panel};
widget_type but_win_type = {
    "Re-map Window",
    create_button};
widget_type sec_win_type = {
    "Sector Window",
    create_sector};
widget_type in_win_type = {
    "Input Window",
    create_input};

void send_client_survey(char *s, int changescope)
{
    char csbuf1[200];
    char csbuf2[200];

    if (changescope) {
	sprintf(csbuf1, "cs %s\n", build_scope());
	sprintf(csbuf2, "cs %s\n", x_scope);
	send_gb(csbuf2, strlen(csbuf2));
    }
    if (GBDT()) {
	csp_send_request(CSP_SURVEY_COMMAND, s);
    } else {
	sprintf(csbuf2, "client_survey %s\n", s);
	send_gb(csbuf2, strlen(csbuf2));
    }
    if (changescope) {
	send_gb(csbuf1, strlen(csbuf1));
    }
}

void send_xmap_command(char *s, int changescope)
{
    char xcbuf1[200];
    char xcbuf2[200];

    if (changescope) {
	sprintf(xcbuf1, "cs %s\n", build_scope());
	sprintf(xcbuf2, "cs %s\n", x_scope);
	send_gb(xcbuf2, strlen(xcbuf2));
    }
    send_gb(s, strlen(s));
    if (changescope) {
	send_gb(xcbuf1, strlen(xcbuf1));
    }
}

/* button utilities */
void button_redraw(but_win *win)
{
    int x, y;
    int dir, ascent, descent;
    XCharStruct overall;

    XClearWindow(win->core.dpy, win->core.win);
    XQueryTextExtents(win->core.dpy, button_font, win->label,
		      strlen(win->label), &dir, &ascent, &descent, &overall);
    x = (win->core.w - (overall.lbearing + overall.rbearing)) / 2;
    y = (win->core.h - (overall.ascent + overall.descent)) / 2 + overall.ascent;

    XDrawString(win->core.dpy, win->core.win, but_text_gc, x, y, win->label,
		strlen(win->label));

    if (win == last_but_win || (win == graph_win && !normal_map)) {
	XFillRectangle(win->core.dpy, win->core.win, invert_gc, 0, 0,
		       win->core.w, win->core.h);
    }
}

void hilite_command_in(but_win *win)
{
    XDrawRectangle(win->core.dpy, win->core.win, but_text_gc, 2, 2,
		   win->core.w - 5, win->core.h - 5);
}

void hilite_command_out(but_win *win)
{
    XDrawRectangle(win->core.dpy, win->core.win, clear_gc, 2, 2,
		   win->core.w - 5, win->core.h - 5);
}

void do_button_down(but_win *win, XButtonEvent *ev)
{
    if (!win->toggle) {
	XFillRectangle(win->core.dpy, win->core.win, invert_gc, 0, 0,
		       win->core.w, win->core.h);
    }
    if ((ev->x >= 0) && (ev->y >= 0) && (ev->x <= win->core.w) &&
	(ev->y <= win->core.h))
	if (win->button_proc_down)
	    (*win->button_proc_down) (win);
}

void do_button_up(but_win *win, XButtonEvent *ev)
{
    if (!win->toggle) {
	XFillRectangle(win->core.dpy, win->core.win, invert_gc, 0, 0,
		       win->core.w, win->core.h);
    }
    if ((ev->x >= 0) && (ev->y >= 0) && (ev->x <= win->core.w) &&
	(ev->y <= win->core.h))
	if (win->button_proc_up)
	    (*win->button_proc_up) (win);
}

/* button procecures */
void do_remap_up(but_win *win)
{
    char a[200];

    if (!(x_is_map || x_is_orbit))
	return;
    strcpy(xmap_map_command_scope, x_scope);
    sprintf(a, "map %s\n", x_scope);
    send_xmap_command(a, 0);
}

void do_mapnow_up(but_win *win)
{
    strcpy(xmap_map_command_scope, build_scope());
    send_xmap_command("map\n", 0);
}

void do_anal_up(but_win *win)
{
    char a[200];

    sprintf(a, "analysis %s\n", input_win->info);
    send_xmap_command(a, 1);
    clear_input();
}

void do_sur_up(but_win *win)
{
    char a[200];

    sprintf(a, "survey %s\n", input_win->info);
    send_xmap_command(a, 1);
    clear_input();
}

void do_rep_up(but_win *win)
{
    char a[200];

    sprintf(a, "report %s\n", input_win->info);
    send_xmap_command(a, 1);
    clear_input();
}

void do_graph_up(but_win *win)
{
    XFillRectangle(win->core.dpy, win->core.win, invert_gc, 0, 0,
		   win->core.w, win->core.h);

    if (win->core.procs.leave == hilite_command_out) {
	hilite_command_out(win);
	win->core.procs.leave = hilite_command_in;
	win->core.procs.enter = hilite_command_out;
    } else {
	hilite_command_in(win);
	win->core.procs.leave = hilite_command_out;
	win->core.procs.enter = hilite_command_in;
    }

    if (normal_map) {
	normal_map = 0;
    } else
	normal_map = 1;

    if (x_graph_type && x_is_map) {
	xmap_plot_redraw();
    }
}

void do_gbut_up(but_win *win)
{
    if (!last_but_win || win != last_but_win) {
	XFillRectangle(win->core.dpy, win->core.win, invert_gc, 0, 0,
		       win->core.w, win->core.h);
	if (last_but_win) {
	    if (last_but_win->core.procs.leave == hilite_command_out) {
		last_but_win->core.procs.leave = hilite_command_in;
		last_but_win->core.procs.enter = hilite_command_out;
	    } else {
		last_but_win->core.procs.leave = hilite_command_out;
		last_but_win->core.procs.enter = hilite_command_in;
	    }
	    XFillRectangle(last_but_win->core.dpy, last_but_win->core.win,
	       invert_gc, 0, 0, last_but_win->core.w, last_but_win->core.h);
	}
	hilite_command_out(win);
	if (win->core.procs.leave == hilite_command_out) {
	    win->core.procs.leave = hilite_command_in;
	    win->core.procs.enter = hilite_command_out;
	} else {
	    win->core.procs.leave = hilite_command_out;
	    win->core.procs.enter = hilite_command_in;
	}

	last_but_win = win;
	x_graph_type = win->graph_type;

	if (!normal_map) {
	    xmap_plot_redraw();
	}
    }
}

void do_revbut_up(but_win *win)
{
    XFillRectangle(win->core.dpy, win->core.win, invert_gc, 0, 0,
		   win->core.w, win->core.h);
    if (win->core.procs.leave == hilite_command_out) {
	hilite_command_out(win);
	win->core.procs.leave = hilite_command_in;
	win->core.procs.enter = hilite_command_out;
    } else {
	hilite_command_in(win);
	win->core.procs.leave = hilite_command_out;
	win->core.procs.enter = hilite_command_in;
    }
    reverse_orbit ^= 1;		/* toggle reverse flag */
    if (x_is_orbit) {
	clear(map_win);
	xmap_replot_orbit(xOrbit_head);
    }
}

void do_shipbut_up(but_win *win)
{
    if (++show_ships > 2)
	show_ships = ShowAll;
    strcpy(win->label, ShipLabel[show_ships]);
    button_redraw(win);
    if (x_is_orbit) {
	clear(map_win);
	xmap_replot_orbit(xOrbit_head);
    }
}

void do_zoom_up(but_win *win)
{
    float mid, range;
    float new_zoom;

    new_zoom = atof(input_win->info);
    clear_input();
    if (new_zoom == 0.0) {
	if (!is_zoom)
	    return;
	orb_min_x = Orb_Default_min_x;
	orb_max_x = Orb_Default_max_x;
	orb_min_y = Orb_Default_min_y;
	orb_max_y = Orb_Default_max_y;
	is_zoom = FALSE;
    } else {
	mid = (orb_max_x + orb_min_x) / 2.0;
	range = (orb_max_x - orb_min_x) / 2.0;
	orb_max_x = mid + range * new_zoom;
	orb_min_x = mid - range * new_zoom;
	mid = (orb_max_y + orb_min_y) / 2.0;
	range = (orb_max_y - orb_min_y) / 2.0;
	orb_max_y = mid + range * new_zoom;
	orb_min_y = mid - range * new_zoom;
	is_zoom = TRUE;
    }
    if (x_is_orbit) {
	clear(map_win);
	xmap_replot_orbit(xOrbit_head);
    }
}

void toggle_xmap_on(but_win *win)
{
    if (++xmap_on > 3)
	xmap_on = 0;
    strcpy(mapon_win->label, xmaplabel[xmap_on]);
    button_redraw(mapon_win);
}

void do_cd_up(but_win *win)
{
    send_xmap_command("cs ..\n", 0);
}

void do_cd_uni(but_win *win)
{
    send_xmap_command("cs /\n", 0);
}

void do_cd_here(but_win *win)
{
    char a[200];

    sprintf(a, "cs %s\n", x_scope);
    send_xmap_command(a, 0);
}

void do_cd_home(but_win *win)
{
    send_xmap_command("cs\n", 0);
}

/* bottom panel and sector window utilities */
void draw_panel(pan_win *win)
{
    char a[30];

/* clear not needed for expose, but draw_panel also called when */
/* plotting orbit to clear out leftover stats */
    XClearWindow(win->core.dpy, win->core.win);

    if (x_is_map) {
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 5, Row1,
		    "RES:", 4);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 5, Row2,
		    "FUL:", 4);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 5, Row3,
		    "DES:", 4);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 100, Row1,
		    "POPN:", 5);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 94, Row2,
		    "^POPN:", 6);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 106, Row3,
		    "TOX:", 4);

	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 196, Row2,
		    "own:", 4);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 196, Row3,
		    "mob:", 4);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 292, Row1,
		    "res:", 4);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 292, Row2,
		    "eff:", 4);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 286, Row3,
		    "fert:", 5);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 376, Row1,
		    "^popn:", 6);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 382, Row2,
		    "popn:", 5);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 388, Row3,
		    "mil:", 4);

	sprintf(a, "%-d", xcur_map.res);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 30, Row1, a,
		    strlen(a));
	sprintf(a, "%-d", xcur_map.fuel);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 30, Row2, a,
		    strlen(a));
	sprintf(a, "%-d", xcur_map.des);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 30, Row3, a,
		    strlen(a));
	sprintf(a, "%-d", xcur_map.popn);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 131, Row1, a,
		    strlen(a));
	sprintf(a, "%-d", xcur_map.mpopn);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 131, Row2, a,
		    strlen(a));
	sprintf(a, "%-d", xcur_map.tox);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 131, Row3, a,
		    strlen(a));
	sector_display_stats(bottom_panel);
    }
}

void clear_sector_data(pan_win *win)
{
    /* clear out old info */
    XFillRectangle(win->core.dpy, win->core.win, clear_gc, 196, 0, 48, 15);
    XFillRectangle(win->core.dpy, win->core.win, clear_gc, 220, 0, 48, 50);
    XFillRectangle(win->core.dpy, win->core.win, clear_gc, 316, 0, 48, 50);
    XFillRectangle(win->core.dpy, win->core.win, clear_gc, 412, 0,
		   win->core.w, win->core.h);
}

void sector_display_stats(pan_win *win)
{
    char buf[20];
    xSector p;
    int textx, texty, i;

    if ((!x_is_map) || (xmap_expecting) || (cursect_x < 0))
	return;
    p = *(xcur_map.ptr + cursect_x + (cursect_y * xcur_map.maxx));
    sprintf(buf, "%2d,%-2d %c", cursect_x, cursect_y, p.sectc);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 196, Row1, buf,
		strlen(buf));
    sprintf(buf, "%-2d", p.own);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 220, Row2, buf,
		strlen(buf));
    sprintf(buf, "%-2d", p.mob);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 220, Row3, buf,
		strlen(buf));
    sprintf(buf, "%-3d", p.res);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 316, Row1, buf,
		strlen(buf));
    sprintf(buf, "%-3d", p.eff);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 316, Row2, buf,
		strlen(buf));
    sprintf(buf, "%-3d", p.frt);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 316, Row3, buf,
		strlen(buf));
    sprintf(buf, "%-7d", p.mpopn);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 412, Row1, buf,
		strlen(buf));
    sprintf(buf, "%-7d", p.civ);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 412, Row2, buf,
		strlen(buf));
    sprintf(buf, "%-7d", p.mil);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 412, Row3, buf,
		strlen(buf));

    textx = 484;
    texty = Row1;		/* #define'd in xmap.h */
    if (p.xtal) {
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, textx, texty,
		    "CRYSTAL", 7);
	texty += 15;
    }
    if (p.wasted) {
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, textx, texty,
		    "WASTED", 6);
	texty += 15;
    }
    for (i = 0; i < p.numships; i++) {
	sprintf(buf, "%d %c %d", p.ships[i].owner, p.ships[i].ltr,
		p.ships[i].shipno);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, textx, texty,
		    buf, strlen(buf));
	texty += 15;
	if (texty > 45) {
	    texty = Row1;
	    textx += 96;
	}
    }
}

void sector_redraw_info(sec_win *win, int force_redraw)
{
    int w, h;

    w = win->core.w;
    h = win->core.h;

    if (force_redraw || strcmp(win->info, win->old_info)) {
	XFillRectangle(win->core.dpy, win->core.win, clear_gc, 10, 15, w, h);
	XDrawString(win->core.dpy, win->core.win, sec_text_gc, 10, 25,
		    win->info, strlen(win->info));
	sprintf(win->old_info, "%s", win->info);
	clear_sector_data(bottom_panel);
	if ((sscanf(win->info, "%d,%d to %d,%d", &w, &h,
		    &cursect_x, &cursect_y) == 4) ||
	    (sscanf(win->info, "%d,%d", &cursect_x, &cursect_y) == 2)) {
	    sector_display_stats(bottom_panel);
	} else {
	    cursect_x = cursect_y = -1;
	}
    }
}

void sector_redraw(sec_win *win)
{
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 8, 10, win->label,
		strlen(win->label));
    sector_redraw_info(win, TRUE); /* force redraw of contents, stats */
}

/* map window utilities */
void clear(mwin *win)
{
    int w, h;

    w = win->core.w;		/* find window dimensions */
    h = win->core.h;

    XFillRectangle(win->core.dpy, win->buffer, clear_gc, 0, 0, w, h);
    XFillRectangle(win->core.dpy, win->core.win, clear_gc, 0, 0, w, h);
}

/* Redraw window by copying from offscreen buffer */
void redraw(mwin *win)
{
    int w, h;

    w = win->core.w;		/* find window dimensions */
    h = win->core.h;

    XFillRectangle(win->core.dpy, win->core.win, clear_gc, 0, 0, w, h);
    XCopyArea(win->core.dpy, win->buffer, win->core.win, sec_text_gc, 0, 0,
	      w, h, 0, 0);
    XFlush(win->core.dpy);
}

void resize(mwin *win)
{
    if (win->core.resized) {
	win->core.resized = 0;
	if (win->buffer)
	    XFreePixmap(win->core.dpy, win->buffer);
	win->buffer = XCreatePixmap(win->core.dpy, win->core.win,
				    win->core.w, win->core.h, win->bitdepth);
	XMoveWindow(bottom_panel->core.dpy, bottom_panel->core.win, 0,
		    win->core.h - bottom_panel->core.h - 1);
	XMoveWindow(right_panel->core.dpy, right_panel->core.win,
		    win->core.w - right_panel->core.w - 1, -1);
	XMoveWindow(orb_panel->core.dpy, orb_panel->core.win,
		    win->core.w - orb_panel->core.w - 1,
		    right_panel->core.h + PanelSpacing);
	x_is_redraw = TRUE;	/* need to redraw image in buffer */
	clear(win);
	if (x_is_map)
	    xmap_plot_redraw();
	else if (x_is_orbit)
	    xmap_replot_orbit(xOrbit_head);
	x_is_redraw = FALSE;
    }
}

void draw_name(mwin *win, char *u)
{
    XStoreName(win->core.dpy, win->core.win, u);
}

/* In draw_orb, x,y refer to upper left corner of character position */
void draw_orb(mwin *win, int x, int y, char *u, int len, int inv)
{
    /* get proper background */
    if (inv) {
	XFillRectangle(win->core.dpy, win->buffer, sec_text_gc,
		       x - 1, y, 6 * len + 1, 14);
    } else {
	XFillRectangle(win->core.dpy, win->buffer, clear_gc,
		       x, y, 6 * len, 14);
    }
    XDrawString(win->core.dpy, win->buffer, invert_gc, x, y + 10, u, len);
}

void draw_sec(mwin *win, int Maxx, int Maxy, int x, int y, char u, int inv)
{
    char n[2];
    int adjx, adjy;

    adjx = (win->core.w - Maxx * 16) / 2 + 1;
    adjy = (win->core.h - Maxy * 16) / 2 + 1;

    if (x == 0) {
	sprintf(n, "%d", y / 10);
	XDrawString(win->core.dpy, win->buffer, sec_text_gc,
		    (x - 2) * 16 + adjx + 8, y * 16 + adjy + 10, n, 1);
	XDrawString(win->core.dpy, win->buffer, sec_text_gc,
		    (x + Maxx) * 16 + adjx + 4, y * 16 + adjy + 10, n, 1);
	sprintf(n, "%d", y % 10);
	XDrawString(win->core.dpy, win->buffer, sec_text_gc,
		    (x - 1) * 16 + adjx + 4, y * 16 + adjy + 10, n, 1);
	XDrawString(win->core.dpy, win->buffer, sec_text_gc,
		    (x + Maxx + 1) * 16 + adjx, y * 16 + adjy + 10, n, 1);
    }
    if (y == 0) {
	sprintf(n, "%d", x / 10);
	XDrawString(win->core.dpy, win->buffer, sec_text_gc, x * 16 + adjx + 4,
		    (y - 2) * 16 + adjy + 10, n, 1);
	XDrawString(win->core.dpy, win->buffer, sec_text_gc,
		    x * 16 + adjx + 4, (y + Maxy) * 16 + adjy + 10, n, 1);
	sprintf(n, "%d", x % 10);
	XDrawString(win->core.dpy, win->buffer, sec_text_gc, x * 16 + adjx + 4,
		    (y - 1) * 16 + adjy + 6, n, 1);
	XDrawString(win->core.dpy, win->buffer, sec_text_gc,
		    x * 16 + adjx + 4, (y + Maxy + 1) * 16 + adjy + 6, n, 1);
    }
    if (inv) {
	XFillRectangle(win->core.dpy, win->buffer, sec_text_gc,
		       x * 16 + adjx - 1, y * 16 + adjy - 1, 16, 16);
    } else {
	XFillRectangle(win->core.dpy, win->buffer, clear_gc,
		       x * 16 + adjx - 1, y * 16 + adjy - 1, 16, 16);
    }
    XFillRectangle(win->core.dpy, win->buffer, invert_gc,
		   x * 16 + adjx, y * 16 + adjy, 14, 14);
    if (u == '.') {
	XDrawString(win->core.dpy, win->buffer, invert_gc,
		    x * 16 + adjx + 4, y * 16 + adjy + 6, &u, 1);
    } else {
	XDrawString(win->core.dpy, win->buffer, invert_gc,
		    x * 16 + adjx + 4, y * 16 + adjy + 10, &u, 1);
    }
}

void draw_line(int x1, int y1, int x2, int y2)
{
    int xu1, xd1, yu1, yd1, xu2, xd2, yu2, yd2, xx1, xx2, yy1, yy2;

    xu1 = xd1 = xx1 = x1;
    xu2 = xd2 = xx2 = x2;
    yu1 = yd1 = yy1 = y1;
    yu2 = yd2 = yy2 = y2;

    if ((x1 == x2) && (y1 >= y2 + 1)) {
	++xu1; ++xu2; --xd1; --xd2;
	--yu1; --yu2; --yd1; --yd2;
	++yy1; ++yy2;
    }
    if ((x1 == x2) && (y1 <= y2 - 1)) {
	++xu1; ++xu2; --xd1; --xd2;
	++yu1; ++yu2; ++yd1; ++yd2;
	--yy1; --yy2;
    }
    if ((y1 == y2) && (x1 <= x2 - 1)) {
	--xu1; --xu2; --xd1; --xd2;
	++yu1; ++yu2; --yd1; --yd2;
	++xx1; ++xx2;
    }
    if ((y1 == y2) && (x1 >= x2 + 1)) {
	++xu1; ++xu2; ++xd1; ++xd2;
	++yu1; ++yu2; --yd1; --yd2;
	--xx1; --xx2;
    }
    if ((x1 <= x2 - 1) && (y1 >= y2 + 1)) {
	--xd1; --xd2; ++yu1; ++yu2;
	++xx1; ++xx2; --yy1; --yy2;
    }
    if ((x1 >= x2 + 1) && (y1 >= y2 + 1)) {
	++xu1; ++xu2; ++yd1; ++yd2;
	--xx1; --xx2; --yy1; --yy2;
    }
    if ((x1 >= x2 + 1) && (y1 <= y2 - 1)) {
	++xu1; ++xu2; --yd1; --yd2;
	--xx1; --xx2; ++yy1; ++yy2;
    }
    if ((x1 <= x2 - 1) && (y1 <= y2 - 1)) {
	--xd1; --xd2; --yu1; --yu2;
	++xx1; ++xx2; ++yy1; ++yy2;
    }
    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      xx1, yy1, xx2, yy2);
    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      xu1, yu1, xu2, yu2);
    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      xd1, yd1, xd2, yd2);
}

void draw_box(int x1, int y1, int x2, int y2)
{
    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      x1 - 1, y1 - 1, x1 - 1, y2 + 1);
    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      x1, y1 - 1, x1, y2 + 1);
    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      x1 + 1, y1 - 1, x1 + 1, y2 + 1);

    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      x1 - 1, y2 - 1, x2 + 1, y2 - 1);
    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      x1 - 1, y2, x2 + 1, y2);
    XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
	      x1 - 1, y2 + 1, x2 + 1, y2 + 1);

    if (x1 != x2) {
	XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
		  x2 - 1, y2 + 1, x2 - 1, y1 - 1);
	XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
		  x2, y2 + 1, x2, y1 - 1);
	XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
		  x2 + 1, y2 + 1, x2 + 1, y1 - 1);
    }
    if (y1 != y2) {
	XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
		  x2 + 1, y1 - 1, x1 - 1, y1 - 1);
	XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
		  x2 + 1, y1, x1 - 1, y1);
	XDrawLine(map_win->core.dpy, map_win->core.win, invert_gc,
		  x2 + 1, y1 + 1, x1 - 1, y1 + 1);
    }
}

void map_move(mwin *win, XMotionEvent *ev)
{
    int x1, y1, x2, y2, xt;
    float x3, y3;
    int adjx, adjy;

    if (x_is_orbit && x_is_pressed) {
	draw_box(x_pressed_x, x_pressed_y, x_prev_x, x_prev_y);
	x_prev_x = ev->x;
	x_prev_y = ev->y;
	draw_box(x_pressed_x, x_pressed_y, x_prev_x, x_prev_y);
	return;
    }
    if (!x_is_map) {
	if (!no_sector) {
	    sprintf(sector_win->info, "None");
	    sector_redraw_info(sector_win, FALSE);
	}
	return;
    }
    adjx = (win->core.w - x_maxx * 16) / 2 + 1;
    adjy = (win->core.h - x_maxy * 16) / 2 + 1;

    /* undo current box/line if any */
    if (x_is_pressed) {
	x1 = x_pressed_x * 16 + adjx + 7;
	y1 = x_pressed_y * 16 + adjy + 7;
	x2 = x_prev_x * 16 + adjx + 7;
	y2 = x_prev_y * 16 + adjy + 7;
	if (x_is_pressed == 1)
	    draw_box(x1, y1, x2, y2);
	else
	    draw_line(x1, y1, x2, y2);
    }
    x3 = (ev->x - adjx) / 16.0;
    y3 = (ev->y - adjy) / 16.0;

    /* for buttons 2,3 (move, deploy), can leave map area and can wrap in */
    /* the x direction.  Currently restricted to +- 1 in x and y. */
    if (x_is_pressed >= 2) {
	if (x3 >= x_pressed_x + 1)
	    x_prev_x = x_pressed_x + 1;
	else if (x3 < x_pressed_x)
	    x_prev_x = x_pressed_x - 1;
	else
	    x_prev_x = (int) x3;

	if (x_prev_x == x_maxx)	/* see if wrap left or right. set xt */
	    xt = 0;		/* for display in sector window */
	else if (x_prev_x < 0)
	    xt = x_maxx - 1;
	else
	    xt = x_prev_x;

	if (y3 >= x_pressed_y + 1)
	    x_prev_y = min(x_pressed_y + 1, x_maxy - 1);
	else if (y3 < x_pressed_y)
	    x_prev_y = max(x_pressed_y - 1, 0);
	else
	    x_prev_y = (int) y3;

	no_sector = FALSE;
	sprintf(sector_win->info, "%2d,%-2d to %2d,%-2d", x_pressed_x,
		x_pressed_y, xt, x_prev_y);
	x2 = x_prev_x * 16 + adjx + 7;
	y2 = x_prev_y * 16 + adjy + 7;
	draw_line(x1, y1, x2, y2);
    } else {
	/*
	 * for button 1, if in map area update corner of survey rectangle. If
	 * no button, update current sector if in map area, otherwise "None".
	 */
	if ((x3 >= 0) && (x3 < x_maxx) && (y3 >= 0) && (y3 < x_maxy)) {
	    no_sector = FALSE;
	    x_prev_x = (int) x3;
	    x_prev_y = (int) y3;
	    if (!x_is_pressed)
		sprintf(sector_win->info, "%2d,%-2d", x_prev_x, x_prev_y);
	} else if (!x_is_pressed && !no_sector) {
	    sprintf(sector_win->info, "None");
	    no_sector = TRUE;
	}
	if (x_is_pressed) {
	    sprintf(sector_win->info, "%2d,%-2d to %2d,%-2d", x_pressed_x,
		    x_pressed_y, x_prev_x, x_prev_y);
	    x2 = x_prev_x * 16 + adjx + 7;
	    y2 = x_prev_y * 16 + adjy + 7;
	    draw_box(x1, y1, x2, y2);
	}
    }
    sector_redraw_info(sector_win, FALSE);
}

void button_press(mwin *win, XButtonEvent *ev)
{
    int adjx, adjy;

    if (x_is_map) {
	adjx = (win->core.w - x_maxx * 16) / 2 + 1;
	adjy = (win->core.h - x_maxy * 16) / 2 + 1;
	x_pressed_x = (int) (ev->x - adjx);
	x_pressed_y = (int) (ev->y - adjy);
	if ((x_pressed_x >= 0 && x_pressed_y >= 0)) {
	    x_pressed_x = (int) (x_pressed_x / 16);
	    x_pressed_y = (int) (x_pressed_y / 16);
	    if (x_pressed_x <= (x_maxx - 1) && x_pressed_y <= (x_maxy - 1)) {
		x_prev_x = x_pressed_x;
		x_prev_y = x_pressed_y;
		switch (ev->button) {
		case (Button1):
		    draw_box(x_pressed_x, x_pressed_y, x_prev_x, x_prev_y);
		    x_is_pressed = 1;
		    break;
		case (Button2):
		    draw_line(x_pressed_x, x_pressed_y, x_prev_x, x_prev_y);
		    x_is_pressed = 2;
		    break;
		case (Button3):
		    draw_line(x_pressed_x, x_pressed_y, x_prev_x, x_prev_y);
		    x_is_pressed = 3;
		    break;
		}
	    } else {
		x_is_pressed = FALSE;
	    }
	} else
	    x_is_pressed = FALSE;
    } else if (x_is_orbit) {
	x_prev_x = x_pressed_x = ev->x;
	x_prev_y = x_pressed_y = ev->y;
	x_is_pressed = ev->button;
    } else
	x_is_pressed = FALSE;
}

void button_release(mwin *win, XButtonEvent *ev)
{
    char n[30];
    char u[10];
    int x1, x2, y1, y2, xt;
    int adjx, adjy;

    if (!x_is_pressed)
	return;

    if (x_is_map) {
	adjx = (win->core.w - x_maxx * 16) / 2 + 1;
	adjy = (win->core.h - x_maxy * 16) / 2 + 1;

	x1 = x_pressed_x * 16 + adjx + 7;
	y1 = x_pressed_y * 16 + adjy + 7;
	x2 = x_prev_x * 16 + adjx + 7;
	y2 = x_prev_y * 16 + adjy + 7;
	if (x_is_pressed == 1)
	    draw_box(x1, y1, x2, y2);
	else
	    draw_line(x1, y1, x2, y2);

	/*
	 * for move/deploy wrapping in x direction, x_prev_x will be off the
	 * map.  xt is the wrapped x coordinate.
	 */
	if (x_prev_x < 0)
	    xt = x_maxx - 1;
	else if (x_prev_x == x_maxx)
	    xt = 0;
	else
	    xt = x_prev_x;

	switch ((3 * (x_pressed_y - x_prev_y)) + (x_pressed_x - x_prev_x)) {
	case (1):
	    sprintf(u, "4");
	    break;
	case (2):
	    sprintf(u, "9");
	    break;
	case (3):
	    sprintf(u, "8");
	    break;
	case (4):
	    sprintf(u, "7");
	    break;
	case (-1):
	    sprintf(u, "6");
	    break;
	case (-2):
	    sprintf(u, "1");
	    break;
	case (-3):
	    sprintf(u, "2");
	    break;
	case (-4):
	    sprintf(u, "3");
	    break;
	}
	switch (ev->button) {
	case (Button1):
	    if ((x_pressed_x != x_prev_x) || (x_pressed_y != x_prev_y)) {
		sprintf(u, "%d:%d,%d:%d", min(x_pressed_x, x_prev_x),
			max(x_pressed_x, x_prev_x),
			min(x_pressed_y, x_prev_y),
			max(x_pressed_y, x_prev_y));
		sprintf(n, "survey %s\n", u);
		send_xmap_command(n, 1);
		send_client_survey(u, 1);
	    } else {
		sprintf(u, "%d,%d", x_pressed_x, x_pressed_y);
		sprintf(n, "survey %s\n", u);
		send_xmap_command(n, 1);
		send_client_survey(u, 1);
	    }
	    break;
	case (Button2):
	    if ((x_pressed_x != x_prev_x) || (x_pressed_y != x_prev_y)) {
		xmap_mover(x_pressed_x, x_pressed_y, xt, x_prev_y, u);
	    } else {
		sprintf(sector_win->info, "%2d,%-2d", x_prev_x, x_prev_y);
		sector_redraw_info(sector_win, FALSE);
	    }
	    break;
	case (Button3):
	    if ((x_pressed_x != x_prev_x) || (x_pressed_y != x_prev_y)) {
		xmap_deploy(x_pressed_x, x_pressed_y, xt, x_prev_y, u);
	    } else {
		sprintf(sector_win->info, "%2d,%-2d", x_prev_x, x_prev_y);
		sector_redraw_info(sector_win, FALSE);
	    }
	    break;
	}
    } else if (x_is_orbit) {
	draw_box(x_pressed_x, x_pressed_y, x_prev_x, x_prev_y);
	if (set_zoom(x_pressed_x, x_pressed_y, ev->x, ev->y)) {
	    clear(map_win);
	    xmap_replot_orbit(xOrbit_head);
	}
    }
    x_is_pressed = FALSE;
}

/*
 * xmap_mover and xmap_deploy are taken from the Imap routines written by
 * Evan Koffler (koffleva@darkwing.uoregon.edu) modified for use in xmap by
 * Rich Fitzgerald (fitz@rsj.harvard.edu)
 *
 * See the COPYRIGHT file
 */
void xmap_mover(int x, int y, int x2, int y2, char *u)
{
    xSector *p = xcur_map.ptr;
    xSector *q = xcur_map.ptr;
    int amt;
    double apcost;
    char u2;
    char sbuf[40];

    p += x + (y * xcur_map.maxx);
    if (p->own != profile.raceid) {
	msg("You don't own that sector!");
	clear_input();
	return;
    }
    q += x2 + (y2 * xcur_map.maxx);
    if (input_win->info[0] == '\0')
	amt = XDefaultCivsToMove;	/* specified in xmap.h */
    else
	amt = atoi(input_win->info);
    clear_input();
    if (!amt)
	return;
    if (p->civ >= abs(amt)) {
	if (p->own && q->own && p->own != q->own) {
	    apcost = (int) log(1.0 + (double) amt) + 2;
	} else
	    apcost = (int) log(1.0 + (double) amt) + 1;
	if (apcost > scope.aps) {
	    msg("You don't have enough APs.  Need %.lf", apcost);
	    return;
	}
	sprintf(sbuf, "move %d,%-d %s %d\n", x, y, u, amt);
	send_xmap_command(sbuf, 1);
	sprintf(sector_win->info, "%2d,%-2d", x2, y2);

	/* update moved-from, to sectors.  If conflict, ping & redraw later */
	if (q->own && (q->own != profile.raceid)) {
	    xmap_ping(p);	/* in case of repulsed attack */
	    xmap_ping(q);	/* need to get result of attack */
	} else {
	    if (!q->own) {
		q->own = profile.raceid;
		q->inverse = 1;
	    }
	    if (amt < 0) {
		q->civ += p->civ + amt;
		p->civ = -amt;
	    } else {
		p->civ -= amt;
		q->civ += amt;
	    }
	    if (!p->mil && !p->civ) {
		p->own = 0;
		p->inverse = 0;
	    }
	    u2 = xmap_get_graph(x, y);
	    draw_sec(map_win, xcur_map.maxx, xcur_map.maxy, x, y, u2,
		     p->inverse);
	    u2 = xmap_get_graph(x2, y2);
	    draw_sec(map_win, xcur_map.maxx, xcur_map.maxy, x2, y2, u2,
		     q->inverse);

	    /* update sector data for moved-to sector */
	    sector_redraw_info(sector_win, FALSE);

	    /* update display */
	    (*map_win->core.procs.expose) (map_win);
	}
    } else {
	msg("Not enough people to move.");
	return;
    }
}

void xmap_deploy(int x, int y, int x2, int y2, char *u)
{
    xSector *p = xcur_map.ptr;
    xSector *q = xcur_map.ptr;
    int amt;
    double apcost;
    char u2;
    char sbuf[40];

    p += x + (y * xcur_map.maxx);
    if (p->own != profile.raceid) {
	msg("You don't own that sector!");
	clear_input();
	return;
    }
    q += x2 + (y2 * xcur_map.maxx);
    if (input_win->info[0] == '\0')
	amt = XDefaultMilToMove;/* specified in xmap.h */
    else
	amt = atoi(input_win->info);
    clear_input();
    if (!amt)
	return;
    if (p->mil >= abs(amt)) {
	if (p->own && q->own && p->own != q->own) {
	    apcost = (int) log10(1.0 + (double) amt) + 2;
	} else
	    apcost = (int) log10(1.0 + (double) amt) + 1;
	if (apcost > scope.aps) {
	    msg("You don't have enough APs.  Need %.lf", apcost);
	    return;
	}
	if (amt < 0) {
	    q->mil += p->mil + amt;
	    p->mil = -amt;
	} else {
	    p->mil -= amt;
	    q->mil += amt;
	}
	sprintf(sbuf, "deploy %d,%-d %s %d\n", x, y, u, amt);
	send_xmap_command(sbuf, 1);
	sprintf(sector_win->info, "%2d,%-2d", x2, y2);
	/* update moved-from, to sectors.  If conflict, ping & redraw later */
	if (!p->mil) {
	    if (!p->numships)
		p->des = p->sectc;	/* no longer show troop symbol */
	    if (!p->civ) {
		p->own = 0;
		p->inverse = 0;
	    }
	}
	if (q->own && (q->own != profile.raceid)) {
	    xmap_ping(p);	/* in case of repulsed attack */
	    xmap_ping(q);	/* need to get result of attack */
	} else {
	    if (!q->own) {
		q->own = profile.raceid;
		q->inverse = 1;
	    }
	    if (!q->numships)
		q->des = 'X';	/* manually add troop symbol */

	    u2 = xmap_get_graph(x, y);
	    draw_sec(map_win, xcur_map.maxx, xcur_map.maxy, x, y, u2,
		     p->inverse);
	    u2 = xmap_get_graph(x2, y2);
	    draw_sec(map_win, xcur_map.maxx, xcur_map.maxy, x2, y2, u2,
		     q->inverse);

	    /* update sector data for moved-to sector */
	    sector_redraw_info(sector_win, FALSE);

	    /* update display */
	    (*map_win->core.procs.expose) (map_win);
	}
    } else {
	msg("Not enough troops to move.");
	return;
    }
}

void xmap_ping(xSector *q)
{
    char pingbuf[25];

    sprintf(pingbuf, "%d,%-d", q->x, q->y);
    send_client_survey(pingbuf, 1);
}

/* pressing any button in input window moves insertion point */
void set_cur_char_pos(in_win *win, XButtonEvent *ev)
{
    int new_pos;

    new_pos = ((float) (ev->x - Input_x)) / 6.0 + 0.5;
    if (new_pos < 0)
	new_pos = 0;
    else if (new_pos > strlen(win->info))
	new_pos = strlen(win->info);
    XDrawString(win->core.dpy, win->core.win, clear_gc, 6 * win->cur_char_pos
		+ Input_x, Input_y + 10, Input_prompt, Input_prompt_len);
    win->cur_char_pos = new_pos;
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, 6 * win->cur_char_pos
		+ Input_x, Input_y + 10, Input_prompt, Input_prompt_len);
}

/* Respond to a keypress sent to the window */
void key_event(mwin *win, XKeyEvent *ev)
{
    int nchar;
    char buf[10];
    KeySym keysym;

    nchar = XLookupString(ev, buf, sizeof buf, &keysym,
			  (XComposeStatus *) NULL);

    /* check for printable ascii, and only no/shift/caps lock modifiers */
    if ((keysym >= XK_space) && (keysym <= XK_asciitilde) && (ev->state < 4))
	add_input(buf[0]);
    else
	switch ((int)keysym) {
	case (XK_Escape):
	    clear_input();
	    break;
	case (XK_Return):
	    do_input();
	    break;
	case (XK_BackSpace):
	case (XK_Delete):
	    remove_input();
	    break;
	}
}

void add_input(char the_char)
{
    int i;

    if (input_win->len <= 100) {
	for (i = input_win->len++; i >= input_win->cur_char_pos; i--)
	    input_win->info[i + 1] = input_win->info[i];
	input_win->info[input_win->cur_char_pos] = the_char;
	input_win->cur_char_pos++;
	input_update(input_win, input_win->cur_char_pos - 1);
    }
}

void remove_input()
{
    int i;

    if (input_win->cur_char_pos >= 1) {
	for (i = --input_win->cur_char_pos; i < input_win->len; i++)
	    input_win->info[i] = input_win->info[i + 1];
	input_win->len--;
	input_update(input_win, input_win->cur_char_pos);
    }
}

void do_input()
{
    add_history(input_win->info);	/* must be here to prevent prompt */
    process_key(input_win->info, TRUE);
    clear_input();
}

void clear_input()
{
    input_win->cur_char_pos = input_win->len = 0;
    input_win->info[0] = '\0';
    input_update(input_win, 0);
}

/* Input window parameters are defined in xmap.h.  */
void input_redraw(in_win *win)
{
    int x;

    XClearWindow(win->core.dpy, win->core.win);

    x = Input_x - 15;
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, x, Input_y,
		win->label, strlen(win->label));

    XDrawString(win->core.dpy, win->core.win, sec_text_gc, Input_x, Input_y,
		win->info, strlen(win->info));

    x = 6 * (strlen(win->info)) + Input_x;
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, x, Input_y + 10,
		Input_prompt, Input_prompt_len);
}

void input_update(in_win *win, int pos)
{
    int x;

    x = 6 * pos + Input_x;

    XFillRectangle(win->core.dpy, win->core.win, clear_gc, x, 0,
		   win->core.w, win->core.h);
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, x, Input_y,
		win->info + pos, strlen(win->info + pos));
    x = 6 * win->cur_char_pos + Input_x;
    XDrawString(win->core.dpy, win->core.win, sec_text_gc, x, Input_y + 10,
		Input_prompt, Input_prompt_len);
}

/* Keyboard commands */
void key_command(win, cmd)
mwin *win;
char cmd;
{
}


void mwin_event_loop()
{
    register widget *trv;
    int moo;
    XEvent ev, dummy;

    if (Xavailable == FALSE)
	return;

    if (xmap_expecting && !is_xmap_survey)
	xmap_plot_continue();

    moo = XPending(mdpy);
    while (moo >= 1) {
	moo = moo - 1;
	XNextEvent(mdpy, &ev);
	for (trv = widget_list; trv->next != NULL; trv = trv->next)
	    if (trv->win == ev.xany.window)
		break;
	if (trv != NULL)
	    switch (ev.type) {
	    case ConfigureNotify:
		while (XCheckTypedWindowEvent(mdpy, trv->win,
					      ConfigureNotify, &dummy))
		    moo--;
		if (ev.xconfigure.width != trv->w ||
		    ev.xconfigure.height != trv->h)
		    trv->resized = 1;
		trv->x = ev.xconfigure.x;
		trv->y = ev.xconfigure.y;
		trv->w = ev.xconfigure.width;
		trv->h = ev.xconfigure.height;
		(*trv->procs.resize) (trv, ev.xconfigure);
		break;

	    case ButtonPress:
		(*trv->procs.btn_down) (trv, &ev.xbutton);
		break;

	    case ButtonRelease:
		(*trv->procs.btn_up) (trv, &ev.xbutton);
		break;

	    case Expose:
		while (XCheckTypedWindowEvent(mdpy, trv->win, Expose, &dummy))
		    moo--;
		(*trv->procs.expose) (trv, &ev.xexpose);
		break;

	    case MotionNotify:
		(*trv->procs.motion) (trv, &ev.xmotion);
		break;

	    case EnterNotify:
		(*trv->procs.enter) (trv, &ev.xcrossing);
		break;

	    case LeaveNotify:
		(*trv->procs.leave) (trv, &ev.xcrossing);
		break;

	    case KeyPress:
		(*trv->procs.key) (trv, &ev.xkey);
		break;

	    case MapNotify:
		trv->is_mapped = 1;
		break;
	    case UnmapNotify:
		trv->is_mapped = 0;
		break;

	    }
    }
}

/*
 * The following map and orbit routines were taken from map.c and imap.c
 * written by Evan D. Koffler and modified for use in xmap by Scott
 * Anderson and Rich Fitzgerald (fitz@rsj.harvard.edu).
 * See the COPYRIGHT file.
 */
void xmap_plot_orbit(char *t)
{
    char *p, *q;
    xOrb *cur_pos, *next_pos;
    xOrb orbit_data;
    int gave_msg;

#ifdef CLIENT_DEVEL
    if (client_devel)
	msg(":: xmap_plot_orbit()");
#endif

    if ((Xavailable == FALSE) || !(xmap_on & Do_Orbit)) {
	plot_orbit(t);
	return;
    }
    if (!orb_panel->core.is_mapped)
	MapWidget(orb_panel->core);
    if (!cdhere_win->core.is_mapped)
	MapWidget(cdhere_win->core);

    /* start off showing full range */
    is_zoom = 0;
    orb_min_x = Orb_Default_min_x;
    orb_max_x = Orb_Default_max_x;
    orb_min_y = Orb_Default_min_y;
    orb_max_y = Orb_Default_max_y;

    clear(map_win);

    /* free old orbit info */
    cur_pos = xOrbit_head;
    while (cur_pos) {
	next_pos = cur_pos->next;
	free(cur_pos);
	cur_pos = next_pos;
    }
    xOrbit_head = NULL;

    gave_msg = FALSE;

    q = t + 1;
    while ((p = strchr(q, ';'))) {	/* new */
	*p = '\0';		/* new */
	if ((next_pos = (xOrb *) malloc(sizeof(xOrb))) == NULL) {
	    if (!gave_msg) {
		msg("Unable to allocate orbit memory; functions may not work.");
		gave_msg = TRUE;
	    }
	    sscanf(q, "%d %d %d %d %c %d %s", &orbit_data.stand1, &orbit_data.x,
		   &orbit_data.y, &orbit_data.array, &orbit_data.symbol,
		   &orbit_data.stand2, orbit_data.name);
	    orbit_data.next = orbit_data.prev = NULL;
	    xmap_replot_orbit(&orbit_data);
	} else {
	    sscanf(q, "%d %d %d %d %c %d %s", &next_pos->stand1, &next_pos->x,
		   &next_pos->y, &next_pos->array, &next_pos->symbol,
		   &next_pos->stand2, next_pos->name);
	    if (cur_pos) {
		cur_pos->next = next_pos;
		next_pos->prev = cur_pos;
	    } else {
		next_pos->prev = NULL;
		xOrbit_head = next_pos;
	    }
	    next_pos->next = NULL;
	    cur_pos = next_pos;
	}
	q = p + 1;		/* new */
    }

    /* take guess at scope and update window name */
    if ((xOrbit_head) && (xOrbit_head->symbol == '*')) {
	if ((xOrbit_head->next) && (xOrbit_head->next->symbol == '*')) {
	    /* 2+ stars: universal scope */
	    strcpy(x_scope, "/");
	} else {		/* 1 star: guess star scope */
	    sprintf(x_scope, "/%s", xOrbit_head->name);
	}
    } else {			/* don't know: guess current scope */
	strcpy(x_scope, build_scope());
    }
    draw_name(map_win, x_scope);

    xmap_replot_orbit(xOrbit_head);
    x_is_map = FALSE;
    x_is_orbit = TRUE;
    x_is_pressed = FALSE;
    /* clear out any leftover planet stats */
    (*bottom_panel->core.procs.expose) (bottom_panel);
}

void xmap_replot_orbit(xOrb *head)
{
    xOrb *cur_pos = head;
    xOrb *next_pos;
    int x, y;
    float scalex, scaley;
    int not_ship;

#ifdef CLIENT_DEVEL
    if (client_devel)
	msg(":: xmap_replot_orbit()");
#endif

/* Mirrors and Novae no longer supported */
/*
    extern char *Novae[16][7];
    extern char *Mirror[8][5];
*/

    scalex = (map_win->core.w - XRESERVED) / (orb_max_x - orb_min_x);
    scaley = (map_win->core.h - YRESERVED) / (orb_max_y - orb_min_y);

    if (reverse_orbit && cur_pos) {	/* go to end of list */
	next_pos = cur_pos->next;
	while (next_pos) {
	    cur_pos = next_pos;
	    next_pos = cur_pos->next;
	}
    }
    while (cur_pos) {
	not_ship = FALSE;
	switch (cur_pos->symbol) {
	case '*':
	case '@':
	case 'o':
	case 'O':
	case '-':
	case '#':
	case '.':
	case '~':
	case ')':
	case '?':
	    if (show_ships == ShowShips)
		break;
	    not_ship = TRUE;
	default:
	    if ((show_ships == ShowNoShips) && (!not_ship))
		break;
	    x = (int) (cur_pos->x - orb_min_x) * scalex + LeftMargin;
	    y = (int) (cur_pos->y - orb_min_y) * scaley + TopMargin;

/*
	    if (cur_pos->symbol == 'M' || cur_pos->symbol == 'm')
		x_DispArray (x, y, 9, 5, Mirror[cur_pos->array], 1.0);
	    else if (cur_pos->symbol == '*' && cur_pos->array > 0)
		x_DispArray (x, y, 11, 7, Novae[cur_pos->array - 1], 1.0);
*/

	    draw_orb(map_win, x, y, &(cur_pos->symbol), 1, cur_pos->stand1);
	    draw_orb(map_win, x + 6, y, " ", 1, 0);
	    draw_orb(map_win, x + 12, y, cur_pos->name, strlen(cur_pos->name),
		     cur_pos->stand2);
	}
	if (reverse_orbit) {
	    next_pos = cur_pos->prev;
	} else {
	    next_pos = cur_pos->next;
	}
	cur_pos = next_pos;
    }
    if (!x_is_redraw)
	(*map_win->core.procs.expose) (map_win);
}

void x_DispArray(int x, int y, int maxx, int maxy, char *array[], float mag)
{
    int x2, y2;
    int cury;

    x2 = x - maxx * 3;		/* with chars 6 pixels wide, this will center */
    cury = y - (maxy / 2) * 13;	/* use 13 pixels/line spacing */
    for (y2 = 0; y2 < maxy; y2++) {
	draw_orb(map_win, x2, cury, array[y2], strlen(array[y2]), 0);
	cury += 13;
    }
}

/*
 * for orbits, need to take data originally in [0:200,0:200] and map into
 * available space in map window.  Zooming will change the size of the data
 * window which gets mapped to the map window.
 */

int set_zoom(int x1, int y1, int x2, int y2)
{
    int x3, x4, y3, y4;		/* for conversion to orbit coordinates */
    float scalex, scaley;

    scalex = (orb_max_x - orb_min_x) / (map_win->core.w - XRESERVED);
    scaley = (orb_max_y - orb_min_y) / (map_win->core.h - YRESERVED);

    x3 = (x1 - LeftMargin) * scalex + orb_min_x;
    x4 = (x2 - LeftMargin) * scalex + orb_min_x;
    y3 = (y1 - TopMargin) * scaley + orb_min_y;
    y4 = (y2 - TopMargin) * scaley + orb_min_y;

    if ((x3 != x4) && (y3 != y4)) {
	orb_min_x = min(x3, x4);
	orb_max_x = max(x3, x4);
	orb_min_y = min(y3, y4);
	orb_max_y = max(y3, y4);
	is_zoom = TRUE;
	return (1);
    } else
	return (0);	/* zoom didn't change; don't need to replot */
}

/* 
 * called by client with map data; set up for drawing map and send for
 * client survey data.
 */
void xmap_plot_surface(char *t)
{

#ifdef CLIENT_DEVEL
    if (client_devel)
	msg(":: xmap_plot_surface()");
#endif

    if ((Xavailable == FALSE) || !(xmap_on & Do_Map)) {
	plot_surface(t);
	return;
    }
    if (!cdhere_win->core.is_mapped)
	MapWidget(cdhere_win->core);
    if (!left_panel->core.is_mapped)
	MapWidget(left_panel->core);
    if (!right_panel->core.is_mapped)
	MapWidget(right_panel->core);

    clear(map_win);

    strcpy(x_map_data, t);
    is_xmap_survey = 1;
    xmap_expecting = 1;

    /*
     * to allow for mapping other scopes, requested scope has been stored in
     * xmap_map_command_scope. need to put in x_scope; need to cs to
     * proper scope for client_survey.
     */

    strcpy(x_scope, xmap_map_command_scope);
    send_client_survey("-", 1);
}

/*
 * called after receiving client survey data; finish setting up map window &
 * display map.
 */
void xmap_plot_continue()
{
    xSector p;
    xSector *q = xcur_map.ptr;
    register int x;
    register int y;
    char maplog[BUFSIZ];
    char convbuf[SMABUF];

    xmap_expecting = 0;

#ifdef CLIENT_DEVEL
    if (client_devel)
	msg(":: xmap_plot_continue()");
#endif

    if (q == NULL) {
	msg("-- Xmap: map allocation failed.");
	x_is_map = FALSE;
	x_is_orbit = FALSE;
	x_is_pressed = FALSE;
	clear(map_win);
	return;
    }
    sprintf(x_scope, "/%s/%s", xcur_map.star, xcur_map.planet);
    draw_name(map_win, x_scope);

    for (y = 0; y < xcur_map.maxy; y++) {
	sprintf(convbuf, "%c%c ", y / 10 + '0', y % 10 + '0');
	strcpy(maplog, convbuf);
	for (x = 0; x < xcur_map.maxx; x++) {
	    p = *(q + x + (y * xcur_map.maxx));
	    strncat(maplog, &(p.des), 1);
	}
	if (GET_BIT(options, MAP_DOUBLE)) {
	    sprintf(convbuf, "%s%c%c",
		    (GET_BIT(options, MAP_SPACE) ? " " : ""),
		    y / 10 + '0', y % 10 + '0');
	    strcat(maplog, convbuf);
	}
	add_recall(maplog, 0);
    }

    x_is_map = TRUE;
    x_is_orbit = FALSE;
    x_is_pressed = FALSE;
    x_maxx = xcur_map.maxx;
    x_maxy = xcur_map.maxy;

    (*bottom_panel->core.procs.expose) (bottom_panel);

    xmap_plot_redraw();
}

void xmap_plot_redraw()
{
    xSector p;
    xSector *q = xcur_map.ptr;
    register int x;
    register int y;
    char u2;

    for (y = 0; y < xcur_map.maxy; y++) {
	for (x = 0; x < xcur_map.maxx; x++) {
	    p = *(q + x + (y * xcur_map.maxx));
	    u2 = xmap_get_graph(x, y);
	    draw_sec(map_win, xcur_map.maxx, xcur_map.maxy, x, y, u2,
		     p.inverse);
	}
    }
    if (!x_is_redraw)
	(*map_win->core.procs.expose) (map_win);
}

/* True if we're ready to take client survey data */
int doing_xmap_command()
{
    return ((x_is_map || is_xmap_survey) && xmap_on && xmap_active);
}

void xmap_input(int comm_num, char *procbuf)
{
    process_xmap_survey(comm_num, procbuf);
}

void process_xmap_survey(int num, char *s)
{
    xSector p;
    char *ptr;
    char *qtr;
    int i;
    char u2;

    switch (num) {
    case CSP_SURVEY_INTRO:
	if (xcur_map.ptr)
	    free(xcur_map.ptr);
	sscanf(s, "%d %d %s %s %d %d %d %d %d %d %lf %d",
	       &xcur_map.maxx, &xcur_map.maxy,
	       xcur_map.star, xcur_map.planet,
	       &xcur_map.res, &xcur_map.fuel, &xcur_map.des,
	       &xcur_map.popn, &xcur_map.mpopn, &xcur_map.tox,
	       &xcur_map.compat, &xcur_map.enslaved);
	xcur_map.maxfert = 0;
	xcur_map.maxres = 0;
	xcur_map.maxeff = 0;
	xcur_map.maxmob = 0;
	xcur_map.maxmil = 0;
	xcur_map.maxpopn = 0;
	xcur_map.maxmpopn = 0;
	xcur_map.minfert = 999999;
	xcur_map.minres = 999999;
	xcur_map.mineff = 999999;
	xcur_map.minmob = 999999;
	xcur_map.minmil = 999999;
	xcur_map.minpopn = 999999;
	xcur_map.minmpopn = 999999;
	xcur_map.ptr = (xSector *) malloc((sizeof(xSector)
					   * xcur_map.maxx * xcur_map.maxy));
	break;
    case CSP_SURVEY_SECTOR:
	if (!xcur_map.ptr)
	    return;
	ptr = strchr(s, ';');
	if (ptr) {
	    *ptr = '\0';
	    ptr++;
	}
	sscanf(s, "%d %d %c %c %d %d %d %d %d %d %d %d %d %d",
	       &p.x, &p.y, &p.sectc, &p.des, &p.wasted, &p.own,
	       &p.eff, &p.frt, &p.mob, &p.xtal, &p.res, &p.civ,
	       &p.mil, &p.mpopn);
	p.numships = 0;
	if (p.frt > xcur_map.maxfert)
	    xcur_map.maxfert = p.frt;
	if (p.res > xcur_map.maxres)
	    xcur_map.maxres = p.res;
	if (p.eff > xcur_map.maxeff)
	    xcur_map.maxeff = p.eff;
	if (p.mob > xcur_map.maxmob)
	    xcur_map.maxmob = p.mob;
	if (p.mil > xcur_map.maxmil)
	    xcur_map.maxmil = p.mil;
	if (p.civ > xcur_map.maxpopn)
	    xcur_map.maxpopn = p.civ;
	if (p.mpopn > xcur_map.maxmpopn)
	    xcur_map.maxmpopn = p.mpopn;
	if (p.frt < xcur_map.minfert)
	    xcur_map.minfert = p.frt;
	if (p.res < xcur_map.minres)
	    xcur_map.minres = p.res;
	if (p.eff < xcur_map.mineff)
	    xcur_map.mineff = p.eff;
	if (p.mob < xcur_map.minmob)
	    xcur_map.minmob = p.mob;
	if (p.mil < xcur_map.minmil)
	    xcur_map.minmil = p.mil;
	if (p.civ < xcur_map.minpopn)
	    xcur_map.minpopn = p.civ;
	if (p.mpopn < xcur_map.minmpopn)
	    xcur_map.minmpopn = p.mpopn;
	if (ptr) {
	    i = 0;
	    qtr = ptr;
	    while (qtr && *qtr && (ptr = strchr(qtr, ';')) &&
		   i < MAX_SHIPS) {
		*ptr = '\0';
		sscanf(qtr, "%d %c %d", &p.ships[i].shipno,
		       &p.ships[i].ltr, &p.ships[i].owner);
		i++;
		qtr = ptr + 1;
	    }
	    p.numships = i;
	    if (i)
		p.des = p.ships[i - 1].ltr;
	}
	p.inverse = (p.own == profile.raceid);
	/*
	 * above setting of inverse IS NOT CORRECT!  Doesn't properly reflect
	 * toggle highlight.
	 */
	*(xcur_map.ptr + p.x + (p.y * xcur_map.maxx)) = p;

	/* must be ping if !is_xmap_survey */
	if (!is_xmap_survey) {
	    u2 = xmap_get_graph(p.x, p.y);
	    draw_sec(map_win, xcur_map.maxx, xcur_map.maxy, p.x, p.y, u2,
		     p.inverse);
	}
	break;
    case CSP_SURVEY_END:
	if (!is_xmap_survey) {
	    /* finished with ping; update bottom_panel and redraw map */
	    clear_sector_data(bottom_panel);
	    sector_display_stats(bottom_panel);
	    (*map_win->core.procs.expose) (map_win);
	}
	is_xmap_survey = 0;
	break;
    default:
	msg("-- Unknown client_survey #%d", num);
	break;
    }
}

/* character to display on map */
char xmap_get_graph(int x, int y)
{
    xSector p;
    xSector *q = xcur_map.ptr;
    int i;

    p = *(q + x + (y * xcur_map.maxx));

    if (normal_map || !x_graph_type)
	return p.des;		/* orig. p.sectc */

    switch (x_graph_type) {
    case (FERT_GRAPH):
	i = (p.frt - xcur_map.minfert) * 10 / (xcur_map.maxfert ==
	      xcur_map.minfert ? 1 : (xcur_map.maxfert - xcur_map.minfert));
	break;
    case (RES_GRAPH):
	i = (p.res - xcur_map.minres) * 10 / (xcur_map.maxres ==
		   xcur_map.minres ? 1 : xcur_map.maxres - xcur_map.minres);
	break;
    case (EFF_GRAPH):
	i = (p.eff - xcur_map.mineff) * 10 / (xcur_map.maxeff ==
		   xcur_map.mineff ? 1 : xcur_map.maxeff - xcur_map.mineff);
	break;
    case (MOB_GRAPH):
	i = (p.mob - xcur_map.minmob) * 10 / (xcur_map.maxmob ==
		   xcur_map.minmob ? 1 : xcur_map.maxmob - xcur_map.minmob);
	break;
    case (MIL_GRAPH):
	i = (p.mil - xcur_map.minmil) * 10 / (xcur_map.maxmil ==
		   xcur_map.minmil ? 1 : xcur_map.maxmil - xcur_map.minmil);
	break;
    case (POPN_GRAPH):
	i = (p.civ - xcur_map.minpopn) * 10 / (xcur_map.maxpopn ==
		xcur_map.minpopn ? 1 : xcur_map.maxpopn - xcur_map.minpopn);
	break;
    case (MPOPN_GRAPH):
	i = (p.mpopn - xcur_map.minmpopn) * 10 / (xcur_map.maxmpopn ==
	     xcur_map.minmpopn ? 1 : xcur_map.maxmpopn - xcur_map.minmpopn);
	break;
    case (GEOG_GRAPH):
	return p.sectc;
    }

    i = (i == 10 ? 9 : i);
    switch (i) {
    case (0):
	return '0';
    case (1):
	return '1';
    case (2):
	return '2';
    case (3):
	return '3';
    case (4):
	return '4';
    case (5):
	return '5';
    case (6):
	return '6';
    case (7):
	return '7';
    case (8):
	return '8';
    case (9):
	return '9';
    }
    return p.des;
}

/* Xmap setup utilities */
widget *create_mwin(Display *dpy, widget *parent, int x, int y, int w, int h)
{
    mwin *win;
    Window root;
    int black, white;

    if (!Xavailable)
	return NULL;
    win = (mwin *) malloc(sizeof(mwin));
    if (win == NULL) {
	fprintf(stderr, "Map widget allocation failed\n");
	Xavailable = FALSE;
	return NULL;
    }
    root = DefaultRootWindow(dpy);

    black = BlackPixel(dpy, DefaultScreen(dpy));
    white = WhitePixel(dpy, DefaultScreen(dpy));

    win->core.next = NULL;
    win->core.win = XCreateSimpleWindow(dpy, root, x, y, w, h, 1, white, black);
    /* Fill it in */
    XSelectInput(dpy, win->core.win, WIDGET_EVENT_MASK);
    win->core.dpy = dpy;
    win->core.parent = parent;
    win->core.type = &mwin_type;
    win->core.x = x;
    win->core.y = y;
    win->core.des_w = win->core.w = 0;	/* set to 0 to trigger resize calcs */
    win->core.des_h = win->core.h = 0;	/* when first drawn. */
    win->core.is_mapped = 0;
    win->core.procs.btn_up = button_release;
    win->core.procs.btn_down = button_press;
    win->core.procs.motion = map_move;
    win->core.procs.expose = redraw;
    win->core.procs.resize = resize;
    win->core.procs.leave = empty_proc;
    win->core.procs.enter = empty_proc;
    win->core.procs.key = key_event;

    win->bitdepth = DefaultDepth(dpy, 0);

    win->buffer = XCreatePixmap(win->core.dpy, win->core.win, w, h,
				win->bitdepth);
    win->core.resized = 0;

    XStoreName(win->core.dpy, win->core.win, XMAP_VERSION);

    add_new_widget((widget *)win);
    return (widget *) win;
}

widget *create_panel(Display *dpy, widget *parent, int x, int y, int w, int h)
{
    pan_win *win;
    Window root;
    int white, black;

    if (!Xavailable)
	return NULL;
    win = (pan_win *) malloc(sizeof(pan_win));
    if (win == NULL) {
	fprintf(stderr, "Panel widget allocation failed\n");
	Xavailable = FALSE;
	return NULL;
    }
    root = DefaultRootWindow(dpy);
    black = BlackPixel(dpy, DefaultScreen(dpy));
    white = WhitePixel(dpy, DefaultScreen(dpy));


    win->core.next = NULL;
    win->core.win = XCreateSimpleWindow(dpy, parent ? parent->win : root,
					x, y, w, h, 1, white, black);
    /* Fill it in */
    XSelectInput(dpy, win->core.win, WIDGET_EVENT_MASK);
    win->core.dpy = dpy;
    win->core.parent = parent;
    win->core.type = &pan_win_type;
    win->core.x = x;
    win->core.y = y;
    win->core.des_w = win->core.w = w;
    win->core.des_h = win->core.h = h;
    win->core.is_mapped = 0;
    win->core.procs.btn_up = empty_proc;
    win->core.procs.btn_down = empty_proc;
    win->core.procs.expose = empty_proc;
    win->core.procs.leave = empty_proc;
    win->core.procs.enter = empty_proc;
    win->core.procs.resize = empty_proc;
    win->core.procs.key = key_event;
    win->core.procs.motion = empty_proc;

    win->core.resized = 0;

    add_new_widget((widget *)win);
    XClearWindow(win->core.dpy, win->core.win);
    return (widget *) win;
}

widget *create_button(Display *dpy, widget *parent, int x, int y, int w, int h)
{
    but_win *win;
    Window root;
    int black, white;

    if (!Xavailable)
	return NULL;
    win = (but_win *) malloc(sizeof(but_win));
    if (win == NULL) {
	fprintf(stderr, "Button widget allocation failed\n");
	Xavailable = FALSE;
	return NULL;
    }
    root = DefaultRootWindow(dpy);

    black = BlackPixel(dpy, DefaultScreen(dpy));
    white = WhitePixel(dpy, DefaultScreen(dpy));

    win->core.next = NULL;
    win->core.win = XCreateSimpleWindow(dpy, parent ? parent->win : root,
					x, y, w, h, 1, white, black);
    /* Fill it in */
    XSelectInput(dpy, win->core.win, WIDGET_EVENT_MASK);
    win->core.dpy = dpy;
    win->core.parent = parent;
    win->core.type = &but_win_type;
    win->core.x = x;
    win->core.y = y;
    win->core.des_w = win->core.w = w;
    win->core.des_h = win->core.h = h;
    win->core.is_mapped = 0;
    win->core.procs.btn_up = do_button_up;
    win->core.procs.btn_down = do_button_down;
    win->core.procs.expose = button_redraw;
    win->core.procs.leave = hilite_command_out;
    win->core.procs.enter = hilite_command_in;
    win->core.procs.resize = empty_proc;
    win->core.procs.key = key_event;
    win->core.procs.motion = empty_proc;

    win->core.resized = 0;

    add_new_widget((widget *)win);
    XClearWindow(win->core.dpy, win->core.win);
    return (widget *) win;
}

widget *create_input(Display *dpy, widget *parent, int x, int y, int w, int h)
{
    in_win *win;
    Window root;
    int black, white;

    if (!Xavailable)
	return NULL;
    win = (in_win *) malloc(sizeof(in_win));
    if (win == NULL) {
	fprintf(stderr, "Input widget allocation failed\n");
	Xavailable = FALSE;
	return NULL;
    }
    root = DefaultRootWindow(dpy);

    black = BlackPixel(dpy, DefaultScreen(dpy));
    white = WhitePixel(dpy, DefaultScreen(dpy));

    win->core.next = NULL;
    win->core.win = XCreateSimpleWindow(dpy, parent ? parent->win : root,
					x, y, w, h, 1, white, black);
    /* Fill it in */
    XSelectInput(dpy, win->core.win, WIDGET_EVENT_MASK);
    win->core.dpy = dpy;
    win->core.parent = parent;
    win->core.type = &in_win_type;
    win->core.x = x;
    win->core.y = y;
    win->core.des_w = win->core.w = w;
    win->core.des_h = win->core.h = h;
    win->core.is_mapped = 0;
    win->core.procs.btn_up = set_cur_char_pos;
    win->core.procs.btn_down = empty_proc;
    win->core.procs.motion = empty_proc;
    win->core.procs.expose = input_redraw;
    win->core.procs.resize = empty_proc;
    win->core.procs.leave = empty_proc;
    win->core.procs.enter = empty_proc;
    win->core.procs.key = key_event;

    win->core.resized = 0;

    add_new_widget((widget *)win);
    XClearWindow(win->core.dpy, win->core.win);
    return (widget *) win;
}

widget *create_sector(Display *dpy, widget *parent, int x, int y, int w, int h)
{
    sec_win *win;
    Window root;
    int black, white;

    if (!Xavailable)
	return NULL;
    win = (sec_win *) malloc(sizeof(sec_win));
    if (win == NULL) {
	fprintf(stderr, "Sector widget allocation failed\n");
	Xavailable = FALSE;
	return NULL;
    }
    root = DefaultRootWindow(dpy);

    black = BlackPixel(dpy, DefaultScreen(dpy));
    white = WhitePixel(dpy, DefaultScreen(dpy));

    win->core.next = NULL;
    win->core.win = XCreateSimpleWindow(dpy, parent ? parent->win : root,
					x, y, w, h, 1, white, black);
    /* Fill it in */
    XSelectInput(dpy, win->core.win, WIDGET_EVENT_MASK);
    win->core.dpy = dpy;
    win->core.parent = parent;
    win->core.type = &sec_win_type;
    win->core.x = x;
    win->core.y = y;
    win->core.des_w = win->core.w = w;
    win->core.des_h = win->core.h = h;
    win->core.is_mapped = 0;
    win->core.procs.btn_up = empty_proc;
    win->core.procs.btn_down = empty_proc;
    win->core.procs.motion = empty_proc;
    win->core.procs.expose = sector_redraw;
    win->core.procs.resize = empty_proc;
    win->core.procs.leave = empty_proc;
    win->core.procs.enter = empty_proc;
    win->core.procs.key = key_event;

    win->core.resized = 0;

    add_new_widget((widget *)win);
    XClearWindow(win->core.dpy, win->core.win);
    return (widget *) win;
}

void create_gcs(Display *dpy)
{
    XGCValues gcv;
    unsigned long gc_mask;
    Window root;
    int black, white;

    button_font = XLoadFont(mdpy, "9x15");
    sec_font = XLoadFont(mdpy, "6x13");

    root = DefaultRootWindow(dpy);

    black = BlackPixel(dpy, DefaultScreen(dpy));
    white = WhitePixel(dpy, DefaultScreen(dpy));

    /* define graphics contexts for drawing and redrawing lines */

    gcv.function = GXcopy;
    gcv.font = sec_font;
    gcv.background = white;
    gcv.foreground = black;
    gcv.fill_style = FillSolid;
    gc_mask = GCFont | GCFunction | GCForeground | GCBackground | GCFillStyle;
    clear_gc = XCreateGC(dpy, root, gc_mask, &gcv);

    gcv.function = GXxor;
    gcv.font = sec_font;
    gcv.foreground = white ^ black;
    gcv.fill_style = FillSolid;
    gc_mask = GCFont | GCFunction | GCForeground | GCFillStyle;
    invert_gc = XCreateGC(dpy, root, gc_mask, &gcv);

    gcv.font = button_font;
    gcv.function = GXcopy;
    gcv.foreground = white;
    gcv.background = black;
    gcv.fill_style = FillSolid;
    gc_mask = GCFont | GCFunction | GCForeground | GCBackground | GCFillStyle;
    but_text_gc = XCreateGC(dpy, root, gc_mask, &gcv);

    gcv.font = sec_font;
    gcv.function = GXcopy;
    gcv.foreground = white;
    gcv.background = black;
    gcv.fill_style = FillSolid;
    gc_mask = GCFont | GCFunction | GCForeground | GCBackground | GCFillStyle;
    sec_text_gc = XCreateGC(dpy, root, gc_mask, &gcv);
}

void setup_button(but_win *win, char *name, int tog, function proc, int graph)
{
    sprintf(win->label, name);
    win->toggle = tog;
    win->button_proc_down = NULL;
    win->button_proc_up = proc;
    win->graph_type = graph;
}

void make_mwin()
{
    widget_list = NULL;

    x_is_map = FALSE;
    x_is_orbit = FALSE;
    x_is_redraw = FALSE;
    no_sector = TRUE;
    x_maxx = 0;
    x_maxy = 0;
    Xavailable = TRUE;

    mdpy = XOpenDisplay("");
    if (mdpy == NULL)
	Xavailable = FALSE;
    else
	create_gcs(mdpy);

    xmap_on = Do_Map | Do_Orbit;

    map_win = (mwin *) create_widget(mwin_type, mdpy, NULL, 0, 0, 884, 430);

    input_win = (in_win *) create_widget(in_win_type, mdpy, map_win,
					 102, 0, 1000, 35);

    bottom_panel = (pan_win *) create_widget(pan_win_type, mdpy, map_win,
					     0, 250, 884, 50);
    left_panel = (pan_win *) create_widget(pan_win_type, mdpy, map_win,
					   -1, -1, 102, 354);
    right_panel = (pan_win *) create_widget(pan_win_type, mdpy, map_win,
					    698, -1, 102, 252);

    sector_win = (sec_win *) create_widget(sec_win_type, mdpy, left_panel,
					   0, 0, 101, 35);

    mapon_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					  0, 36, 101, 26);
    remap_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					  0, 63, 101, 26);
    mapnow_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					   0, 90, 101, 26);

    anal_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					 0, 141, 101, 26);
    sur_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					0, 168, 101, 26);
    rep_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					0, 195, 101, 26);

    cdup_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					 0, 246, 101, 26);
    cduni_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					  0, 273, 101, 26);
    cdhome_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					   0, 300, 101, 26);
    cdhere_win = (but_win *) create_widget(but_win_type, mdpy, left_panel,
					   0, 327, 101, 26);

    graph_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					  -1, 0, 101, 35);
    fert_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					 -1, 36, 101, 26);
    res_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					-1, 63, 101, 26);
    eff_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					-1, 90, 101, 26);
    mob_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					-1, 117, 101, 26);
    mil_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					-1, 144, 101, 26);
    popn_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					 -1, 171, 101, 26);
    mpopn_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					  -1, 198, 101, 26);
    geog_win = (but_win *) create_widget(but_win_type, mdpy, right_panel,
					 -1, 225, 101, 26);
    orb_panel = (pan_win *) create_widget(pan_win_type, mdpy, map_win,
					  698, 274, 102, 80);
    ships_win = (but_win *) create_widget(but_win_type, mdpy, orb_panel,
					  -1, -1, 101, 26);
    reverse_win = (but_win *) create_widget(but_win_type, mdpy, orb_panel,
					    -1, 26, 101, 26);
    zoom_win = (but_win *) create_widget(but_win_type, mdpy, orb_panel,
					 -1, 53, 101, 26);
    last_but_win = NULL;

    if (!Xavailable) {
	msg("-- Couldn't open Xmap display. Going to standard display.");
	if (mdpy)
	    XCloseDisplay(mdpy);
	return;
    }
    bottom_panel->core.procs.expose = draw_panel;

    input_win->cur_char_pos = input_win->len = 0;
    sprintf(input_win->label, "> ");
    input_win->info[input_win->cur_char_pos] = '\0';

    sprintf(sector_win->label, "Current Sector");
    sprintf(sector_win->info, "None");
    sprintf(sector_win->old_info, "None");

    setup_button(remap_win, "RE-MAP", 0, do_remap_up, NOT_GRAPH);
    setup_button(mapnow_win, "MAP", 0, do_mapnow_up, NOT_GRAPH);
    setup_button(anal_win, "ANALYSIS", 0, do_anal_up, NOT_GRAPH);
    setup_button(sur_win, "SURVEY", 0, do_sur_up, NOT_GRAPH);
    setup_button(rep_win, "REPORT", 0, do_rep_up, NOT_GRAPH);
    setup_button(graph_win, "GRAPH", 1, do_graph_up, NOT_GRAPH);
    setup_button(fert_win, "FERTILITY", 1, do_gbut_up, FERT_GRAPH);
    setup_button(res_win, "RESOURCES", 1, do_gbut_up, RES_GRAPH);
    setup_button(eff_win, "EFFICIENCY", 1, do_gbut_up, EFF_GRAPH);
    setup_button(mob_win, "MOBILITY", 1, do_gbut_up, MOB_GRAPH);
    setup_button(mil_win, "MILITARY", 1, do_gbut_up, MIL_GRAPH);
    setup_button(popn_win, "POPULATION", 1, do_gbut_up, POPN_GRAPH);
    setup_button(mpopn_win, "MAX. POPN.", 1, do_gbut_up, MPOPN_GRAPH);
    setup_button(geog_win, "GEOGRAPHY", 1, do_gbut_up, GEOG_GRAPH);
    setup_button(ships_win, ShipLabel[ShowAll], 0, do_shipbut_up, NOT_GRAPH);
    setup_button(reverse_win, "Rev. Orb.", 1, do_revbut_up, NOT_GRAPH);
    setup_button(zoom_win, "Zoom", 0, do_zoom_up, NOT_GRAPH);
    setup_button(mapon_win, xmaplabel[xmap_on], 0, toggle_xmap_on, NOT_GRAPH);
    setup_button(cdup_win, "CS ..", 0, do_cd_up, NOT_GRAPH);
    setup_button(cduni_win, "CS /", 0, do_cd_uni, NOT_GRAPH);
    setup_button(cdhere_win, "CS Here", 0, do_cd_here, NOT_GRAPH);
    setup_button(cdhome_win, "CS Home", 0, do_cd_home, NOT_GRAPH);

    MapWidget(sector_win->core);
    MapWidget(input_win->core);
    MapWidget(mapon_win->core);
    MapWidget(remap_win->core);
    MapWidget(mapnow_win->core);
    MapWidget(anal_win->core);
    MapWidget(rep_win->core);
    MapWidget(sur_win->core);
    MapWidget(graph_win->core);
    MapWidget(fert_win->core);
    MapWidget(res_win->core);
    MapWidget(eff_win->core);
    MapWidget(mob_win->core);
    MapWidget(mil_win->core);
    MapWidget(popn_win->core);
    MapWidget(mpopn_win->core);
    MapWidget(geog_win->core);
    MapWidget(cdup_win->core);
    MapWidget(cduni_win->core);
    MapWidget(cdhome_win->core);
    MapWidget(reverse_win->core);
    MapWidget(ships_win->core);
    MapWidget(zoom_win->core);
    MapWidget(bottom_panel->core);
    MapWidget(left_panel->core);
    MapWidget(map_win->core);

    x_graph_type = 0;
   
    xmap_active = TRUE;
}

void empty_proc()
{
}

void add_new_widget(widget *w)
{
    w->next = widget_list;
    widget_list = w;
}

void MapWidget(widget w)
{
    XMapWindow(w.dpy, w.win);
}

/*
 * routine to handle map commands to get scope.  A must for planet maps,
 * needed for proper client survey.
 */
void cmd_map(char *args)
{
    /*
     * map on server will take care of syntax.  xmap will currently only be
     * called to map planet.  SO, can just parse args for names:  if 1, must
     * be planet; if 2, must be planet & star; all else should default to
     * current scope.
     */

    char star[200], planet[200];
    char *p;

    p = args;
    while ((*p == '.') || (*p == '/'))
	p++;			/* skip past non-names */

    if ((sscanf(p, "%[^/ \n]/%[^/ \n]", star, planet)) == 2) {
	sprintf(xmap_map_command_scope, "/%s/%s", star, planet);
    } else if ((sscanf(p, "%[^/ \n]", planet)) == 1) {
	sprintf(xmap_map_command_scope, "/%s/%s", scope.star, planet);
    } else
	sprintf(xmap_map_command_scope, "/%s/%s", scope.star, scope.planet);
    sprintf(star, "map %s", args);
    strcpy(args, star);
}

/* Initalize xmap */
void cmd_xmap(void)
{
#ifndef RESTRICTED_ACCESS
	/* If XMAP is on, then turn it off. */
	if (xmap_active)
	{
		XCloseDisplay(mdpy);
		xmap_active = FALSE;
	}
	else
	{
		make_mwin();
	}

	if (xmap_active)
	{
		msg("-- Xmap: Activated.");
	}
	else
	{
		msg("-- Xmap: Deactivated.");
	}
		
	return;
#else
	msg ("-- Restricted Access: xmap not available.");
	return;
#endif
}

#endif
/* XMAP */
