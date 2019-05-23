/*
 * term.h: header for the term info stuff
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991 
 *
 * See the COPYRIGHT file.
 */

#include <sys/termios.h>

extern char *tgoto (char *, int, int);
extern putchar_x ();

extern speed_t ospeed;			/* for tputs */
extern int num_columns;
extern int num_rows;
extern int output_row;
extern int last_output_row;

extern int (*term_scroll) ();
extern int (*term_insert) ();
extern int (*term_delete) ();
extern int (*term_cursor_left) ();
extern int (*term_cursor_right) ();
extern int (*term_clear_to_eol) ();
extern void term_standout_on ();
extern void term_standout_off ();
extern int term_standout_status ();

/* The termcap variables */
extern char *CM;
extern char *CE;
extern char *CR;
extern char *NL;
extern char *AL;
extern char *DL;
extern char *CS;
extern char *DC;
extern char *IC;
extern char *IM;
extern char *EI;
extern char *SO;
extern char *SE;
extern char *SF;
extern char *ND;
extern char *LE;
extern char *BL;
extern int SG;


extern int term_CE_clear_to_eol ();
extern int term_SPACE_clear_to_eol ();
extern int term_CS_scroll ();
extern int term_ALDL_scroll ();
extern int term_param_ALDL_scroll ();
extern int term_IC_insert ();
extern int term_IMEI_insert ();
extern int term_DC_delete ();
extern int term_null_function ();
extern int term_BS_cursor_left ();
extern int term_LE_cursor_left ();
extern int term_ND_cursor_right ();


# define NAMESIZE 18
# define SCALE 100.0

# define S_X  (num_columns - NAMESIZE)
# define S_Y  (output_row)
# define Midx (0.5 * (S_X - NAMESIZE) / SCALE)
# define Midy (0.5 * S_Y / SCALE)

