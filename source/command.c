/* command.c -- client commands.  
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 *
 */

# include <stdlib.h>
# include <stddef.h>
# include <stdio.h>

# include <sys/types.h>
# include <sys/stat.h>
# include <ctype.h>
# include <malloc.h>
# include <string.h>
# include <time.h>

# include "gb.h"
# include "args.h"
# include "command.h"
# include "csp.h"
# include "option.h"
# include "str.h"
# include "term.h"
# include "vars.h"

# define GBRC_SAVE_LINE	"# Put your stuff below here -- Keep this line here\n"

extern int end_msg;
extern int exit_now;
extern int gb;
extern int hide_msg;
extern int kill_socket_output;
extern int max_recall;
extern int pipe_running;
extern int server_help;
extern int robo;

char tbuf[MAXSIZ];
char post_buf[BUFSIZ];

extern char *ctime (const time_t *);
extern char *getenv (const char *);
extern int atoi (const char *);
extern int fclose (FILE *);
extern int fprintf (FILE *, const char *, ...);
extern pid_t fork (void);
extern time_t time (time_t *);
extern unsigned int sleep (unsigned int);
extern void *realloc (void *, size_t);
extern void socket_final_process (char *, int);
int add_to_post (char *s);
void robo_say (char *, int);

/* return '1' if it is a client command
 * else return '0'
 */
int client_command (char *s, int interactive)
{
char cmd[BUFSIZ];
char args[MAXSIZ];
char buf[MAXSIZ];
char *c;
char mode[3];
Command *handler;
 
	if (input_mode.post) {
		add_to_post (s);
		return (TRUE);
	}

	if (*s == '!') {
		cmd_proc (s+1);
		return (TRUE);
	}

	split (s, cmd, args);

	if (*s == '^') {
		history_sub (s+1);
		return (TRUE);
	}

	handler = binary_search (cmd);

	if (handler == NULL)
		return (FALSE);
	handler->cnt++;
	debug (3, "handler client_command: '%s' %d/%d/%d echo/parse/inter",
		args, handler->echo_command, handler->parse_var, interactive);

	if (handler->echo_command && interactive)
		print_key_string (handler->parse_var);

	if (handler->parse_var) {
		parse_variables (args);
		debug (3, "PGS in client_command: %s", args);
	}

	sprintf (buf, "%s %s", cmd, args);
	argify (buf);

	if (handler->parse_var && parse_for_loops (buf)) {
		add_queue (buf);
		return (TRUE);
	}

	handler->func (args);

	if (handler->send_to_socket) {
		send_gb (args, strlen (args));
	}

	return (TRUE);
}

Command *binary_search (char *cmd)
{
int bottom = 0;
int top = NUM_COMMANDS - 1;
int mid; 
int value;

	while (bottom <= top) {
		mid = bottom + ((top - bottom) / 2);
		value = strcmp (cmd, command_table[mid].name);
		if (value == 0) 
			return (&command_table[mid]);
		else if (value < 0) 
			top = mid - 1;
		else 
			bottom = mid + 1;
	}
	return ((Command *) NULL);
}

/** Handle routines **/
void cmd_info (char *args)
{
int i;
int numc;
int total = 0;
int total_only = TRUE;
char buf[SMABUF];

	msg ("-- Info:");
	if (!streq (args, "total")) {
		promptfor ("Are you sure? (y/n) ", buf, PROMPT_CHAR);
		if (!YES (*buf))
			return;

		total_only = FALSE;
		set_column_maker (20);
	}

	numc = NUM_COMMANDS;
	for (i = 0; i < numc; i++) {
		if (!total_only) {
			sprintf (buf, "%s (%d)", command_table[i].name, command_table[i].cnt);
			do_column_maker (buf);
		}
		total += command_table[i].cnt;
	}
	msg ("  Total client commands received: %d", total);
	msg ("  Read: %lu bytes, %lu lines.", info.bytes_read, info.lines_read);
	msg ("  Sent: %lu bytes, %lu lines.", info.bytes_sent, info.lines_sent);

}

void cmd_wait (char *args)
{
	if (streq (args, "none") || streq (args, "off")) {
		wait_status = WAIT_NONE;
	} else if (streq (args, "booted")) {
		wait_status = WAIT_BOOTED;
	} else if (streq (args, "connect")) {
		wait_status = WAIT_CONNECT;
	} else if (streq (args, "password")) {
		wait_status = WAIT_PASSWORD;
	} else if (streq (args, "login")) {
		wait_status = WAIT_LOGIN;
	}
}

