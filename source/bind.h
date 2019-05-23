/*
 * bind.h:  contains the data structs for bind.c
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1992, 1993
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"

# include <stdio.h>
# include <sys/types.h>
# include <ctype.h>
# include <memory.h>

# include "str.h"
# include "vars.h"

extern void backspace ();
extern void cancel_input ();
extern void clear_screen ();
extern void cmd_talk_off ();
extern void icomm_command_done ();
extern void cursor_backward ();
extern void cursor_begin ();
extern void cursor_end ();
extern void cursor_forward ();
extern void delete_under_cursor ();
extern void delete_word_left ();
extern void delete_word_right ();
extern void do_edit_mode ();
extern void do_recallb ();
extern void do_recallf ();
extern void esc_default ();
extern void esc_escape ();
extern void escape_key ();
extern void handle_key_buf ();
extern void input_ch_into_buf ();
extern void kill_to_end_line ();
extern void recall_crypts ();
extern void refresh_input ();
extern void refresh_screen ();
extern void stop_things ();
extern void test_client ();
extern void transpose_chars ();
extern void quote_key ();


/* imap fxns */
# ifdef IMAP
extern void imap_bombard (void);
extern void imap_capture_ship (void);
extern void imap_complex_move (void);
extern void imap_default (void);
extern void imap_defend (void);
extern void imap_deploy (void);
extern void imap_fire (void);
extern void imap_force_redraw (void);
extern void imap_land_ship (void);
extern void imap_launch_ship (void);
extern void imap_map_mode (void);
extern void imap_move_e (void);
extern void imap_move_n (void);
extern void imap_move_ne (void);
extern void imap_move_nw (void);
extern void imap_move_s (void);
extern void imap_move_se (void);
extern void imap_move_sw (void);
extern void imap_move_w (void);
extern void imap_mover (void);
extern void imap_ping_sector (void);
extern void imap_say_mode (void);
extern void imap_test (void);
extern void imap_toggle_geography (void);
extern void imap_toggle_inverse (void);
extern void imap_toggle_ansi (void);
extern void imap_zoom_sector (void);
# endif

/* declarations for the more */
void bind_bell (void);
void bind_boldface (void);
void bind_inverse (void);
void bind_underline (void);
void more_cancel (void);
void more_clear (void);
void more_forward (void);
void more_nonstop (void);
void more_oneline (void);
void more_quit (void);

/* typedefs */
typedef struct bindkey {
	char *cptr;
	void (*func) ();
	int is_string;
	int has_changed;
} BindKey;

typedef struct bindname {
	char *name;
	void (*func) (void);
} BindName;


