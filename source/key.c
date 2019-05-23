/*
 * key.c: handles the input.
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991, 1992, 1993 
 *
 * See the COPYRIGHT file.
 *
 * NOTES: KEY_DEBUG is a # define which will enable a 'beep' in
 * select routinues which may (or may not) facilitate debugging.
 *
 */

# include "gb.h"

# include <stdio.h>
# include <sys/types.h>
# include <fcntl.h>
# ifndef SYSV
# include <sgtty.h>
# include <sys/ioctl.h>
# else
# include <termio.h>
# endif
# include <string.h>

# undef TRUE
# undef FALSE

/* need this for the termcap stuff */
# include <curses.h>
# include <signal.h>
# include <sys/errno.h>

# include "args.h"
# include "option.h"
# include "str.h"
# include "term.h"
# include "vars.h"

# define UPDATE_CHAR			1
# define UPDATE_CURSOR_END		2
# define UPDATE_LOWER_MARK		3

# define WIDTH 				10

extern char last_update_buf[];
extern char post_buf[];
extern char *input_prompt;
extern char *output_prompt;

extern int do_queue;
extern int errno;
extern int prompt_return;
extern int queue_clear;

extern long map_time;

static char gbch;
static char key_buf[MAXSIZ+1];
static char key_store[MAXSIZ+1];

char pbuf[MAXSIZ+1];

int cursor_display = FALSE;
int escape = FALSE;
int quoted_char = FALSE;

struct posstruct {
	int key;
	int cur;
	int scr;
	char *prompt;
	char *talk;
	int plen;
} pos = { 0, 0, 0, (char *) NULL, (char *) NULL, 0 };

struct markstruct {
	int upper;
	int lower;
	int old_lower;
} mark = { 0, 0, 0};


extern int fclose (FILE *);
extern int read (int, void *, unsigned int);
extern pid_t getpid (void);
extern void free (void *);

int add_key_buf (char ch);
int clear_buf (void);
int cmd_talk_off (void);
int cursor_to_input (void);
int do_key (char *buf, int interactive);
int process_key (char *s, int interactive);
int promptfor (char *prompt, char *s, int mode);
int put_input_prompt (void);
int reset_key (void);
int set_marks (void);
int update_input_prompt (char *str);
int update_key (int mode);
void cancel_input (void);
void clear_screen (void);
void erase_input (int position);
void refresh_input (void);
void refresh_screen (void);
void signal_int (int);
void signal_pipe (int);
void signal_tstp (int);
void signal_usr1 (int);
void signal_winch (int);
void stop_things (void);


init_key (void)
{
	term_mode_off ();

	get_termcap ();

	signal (SIGINT, signal_int);
# ifdef SIGTSTP
	signal (SIGTSTP, signal_tstp);
# endif
# ifdef SIGWINCH
	signal (SIGWINCH, signal_winch);
#endif
	signal (SIGUSR1, signal_usr1);
	signal (SIGPIPE, signal_pipe);
	/* input and status bar is 2, and then -1 more to start at 0 */
	output_row = num_rows - 3;
	more_val.num_rows = output_row;
	pos.prompt = string (input_prompt);
	pos.plen = strlen (input_prompt);
	clear_buf ();
}

quit_gb (int exitstatus, char *s, char *a1, char *a2)
{

	sprintf (pbuf, s, a1, a2);
	if (*pbuf)
		msg (pbuf);
	if (logfile.on)
		fclose (logfile.fd);
	reset_key ();
	term_move_cursor (0, num_rows - 1);
	exit (exitstatus);
}

void signal_int (int sig)
{
	stop_things ();
}

void signal_tstp (int sig)
{
# ifdef SIGSTOP
	term_scroll (0, num_rows-1, 1);
	term_move_cursor (0, num_rows - 1);
	term_mode_on ();
	kill (getpid (), SIGSTOP);
	term_mode_off ();
	term_scroll (0, num_rows-1, 2);
	refresh_screen ();
	put_status ();
	refresh_input ();
# endif
}

void signal_usr1 (int sig)
{
	msg ("-- I shall exit now sir");
}

void signal_winch (int sig)
{
int diff;
int old_output_row = output_row;

# ifdef SIGWINCH
	diff = num_rows - output_row;
	clear_refresh_line ();
	clear_refresh_line_node ();
	signal (SIGWINCH, SIG_IGN);
	get_screen_size ();
	signal (SIGWINCH, signal_winch);		/* reset */
	term_clear_screen ();
	output_row = num_rows - diff;
# ifdef IMAP
	if (input_mode.map && invalid_map_screen_sizes ())
		handle_map_mode (1);
# endif
	init_refresh_lines ();
	(void) clear_screen ();
	put_status ();
# endif
}