void cmd_help (char *args)		/* server command */
           
{
char temp[BUFSIZ];

	sprintf (temp, "help %s", args);
	strcpy (args, temp);
	server_help = TRUE;
}

void cmd_read (char *args)
{
	/* read news was typed */
	if (streqrn (args, "n")) {
		start_command (C_RNEWS);
 	} else {
		start_command (C_TELEGRAM);
	}
}

void cmd_cd (char *args)
{
char buf[MAXSIZ];

	sprintf (buf, "cs %s", args);
	strcpy (args, buf);
}

void cmd_hide (char *args)
{
int cnt = 1;

	if (isdigit (*args))
		cnt = atoi (args);
	if (cnt > 0) {
		hide_msg = cnt;
		kill_socket_output++;
	} else if (cnt < 0) {
		msg ("-- Hide: count must be a positive value.");
	}
}

void cmd_internal (char *args)
{
	if (streq (args, "scope"))
		strcpy (args, " ( [123] /Test/Scope )");
	process_gb (args);
}


void cmd_detach (char *args)
{
int pid;

# ifndef RESTRICTED_ACCESS
	switch ((pid = fork ())) {
	case -1:
		msg ("-- Detach: Could not fork into background. Abort.");
		break;
	case 0:	/* I'm the child */
		detached = TRUE;
		break;
	default: /* I'm the parent */
		quit_gb (0, "-- I am detached. Process id (%d)", pid);
		break;
	}
# else
	msg ("-- Restricted Access: detach is not allowed for security.");
# endif
}

void cmd_post (char *args)
{
	input_mode.post = TRUE;
	strcpy (post_buf, "post ");
	if (*args)
		strcat (post_buf, args);
	msg ("-- You are now in posting mode. Type a single period (.) to end.");
	force_update_status ();
}

void cmd_echo (char *args)
{
char *p;
	
	socket_final_process (args, 0);
}

void cmd_msg (void)
{
char *p;
char *q;
	
	q = get_args (1, 0);
	parse_variables (q);
	p = q;
	msg ("%s %s", client_prompt, p);
	strfree (q);
}

void cmd_queue (char *args)
{
char buf[MAXSIZ];

	sprintf (buf, "echo queue: '%s'", args);
	add_queue (buf);
}

void cmd_helpc (char *args)
{
FILE *fhelp;

	if ((fhelp = fopen (help_client, "r")) == NULL) {
		msg ("-- Could not open %s for reading. Help Unavailable.",
			help_client);
		msg ("   Use 'set help <path_to_help_file>' and try again.");
		return;
	}
	help (args, fhelp);
}

void cmd_helps (char *args)
{
FILE *fhelp;

	if ((fhelp = fopen (HELP_SERVER, "r")) == NULL) {
		msg ("-- Could not open %s for reading. Help Unavailable.",
			HELP_SERVER);
		return;
	}
	help (args, fhelp);
}

void cmd_watch4 (char *args)
{
int id;
char temp[BUFSIZ];

# ifdef RWHO
	if (streq (args, "all")) {
		for (id = 0; id <= cur_game.maxplayers; id++)
			rwho.info[id].watch4 = TRUE;
		msg ("-- watch4 placed on all players.");
		return;
	} else if (*args == '-') {
		if (isdigit (*(args+1))) {
			id = atoi (args+1);
			if (id > cur_game.maxplayers) {
				msg ("-- watch4: invalid player number.");
				return;
			}
			rwho.info[id].watch4 = FALSE;
			msg ("-- watch4: turned off for player #%d", id);
		} else if (streq ((args+1), "all")) {
			for (id = 0; id < cur_game.maxplayers; id++)
				rwho.info[id].watch4 = FALSE;
			msg ("-- watch4 removed on all players.");
			return;
		}
	} else if (isdigit (*args)) {
		id = atoi (args);
		if (id > cur_game.maxplayers) {
			msg ("-- Invalid player number.");
			return;
		}
		rwho.info[id].watch4 = TRUE;
		msg ("-- watch4 placed on %d.", id);
		return;
	} else {
		set_column_maker (25);
		msg ("-- Watch4 List:");
		for (id = 1; id <= cur_game.maxplayers; id++) {
			if (rwho.info[id].watch4) {
				sprintf (temp, "  Player #%-2d - %s", id,
					(rwho.info[id].on ? "ON" : "not on"));
				do_column_maker (temp);
			}
		}
		flush_column_maker ();
		return;
	}
# else
	msg ("-- You must compile with the RWHO defined for watch4 to work.");
# endif
}

