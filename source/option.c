/*
 * option.c: handles option setting characteristics for user
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"

# include <stdio.h>
# include <sys/types.h>
# include <ctype.h>

# include "args.h"
# include "str.h"
# include "term.h"
# include "vars.h"

# define _OPTION_
# include "option.h"

extern int end_msg;
extern int max_history;
extern int max_recall;
extern int reconnect_delay;
extern char *client_prompt;
extern char *input_prompt;
extern char *output_prompt;
extern char pbuf[];
# ifdef IMAP
extern char cursor_sector;		/* Imap */
# endif
extern int robo;
extern int notify;
extern int debug_level;

#ifdef CLIENT_DEVEL
extern int client_devel;
#endif

char *entry_quote;
char *exit_quote;

extern char *strncpy (char *, const char *, size_t);
extern int atoi (const char *);
extern int fprintf (FILE *, const char *, ...);
extern int strncmp (const char *, const char *, size_t);
extern long int atol (const char *);
void display_set (void);

void display_set (void)
{
char dsbuf[BUFSIZ];

	set_column_maker (num_columns / 2);
	msg ("-- set:");
	sprintf (dsbuf, "actions: %s",
		(GET_BIT (options, ACTIONS) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "ansi: %s",
		(GET_BIT (options, DISP_ANSI) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "more_delay: %d secs", more_val.delay);
	sprintf (dsbuf, "autologin: %s",
		(GET_BIT (options, AUTOLOGIN_STARTUP) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "beep: %s", 
		(GET_BIT (options, BEEP) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "bell: %s", 
		(GET_BIT (options, DO_BELLS) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "bold_communication: %s", 
		(GET_BIT (options, BOLD_COMM) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "client_prompt: '%s'", client_prompt);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "connect_delay: %3d secs", reconnect_delay);
	do_column_maker (dsbuf);
# ifdef IMAP
	sprintf (dsbuf, "cursor_sector: %c", cursor_sector);
	do_column_maker (dsbuf);
# endif
	sprintf (dsbuf, "display_from_top: %s", 
		(GET_BIT (options, DISPLAY_TOP) ? "on" : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "edit mode: %s",
		(input_mode.edit == EDIT_INSERT ? "insert" : "overwrite"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "encrypt: %s messages",
		(GET_BIT (options, ENCRYPT) ? "hidden" : "shown"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "entry_quote: '%s'",
		(entry_quote ? entry_quote : "none"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "exit_quote: '%s'",
		(exit_quote ? exit_quote : "none"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "full_screen: %s", 
		(GET_BIT (options, FULLSCREEN) ? "on" : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "help: %s", help_client);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "hide_end_prompt: %s", 
		(GET_BIT (options, HIDE_END_PROMPT) ? "on" : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "history: %d lines",
		max_history);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "input_prompt: '%s'", input_prompt);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "logging: %s", (logfile.on ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "login_suppress: %s",
		(GET_BIT (options, LOGINSUPPRESS_STARTUP) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "macro_char: %c", macro_char);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "map double: %s",
		(GET_BIT (options, MAP_DOUBLE) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "map space: %s",
		(GET_BIT (options, MAP_SPACE) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "more: %s", (more_val.on ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "more_delay: %d secs", more_val.delay);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "no_logout: %s",
	       (GET_BIT (options, NO_LOGOUT) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "noclobber: %s",
	       (GET_BIT (options, NOCLOBBER) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "notify: %s (beeps: %d)",
		(notify > 0 ? "on " : "off"),
		(notify > 0 ? notify : -notify));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "output_prompt: '%s'", output_prompt);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "password: %s (%s)", 
		(cur_game.game.pripassword == NULL ? "none" :
			cur_game.game.pripassword),
		(cur_game.game.secpassword == NULL ? "none" :
			cur_game.game.secpassword));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "quit_all: %s",
		(GET_BIT (options, QUIT_ALL) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "recall: %d lines", max_recall);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "repeat_connect: %s",
		(GET_BIT (options, CONNECT_STARTUP) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "scroll: %s", 
		(GET_BIT (options, SCROLL) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "scroll_clear: %s", 
		(GET_BIT (options, SCROLL_CLR) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "server_version: %d", servinfo.version);
	do_column_maker (dsbuf);
	sprintf (dsbuf, "show_actions: %s", 
		(GET_BIT (options, SHOW_ACTIONS) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "show_mail: %s",
		(GET_BIT (options, SHOW_MAIL) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "show_clock: %s",
		(GET_BIT (options, SHOW_CLOCK) ? "on " : "off"));
	do_column_maker (dsbuf);
	sprintf (dsbuf, "slash_commands: %s",
		(GET_BIT (options, SLASH_COMMANDS) ? "on " : "off"));
	do_column_maker (dsbuf);

	if (GET_BIT (options, BRACKETS)) {
		do_column_maker ("brackets: on");
	}

	flush_column_maker ();

	msg ("shell: '%s'", (shell ? shell : "NONE SET"));

	if (debug_level)
		msg ("DEBUG ON -- Level: %d", debug_level);

	if (GET_BIT (options, RAWMODE))
		msg ("RAWMODE ON.");

	msg ("-- end of set.");
}

/*
 * takes the args and if on or off set/clr's the bits. 
 * otherwise it flips the bit from which everway it was.
 */
void toggle (char *args, int type, char *name)
{

	if (*args && streq (args, "on")) {
		if (*name)
			msg ("-- %s: on", name);
		SET_BIT (options, type);
	} else if (*args && streq (args, "off")) {
		CLR_BIT (options, type);
		if (*name)
			msg ("-- %s: off", name);
	} else if (GET_BIT (options, type)) {
		CLR_BIT (options, type);
		if (*name)
			msg ("-- %s: off", name);
	} else {
		if (*name)
			msg ("-- %s: on", name);
		SET_BIT (options, type);
	}
}

void doubletoggle (char *args, int type, char *name)
{
	toggle (args, type, name);
	toggle (args, type - 1, "");
}


void cmd_set (char *s)
{
CommandSet *handler;
char cmd[MAXSIZ];
char args[BUFSIZ];
char *c;
 
	c = first (s);
	if (c && *c) {
		strcpy (cmd, c);
	} else {
		display_set ();
		return;
	}
	c = rest (s);
	if (c)
		strcpy (args, c);
	else {
		*args = '\0';
	}
	handler = binary_set_search (cmd);

	if (handler == NULL) {
		msg ("-- set: no such option %s.", cmd);
		return;
	}
	handler->has_changed = TRUE;
	handler->func (args, handler->value, handler->name);
}

CommandSet *binary_set_search (char *cmd)
{
int bottom = 0;
int top = NUM_OPTIONS - 1;
int mid; 
int value;

	while (bottom <= top) {
		mid = bottom + ((top - bottom) / 2);
		value = strcmp (cmd, commandset_table[mid].name);
		if (value == 0) 
			return (&commandset_table[mid]);
		else if (value < 0) 
			top = mid - 1;
		else 
			bottom = mid + 1;
	}
	return ((CommandSet *) NULL);
}

/** Handle routines **/
void set_client_prompt (char *args)
{
char *p;

	p = get_args (2, 0);
	if (streq (p, "default")) {
		strfree (client_prompt);
		client_prompt = string (DEFAULT_CLIENT_PROMPT);
		strfree (p);
	} else if (*p) {
		strfree (client_prompt);
		client_prompt = p;
	} else {
		*client_prompt = '\0';
		strfree (p);
	}
	msg ("-- client_prompt is: '%s'", client_prompt);
}

void set_connect_delay (char *args)
{
	if (isdigit (*args)) {
		reconnect_delay = atoi (args);
		msg ("-- reconnect delay set at %d", atoi (args));
	} else
		msg ("-- reconnect delay must be an integer.");
}

# ifdef IMAP
void set_cursor_sector (char *args, int val)
{
		if (*args) {
			cursor_sector = *args;
			msg ("-- cursor_sector set to: %c", *args);
		} else
			msg ("-- You must specify a character.");
}
# endif

void set_debug (char *args)
{
int i;
	if (isdigit (*args)) {
		i = atoi (args);
		if (i < 0)
			msg ("-- debug: level must be positive");
		else {
			debug_level = i;
			msg ("-- debug level: %d", debug_level);
		}
	} else
		msg ("-- debug: value must be an positive integer.");
}

void set_encrypt (char *args)
{
	if (streq (args, "on") || streqln (args, "hide")) {
		SET_BIT (options, ENCRYPT);
		msg ("-- encrypt: hidden.");
	} else if (streq (args, "off") || streqln (args, "shown")) {
		CLR_BIT (options, ENCRYPT);
		msg ("-- encrypt: shown.");
	} else
		msg ("-- encrypt options: on | off.");
}

void set_entry_quote (char *args)
{
	if (streq (args, "none")) {
		strfree (entry_quote);
		entry_quote = (char *) NULL;
		msg ("-- entry_quote: cleared.");
	} else {
		strfree (entry_quote);
		entry_quote = string (args);
		msg ("-- entry_quote: %s", entry_quote);
	}
}

void set_exit_quote (char *args)
{
	if (streq (args, "none")) {
		strfree (exit_quote);
		exit_quote = (char *) NULL;
		msg ("-- exit_quote: cleared.");
	} else {
		strfree (exit_quote);
		exit_quote = string (args);
		msg ("-- exit_quote: %s", exit_quote);
	}
}

void set_full_screen (char *args)
{
	scroll_output_window ();
	toggle (args, FULLSCREEN, "full_screen");
}

void set_help (char *args)
{
	if (*args) {
		strfree (help_client);
		if (streq (args, "default")) {
# ifdef HELP_CLIENT
			help_client = string (HELP_CLIENT);
# else
			help_client = string ("./Help");
# endif
		} else {
			help_client = string (args);
		}
		msg ("-- help: %s", help_client);
	} else {
		msg ("-- help is currently found at: %s", help_client);
	}
}

void set_history (char *args)
{
	if (isdigit (*args)) {
		max_history = atoi (args);
		msg ("-- history set at %d", atoi (args));
	} else
		msg ("-- history value must be an integer.");
}

void set_input_prompt (char *args)
{
char *p;

	p = get_args (2, 0);
	if (streq (p, "default")) {
		strfree (input_prompt);
		input_prompt = string (DEFAULT_INPUT_PROMPT);
		strfree (p);
	} else if (*p) {
		strfree (input_prompt);
		input_prompt = p;
	} else {
		*input_prompt = '\0';
		strfree (p);
	}
	update_input_prompt (input_prompt);
	cancel_input ();
	msg ("-- input_prompt is: '%s'", input_prompt);
}

void set_insert_edit_mode (char *args)
{
	input_mode.edit = EDIT_INSERT;
	msg ("-- edit mode: insert");
	force_update_status ();
}

void set_macro_char (char *args)
{
extern char *display_char ();

	if (streq (args, "default")) {
# ifdef DEFAULT_MACRO_CHAR
	macro_char = DEFAULT_MACRO_CHAR;
# else
	macro_char = '/';
# endif
	} else {
		macro_char = *args;
	}
	msg ("-- macro_char: %s", display_char (macro_char));
}

void set_map_opts (char *args)
{
	if (streqln (args, "double")) {
		if (GET_BIT (options, MAP_DOUBLE)) {
			CLR_BIT (options, MAP_DOUBLE);
			msg ("-- map: double numbers turned off.");
		} else {
			SET_BIT (options, MAP_DOUBLE);
			msg ("-- map: double numbers turned on.");
		}
	} else if (streqln (args, "space")) {
		if (GET_BIT (options, MAP_SPACE)) {
			CLR_BIT (options, MAP_SPACE);
			msg ("-- map: spacing disabled.");
		} else {
			SET_BIT (options, MAP_SPACE);
			msg ("-- map: spacing enabled.");
		}
	} else 
		msg ("-- map options: double | space");
}

void set_more (char *args)
{
	if (*args && streq (args, "on")) {
		more_val.on = TRUE;
	} else if (*args && streq (args, "off")) {
		more_val.on = FALSE;
	} else {
		more_val.on = (more_val.on + 1) % 2;
	}
	msg ("-- more: %s", (more_val.on ? "on" : "off"));
}

void set_more_delay (char *args)
{
	if (isdigit (*args)) {
		more_val.delay = atol (args);
		msg ("-- more_delay set at %d", atoi (args));
	} else
		msg ("-- more_delay must be an integer.");
}

void set_more_rows (char *args)
{
	if (isdigit (*args)) {
		more_val.num_rows = atoi (args);
		msg ("-- more_rows set at %d", atoi (args));
	} else
		msg ("-- more_rows must be an integer.");
}

void set_notify (char *args)
{
	if ((notify > 0 || streq (args, "off")) && !streq (args, "on")) {
		if (notify > 0)
			notify = 0 - notify;
		msg ("-- notify: off");
	} else if (notify < 0 || streq (args, "on")) {
		if (notify < 0)
			notify = 0 - notify;
		msg ("-- notify: on");
	}
}

void set_notify_beeps (char *args)
{
	if (isdigit (*args) && atoi (args) > 0) {
		if (notify < 0)
			notify = 0 - atoi (args);
		else
			notify = atoi (args);
		msg ("-- notify_beeps set at %d", atoi (args));
	} else
		msg ("-- notify_beeps must be a postive integer.");
}

void set_output_prompt (char *args)
{
char *p;

	p = get_args (2, 0);
	if (streq (p, "default")) {
		strfree (output_prompt);
# ifdef DEFAULT_OUTPUT_PROMPT
		output_prompt = string (DEFAULT_OUTPUT_PROMPT);
# else
		output_prompt = string (NULL_STRING);
# endif
		strfree (p);
	} else if (*p) {
		strfree (output_prompt);
		output_prompt = p;
	} else {
		*output_prompt = '\0';
		strfree (p);
	}
	msg ("-- output_prompt is: '%s'", output_prompt);
}

void set_overwrite_edit_mode (char *args)
{
	input_mode.edit = EDIT_OVERWRITE;
	msg ("-- edit mode: overwrite");
	force_update_status ();
}

void set_primary_password (char *args)
{
	strfree (cur_game.game.pripassword);
	cur_game.game.pripassword = string (args);
	msg ("-- Primary_password set to: %s", cur_game.game.pripassword);
}

void set_recall (char *args)
{
	if (isdigit (*args)) {
		max_recall = atoi (args);
		msg ("-- recall set at %d", atoi (args));
	} else
		msg ("-- recall value must be an integer.");
}

void set_rwho (char *args)
{
	rwho.on = (rwho.on + 1) % 2;
	msg ("-- set: rwho %s", (rwho.on ? "on" : "off"));
}

void set_secondary_password (char *args)
{
	strfree (cur_game.game.secpassword);
	cur_game.game.secpassword = string (args);
	msg ("-- Secondary_password set to: %s", cur_game.game.secpassword);
}

void set_show_clock (char *args, int val, char *name)
{
	toggle (args, val, name);
	force_update_status ();
}

void set_show_mail (char *args, int val, char *name)
{
	toggle (args, val, name);
	force_update_status ();
}

void set_status_bar (char *args)
{
	if (streq (args, "default"))
# ifdef DEFAULT_STATUS_BAR
		strcpy (status.format, DEFAULT_STATUS_BAR);
# else
		strcpy (status.format, "-");
# endif
	else
		strncpy (status.format, args, SMABUF);
	msg ("-- set: status_bar has been changed.");
	force_update_status ();
}

void set_status_bar_char (char *args)
{
	if (*args) {
		status.schar[0] = *args;
		force_update_status ();
	} else
		msg ("-- set status_bar_char: Specify a character.");
}

void set_robo (char *args)
{
		robo = (robo + 1) % 2;
		msg ("--set: robo %s", (robo ? "on" : "off"));
}

#ifdef CLIENT_DEVEL

void set_devel (char *args)
{
	client_devel = (client_devel + 1) % 2;
	msg (":: Developer Output: %s", (client_devel ? "On" : "Off"));
}

#endif

void set_ansi (char *args, int val, char *name)
{
	toggle (args, val, name);
	force_update_status ();
}

void save_settings (FILE *fd)
{

	fprintf (fd, "\n#\n# Settings (set <option> <args>)\n#\n");
	if (GET_BIT (options, ACTIONS))
		fprintf (fd, "set actions on\n");
	if (GET_BIT (options, DISP_ANSI))
		fprintf (fd, "set ansi on\n");
	if (GET_BIT (options, AUTOLOGIN_STARTUP))
		fprintf (fd, "set autologin on\n");
	if (GET_BIT (options, BEEP))
		fprintf (fd, "set beep on\n");
	if (GET_BIT (options, DO_BELLS))
		fprintf (fd, "set bell on\n");
	if (GET_BIT (options, BOLD_COMM))
		fprintf (fd, "set bold_communication on\n");
	if (GET_BIT (options, BRACKETS))
		fprintf (fd, "set brackets on\n");
	if (!streq (client_prompt, DEFAULT_CLIENT_PROMPT))
		fprintf (fd, "set client_prompt \"%s\"\n", client_prompt);
	if (GET_BIT (options, CONNECT_STARTUP))
		fprintf (fd, "set connect on\n");
	if (reconnect_delay)
		fprintf (fd, "set connect_delay %d\n", reconnect_delay);
# ifdef IMAP
	if (cursor_sector != DEFAULT_CURSOR_SECTOR)
		fprintf (fd, "set cursor_sector %c\n", cursor_sector);
# endif
	if (GET_BIT (options, DISPLAY_TOP))
		fprintf (fd, "set display_from_top on\n");
	if (GET_BIT (options, ENCRYPT))
		fprintf (fd, "set encrypt on\n");
	if (entry_quote)
		fprintf (fd, "set entry_quote %s\n", fstring (entry_quote));
	if (exit_quote)
		fprintf (fd, "set exit_quote %s\n", fstring (exit_quote));
	if (GET_BIT (options, FULLSCREEN))
		fprintf (fd, "set full_screen on\n");
	if (!streq (help_client, HELP_CLIENT))
		fprintf (fd, "set help %s\n", help_client);
	if (GET_BIT (options, HIDE_END_PROMPT))
		fprintf (fd, "set hide_end_prompt on\n");
	if (max_history)
		fprintf (fd, "set history %d\n", max_history);
	if (!streq (input_prompt, DEFAULT_INPUT_PROMPT))
		fprintf (fd, "set input_prompt \"%s\"\n", input_prompt);
	if (GET_BIT (options, LOGINSUPPRESS_STARTUP))
		fprintf (fd, "set login_suppress on\n");
	if (GET_BIT (options, MAP_DOUBLE))
		fprintf (fd, "set map double on\n");
	if (GET_BIT (options, MAP_SPACE))
		fprintf (fd, "set map space on\n");
	fprintf (fd, "set more %s\n", (more_val.on ? "on" : "off"));
	if (more_val.delay)
		fprintf (fd, "set more_delay %d\n", more_val.delay);
	if (GET_BIT (options, NO_LOGOUT))
		fprintf (fd, "set no_logout on\n");
	if (GET_BIT (options, NOCLOBBER))
		fprintf (fd, "set noclobber on\n");
	if (notify > 0)
		fprintf (fd, "set notify on\n");
	if (notify)
		fprintf (fd, "set notify_beeps %d\n", (notify > 0 ? 
			notify : -notify));
	if (!streq (output_prompt, DEFAULT_OUTPUT_PROMPT))
		fprintf (fd, "set output_prompt \"%s\"\n", output_prompt);
	if (input_mode.edit != EDIT_INSERT)
		fprintf (fd, "set overwrite_edit_mode\n");
	if (GET_BIT (options, PARTIAL_LINES))
		fprintf (fd, "#set partial_lines on\n");
	if (GET_BIT (options, QUIT_ALL))
		fprintf (fd, "set quit_all on\n");
	if (max_recall != DEFAULT_RECALL)
		fprintf (fd, "set recall %d\n", max_recall);
	if (GET_BIT (options, SCROLL))
		fprintf (fd, "set scroll on\n");
	if (GET_BIT (options, SCROLL_CLR))
		fprintf (fd, "set scroll_clear on\n");
	if (GET_BIT (options, SHOW_ACTIONS))
		fprintf (fd, "set show_actions on\n");
	if (GET_BIT (options, SHOW_CLOCK))
		fprintf (fd, "set show_clock on\n");
	if (GET_BIT (options, SHOW_MAIL))
		fprintf (fd, "set show_mail on\n");
	if (!streq (status.format, DEFAULT_STATUS_BAR))
		fprintf (fd, "set status_bar %s\n", status.format);
	if (!streq (status.schar, DEFAULT_STATUS_BAR_CHAR))
		fprintf (fd, "set status_bar_char %c\n", status.schar[0]);
}