reset_key (void)
{
	(void) erase_input (0);
	term_scroll (0, num_rows-1, 1);
	term_move_cursor (0, num_rows-2);
	term_clear_to_eol ();
	term_move_cursor (0, num_rows-1);
	term_mode_on ();
# ifdef SIGTSTP
	signal (SIGTSTP, SIG_IGN);
# endif
}

cursor_to_window (void)
{
	if (!cursor_display) {
# ifdef IMAP
		if (!input_mode.promptfor && input_mode.map &&
		    !input_mode.say) {
			cursor_to_map ();
		} else {
# endif
			cursor_to_input ();
# ifdef IMAP
		}
# endif
	cursor_display = TRUE;
	}
}

cursor_to_input (void)
{
	term_move_cursor (pos.scr, num_rows-1);
}

clear_buf (void)
{

	pos.cur = 0;
	pos.scr = 0;
	set_marks ();
	pos.key = 0;
}

void input_ch_into_buf (char ch)
{
	add_key_buf (ch);
}

input_string_into_buf (char *s)
{
char *p;

	for (p = s; *p; p++)
		add_key_buf (*p);
}

add_key_buf (char ch)
{
char sbuf[MAXSIZ];

	if (input_mode.promptfor == PROMPT_CHAR) {
		prompt_return = TRUE;
		return;
	}	
	if (pos.key >= MAXSIZ - 1) {
		term_beep (1);
		return;
	}
	if (input_mode.post) {
		if ((strlen (post_buf) + pos.key + 1) >= MAX_POST_LEN) {
			if (input_mode.post == TRUE) {
				input_mode.post++;
				msg ("-- Post: The maximum post size has been reached.");
			}
			if (pos.key != 0)
				return;
		}
	}
	if (input_mode.edit == EDIT_OVERWRITE || pos.cur == pos.key) {
		key_buf[pos.cur] = ch;
		if (pos.cur == pos.key) {
			key_buf[pos.cur+1] = '\0';
		}
		pos.cur++;
		set_marks ();
		if (mark.old_lower < mark.lower) {
			update_key (UPDATE_LOWER_MARK);
		} else {
			update_key (UPDATE_CHAR);
		}
	} else {
		strncpy (sbuf, key_buf, pos.cur);
		sbuf[pos.cur] = ch;
		sbuf[pos.cur+1] = '\0';
		strcat (sbuf, key_buf+pos.cur);
		strcpy (key_buf, sbuf);
		pos.cur++;
		set_marks ();
		if (mark.old_lower < mark.lower) {
			update_key (UPDATE_LOWER_MARK);
		} else {
			pos.cur--;
			update_key (UPDATE_CURSOR_END);
			pos.cur++;
		}
		term_move_cursor (pos.scr, num_rows-1);
	}
}

void transpose_chars (void)
{
char ch;

	ch = key_buf[pos.cur];
	key_buf[pos.cur] = key_buf[pos.cur+1];
	key_buf[pos.cur+1] = ch;
	update_key (UPDATE_CURSOR_END);
}


void erase_space_left (void)
{
char temp[MAXSIZ];

	if (!pos.cur)
		return;
	*(key_buf+pos.cur-1) = '\0';
	if (pos.cur !=  pos.key) {
		sprintf (temp, "%s%s", key_buf, (key_buf+pos.cur));
		strcpy (key_buf, temp);
	}
	pos.key--;
	pos.scr--;
	pos.cur--;
}

void erase_space_right (void)
{
char temp[MAXSIZ];

	if (pos.cur == pos.key)
		return;
	*(key_buf+pos.cur) = '\0';
	if (pos.cur !=  pos.key) {
		sprintf (temp, "%s%s", key_buf, (key_buf+pos.cur+1));
		strcpy (key_buf, temp);
	}
	pos.key--;
}

void erase_input (int position)
{
	term_move_cursor (position, num_rows-1);
	term_clear_to_eol ();
}

void delete_under_cursor (void)
{

	if (pos.cur == pos.key) {
# ifdef KEY_DEBUG
		term_beep (1);
# endif
		return;
	}

	(void) erase_space_right ();
	set_marks ();
	(void) erase_input (pos.scr);
	update_key (UPDATE_CURSOR_END);
}