done_rwho (void)
{
int i;

	for (i = 0; i <= cur_game.maxplayers; i++) {
		if (rwho.info[i].watch4 == RWHO_ON && rwho.info[i].on) {
			msg ("-- watch4: %s [%d] is on or is now visible.", 
				rwho.info[i].name, i);
			rwho.info[i].watch4 = RWHO_NOTIFIED;
		} else if (rwho.info[i].watch4 == RWHO_NOTIFIED &&
		  !rwho.info[i].on) {
			msg ("-- watch4: %s [%d] is no longer visible or signed off.", rwho.info[i].name, i);
			rwho.info[i].watch4 = RWHO_ON;
		}
	}
	
}

start_rwho (void)
{
int i;
	for (i = 0; i <= cur_game.maxplayers; i++)
		rwho.info[i].on = FALSE;
}

/*
 * displays the version and other useful information to aid in
 * discussions about what version people are or aren't using.
 * -b flag will broadcast the information out as well as display it
 */
void cmd_version (char *args)
{
char opts[BUFSIZ];
char broad[BUFSIZ];

	csp_send_request (CSP_VERSION_COMMAND, NULL);

	msg ("-- Client version %s.", VERSION);

	strcpy (opts, "-- Client options set:");
# ifdef ARRAY
	strcat (opts, " ARRAY");
# endif
# ifdef CTIX
	strcat (opts, " CTIX");
# endif
# ifdef IMAP
	strcat (opts, " IMAP");
# endif
# ifdef OPTTECH
	strcat (opts, " OPTTECH");
# endif
# ifdef POPN
	strcat (opts, " POPN");
# endif
# ifdef SMART_CLIENT
	strcat (opts, " SMART_CLIENT");
# endif
# ifdef USE_INDEX
	strcat (opts, " INDEX");
# endif
# ifdef SYSV
	strcat (opts, " SYSV");
# endif
# ifdef XMAP
	strcat (opts, " XMAP");
# endif
# ifdef TERMIO
	strcat (opts, " TERMIO");
# endif
	msg (opts);

	if (streq (args, "-b")) {
		sprintf (broad, "broadcast Version (%s), Term (%s)\nbroadcast %s\n", VERSION, getenv ("TERM"), opts);
		send_gb (broad, strlen (broad));
	}
# ifdef RESTRICTED_ACCESS
	if (streq (args, "-b"))
		send_gb ("broadcast Client is running in RESTRICTED_ACCESS MODE\n", 54);
	msg ("-- Client is running in RESTRICTED_ACCESS MODE");
# endif

	msg (GBSAYING);
}

void cmd_clear (char *args)
{
	clear_screen ();
}

void cmd_quote (char *args)
{

	if (*args) {
		send_gb (args, strlen (args));
	}
}