/* Normal keys nothing fancy */
static BindKey BindNormKeys[] = {
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ^@ (null)  0 */
	{ (char *) NULL, cursor_begin     , 0, 0 }, /* ctrl-a */
	{ (char *) NULL, cursor_backward  , 0, 0 }, /* ctrl-b */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ctrl-c */
	{ (char *) NULL, delete_under_cursor, 0, 0 }, /* ctrl-d */
	{ (char *) NULL, cursor_end       , 0, 0 }, /* ctrl-e     5 */
	{ (char *) NULL, cursor_forward   , 0, 0 }, /* ctrl-f */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ctrl-g */
	{ (char *) NULL, backspace        , 0, 0 }, /* ctrl-h */
	{ (char *) NULL, do_edit_mode     , 0, 0 }, /* ctrl-i */
	{ (char *) NULL, handle_key_buf   , 0, 0 }, /* ctrl-j    10 */
	{ (char *) NULL, kill_to_end_line , 0, 0 }, /* ctrl-k */
	{ (char *) NULL, refresh_screen   , 0, 0 }, /* ctrl-l */
	{ (char *) NULL, handle_key_buf   , 0, 0 }, /* ctrl-m */
	{ (char *) NULL, do_recallf       , 0, 0 }, /* ctrl-n */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ctrl-o    15 */
	{ (char *) NULL, do_recallb       , 0, 0 }, /* ctrl-p */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ctrl-q */
	{ (char *) NULL, refresh_input     , 0, 0 }, /* ctrl-r */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ctrl-s */
	{ (char *) NULL, transpose_chars  , 0, 0 }, /* ctrl-t    20 */
	{ (char *) NULL, cancel_input      , 0, 0 }, /* ctrl-u */
	{ (char *) NULL, quote_key        , 0, 0 }, /* ctrl-v */
	{ (char *) NULL, delete_word_left , 0, 0 }, /* ctrl-w */
	{ (char *) NULL, delete_word_right, 0, 0 }, /* ctrl-x */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ctrl-y    25 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ctrl-z */
	{ (char *) NULL, escape_key       , 0, 0 }, /* ^[ (ESC) */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ^\ */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ^] */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ^^        30 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ^_ */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* space_bar */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ! */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* " */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* #         35 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* $ */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* % */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* & */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ' */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* (         40 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ) */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* * */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* + */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ' */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* -         45 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* . */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* / */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 0 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 0 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 2         50 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 3 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 4 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 5 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 6 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 7         55 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 8 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* 9 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* : */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ; */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* <         60 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* = */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* > */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ? */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* @ */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* A         65 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* B */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* C */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* D */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* E */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* F         70 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* G */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* H */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* I */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* J */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* K         75 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* L */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* M */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* N */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* O */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* P         80 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* Q */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* R */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* S */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* T */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* U         85 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* V */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* W */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* X */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* Y */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* Z         90 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* [ */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* \ */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ] */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* ^ */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* _         95 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* back ' */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* a */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* b */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* c */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* d        100 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* e */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* f */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* g */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* h */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* i        110 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* j */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* k */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* l */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* m */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* n        115 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* o */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* p */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* q */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* r */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* s        120 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* t */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* u */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* v */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* w */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* x        120 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* y */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* z */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* { */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* | */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* }        125 */
	{ (char *) NULL, input_ch_into_buf, 0, 0 }, /* tilde */
	{ (char *) NULL, backspace        , 0, 0 } /* ^? (delete) */
};