void delete_word_left (void)
{

	(void) erase_space_left ();
	while ((pos.cur) && key_buf[pos.cur-1] != ' ')
		(void) erase_space_left ();
	set_marks ();
	if (mark.old_lower != mark.lower)
		update_key (UPDATE_LOWER_MARK);
	else {
		(void) erase_input (pos.scr);
		update_key (UPDATE_CURSOR_END);
	}
}

void delete_word_right (void)
{
	(void) erase_space_right ();
	while ((pos.cur != pos.key) && key_buf[pos.cur] != ' ')
		(void) erase_space_right ();
	(void) erase_space_right ();
	set_marks ();
	(void) erase_input (pos.scr);
	update_key (UPDATE_CURSOR_END);
}

void backspace (void)
{

	if (!pos.cur) {
# ifdef KEY_DEBUG
		term_beep (1);
# endif
		return;
	}

	(void) erase_space_left ();
	set_marks ();
	if (mark.old_lower != mark.lower)
		update_key (UPDATE_LOWER_MARK);
	else  {
		(void) erase_input (pos.scr);
		update_key (UPDATE_CURSOR_END);
	}
}


set_marks (void)
{
	mark.old_lower = mark.lower;
	mark.lower = 0;
	mark.upper = num_columns - WIDTH - pos.plen;
	while (mark.upper < pos.cur) {
		mark.lower = mark.upper - WIDTH;
		mark.upper += num_columns - (2 * WIDTH);
	}
	pos.key = strlen (key_buf);
	if (mark.lower)
		pos.scr = pos.cur - mark.lower;
	else
		pos.scr = pos.cur + pos.plen;
}

void refresh_input (void)
{
	set_marks ();
	update_key (UPDATE_LOWER_MARK);
}

update_key (int mode)
{
int len;
int i;

	if (hide_input) {
		if (mode == UPDATE_CHAR) {
			term_puts ("*", 1);
		} else if (mode == UPDATE_LOWER_MARK) {
			term_move_cursor (0, num_rows-1);
			put_input_prompt ();
			len = (pos.key < mark.upper ? pos.key - mark.lower :
				num_columns);
			for (i = 0; i < len; i++)
				term_puts ("*", 1);
			term_clear_to_eol ();
			term_move_cursor (pos.scr, num_rows-1);
		} else if (mode == UPDATE_CURSOR_END) {
			if (pos.key < mark.upper) {
				len = pos.key - mark.lower;
			} else {
				len = num_columns;
				if (pos.cur < num_columns) {
					len -= pos.plen;
				}
			}
			for (i = 0; i < len; i++)
				term_puts ("*", 1);
		} else
			msg ("-- UNKNOWN update_key type, please report");
		return;
	} 

	if (mode == UPDATE_CHAR) {
		term_puts (key_buf+pos.cur-1, 1);
	} else if (mode == UPDATE_LOWER_MARK) {
		term_move_cursor (0, num_rows-1);
		put_input_prompt ();
/* len = (pos.key < mark.upper ? pos.key - mark.lower : num_columns); */
		if (pos.key < mark.upper) {
			len = pos.key - mark.lower;
		} else {
			len = num_columns;
			if (pos.cur < num_columns) {
				len -= pos.plen;
			}
		}
		term_puts (key_buf+mark.lower, len);
		term_clear_to_eol ();
		term_move_cursor (pos.scr, num_rows-1);
	} else if (mode == UPDATE_CURSOR_END) {
		len = (pos.key < mark.upper ? pos.key - (pos.cur - mark.lower) :
			num_columns - pos.scr);
		term_puts (key_buf+pos.cur, len);
	} else
		msg ("-- UNKNOWN update_key type, please report");
}	 

void cursor_forward (void)
{

	if (pos.cur == pos.key) {
# ifdef KEY_DEBUG
		term_beep (1);
# endif
		return;
	}
	pos.cur++;
	set_marks ();
	if (mark.old_lower == mark.lower)
		term_cursor_right ();
	else
		update_key (UPDATE_LOWER_MARK);
}

void cursor_backward (void)
{

	if (pos.cur == 0) {
# ifdef KEY_DEBUG
		term_beep (1);
# endif
		return;
	}
	pos.cur--;
	set_marks ();
	if (mark.old_lower == mark.lower)
		term_cursor_left ();
	else
		update_key (UPDATE_LOWER_MARK);
} 

void cancel_input (void)
{
	(void) erase_input (0);
	clear_buf ();
	put_input_prompt ();
	*key_buf = '\0';
}

