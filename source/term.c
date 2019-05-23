/*
 * term.c: handles the termcap info
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"

# include <stdlib.h>
# include <stdio.h>
# include <sys/types.h>
# ifdef SYSV
# include <termio.h>
# else
# include <sgtty.h>
# include <sys/ioctl.h>
# endif

# include "str.h"
# include "option.h"
# include "vars.h"

# define term_move(c,r)		(term_put_termstring(tgoto(CM, (c), (r))))

# define NULL_STRING	""

extern int cursor_display;

int num_rows = -1;
int output_row;
int last_output_row = 0;
int num_columns = -1;

short ospeed;

/* The termcap variables */
static char *CM;
static char *UP;
static char *BC;
static char *CE;
static char *CR;
static char *NL;
static char *AL;
static char *DL;
static char *CS;
static char *DC;
static char *IC;
static char *IM;
static char *EI;
static char *SO;
static char *SE;
static char *SF;
static char *ND;
static char *LE;
static char *BL;
static char *US;		/* underline on */
static char *UE;		/* underline off */
static char *MD;		/* bold/bright mode on */
static char *ME;		/* end all modes */
static int SG;

static int boldface = FALSE;
static int inverse = FALSE;
static int underline = FALSE;

/* terminal structures saved ones and ones client plays with */
# ifdef TERMIO
static struct termio save_term;
# else
static struct sgttyb save_term;
static struct ltchars oldltchars;
static struct ltchars save_ltchars;
static struct tchars oldtchars;
static struct tchars save_tchars;
# endif

extern char *getenv (const char *);
extern char *strcat (char *, const char *);
extern char *tgetstr (char *, char **);
extern char *tgoto (char *, int, int);
/* extern int ioctl (int, int, ...); */
extern int printf (const char *, ...);
extern int tgetent (char *, char *);
extern int tgetflag (char *);
extern int tgetnum (char *);
extern int tputs (char *, int, int (*) (char));
extern int write (int, const void *, unsigned int);
int (*term_clear_to_eol) ();
int (*term_cursor_left) ();
int (*term_cursor_right) ();
int (*term_delete) ();
int (*term_scroll) ();
int get_screen_size (void);
int term_ALDL_scroll (int l1, int l2, int n);
int term_BS_cursor_left (void);
int term_CE_clear_to_eol (void);
int term_CS_scroll (int l1, int l2, int n);
int term_DC_delete (void);
int term_LE_cursor_left (void);
int term_ND_cursor_right (void);
int term_SPACE_clear_to_eol (int x, int y);
int term_clear (int l1, int l2);
int term_move_cursor (int x, int y);
int term_null (void *);
int term_param_ALDL_scroll (int l1, int l2, int n);
int term_put_termstring (char *c);
int term_putchar (char c);
void term_boldface_off (void);
void term_boldface_on (void);
void term_standout_off (void);
void term_standout_on (void);
void term_underline_off (void);
void term_underline_on (void);

# if 0
int (*term_insert) (char *c);
int term_IC_insert (char *c);
int term_IMEI_insert (char *c);
# endif

/*
 * local test function, call from gb.c test_client()
 */
void term_test (void)
{
extern char *display_char ();
char *p;
char buf[BUFSIZ];
char *q;
char *r;

	*buf = '\0';
	for (q = buf, p = MD; *p; p++) {
		r = display_char (*p, 0);
		strcat (q, r);
		q += strlen (r);
	}
	*q = '\0';
	msg ("boldon : %s", buf);
	*buf = '\0';
	for (q = buf, p = ME; *p; p++) {
		r = display_char (*p, 0);
		strcat (q, r);
		q += strlen (r);
	}
	*q = '\0';
	msg ("boldoff: %s", buf);
	msg ("%sbold%s", MD, ME);
}

/*
 * initialize term variables for future use
 */