void cmd_savef (char *args)
{
FILE *fd;
char askbuf[MAXSIZ];
char str[MAXSIZ];
time_t clk;
char *p;
char *gbrc_save_buf;
int gbrc_buf_size;
int first_blanks = TRUE;

# ifdef RESTRICTED_ACCESS
	msg ("-- Restricted access: savef not available.");
# else
	if (!*args) {
		expand_file (gbrc_path);
		sprintf (askbuf, "Really save to %s (y/n)? ", gbrc_path);
		promptfor (askbuf, str, PROMPT_STRING);
		if (!YES (*str))
			return;
		strcpy (askbuf, gbrc_path);
	} else {
		strcpy (askbuf, args);
		expand_file (askbuf);
	}

	/* open to read for the GBRC_SAVE_LINE stuff */
	if ((fd = fopen (askbuf, "r")) != NULL) {
		gbrc_buf_size = FALSE;	/* using it as flag for now */
		while (fgets (str, MAXSIZ - 1, fd)) {
			if (streq (str, GBRC_SAVE_LINE)) {
				gbrc_save_buf = (char *) malloc (MAXSIZ);
				*gbrc_save_buf = '\0';
				gbrc_buf_size = MAXSIZ;
				continue;
			} else if (!gbrc_buf_size)
				continue;
			/* skip blanks */
			if (*str == '\n' && first_blanks)
				continue;

			/* realloc the buffer */
			if (strlen (str) >
			   (gbrc_buf_size - strlen (gbrc_save_buf))) {
				p = (char *) realloc (gbrc_save_buf, MAXSIZ);
				gbrc_save_buf = p;
				gbrc_buf_size += MAXSIZ;
			}
			strcat (gbrc_save_buf, str);
			first_blanks = FALSE;
		}
		fclose (fd);
	}

	if ((fd = fopen (askbuf, "w")) == NULL) {
		msg ("-- savef: could not open '%s' for writing.", askbuf);
		strfree (gbrc_save_buf);
		return;
	}

	clk = time (0);
	fprintf (fd, "#\n# Galactic Bloodshed Client II Initialization File\n");
	fprintf (fd, "# Created: %s#\n\n", ctime (&clk));
	fprintf (fd, "set display off\n\n");

	save_actions (fd);
	save_assigns (fd);
	save_binds (fd);
	save_crypts (fd);
	save_defs (fd);
	save_gags (fd);
	save_games (fd);
	save_settings (fd);

	fprintf (fd, "\nset display on\n\n");
	fprintf (fd, "# Do NOT put anything below here that is SAVED above.\n");
	fprintf (fd, "# Commands (like connect) NOT saved by the client\n");
	fprintf (fd, "# should go below these comment lines.\n");
	fprintf (fd, "%s\n", GBRC_SAVE_LINE);
	if (gbrc_buf_size)
		fprintf (fd, gbrc_save_buf);
	strfree (gbrc_save_buf);
	fclose (fd);
	msg ("-- savef: Saved settings to '%s'", askbuf);
# endif
}

void cmd_sleep (char *args)
{
	if (isdigit (*args))
		sleep ((unsigned) atoi (args));
}

void cmd_repeat (char *args)
{
char *c;
char cmd[BUFSIZ];
int i;
	if (isdigit (*args)) {
		c = first (args);
		if (c)
			strcpy (cmd, c);
		else
			*cmd = '\0';
		c = rest (args);
		if (c)
			strcpy (args, c);
		else
			*args = '\0';
		for (i = atoi (cmd); i; i--)
			add_queue (args);
	}
}

void cmd_quit (char *args)
{
  char c;
  extern char *exit_quote;

  if (is_connected ())
  {
    if (exit_quote)
    {
      send_gb (exit_quote, strlen (exit_quote));
      send_gb ("", 0);
    }

    gb_close_socket = 1;

    send_gb ("quit", 4);

    CLR_BIT (options, CONNECT);

    if (GET_BIT (options, QUIT_ALL) || streq(args, "all"))
    {
      quit_all = TRUE;
    }
    else
    {
      promptfor ("Quit client (y/n)? ", &c, PROMPT_CHAR);

      if (YES (c))
      {
        quit_all = TRUE;
      }
      else
      {
        update_input_prompt (input_prompt);
      }
    }
  }
  else
  {
    exit_now = TRUE;
  }
}

void cmd_recall (char *args)
{
	if (!args)
		msg ("Usage: recall [num | *pat* | all]");
	else if (streq (args, "all"))
		recall (max_recall, 0);
	else if (MATCH (args, "*,*") ||
	         MATCH (args, "*, *") ||
	         MATCH (args, "*-*") ||
	         MATCH (args, "* *")) {
		if (isdigit (*pattern1) && isdigit (*pattern2)) {
			recall_n_m (atoi (pattern1), atoi (pattern2), 0);
		} else {
			msg ("-- Usage: recall n,m");
		}
	} else if (isdigit (args[0]))
		recall (atoi (args), 0);
	else
		recall_match (args, 0);
}
 
void cmd_convo (char *args)
{
	if (!args)
		msg ("Usage: convo [num | *pat* | all]");
	else if (streq (args, "all"))
		recall (max_recall, 1);
	else if (MATCH (args, "*,*") ||
	         MATCH (args, "*, *") ||
	         MATCH (args, "*-*") ||
	         MATCH (args, "* *")) {
		if (isdigit (*pattern1) && isdigit (*pattern2)) {
			recall_n_m (atoi (pattern1), atoi (pattern2), 1);
		} else {
			msg ("-- Usage: convo n,m");
		}
	} else if (isdigit (args[0]))
		recall (atoi (args), 1);
	else
		recall_match (args, 1);
}

