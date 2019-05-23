/*
 * option.h: header for the bit operations of options
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

/* defines for the setable options */
# define OP_NONE				0
# define AUTOLOGIN 				1
# define AUTOLOGIN_STARTUP			2
# define BEEP					3
# define BRACKETS				4
# define CONNECT  				5
# define CONNECT_STARTUP 			6
# define DISPLAYING				7
# define DISPLAY_TOP				8
# define ENCRYPT				9
# define HIDE_END_PROMPT			10
# define LOGINSUPPRESS				11
# define LOGINSUPPRESS_STARTUP			12
# define NO_LOGOUT				13
# define MAP_DOUBLE 				14
# define MAP_SPACE				15
# define PARTIAL_LINES				16
# define RAWMODE				17
# define SCROLL_CLR		 		18
# define SHOW_MAIL				19	
# define SHOW_CLOCK				20
# define SLASH_COMMANDS				21
# define DO_BELLS				22
# define SHOW_ACTIONS				23
# define NOCLOBBER				24
# define BOLD_COMM				25
# define FULLSCREEN				26
# define SCROLL					27
# define QUIT_ALL				28
# define ACTIONS				29
# define NUM_BITOPTIONS				30
# define DISP_ANSI				31  /* display ansi colors -mfw */

# define set_display(A)		toggle ((A), DISPLAYING, "display")

/* bit functions for the user options */
#define SET_BIT(A,I)	((A)[(I)/32] |= ((I)<32 ? (1<<(I)) : (1<<((I)%32))))
#define CLR_BIT(A,I)	((A)[(I)/32] &= ~((I)<32 ? (1<<(I)) : (1<<((I)%32))))
#define GET_BIT(A,I)	((A)[(I)/32] & ((I)<32 ? (1<<(I)) : (1<<((I)%32))))
#define GET_BITx(A,I)	((A) & (1<<(I)))

# ifdef _OPTION_
# define EXTERN
void set_client_prompt (char *args);
void set_connect_delay (char *args);
# ifdef IMAP
void set_cursor_sector (char *args, int val);
# endif
void set_debug (char *args);
void set_encrypt (char *args);
void set_entry_quote (char *args);
void set_exit_quote (char *args);
void set_full_screen (char *args);
void set_help (char *args);
void set_history (char *args);
void set_input_prompt (char *args);
void set_insert_edit_mode (char *args);
void set_macro_char (char *args);
void set_map_opts (char *args);
void set_more (char *args);
void set_more_delay (char *args);
void set_more_rows (char *args);
void set_notify (char *args);
void set_notify_beeps (char *args);
void set_output_prompt (char *args);
void set_overwrite_edit_mode (char *args);
void set_primary_password (char *args);
void set_recall (char *args);
void set_rwho (char *args);
void set_scroll ();
void set_secondary_password (char *args);
void set_show_clock (char *args, int val, char *name);
void set_show_mail (char *args, int val, char *name);
void set_status_bar (char *args);
void set_status_bar_char (char *args);
void set_robo (char *args);
void set_ansi (char *args, int val, char *name); /* -mfw */

#ifdef CLIENT_DEVEL
void set_devel (char *args);
#endif

void doubletoggle (char *args, int type, char *name);
void toggle (char *args, int type, char *name);

/* for binary search */
typedef struct commandset_struct {
	char *name;
	int value;
	void (*func)();
	int has_changed;
} CommandSet;

CommandSet *binary_set_search (char *cmd);