get_termcap (void)
{
static char tinfo[BUFSIZ];
static char terminfo[BUFSIZ];
char *term_name;
char *ptr;

    	if ((term_name = getenv ("TERM")) == NULL) {
		printf ("%s: No TERM variable set!\n", progname);
		exit (1);
    	}
    	if (tgetent (tinfo, term_name) < 1) {
		printf ("%s: No termcap entry for %s.\n", progname, term_name);
		exit(1);
    	}

	ptr = terminfo;

	get_screen_size ();

    	CM = tgetstr ("cm", &ptr);
    	if (CM == NULL) {
		printf ("%s: can't run on this terminal! NO cursor movement ability.\n", progname);
		exit (1);
    	}
    	if ((CR = tgetstr ("cr", &ptr)) == NULL)
		CR = "\r";
    	if ((NL = tgetstr ("nl", &ptr)) == NULL)
		NL = "\n";

    	if (CE = tgetstr ("ce", &ptr))
		term_clear_to_eol = term_CE_clear_to_eol;
    	else
		term_clear_to_eol = term_SPACE_clear_to_eol;


    	if (ND = tgetstr ("nd", &ptr))
		term_cursor_right = term_ND_cursor_right;
    	else
		term_cursor_right = term_null;

    	if (LE = tgetstr ("le", &ptr))
		term_cursor_left = term_LE_cursor_left;
    	else if (tgetflag ("bs"))
		term_cursor_left = term_BS_cursor_left;
    	else
		term_cursor_left = term_null;

    	SF = tgetstr ("sf", &ptr);
	if (SF == NULL)
		SF = (char *) NULL;

    	if (CS = tgetstr ("cs", &ptr))
		term_scroll = term_CS_scroll;
    	else if ((AL = tgetstr ("AL", &ptr)) && (DL = tgetstr ("DL", &ptr)))
		term_scroll = term_param_ALDL_scroll;
    	else if ((AL = tgetstr ("al", &ptr)) && (DL = tgetstr ("dl", &ptr)))
		term_scroll = term_ALDL_scroll;
    	else {
		term_scroll = term_null;
		printf ("%s: requires the ability to set a scroll region (termcap ability: cs) OR the ability to add and delete lines (termcap ability: al & dl, or AL & DL).", progname);
		exit (1);
	}

# if 0
    	if (IC = tgetstr ("ic", &ptr))
		term_insert = term_IC_insert;
    	else {
		if ((IM = tgetstr ("im", &ptr)) && (EI = tgetstr ("ei", &ptr)))
	    		term_insert = term_IMEI_insert;
		else
	    		term_insert = term_null;
    	}
# endif

    	if (DC = tgetstr ("dc", &ptr))
		term_delete = term_DC_delete;
    	else
		term_delete = term_null;

	BC = tgetstr ("bc", &ptr);
	UP = tgetstr ("up", &ptr);

    	SO = tgetstr ("so", &ptr);
    	SE = tgetstr ("se", &ptr);
    	if (SO == NULL || SE == NULL) {
		SO = NULL_STRING;
		SE = NULL_STRING;
    	}
    	US = tgetstr ("us", &ptr);
    	UE = tgetstr ("ue", &ptr);
    	if (US == NULL || UE == NULL) {
		US = NULL_STRING;
		UE = NULL_STRING;
    	}
	MD = tgetstr ("md", &ptr);
	ME = tgetstr ("me", &ptr);
	if (MD == NULL || ME == NULL) {
		MD = NULL_STRING;
		ME = NULL_STRING;
	}

    	SG = tgetflag ("sg");
}

int term_null (void *n)
{
        return (1);
}

int term_CE_clear_to_eol (void)
{
        term_put_termstring (CE);
    	return (0);
}

int term_SPACE_clear_to_eol (int x, int y)
{
char c = ' ';
int i = 0;
int cnt = num_columns - x;

    	for ( ; i < cnt; i++)
		term_putchar (c);
    	term_move_cursor (x, y);
	return (0);
}

int term_CS_scroll (int l1, int l2, int n)
{
int i;
char *thing;
static int oldl1 = -1;
static int oldl2 = -1;

    	if (SF)
		thing = SF;
    	else
		thing = "\n";
	if (oldl1 != l1 || oldl2 != l2) {
		term_put_termstring (tgoto (CS, l2, l1));
		oldl1 = l1;
		oldl2 = l2;
	}
    	term_move_cursor (0, l2);
    	for (i = 0; i < n; i++)
		term_put_termstring (thing);
    	return (0);
}

int term_ALDL_scroll (int l1, int l2, int n)
{
int i;

    	term_move_cursor (0, l1);
    	for (i = 0; i < n; i++)
		term_put_termstring (DL);
    	term_move_cursor (0, l2 - n + 1);
    	for (i = 0; i < n; i++)
		term_put_termstring (AL);
    	return (0);
}

int term_param_ALDL_scroll (int l1, int l2, int n)
{
    	term_move_cursor (0, l1);
    	term_put_termstring (tgoto (DL, n, n));
		term_move_cursor (0, l2 - n + 1);
    	term_put_termstring (tgoto (AL, n, n));
    	return (0);
}

# if 0
int term_IC_insert (char *c)
{
    	term_put_termstring (IC);
		term_putchar (*c);
    	return (0);
}

int term_IMEI_insert (char *c)
{
    	term_put_termstring (IM);
    	term_putchar (*c);
    	term_put_termstring (EI);
    	return (0);
}
# endif

int term_DC_delete (void)
{
        term_put_termstring (DC);
	return (0);
}

int term_ND_cursor_right (void)
{
        term_put_termstring (ND);
	return (0);
}

int term_LE_cursor_left (void)
{
        term_put_termstring (LE);
	return (0);
}

int term_BS_cursor_left (void)
{
char c = '\010';

	term_putchar (c);
	return (0);
}

term_putchar (char c)
{
	printf ("%c", c);
/*
	write (1, &c, 1);
*/
}

