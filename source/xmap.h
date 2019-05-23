/****************************************************************************/
/*** XMAP header file...						    */
/*** Written by Scott Anderson (sander@engin.umich.edu)                     */
/*** Modified by Rich Fitzgerald (fitz@rsj.harvard.edu)			    */
/***          								    */
/*** Bugs: See the README_XMAP file     	 			    */
/***                               					    */
/*** History:                                                               */
/*** 3/91 or so: first version of xmap. map drawn in window, and that's it. */
/*** 9/91 or so: small bug fix. not very interesting yet...                 */
/*** 9/18/91   : saves map string for redraw. fixed resize.                 */
/*** 9/30/91   : click on sector for sector survey.                         */
/***             click and drag for sector range survey.                    */
/*** 10/11/91  : CD buttons                                                 */
/*** 10/12/91  : fixed string pointer problem and added REMAP               */
/*** 10/13/91  : fixed scope problem for name                               */
/*** 10/14/91  : added input window. fixed resize (previously the bottom    */
/***             CD panel did not float with the window size). input window */
/***             send commands to the client now. added cursors in input    */
/***             window. Added SURVEY ANALYSIS and REPORT buttons. sector   */
/***             window now only redraws when the sector changes.           */
/*** 10/16/91  : fixed erroneous CS with report button.                     */
/*** 03/09/92  : fixed (temporary) seg fault with resize. added support for */
/***             future graph function. i kinda fixed that annoying multi-  */
/***             flicker with resizes. :)                                   */
/*** 03/10/92  : Added right button panel. Starting changeover to using     */
/***             client_survey for all mapping.                             */
/*** 03/16/92  : Uses client_survey now. Adding planetary data at bottom    */
/***             of XMAP screen. Moved CD buttons to left side.             */
/*** 08/01/92  : Added parsing of map command to store desired map scope in */
/***		 xmap_current_map_scope.  Added cs to xmap_current_map_scope*/
/***		 for new map.  Modified do_mapnow_up for consistency.       */
/***		 Rich Fitzgerald (fitz@rsj.harvard.edu)			    */
/*** 11/07/92  : Incorporated several changes listed in CHANGES.XMAP        */
/***		 Rich Fitzgerald (fitz@rsj.harvard.edu)			    */
/*** 11/09/92  : Rewrote map_move and changed button_release to allow for   */
/***		 wrapping in the x direction for move and deploy 	    */
/***		 Rich Fitzgerald (fitz@rsj.harvard.edu)			    */
/*** 11/10/92  : added click and drag zoom, zoom out button to xorbit. RF   */
/*** 11/12/92  : Removed most of the flicker upon resize by touching up     */
/***		 mwin_event_loop(), resize(), and xmap_plot_redraw(). RF    */
/*** 11/16/92  : Switched hilite procs to XDrawRectangle, cleaned up remap  */
/***		 code a bit, fixed bug in widget allocation. RF		    */
/*** 11/24/92  : Changed keyboard handling from keycodes to keysyms. RF     */
/*** 12/02/92  : Added cursor positioning in input window using mouse. RF   */
/*** 01/26/93  : Modified process_xmap_survey to put ship letter in p.des.  */
/***		 Modified sector_update_stats to show destination stats.    */
/*** 02/05/93  : Made CSP compliant.					    */
/*** 02/08/93  : changed to link lists for orbits, improved scope id,	    */
/***		 added numerical zoom.					    */
/*** 11/30/93  : made ANSI-compatible, cleaned up.			    */
/****************************************************************************/


# ifdef XMAP
#define XMAP_VERSION "XMAP v. 3.2.4 11/30/93"

/* Don't execute this file more than once, even if included multiple times */
#ifndef _WIDGET_H
#define _WIDGET_H

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/* program parameters */
#define XDefaultCivsToMove	2
#define XDefaultMilToMove	2

#define MAXDATA 2000

/* row positions in pixels for bottom panel display */
#define Row1 10
#define Row2 Row1 + 15
#define Row3 Row2 + 15

/* parameters for input window */
#define Input_x         20
#define Input_y         22
#define Input_prompt    "^"
#define Input_prompt_len 1

/* parameters for orbit display button (ships_win) */
#define ShowAll         0
#define ShowShips       1
#define ShowNoShips     2

/* parameters for mapon_win */
#define Do_Map  1
#define Do_Orbit 2

/* parameters for resize */
#define PanelSpacing    22      /* space between right and orbit panels */

/* parameters for displaying orbits */
#define Orb_Default_min_x       0.0
#define Orb_Default_max_x       200.0
#define Orb_Default_min_y       0.0
#define Orb_Default_max_y       200.0

#define LeftMargin      102     /* space for left panel */
#define Res_Name_Len    6       /* # char to ensure space for */
#define RightMargin     102 + 6*(Res_Name_Len)  /* space for right panel,text */
#define TopMargin       35      /* space for input panel */
#define BottomMargin    50 + 15 /* space for bottom panel,text */
#define XRESERVED (LeftMargin + RightMargin)
#define YRESERVED (TopMargin + BottomMargin)

#define MAX_SHIPS 10