/* escape set of the keys */
static BindKey BindEscKeys[] = {
	{ (char *) NULL, esc_default, 0, 0 }, /* ^@ (null)  0 */
	{ (char *) NULL, esc_default     , 0, 0 }, /* ctrl-a */
	{ (char *) NULL, esc_default       , 0, 0 }, /* ctrl-b */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-c */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-d */
	{ (char *) NULL, esc_default       , 0, 0 }, /* ctrl-e     5 */
	{ (char *) NULL, esc_default   , 0, 0 }, /* ctrl-f */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-g */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-h */
	{ (char *) NULL, esc_default     , 0, 0 }, /* ctrl-i */
	{ (char *) NULL, esc_default   , 0, 0 }, /* ctrl-j    11 */
	{ (char *) NULL, esc_default , 0, 0 }, /* ctrl-k */
	{ (char *) NULL, esc_default   , 0, 0 }, /* ctrl-l */
	{ (char *) NULL, esc_default , 0, 0 }, /* ctrl-m */
	{ (char *) NULL, esc_default       , 0, 0 }, /* ctrl-n */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-o    05 */
	{ (char *) NULL, esc_default       , 0, 0 }, /* ctrl-p */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-q */
	{ (char *) NULL, esc_default     , 0, 0 }, /* ctrl-r */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-s */
	{ (char *) NULL, esc_default  , 0, 0 }, /* ctrl-t    20 */
	{ (char *) NULL, esc_default      , 0, 0 }, /* ctrl-u */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-v */
	{ (char *) NULL, esc_default , 0, 0 }, /* ctrl-w */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-x */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-y    25 */
	{ (char *) NULL, esc_default, 0, 0 }, /* ctrl-z */
	{ (char *) NULL, esc_escape       , 0, 0 }, /* ^[ (ESC) */
	{ (char *) NULL, esc_default, 0, 0 }, /* ^\ */
	{ (char *) NULL, esc_default, 0, 0 }, /* ^] */
	{ (char *) NULL, esc_default, 0, 0 }, /* ^^        30 */
	{ (char *) NULL, esc_default, 0, 0 }, /* ^_ */
	{ (char *) NULL, esc_escape , 0, 0 }, /* space_bar */
	{ (char *) NULL, esc_default, 0, 0 }, /* ! */
	{ (char *) NULL, esc_default, 0, 0 }, /* " */
	{ (char *) NULL, esc_default, 0, 0 }, /* #         35 */
	{ (char *) NULL, esc_default, 0, 0 }, /* $ */
	{ (char *) NULL, esc_default, 0, 0 }, /* % */
	{ (char *) NULL, esc_default, 0, 0 }, /* & */
	{ (char *) NULL, esc_default, 0, 0 }, /* ' */
	{ (char *) NULL, esc_default, 0, 0 }, /* (         40 */
	{ (char *) NULL, esc_default, 0, 0 }, /* ) */
	{ (char *) NULL, esc_default, 0, 0 }, /* * */
	{ (char *) NULL, esc_default, 0, 0 }, /* + */
	{ (char *) NULL, esc_default, 0, 0 }, /* ' */
	{ (char *) NULL, esc_default, 0, 0 }, /* -         45 */
	{ (char *) NULL, esc_default, 0, 0 }, /* . */
	{ (char *) NULL, esc_default, 0, 0 }, /* / */
	{ (char *) NULL, esc_default, 0, 0 }, /* 0 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 1 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 2         50 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 3 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 4 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 5 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 6 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 7         55 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 8 */
	{ (char *) NULL, esc_default, 0, 0 }, /* 9 */
	{ (char *) NULL, esc_default, 0, 0 }, /* : */
	{ (char *) NULL, esc_default, 0, 0 }, /* ; */
	{ (char *) NULL, esc_default, 0, 0 }, /* <         60 */
	{ (char *) NULL, esc_default, 0, 0 }, /* = */
	{ (char *) NULL, esc_default, 0, 0 }, /* > */
	{ (char *) NULL, esc_default, 0, 0 }, /* ? */
	{ (char *) NULL, esc_default, 0, 0 }, /* @ */
	{ (char *) NULL, esc_default, 0, 0 }, /* A         65 */
	{ (char *) NULL, esc_default, 0, 0 }, /* B */
	{ (char *) NULL, esc_default, 0, 0 }, /* C */
	{ (char *) NULL, esc_default, 0, 0 }, /* D */
	{ (char *) NULL, esc_default, 0, 0 }, /* E */
	{ (char *) NULL, esc_default, 0, 0 }, /* F         70 */
	{ (char *) NULL, esc_default, 0, 0 }, /* G */
	{ (char *) NULL, esc_default, 0, 0 }, /* H */
	{ (char *) NULL, esc_default, 0, 0 }, /* I */
	{ (char *) NULL, esc_default, 0, 0 }, /* J */
	{ (char *) NULL, esc_default, 0, 0 }, /* K         75 */
	{ (char *) NULL, esc_default, 0, 0 }, /* L */
	{ (char *) NULL, esc_default, 0, 0 }, /* M */
	{ (char *) NULL, esc_default, 0, 0 }, /* N */
	{ (char *) NULL, esc_default, 0, 0 }, /* O */
	{ (char *) NULL, esc_default, 0, 0 }, /* P         80 */
	{ (char *) NULL, esc_default, 0, 0 }, /* Q */
	{ (char *) NULL, esc_default, 0, 0 }, /* R */
	{ (char *) NULL, esc_default, 0, 0 }, /* S */
	{ (char *) NULL, esc_default, 0, 0 }, /* T */
	{ (char *) NULL, esc_default, 0, 0 }, /* U         85 */
	{ (char *) NULL, esc_default, 0, 0 }, /* V */
	{ (char *) NULL, esc_default, 0, 0 }, /* W */
	{ (char *) NULL, esc_default, 0, 0 }, /* X */
	{ (char *) NULL, esc_default, 0, 0 }, /* Y */
	{ (char *) NULL, esc_default, 0, 0 }, /* Z         90 */
	{ (char *) NULL, esc_default, 0, 0 }, /* [ */
	{ (char *) NULL, esc_default, 0, 0 }, /* \ */
	{ (char *) NULL, esc_default, 0, 0 }, /* ] */
	{ (char *) NULL, esc_default, 0, 0 }, /* ^ */
	{ (char *) NULL, esc_default, 0, 0 }, /* _         95 */
	{ (char *) NULL, esc_default, 0, 0 }, /* back ' */
	{ (char *) NULL, esc_default, 0, 0 }, /* a */
	{ (char *) NULL, bind_boldface, 0, 0 }, /* b */
	{ (char *) NULL, recall_crypts, 0, 0 }, /* c */
	{ (char *) NULL, esc_default, 0, 0 }, /* d        110 */
	{ (char *) NULL, esc_default, 0, 0 }, /* e */
	{ (char *) NULL, esc_default, 0, 0 }, /* f */
	{ (char *) NULL, esc_default, 0, 0 }, /* g */
	{ (char *) NULL, esc_default, 0, 0 }, /* h */
	{ (char *) NULL, esc_default, 0, 0 }, /* i        115 */
	{ (char *) NULL, esc_default, 0, 0 }, /* j */
	{ (char *) NULL, esc_default, 0, 0 }, /* k */
	{ (char *) NULL, clear_screen, 0, 0 }, /* l */
# ifdef IMAP
	{ (char *) NULL, imap_map_mode, 0, 0 }, /* m */
# else
	{ (char *) NULL, esc_default, 0, 0 }, /* m */
# endif
	{ (char *) NULL, esc_default, 0, 0 }, /* n        110 */
	{ (char *) NULL, esc_default, 0, 0 }, /* o */
	{ (char *) NULL, esc_default, 0, 0 }, /* p */
	{ (char *) NULL, esc_default, 0, 0 }, /* q */
	{ (char *) NULL, bind_inverse, 0, 0 }, /* r */
# ifdef IMAP
	{ (char *) NULL, imap_say_mode, 0, 0 }, /* s        115 */
# else
	{ (char *) NULL, esc_default, 0, 0 }, /* s        115 */
# endif
	{ (char *) NULL, cmd_talk_off, 0, 0 }, /* t */
	{ (char *) NULL, bind_underline, 0, 0 }, /* u */
	{ (char *) NULL, esc_default, 0, 0 }, /* v */
	{ (char *) NULL, esc_default, 0, 0 }, /* w */
	{ (char *) NULL, esc_default, 0, 0 }, /* x        120 */
	{ (char *) NULL, esc_default, 0, 0 }, /* y */
	{ (char *) NULL, esc_default, 0, 0 }, /* z */
	{ (char *) NULL, esc_default, 0, 0 }, /* { */
	{ (char *) NULL, esc_default, 0, 0 }, /* | */
	{ (char *) NULL, esc_default, 0, 0 }, /* }        125 */
	{ (char *) NULL, esc_default, 0, 0 }, /* tilde */
	{ (char *) NULL, esc_default, 0, 0 } /* ^? (delete) */
};