term_puts (char *str, int len)
{
int i;

	for (i = 0; *str && (i < len); str++, i++) {
		//if (*str < 32 && *str != '\t') { allow for escape character below -mfw
		if (*str < 32 && *str != '\t' && *str != 27) {
			term_standout_on ();
			term_putchar ((*str + 64));
			term_standout_off ();
		} else
			term_putchar (*str);
	}
}

void term_normal_mode (void)
{
	if (inverse)
		term_standout_off ();
	if (underline)
		term_underline_off ();
	if (boldface)
		term_boldface_off ();
}

void term_toggle_standout (void)
{
	if (inverse)
		term_standout_off ();
	else
		term_standout_on ();
}

void term_standout_on (void)
{
	term_put_termstring (SO);
	inverse = TRUE;
}

void term_standout_off (void)
{
	term_put_termstring (SE);
	inverse = FALSE;
}

int term_standout_status (void)
{
	return (inverse);
}

void term_toggle_underline (void)
{
	if (underline)
		term_underline_off ();
	else
		term_underline_on ();
}

void term_underline_on (void)
{
	term_put_termstring (US);
	underline = TRUE;
}

void term_underline_off (void)
{
	term_put_termstring (UE);
	underline = FALSE;
}

void term_toggle_boldface (void)
{
	if (boldface)
		term_boldface_off ();
	else
		term_boldface_on ();
}

void term_boldface_on (void)
{
	term_put_termstring (MD);
	boldface = TRUE;
}

void term_boldface_off (void)
{
	term_put_termstring (ME);
	if (inverse)
		term_standout_on ();
	if (underline)
		term_underline_on ();
	boldface = FALSE;
}

term_put_termstring (char *c)
{
	if (*c == '\0') return;		/* bug somewhere */
	tputs (c, 1, term_putchar);
}

term_move_cursor (int x, int y)
{

	cursor_display = FALSE;
	term_move (x, y);

}

term_clear_screen (void)
{
	if (GET_BIT (options, SCROLL_CLR)) {
		term_scroll (0, output_row, output_row+1);
	} else
		term_clear (0, output_row+1);
	last_output_row = 0;
}

term_clear (int l1, int l2)
{
int i = l1;

	for (; i < l2; i++) {
		term_move_cursor (0, i);
		term_clear_to_eol ();
	}
}

term_beep (int n)
{
	for ( ; n; n--)
		write (1, "\007", 1);
}

get_screen_size (void)
{
# ifndef SYSV
struct winsize wsize;

# ifdef TIOCGWINSZ
	if (ioctl (0, TIOCGWINSZ, &wsize) != -1) {
		num_rows = wsize.ws_row;
		num_columns = wsize.ws_col;
	} else {
		num_columns = tgetnum ("co");
		num_rows = tgetnum ("li");
	}
# endif /* TIOCGWINSZ */
# else
    	num_columns = tgetnum ("co");
    	num_rows = tgetnum ("li");
# endif /* SYSV */
	if (num_columns <= 0)
		num_columns = 79;
	if (num_rows <= 0)
		num_rows = 24;
}

term_mode_on (void)
{
# ifdef TERMIO
	ioctl(0, TCSETAW, &save_term);
# else
	ioctl(0, TIOCSETP, &save_term);
	ioctl (0, TIOCSLTC, &save_ltchars);
	ioctl (0, TIOCSETC, &save_tchars);
# endif
}

term_mode_off(void)
{
# ifdef TERMIO
struct termio s;

	ioctl(0, TCGETA, &s);

	save_term = s;
	ospeed = s.c_cflag & CBAUD;

	s.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
	s.c_oflag |=  (OPOST|ONLCR|TAB3);
	s.c_oflag &= ~(OCRNL|ONOCR|ONLRET);
	s.c_cc[VMIN] = 1;
	s.c_cc[VTIME] = 0;
	ioctl(0, TCSETAW, &s);
#else
struct sgttyb s;
char k_word;
char k_refresh;
char k_literal;
char k_flush;

	ioctl(0, TIOCGETP, &s);

	save_term = s;
	ospeed = s.sg_ospeed;

	s.sg_flags |= (CBREAK|XTABS);
	s.sg_flags &= ~ECHO;
	/*
	s.sg_flags |= CBREAK;
	s.sg_flags &= ~(ECHO|XTABS);
	*/
	ioctl(0, TIOCSETP, &s);

	if (ioctl (0, TIOCGLTC, &oldltchars) != -1) {
		save_ltchars = oldltchars;
		k_word = oldltchars.t_werasc;
		oldltchars.t_werasc = -1;
		k_refresh = oldltchars.t_rprntc;
		oldltchars.t_rprntc = -1;
		k_literal = oldltchars.t_lnextc;
		oldltchars.t_lnextc = -1;
		k_flush = oldltchars.t_flushc;
		oldltchars.t_flushc = -1;
		ioctl (0, TIOCSLTC, &oldltchars);
	}
	if (ioctl (0, TIOCGETC, &oldtchars) != -1) {
		save_tchars = oldtchars;
	}
#endif
}