int can_log (char *s)
{

	if (input_mode.post)
		return (1);

	if (*s == '\'' ||			/* short broadcast */
	   *s == '\"' ||
	   (strncmp (s, "announce", strlen (s)) == 0) ||
	   (strncmp (s, "broadcast", MAX (2, strlen (s))) == 0) ||
	   (strncmp (s, "think", strlen (s)) == 0) ||
	   streqrn (s, "sen") ||		/* send */
	   streqrn (s, "cr") ||			/* crypt */
	   streqrn (s, "set") ||		/* settings */
	   streqrn (s, "gag") ||		/* gag */
	   streqrn (s, "ungag") ||		/* ungag */
	   streqrn (s, "def") ||		/* def */
	   streqrn (s, "bind") ||		/* bind */
	   streqrn (s, "sh") || *s == '!' ||	/* shell escape */
	   streqrn (s, "if") ||			/* if command */
	   streqrn (s, "action") ||		/* action */
	   streqrn (s, "pos"))			/* post */
		return (0);
	return (1);
}

# ifdef RWHO
icomm_rwho (char *s)
{
char name[200];
int id;
long unow;
char *p;
int i;

	if (!rwho.on) 
		return;
	if (ICOMM_STATE == S_WAIT) {
		if (streqrn (s, "Current Players:")) {
			ICOMM_STATE = S_PROC;
			ICOMM_IGNORE = TRUE;
		}
		return;
	}
	unow = time (0);
	if ((HAP () && MATCH (s, "*:* [*.*] seconds idle*")) ||
	   MATCH (s, "* \"*\" [*] * idle*")) {
		p = skip_space (pattern1);
		remove_space_at_end (pattern1);
		strcpy (name, p);
		id = atoi (pattern3);
		rwho.info[id].last_on = unow;
		rwho.info[id].on = TRUE;
		if (!streq (rwho.info[id].name, name)) {
			rwho.info[id].changed_names = unow;
			strcpy (rwho.info[id].last_name, name);
			strcpy (rwho.info[id].name, name);
		}
	}
}
# endif

init_rwho (void)
{
# ifdef RWHO
int i;
long now;

	now = time (0);
	for (i = 0; i < MAX_NUM_PLAYERS; i++) {
		rwho.info[i].last_on = -1;
		rwho.info[i].last_spoke = -1;
		rwho.info[i].changed_names = -1;
		rwho.info[i].watch4 = FALSE;
		strcpy (rwho.info[i].name, races[i].name);	
		strcpy (rwho.info[i].last_name, races[i].name);	
	}
# endif
}

process_spoken (char *race, char *gov, int rid, int gid, char *message)
{
char name[SMABUF];
char fword[MAXSIZ];
char rword[MAXSIZ];
char psbuf[MAXSIZ];
long now;
int player;

	strcpy (name, race);
	split (message, fword, rword);

	if (streq (fword, profile.racename) && 
	   (streq (race, "Q Continuum") || streq (gov, "Priam")) &&
	   streq (rword, "GBII_CLIENT_INFORMATION")) {
		sprintf (psbuf,
		"send %s %s Info %s@%s -- version (%s) -- term (%s)\n",
		race, gov,
		getenv("LOGNAME"), getenv("HOSTNAME"),
		VERSION, getenv ("TERM"));
		send_gb (psbuf, strlen (psbuf));
		return;
	}
		
	if (!robo)
		return;

	now = time (0);

# ifdef RWHO
	if (!streq (rwho.info[rid].last_name, name)) {
		strcpy (rwho.info[rid].last_name, rwho.info[rid].name);
		strcpy (rwho.info[rid].name, name);
		rwho.info[rid].changed_names = now;
	}
	rwho.info[rid].last_spoke = now;
	rwho.info[rid].last_on = now;
# endif

#define PHRASE1	ROBONAME

	if (streq ("Robby", fword) ||
	    streq ("robby", fword) ||
	    streq ("Robby:", fword) ||
	    streq ("robby:", fword) ||
	    streq ("Robby,", fword) ||
	    streq ("robby,", fword))
		;
	else 
		return;

	if (streq ("help", rword) || streq ("Help", rword)) {
		robo_say ("Help: fortune; info on #", 0);
		return;
	}
	if (streq ("fortune", rword)) {
		robo_say ("-b fortune", 1);
		return;
	}	

# ifdef RWHO
	if ((MATCH (rword, "*when was * on*") ||
	    MATCH (rword, "*info on *"))) {
		if (isdigit (*pattern2))
			player = atoi (pattern2);
		else {
			for (player = 1; !streq (pattern2, rwho.info[player].name)
				&& (player < MAX_NUM_PLAYERS); player++)
				;
		}
		if (player >= MAX_NUM_PLAYERS) {
			sprintf (tbuf, "broadcast %s [%d] - I do not know of such a player.", name, rid);
			robo_say (tbuf, 0);
			return;
		}
		sprintf (tbuf,
		"broadcast %s [%d] -  I last saw %s [%d] %s ago and last heard %s speak %s ago.",
			name, rid, rwho.info[player].name, player,
			time_dur (rwho.info[player].last_on),
			rwho.info[player].name,
			time_dur (rwho.info[player].last_spoke));
		robo_say (tbuf, 0);
		return;
	}
# endif 
	robo_say ("broadcast If you want something, then ask.", 0); 
}