/* map modes */
# ifdef IMAP
static BindKey BindImapKeys[] = {
	{ (char *) NULL, imap_default, 0, 0 }, /* ^@ (null)  0 */
	{ (char *) NULL, imap_default     , 0, 0 }, /* ctrl-a */
	{ (char *) NULL, imap_default       , 0, 0 }, /* ctrl-b */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-c */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-d */
	{ (char *) NULL, imap_default       , 0, 0 }, /* ctrl-e     5 */
	{ (char *) NULL, imap_default   , 0, 0 }, /* ctrl-f */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-g */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-h */
	{ (char *) NULL, imap_default     , 0, 0 }, /* ctrl-i */
	{ (char *) NULL, imap_default   , 0, 0 }, /* ctrl-j    11 */
	{ (char *) NULL, imap_default , 0, 0 }, /* ctrl-k */
	{ (char *) NULL, imap_default   , 0, 0 }, /* ctrl-l */
	{ (char *) NULL, imap_map_mode , 0, 0 }, /* ctrl-m */
	{ (char *) NULL, imap_default       , 0, 0 }, /* ctrl-n */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-o    05 */
	{ (char *) NULL, imap_default       , 0, 0 }, /* ctrl-p */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-q */
	{ (char *) NULL, imap_default     , 0, 0 }, /* ctrl-r */
	{ (char *) NULL, imap_say_mode, 0, 0 }, /* ctrl-s */
	{ (char *) NULL, imap_default  , 0, 0 }, /* ctrl-t    20 */
	{ (char *) NULL, imap_default      , 0, 0 }, /* ctrl-u */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-v */
	{ (char *) NULL, imap_default , 0, 0 }, /* ctrl-w */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-x */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-y    25 */
	{ (char *) NULL, imap_default, 0, 0 }, /* ctrl-z */
	{ (char *) NULL, escape_key       , 0, 0 }, /* ^[ (ESC) */
	{ (char *) NULL, imap_default, 0, 0 }, /* ^\ */
	{ (char *) NULL, imap_default, 0, 0 }, /* ^] */
	{ (char *) NULL, imap_default, 0, 0 }, /* ^^        30 */
	{ (char *) NULL, imap_default, 0, 0 }, /* ^_ */
	{ (char *) NULL, imap_ping_sector , 0, 0 }, /* space_bar */
	{ (char *) NULL, imap_default, 0, 0 }, /* ! */
	{ (char *) NULL, imap_default, 0, 0 }, /* " */
	{ (char *) NULL, imap_default, 0, 0 }, /* #         35 */
	{ (char *) NULL, imap_default, 0, 0 }, /* $ */
	{ (char *) NULL, imap_default, 0, 0 }, /* % */
	{ (char *) NULL, imap_default, 0, 0 }, /* & */
	{ (char *) NULL, imap_default, 0, 0 }, /* ' */
	{ (char *) NULL, imap_default, 0, 0 }, /* (         40 */
	{ (char *) NULL, imap_default, 0, 0 }, /* ) */
	{ (char *) NULL, imap_default, 0, 0 }, /* * */
	{ (char *) NULL, imap_default, 0, 0 }, /* + */
	{ (char *) NULL, imap_default, 0, 0 }, /* ' */
	{ (char *) NULL, imap_default, 0, 0 }, /* -         45 */
	{ (char *) NULL, imap_default, 0, 0 }, /* . */
	{ (char *) NULL, imap_default, 0, 0 }, /* / */
	{ (char *) NULL, imap_default, 0, 0 }, /* 0 */
	{ (char *) NULL, imap_move_sw, 0, 0 }, /* 0 */
	{ (char *) NULL, imap_move_s, 0, 0 }, /* 2         50 */
	{ (char *) NULL, imap_move_se, 0, 0 }, /* 3 */
	{ (char *) NULL, imap_move_w, 0, 0 }, /* 4 */
	{ (char *) NULL, imap_ping_sector, 0, 0 }, /* 5 */
	{ (char *) NULL, imap_move_e, 0, 0 }, /* 6 */
	{ (char *) NULL, imap_move_nw, 0, 0 }, /* 7         55 */
	{ (char *) NULL, imap_move_n, 0, 0 }, /* 8 */
	{ (char *) NULL, imap_move_ne, 0, 0 }, /* 9 */
	{ (char *) NULL, imap_default, 0, 0 }, /* : */
	{ (char *) NULL, imap_default, 0, 0 }, /* ; */
	{ (char *) NULL, imap_default, 0, 0 }, /* <         60 */
	{ (char *) NULL, imap_default, 0, 0 }, /* = */
	{ (char *) NULL, imap_default, 0, 0 }, /* > */
	{ (char *) NULL, imap_default, 0, 0 }, /* ? */
	{ (char *) NULL, imap_default, 0, 0 }, /* @ */
	{ (char *) NULL, imap_toggle_ansi, 0, 0 }, /* A         65 */
	{ (char *) NULL, imap_default, 0, 0 }, /* B */
	{ (char *) NULL, imap_capture_ship, 0, 0 }, /* C */
	{ (char *) NULL, imap_default, 0, 0 }, /* D */
	{ (char *) NULL, imap_default, 0, 0 }, /* E */
	{ (char *) NULL, imap_fire, 0, 0 }, /* F         70 */
	{ (char *) NULL, imap_toggle_geography, 0, 0 }, /* G */
	{ (char *) NULL, imap_default, 0, 0 }, /* H */
	{ (char *) NULL, imap_toggle_inverse, 0, 0 }, /* I */
	{ (char *) NULL, imap_default, 0, 0 }, /* J */
	{ (char *) NULL, imap_default, 0, 0 }, /* K         75 */
	{ (char *) NULL, imap_land_ship, 0, 0 }, /* L */
	{ (char *) NULL, imap_default, 0, 0 }, /* M */
	{ (char *) NULL, imap_default, 0, 0 }, /* N */
	{ (char *) NULL, imap_default, 0, 0 }, /* O */
	{ (char *) NULL, imap_default, 0, 0 }, /* P         80 */
	{ (char *) NULL, imap_default, 0, 0 }, /* Q */
	{ (char *) NULL, imap_default, 0, 0 }, /* R */
	{ (char *) NULL, imap_default, 0, 0 }, /* S */
	{ (char *) NULL, imap_complex_move, 0, 0 }, /* T */
	{ (char *) NULL, imap_launch_ship, 0, 0 }, /* U         85 */
	{ (char *) NULL, imap_default, 0, 0 }, /* V */
	{ (char *) NULL, imap_default, 0, 0 }, /* W */
	{ (char *) NULL, imap_default, 0, 0 }, /* X */
	{ (char *) NULL, imap_default, 0, 0 }, /* Y */
	{ (char *) NULL, imap_default, 0, 0 }, /* Z         90 */
	{ (char *) NULL, imap_default, 0, 0 }, /* [ */
	{ (char *) NULL, imap_default, 0, 0 }, /* \ */
	{ (char *) NULL, imap_default, 0, 0 }, /* ] */
	{ (char *) NULL, imap_default, 0, 0 }, /* ^ */
	{ (char *) NULL, imap_default, 0, 0 }, /* _         95 */
	{ (char *) NULL, imap_default, 0, 0 }, /* back ' */
	{ (char *) NULL, imap_toggle_ansi, 0, 0 }, /* a */
	{ (char *) NULL, imap_move_sw, 0, 0 }, /* b */
	{ (char *) NULL, imap_default, 0, 0 }, /* c */
	{ (char *) NULL, imap_deploy, 0, 0 }, /* d        100 */
	{ (char *) NULL, imap_default, 0, 0 }, /* e */
	{ (char *) NULL, imap_force_redraw, 0, 0 }, /* f */
	{ (char *) NULL, imap_toggle_geography, 0, 0 },  /* g */
	{ (char *) NULL, imap_move_w, 0, 0 }, /* h */
	{ (char *) NULL, imap_toggle_inverse, 0, 0 }, /* i        105 */
	{ (char *) NULL, imap_move_s, 0, 0 }, /* j */
	{ (char *) NULL, imap_move_n, 0, 0 }, /* k */
	{ (char *) NULL, imap_move_e, 0, 0 }, /* l */
	{ (char *) NULL, imap_mover, 0, 0 }, /* m */
	{ (char *) NULL, imap_move_se, 0, 0 }, /* n        110 */
	{ (char *) NULL, imap_default, 0, 0 }, /* o */
	{ (char *) NULL, imap_default, 0, 0 }, /* p */
	{ (char *) NULL, imap_default, 0, 0 }, /* q */
	{ (char *) NULL, imap_default, 0, 0 }, /* r */
	{ (char *) NULL, imap_say_mode, 0, 0 }, /* s        115 */
	{ (char *) NULL, imap_default, 0, 0 }, /* t */
	{ (char *) NULL, imap_move_ne, 0, 0 }, /* u */
	{ (char *) NULL, imap_default, 0, 0 }, /* v */
	{ (char *) NULL, imap_default, 0, 0 }, /* w */
	{ (char *) NULL, imap_default, 0, 0 }, /* x        120 */
	{ (char *) NULL, imap_move_nw, 0, 0 }, /* y */
	{ (char *) NULL, imap_zoom_sector, 0, 0 }, /* z */
	{ (char *) NULL, imap_default, 0, 0 }, /* { */
	{ (char *) NULL, imap_default, 0, 0 }, /* | */
	{ (char *) NULL, imap_default, 0, 0 }, /* }        125 */
	{ (char *) NULL, imap_default, 0, 0 }, /* tilde */
	{ (char *) NULL, imap_default, 0, 0 } /* ^? (delete) */
};
# endif