set_edit_buffer (char *s)
{
	strcpy (key_buf, s);
	pos.cur = strlen (key_buf);
	refresh_input ();
}

void do_recallf (void)
{
	if (recallf (key_buf)) {
		pos.cur = strlen (key_buf);
		refresh_input ();
	} else
		term_beep (1);
}

void do_recallb (void)
{
	if (recallb (key_buf)) {
		pos.cur = strlen (key_buf);
		refresh_input ();
	} else
		term_beep (1);
}

void recall_crypts (void)
{
int do_crypt_recall ();

	if (do_crypt_recall (key_buf)) {
		pos.cur = strlen (key_buf);
		refresh_input ();
	} else
		term_beep (1);
}

void refresh_screen (void)
{
int i;
int cnt = 0;
int start_pos;

	i = start_refresh_line_index (&start_pos);
	if (GET_BIT (options, DISPLAY_TOP)) {
		cnt = 0;
	} else {
		/* where on screen do we start */
		/* from output row, back up # of lines in buffer */
		cnt = output_row - start_pos;
	}
	for (start_pos = 0; start_pos < cnt; start_pos++) {
		term_move_cursor (0, start_pos);
		term_clear_to_eol ();
	}
	for ( ; cnt <= output_row; cnt++) {
		term_move_cursor (0, cnt);
		if (refresh_line[i] != (char *) NULL) {
			write_string (refresh_line[i],
				strlen (refresh_line[i]));
			term_normal_mode ();
		}
		term_clear_to_eol ();
		i++;
		i %= (output_row+1);
	}
# ifdef IMAP
	if (input_mode.map)
		refresh_map ();
# endif
	force_update_status ();
	erase_input (0);
	refresh_input ();
}

void clear_screen (void)
{
	term_clear_screen ();
	clear_refresh_line ();
	term_move_cursor (0, num_rows-2);
	term_puts (last_update_buf, num_columns);
# ifdef IMAP
	if (input_mode.map)
		refresh_map ();
# endif
	cursor_display = FALSE;
}

void do_edit_mode (void)
{
	if (input_mode.edit == EDIT_OVERWRITE)
		input_mode.edit = EDIT_INSERT;
	else
		input_mode.edit = EDIT_OVERWRITE;
	msg ("-- Edit mode is %s", (input_mode.edit == EDIT_OVERWRITE ? 
		"overwrite" : "insert"));
	force_update_status ();
}

void cursor_begin (void)
{
	pos.cur = 0;
	set_marks ();
	update_key (UPDATE_LOWER_MARK);
}

void cursor_end (void)
{
	pos.cur = pos.key;
	set_marks ();
	update_key (UPDATE_LOWER_MARK);
}

void kill_to_end_line (void)
{
	pos.key = pos.cur;
	key_buf[pos.cur] = '\0';
	term_clear_to_eol ();
}	

void escape_key (void)
{
	escape = TRUE;
}

void esc_escape (void)
{
	term_beep (1);
}

void esc_default (char ch)
{
extern char *display_char ();
	msg ("-- ESC-%s has no string or function attached.",
		display_char (gbch));
}

/*
 * called if this is an interactive string.
 */
print_key_string (int parse)
{
char *p;
char *q;
char outbuf[MAXSIZ];
char cmdbuf[BUFSIZ];

	if (parse) {
		parse_variables (key_store);
	}
	sprintf (outbuf, "%s%s", output_prompt, key_store);

	q = first (key_store);
	if (q)
		strcpy (cmdbuf, q);
	else
		*cmdbuf = '\0';

	if (*key_store == '\'' ||
	    *key_store == '\"' ||
	    *key_store == ';' ||
	    *key_store == ':' ||
	    (strncmp (key_store, "broadcast", MAX (2, strlen (cmdbuf))) == 0) ||
	    (strncmp (key_store, "announce", MAX (1, strlen (cmdbuf))) == 0) ||
	    (strncmp (key_store, "think", MAX (2, strlen (cmdbuf))) == 0) ||
	    streq (cmdbuf, "cr")) {
		add_recall (outbuf, 1);
		msg_type = MSG_COMMUNICATION;
	} else {
		add_recall (outbuf, 0);
		msg_type = MSG_ALL;
	}
	action_match_suppress = TRUE;
	msg ("%s", outbuf);
	action_match_suppress = FALSE;
}