void robo_say (char *message, int proc)
{
	char myname[80];
	char saybuf[MAXSIZ];

	strcpy(myname, "Priam");

	/* Rename ourselves temporarlity */
	sprintf(saybuf, "name gov %s", ROBONAME);
	send_gb(saybuf, sizeof(saybuf));

	if (proc)
	{
		sprintf(saybuf, "%s", message);
		cmd_proc (saybuf);
		sprintf(saybuf, "--Robo ran: %s", message);
		msg(saybuf);
	}
	else
	{
		/* Say what we need to say. */
		sprintf(saybuf, "broadcast %s\n", message);
		send_gb(saybuf, sizeof(saybuf));
		sprintf(saybuf, "--Robo said: %s", message);
		msg(saybuf);
	}

	/* Restore our good name */
	sprintf(saybuf, "name gov %s", myname);
	send_gb(saybuf, sizeof(saybuf));
}

cancel_post (void)
{
	input_mode.post = FALSE;
	*post_buf = '\0';
	force_update_status ();
}

add_to_post (char *s)
{
char *p;

	if (streq (s, ".")) {
		parse_variables (post_buf);
		p = post_buf;
		send_gb (p, strlen (p));
		msg ("-- Posting message.");
		input_mode.post = FALSE;
		*post_buf = '\0';
		force_update_status ();
		return;
	}

	if (GB () || GBDT())
		strcat (post_buf, ";|");
	else
		strcat (post_buf, " ");
	strcat (post_buf, s);
	msg ("POST> %s", s);
	force_update_status ();
}

handle_pipes_and_redirects (char *str)
{
char cmd[BUFSIZ];
char s[MAXSIZ];
char mode[5];
char buf[BUFSIZ];
char *c;

	split (str, cmd, s);
	strcpy (s, str);
	debug (3, "pipes/redirects: %s (%s)", str, cmd);

	if ((c = strchr (s, '|')) && *(c-1) != '\\' && can_log (cmd)) {
		*c = '\0';
# ifndef RESTRICTED_ACCESS
		c++;
		if (*c == '!') {
			c++;
			kill_socket_output = TRUE;
		}
		c = skip_space (c);
		pipe_running = TRUE;

		cmd_proc (c);

		add_queue (s);
		add_queue ("internal_pipe_off");
# else
		msg ("-- Restricted Access: piping not available.");
# endif
		return (1);
	}

	if ((c = strchr (s, '>')) && *(c-1) != '\\' && can_log (cmd)) {
		*c = '\0';
# ifndef RESTRICTED_ACCESS
		if (*(c+1) == '>') { 	/* hmmm append it */
			strcpy (mode, "a");
			if (*(c+2) == '!') {
				c +=3;
				kill_socket_output = TRUE;
			} else
				c += 2;
		} else {		/* over write it */
			strcpy (mode, "w");
			if (*(c+1) == '!') {
				c +=2;
				kill_socket_output = TRUE;
			} else
				c++;
		}

		logfile.redirect = TRUE;
		logfile.level = LOG_ALL;

		c = skip_space (c);
		sprintf (buf, "log -%s %s", mode, c);
		add_queue (buf);
		add_queue (s);
		add_queue ("log off no msg");
# else
		msg ("-- Restricted Access: redirection not available.");
# endif
		return (1);
	}
	return (0);
}