#define NOT_GRAPH 0
#define FERT_GRAPH 1
#define RES_GRAPH 2
#define EFF_GRAPH 3
#define MOB_GRAPH 4
#define MIL_GRAPH 5
#define POPN_GRAPH 6
#define MPOPN_GRAPH 7
#define GEOG_GRAPH 8

# define XMAP_MAP_TYPE	 1
# define XMAP_ORBIT_TYPE 2

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#define min(x,y) (x <= y ? x:y)
#define max(x,y) (x >= y ? x:y)


/*
 * Event dispatching will be through a procedure vector associated with
 * each widget.  First define a function pointer type,
 * then define the procedure vector.
 */

#define WIDGET_EVENT_MASK	(ButtonPressMask | ButtonReleaseMask | \
				 PointerMotionMask | \
				 ExposureMask | StructureNotifyMask | \
				 EnterWindowMask | LeaveWindowMask | \
				 KeyPressMask)

typedef void (*function)();
typedef struct _ev_proc_vec {
    function	btn_down;	/* Function to call when button is pressed */
    function	btn_up;		/* ... when button is released */
    function    motion;         /* ... or moved  */
    function	expose;		/* When window is exposed */
    function	resize;		/* When window is resized */
    function	enter;		/* When pointer enters window */
    function	leave;		/* When pointer leaves window */
    function	key;		/* When a key is pressed */
} ev_proc_vec;

/*
 * Data structure associated with each widget.  Contains basic
 * information about the widget including
 * 	- A pointer for linking widgets into a list.
 * 	- The window containing the widget
 * 	- A pointer to the parent of this widget.
 * 	- The widget type.  This is a pointer to a structure defining
 * 	  the widget type.
 * 	- The position of the window within its parent
 * 	- The actual size of the window
 * 	- The desired size of the window
 * 	- The event procedure vector for the widget
 */

typedef struct _widget_type widget_type;	/* Need forward ref */

typedef struct _widget {
    struct _widget    * next;	/* For linking into a list of all widgets */
    Display	      * dpy;	/* Display this widget lives on */
    Window		win;	/* The window */
    struct _widget    * parent;	/* If NULL then root */
    widget_type	      * type;	/* The widget type */
    int			is_mapped;	/* true if widget is really mapped */
    int			x, y;	/* Position of window within parent */
    int			w, h;	/* Width and height of window */
    int			des_w, des_h;	/* Width and height of window */
    int			resized;	/* size-changed flag */
    ev_proc_vec		procs;	/* Associated procedure vector */
} widget;

/*
 * Data structure defining a widget type.  This provides
 * type-independent generic procedures for things like widget creation.
 * 
 */
struct _widget_type {
    char      * name;		/* Name of widget type */
    widget    * (*create)();	/* Widget creation function */
    /* Called as create( parent, x, y, w, h ) */
};

#define create_widget( type, dpy, parent, x, y, w, h ) \
    (*type.create)( dpy, parent, x, y, w, h )

typedef struct _mwin {
    widget 	core;
    Pixmap	buffer;			/* id of offscreen buffer */
    int		bitdepth;		/* depth of screen */
} mwin;

typedef struct _pan_win {
    widget 	core;
} pan_win;

typedef struct _but_win {
    widget 	core;
    char 	label[30];
    function	button_proc_up;
    function	button_proc_down;
    int         toggle;
    int         graph_type;
} but_win;

typedef struct _sec_win {
    widget 	core;
    char 	info[30];
    char 	old_info[30];
    char 	label[30];
} sec_win;

typedef struct _in_win {
    widget 	core;
    char 	info[351];
    char 	label[30];
    int		cur_char_pos;
    int		len;
} in_win;

typedef struct xshipstruct {
    int     shipno;
    char    ltr;
    int     owner;
}       xShip;

typedef struct xsectorstruct {
    int     x;
    int     y;
    char    sectc;
    char    des;
    int     wasted;
    int     own;
    int     eff;
    int     frt;
    int     mob;
    int     xtal;
    int     res;
    int     civ;
    int     mil;
    int     mpopn;
    int     numships;
    int     inverse;
    xShip   ships[MAX_SHIPS];
}       xSector;

typedef struct xmapstruct {
    int     maxx;
    int     maxy;
    char    star[700];
    char    planet[700];
    int     res;
    int     fuel;
    int     des;
    int     popn;
    int     mpopn;
    int     maxfert;
    int     maxres;
    int     maxeff;
    int     maxmob;
    int     maxmil;
    int     maxpopn;
    int     maxmpopn;
    int     minfert;
    int     minres;
    int     mineff;
    int     minmob;
    int     minmil;
    int     minpopn;
    int     minmpopn;
    int     tox;
    double  compat;
    int     enslaved;
    int     map;
    xSector *ptr;
}	xMap;

typedef struct xorbitstruct {
    struct xorbitstruct *next;
    struct xorbitstruct *prev;
    int		stand1;
    int		x;
    int		y;
    int		array;
    char	symbol;
    int		stand2;
    char	name[200];
}	xOrb;
#endif

extern int xmap_active;

# endif
/* XMAP */