void handle_key_buf (void)
{
char kbuf[MAXSIZ];
char *c;

	if (input_mode.promptfor) {
		prompt_return = TRUE;
		return;
	}

	(void) erase_input (0);
	clear_buf ();
	strcpy (kbuf, key_buf);
	add_history (kbuf);		/* must be here to prevent prompt */
	if (pos.talk) {
		if (streq (kbuf, "talk off") || streq (kbuf, "talk .") ||
		    streq (kbuf, ".")) {
			cmd_talk_off ();
			return;
		}
		sprintf (kbuf, "%s %s", pos.talk, key_buf);
	}
	term_move_cursor (0, num_rows-1);
	put_input_prompt ();
	*key_buf = '\0';
	refresh_input ();
	if (input_file) {
		add_queue (kbuf);
	}
	process_key (kbuf, TRUE);
}

/* 
 * break down a command line into single lines and send to do_key
 * for final processing. Strips out newlines as delimiter.
 * interactive means it came from user input, and not some client
 * generation
 */
process_key (char *s, int interactive)
{
char pkbuf[MAXSIZ];
char fpkbuf[MAXSIZ]; /* fmt proc_key buf */
char *p = s;
char *q = s;
int slashes = 0;
char quoted = '\0';


	if (!p) {
		debug (1, "NULL pointer received in process_key");
		return;
	}
	if (!*p && GET_BIT (options, PARTIAL_LINES)) {
		strcpy (p, "\n");
	}


	if (*p == '-') {
		add_queue ("set display off");
		add_queue (p + 1);
		add_queue ("set display on");
		return;
	}

	while (*p) {
		switch (*p) {
		case '\'':
		case '\"':
			if (quoted == *p)
				quoted = '\0';	/* we match, end quoting */
			else if (quoted == '\0')
				quoted = *p;	/* we have found a quote */
			p++;
			break;
		case '\\':
			if (quoted != '\0') {
				p++;
				break;
			}
			slashes++;
			p++;
			for ( ; *p && *p == '\\'; slashes++, p++)
				;
			if (*p == 'n' && (slashes % 2)) { /* it's a newline */
				*--p = '\0';
				do_key (q, interactive);
				q = p + 2;
				p = q;
			} else
				p++;
			slashes = 0;
			break;
		default:
			p++;
			break;
		}
	}
	*p = '\0';
	if (q != p)
		do_key (q, interactive);
}
		
do_key (char *buf, int interactive)
{
char *p;
char *q;
char *start;
char holdbuf[MAXSIZ];

	debug (2, "do_key prior: %s", buf);
	start = skip_space (buf);
	remove_space_at_end (buf);
	strcpy (key_store, start); /* store this to print it later */

	/* if a pipe/redirect, it gets added onto the queue again */
	if (handle_pipes_and_redirects (start)) {
		debug (4, "pipe/redirect is TRUE");
		return;
	}

	q = parse_given_string (start, PARSE_SLASH);
	strcpy (holdbuf, q);

	debug (2, "do_key after: '%s'", holdbuf);

	if (*holdbuf == '\0') {
		if (GET_BIT (options, PARTIAL_LINES)) {
			send_gb ("", 0);
		} else {
			return;
		}
	} else if (*holdbuf == macro_char) {
		if (GET_BIT (options, SLASH_COMMANDS) &&
		    client_command ((holdbuf+1), interactive)) {
			set_values_on_end_prompt ();
			cursor_to_window ();
			return;
		}
                if (interactive)
                        print_key_string (TRUE);
		parse_variables (holdbuf);
		if (parse_for_loops (holdbuf)) {
			add_queue (holdbuf);
			return;
		}
		/* argify for $0 and stuff */
		argify (holdbuf);
                if (!do_macro (holdbuf + 1)) {
			msg ("-- No such %smacro defined.",
				(GET_BIT (options, SLASH_COMMANDS) ?
				"command " : ""));
		}
	/* argify is called in client_command after variable parsing */
	} else if (!GET_BIT (options, SLASH_COMMANDS) && 
	  client_command (holdbuf, interactive)) {
	  	if (interactive)
		  set_values_on_end_prompt ();
		cursor_to_window ();
	} else {
		if (parse_for_loops (holdbuf)) {
			add_queue (holdbuf);
			return;
		}

		debug (2, "do_key(send to des, before parse_variables) below.");
		debug (2, "'%s'", holdbuf);
		parse_variables (holdbuf);
		debug (2, "do_key(send to des, after parse_variables) below.");
		debug (2, "'%s'", holdbuf);
		p = holdbuf;

		send_gb (p, strlen (p));

		/* things do if interactive - print it out */
		if (interactive) {
			print_key_string (TRUE);
		}
	}
}