static CommandSet commandset_table[] =
{
	{ "actions", 			ACTIONS,	toggle, 0 },
	{ "ansi", 			DISP_ANSI,  	set_ansi, 0 },
	{ "autologin", 			AUTOLOGIN_STARTUP, doubletoggle, 0 },
	{ "beep", 			BEEP,		toggle, 0 },
	{ "bell",			DO_BELLS,		toggle, 0 },
	{ "bold_comm",		 	BOLD_COMM,  	toggle, 0 },
	{ "bold_communication", 	BOLD_COMM,  	toggle, 0 },
	{ "brackets", 			BRACKETS,  	toggle, 0 },
	{ "client_prompt",		OP_NONE,	set_client_prompt, 0 },
	{ "connect", 			CONNECT_STARTUP, doubletoggle, 0 },
	{ "connect_delay", 		OP_NONE,  	set_connect_delay, 0 },
# ifdef IMAP
	{ "cursor_sector", 		OP_NONE,  	set_cursor_sector, 0 },
# endif
	{ "debug",			OP_NONE,	set_debug, 0 },
#ifdef CLIENT_DEVEL
	{ "devel",			OP_NONE,	set_devel, 0 },
#endif
	{ "display",			DISPLAYING,	toggle, 0 },
	{ "display_from_top",		DISPLAY_TOP,	toggle, 0 },
	{ "encrypt", 			OP_NONE, 	set_encrypt, 0 },
	{ "entry_quote",		OP_NONE,	set_entry_quote, 0 },
	{ "exit_quote",			OP_NONE,	set_exit_quote, 0 },
	{ "full_screen",		OP_NONE,	set_full_screen, 0 },
	{ "help", 			OP_NONE, 	set_help, 0 },
	{ "hide_end_prompt",		HIDE_END_PROMPT, toggle, 0 },
	{ "history", 			OP_NONE, 	set_history, 0 },
	{ "input_prompt",		OP_NONE,	set_input_prompt, 0 },
	{ "insert_edit_mode", 		OP_NONE,      set_insert_edit_mode, 0 },
	{ "inverse", 			OP_NONE, 	term_standout_off, 0 },
	{ "login_suppress", 		LOGINSUPPRESS_STARTUP,	doubletoggle, 0 },
	{ "macro_char",			OP_NONE,  	set_macro_char, 0 },
	{ "map", 			OP_NONE,  	set_map_opts, 0 },
	{ "more", 			OP_NONE,  	set_more, 0 },
	{ "more_delay", 		OP_NONE,  	set_more_delay, 0 },
	{ "more_rows", 			OP_NONE,  	set_more_rows, 0 },
	{ "no_logout", 			NO_LOGOUT,  	toggle, 0 },
	{ "noclobber",			NOCLOBBER,	toggle, 0 },
	{ "notify", 			OP_NONE, 	set_notify, 0 },
	{ "notify_beeps", 		OP_NONE, 	set_notify_beeps, 0 },
	{ "output_prompt",		OP_NONE,       set_output_prompt, 0 },
	{ "overwrite_edit_mode",	OP_NONE,  set_overwrite_edit_mode, 0 },
	{ "partial_lines",		PARTIAL_LINES,	toggle, 0 },
	{ "primary_password",		OP_NONE,      set_primary_password, 0 },
	{ "quit_all",			QUIT_ALL,	toggle, 0 },
	{ "raw", 			RAWMODE,  	toggle, 0 },
	{ "recall", 			OP_NONE, 	set_recall, 0 },
	{ "repeat_connect",		CONNECT_STARTUP, doubletoggle, 0 },
	{ "robo", 			OP_NONE, 	set_robo, 0 },
	{ "rwho", 			OP_NONE, 	set_rwho, 0 },
	{ "scroll",			SCROLL,		toggle, 0 },
	{ "scroll_clear", 		SCROLL_CLR, 	toggle, 0 },
	{ "secondary_password", 	OP_NONE,    set_secondary_password, 0 },
	{ "show_actions",		SHOW_ACTIONS,	toggle, 0 },
	{ "show_clock", 		SHOW_CLOCK, 	set_show_clock, 0 },
	{ "show_mail", 			SHOW_MAIL, 	set_show_mail, 0 },
	{ "slash_commands", 		SLASH_COMMANDS,	toggle, 0 },
	{ "status_bar",			OP_NONE,  	set_status_bar, 0 },
	{ "status_bar_char",		OP_NONE,       set_status_bar_char, 0 },
	{ "status_bar_character",	OP_NONE,       set_status_bar_char, 0 }
};

#define NUM_OPTIONS (sizeof (commandset_table) / sizeof (CommandSet))

# else

# define EXTERN extern

# endif

EXTERN int options[2];

# undef EXTERN