/* bind names */
static BindName BindNames[] = {
	{ "backspace",			backspace		},
	{ "boldface",			bind_boldface		},
	{ "cancel_line",		cancel_input		},
	{ "client_test_key",		test_client		},
	{ "clear_screen",		clear_screen		},
	{ "crypt_recall",		recall_crypts		},
	{ "cursor_begin",		cursor_begin		},
	{ "cursor_end",			cursor_end		},
	{ "cursor_forward",		cursor_forward		},
	{ "default",			input_ch_into_buf	},
	{ "default_for_escape",		esc_default		},
# ifdef IMAP
	{ "default_for_imap",		imap_default		},
# endif
	{ "default_for_input",		input_ch_into_buf	},
	{ "delete_under_cursor",	delete_under_cursor	},
	{ "delete_word_left",		delete_word_left	},
	{ "delete_word_right",		delete_word_right	},
# ifdef IMAP
	{ "enter_imap",			imap_map_mode		},
# endif
	{ "escape_key",			escape_key		},
	{ "icomm_flush",		icomm_command_done	},
	{ "inverse",			bind_inverse		},
# ifdef IMAP
	{ "imap_ansi",		imap_toggle_ansi		},
	{ "imap_bombard",		imap_bombard		},
	{ "imap_capture_ship",		imap_capture_ship	},
	{ "imap_complex_move",		imap_complex_move	},
	{ "imap_defend",		imap_defend		},
	{ "imap_deploy_mil",		imap_deploy		},
	{ "imap_fire_ship",		imap_fire		},
	{ "imap_force_redraw",		imap_force_redraw	},
	{ "imap_geography",		imap_toggle_geography	},
	{ "imap_inverse",		imap_toggle_inverse	},
	{ "imap_land_ship",		imap_land_ship		},
	{ "imap_launch_ship",		imap_launch_ship	},
	{ "imap_move_civ",		imap_mover		},
	{ "imap_move_e",		imap_move_e		},
	{ "imap_move_n",		imap_move_n		},
	{ "imap_move_ne",		imap_move_ne		},
	{ "imap_move_nw",		imap_move_nw		},
	{ "imap_move_s",		imap_move_s		},
	{ "imap_move_se",		imap_move_se		},
	{ "imap_move_sw",		imap_move_sw		},
	{ "imap_move_w",		imap_move_w		},
	{ "imap_ping_sector",		imap_ping_sector	}, /* */
	{ "imap_say_mode",		imap_say_mode		}, /* */
	{ "imap_test",			imap_test		}, /* */
	{ "imap_zoom_sector",		imap_zoom_sector	}, /* */
# endif
	{ "kill_to_end_line",		kill_to_end_line	}, /* */
	{ "more_quit",			more_quit		},
	{ "more_clear",			more_clear		},
	{ "more_cancel",		more_cancel		},
	{ "more_nonstop",		more_nonstop		},
	{ "more_forward",		more_forward		},
	{ "more_oneline",		more_oneline		},
	{ "normal",			input_ch_into_buf	},
	{ "on_newline",			handle_key_buf		},
	{ "recall_last_history",	do_recallb		},
	{ "recall_next_history",	do_recallf		},
	{ "refresh_line",		refresh_input		},
	{ "refresh_screen",		refresh_screen		},
	{ "stop_things",		stop_things		},
	{ "talk_off",			cmd_talk_off		},
	{ "toggle_edit_mode",		do_edit_mode		},
	{ "transpose_chars",		transpose_chars		},
	{ "underline",			bind_underline		},
	{ "quote_character",		quote_key		}
};