/*
 * process keyboard input. putting into key_buf and key_null
 * handles on-line editing (ie, backspace, word erase etc.)
 * writes key_buf upon a newline.
 */
get_key (void)
{
char ch;
int count;
char getkey[SMABUF+1];
char *p;

	if (wait_status && wait_status != client_stats)
		return;

	if ((count = read (0, getkey, SMABUF)) == -1) {
		if (errno == EWOULDBLOCK)
			return;
		else
			quit_gb (-1, "Error reading from standard in (stdin)",NULL,NULL);
	}
	getkey[count] = '\0';
	p = getkey;
	while (*p) {
		gbch = *p++;
		if (quoted_char) {
			quoted_char = FALSE;
			input_ch_into_buf (gbch);
		} else if (escape) {
			escape = FALSE;
			bind_translate_char (gbch, BIND_ESC);
		} else if (input_mode.promptfor) {
			cursor_to_window ();
			bind_translate_char (gbch, BIND_NORM);
			cursor_to_window ();
# ifdef IMAP
		} else if (input_mode.map && !input_mode.say) {
			bind_translate_char (gbch, BIND_IMAP);
			cursor_to_map ();
# endif
		} else {	
			cursor_to_window ();
			bind_translate_char (gbch, BIND_NORM);
			cursor_to_window ();
		}
		if (prompt_return) {
			return;
		}
	} /* while */
}

void quote_key (void)
{
	quoted_char = TRUE;
}

void stop_things (void)
{
char c;
int type = 0;
char prbuf[SMABUF];

	term_scroll (0, num_rows-1, 0);
	if (process_running ()) {
		strcpy (prbuf, "Kill running process (y/n)? ");
		type = 1;
	} else if (have_queue ()) {
		strcpy (prbuf, "Clear the command queue (y/n)? ");
		type = 2;
		queue_clear = TRUE;
	} else if (input_mode.post) {
		strcpy (prbuf, "Cancel posting (y/n)? ");
		type = 3;
	} else {
		strcpy (prbuf, "Really exit (y/n)? ");
		type = 4;
	}
	promptfor (prbuf, &c, PROMPT_CHAR);
	if (YES (c)) {
		switch (type) {
		case 1:
			kill_process ();
			break;
		case 2:
			clear_queue ();
			break;
		case 3:
			cancel_post ();
			cancel_input ();
			break;
		case 4:
			cmd_quit ();	
			break;
		}
	}
}

promptfor (char *prompt, char *s, int mode)
{
static char storebuf[MAXSIZ];

	input_mode.promptfor = mode;
	strcpy (storebuf, key_buf);

	free (pos.prompt);
	pos.prompt = string (prompt);
	pos.plen = strlen (prompt);
	cancel_input ();
	
	set_marks ();
	cursor_to_input ();
	gbs ();
	pos.prompt = strfree (pos.prompt);
	update_input_prompt (input_prompt);

	if (mode == PROMPT_STRING)
		strcpy (s, key_buf);
	else if (mode == PROMPT_CHAR)
		*s = gbch;
	else
		msg ("-- Error in promptfor, unknown type mode %d", mode);

	erase_input (0);
	strcpy (key_buf, storebuf);
	pos.cur = strlen (key_buf);
	set_marks ();
	refresh_input ();
	input_mode.promptfor = PROMPT_OFF;
	prompt_return = FALSE;
}

update_input_prompt (char *str)
{
	strfree (pos.prompt);
	pos.prompt = string (str);
	pos.plen = strlen (str);
}

put_input_prompt (void)
{
	if (pos.plen && mark.lower == 0)
		term_puts (pos.prompt, pos.plen);
}

void cmd_talk (char *args)
{
char buf[SMABUF];

	strfree (pos.talk);
	pos.talk = string (args);
	msg ("-- talk: '%s'", pos.talk);
	force_update_status ();
	sprintf (buf, "talk (%s)> ", pos.talk);
	update_input_prompt (buf);
	refresh_input ();
}

cmd_talk_off (void)
{
	if (pos.talk) {
		strfree (pos.talk);
		pos.talk = (char *) NULL;
		msg ("-- talk: off");
		force_update_status ();
		update_input_prompt (input_prompt);
		cancel_input ();
	}
}

int in_talk_mode (void)
{
	return ((pos.talk ? TRUE : FALSE));
}
