/*
 * load.c: loading a file
 *         loading of init file (.gbrc)
 *         saving of init file (.gbrc)
 *         logging files
 *         shell escaping (old version)
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
# include <sys/stat.h>
# include <ctype.h>
# include <memory.h>
# include <string.h>

# include "args.h"
# include "csp.h"
# include "option.h"
# include "str.h"
# include "term.h"
# include "vars.h"

# define GBRC_SAVE_LINE	"# Put your stuff below here -- Keep this line here\n"

extern int end_msg;
extern int hide_msg;
extern int kill_socket_output;

extern char *ctime (const time_t *);
extern char *getenv (const char *);
extern int fclose (FILE *);
extern int fprintf (FILE *, const char *, ...);
extern int system (const char *);
extern time_t time (time_t *);
int expand_file (char *fname);
int load_init_file (FILE *fd);
int load_predefined (char *fname);
int log_file (char *args);

/*
 * loadf: Has several functions.
 * No arguments: it prompts for loading of the defined GBRC file.
 * Arguments trip certain output flags and load the file and take
 * appropriate action based on the flags.
 * If no flags are given, but the file is recognized as a GBRC file
 * it will load the file thru the init file function.
 */
cmd_loadf (char *args)
{
int type = SCREEN_ONLY;
int show = 1;
int hidden = 0;
char *c;
char *p = args;
char buf[MAXSIZ];
char buf2[MAXSIZ];
FILE *fd;
extern char gbrc_path[];

# ifndef RESTRICTED_ACCESS
	/* no args.. ask to loadf GBRC */
	if (*args == '\0') {
		sprintf (buf, "Really load %s (y/n)? ",
			expand_file (gbrc_path));
		promptfor (buf, buf2, PROMPT_STRING);
		if (*buf2 == 'Y' || *buf2 == 'y')
			load_predefined (gbrc_path);
		else {
			msg ("-- Usage: loadf [-a|b|c|d|e|h|q|s|t|D] [-p<hrase>] filename");
			msg ("          loadf without any arguments will prompt you to load your specified GBRC.");
			msg ("          Your GBRC file is: '%s'", gbrc_path);
		}
		return;
	} else if (*args == '-') {
		c = args + 1;
		args = rest (args);
		while (*c != ' ') {
			switch (*c++) {
			case 'a':
				type = NORM_ANNOUNCE;
				break;
			case 'b':
				type = NORM_BROADCAST;
				break;
			case 'e':
				type = GB_EMOTE;
                                break;
			case 't':
				type = GB_THINK;
                                break;
			case 'c':
				type = ENCRYPTED;
				p = first(c);
				while (*c != ' ') c++;
				break;
			case 'h':
				hidden = 1;
				break;	
			case 'p':
				type = USER_DEFINED;
				p = first(c);
				while (*c != ' ') c++;
				break;
			case 's':
			case 'd':
				type = SERVER;
				break;
			case 'D':
				type = DO_SCREEN_ONLY;
				break;
			case 'x':
			case 'q':
				show = 0;
				break;
			default:
				msg ("-- Usage: loadf [-a|b|c|d|h|q|s|t] [-p<hrase>] filename");
				return;
				break;
			}
		}
	} 

	if ((fd = fopen (args, "r")) == NULL) {
		msg ("-- Error. Could not open %s for reading.", args);
		return;
	}

	while (fgets (buf, MAXSIZ, fd)) {
		if (*buf == ';' || *buf == '#') {
			if (type == SCREEN_ONLY && streq (buf, "# Galactic Bloodshed Client II Initialization File\n")) {
				msg ("-- loadf: Loading GB II Init File: '%s'",
					args);
				load_init_file (fd);
				return;
			}
			continue;
		}

		c = strchr (buf, '\n');
		if (c)
			*c = '\0';

		switch (type) {
		case NORM_ANNOUNCE:
			sprintf (buf2, "announce %s", buf);
			break;
		case NORM_BROADCAST:
			sprintf (buf2, "broadcast %s", buf);
			break;
		case GB_EMOTE:
			sprintf (buf2, "emote %s", buf);
			break;
		case GB_THINK:
			sprintf (buf2, "think %s", buf);
			break;
		case SCREEN_ONLY:
		case DO_SCREEN_ONLY:
		case SERVER:
			strcpy (buf2, buf);
			break;
		case ENCRYPTED:
			sprintf (buf2, "cr %s %s", p, buf);
			break;
		case USER_DEFINED:
			sprintf (buf2, "%s %s", p, buf);
			break;
		default: msg ("-- error in loadf of load.c");
			 break;
		}
		if (type != SCREEN_ONLY || type != DO_SCREEN_ONLY) {
			end_msg++;
			add_queue (buf2);
			if (hidden) {
				hide_msg++;
				kill_socket_output = TRUE;
			}
		}
		if (show) {
			strcpy (buf, "echo ");
			strcat (buf, buf2);
			add_queue (buf);
		}
	}
	if (type != SCREEN_ONLY || type != DO_SCREEN_ONLY)
		end_msg--;
	if (hidden)
		hide_msg--;

	fclose (fd);
# else
	msg ("-- Restricted Access: loading files not available.");
# endif
}	

/*
 * This is OLD and here only to facilitate my development of
 * the client when I need a secondary way to test proc commands.
 */
shell_out (char *args)
{
char buf[MAXSIZ];
char fname[BUFSIZ];
int flag = 0;

# ifndef RESTRICTED_ACCESS

	strcpy (fname, "~/.gbtemp");
	expand_file (fname);
	if (*args == '-') {
		sprintf (buf, "%s > %s", rest (args), fname);
		system (buf);
		if (*(args+1) == 'a') {
			sprintf (buf, "-a %s", fname);
			cmd_loadf (buf);
			flag++;
		} else if (*(args+1) == 'b') {
			sprintf (buf, "-b %s", fname);
			cmd_loadf (buf);
			flag++;
		}
	} else {
		sprintf (buf, "%s > %s", args, fname);
		system (buf);
		cmd_loadf (fname);
		flag++;
	}
	sprintf (buf, "rm -f %s", fname);
	system (buf);
	if (!flag)
		msg ("-- shell done.");
# else
	msg ("-- Restriced Access: shell escapes not available.");
# endif
}

/*
 * Loads the file .gbrc in users $HOME if present and executes
 * line by line.
 */
load_predefined (char *fname)
{
FILE *fd;
char buf[MAXSIZ];

	strcpy (buf, fname);
	expand_file (buf);	
	if ((fd = fopen (buf, "r")) == NULL) 
		return;
	load_init_file (fd);
}

load_init_file (FILE *fd)
{
char buf[MAXSIZ];
char *p;

	while (fgets (buf, MAXSIZ, fd)) {
		p = strchr (buf, '\n');
		if (!p)
			continue;
		*p = '\0';
		if (*buf == '#')
			continue;
		process_key (buf, FALSE);
	}

	fclose (fd);
}

void cmd_source (char *args)
{
char buf[BUFSIZ];

	sprintf (buf, "-d %s", args);
	cmd_loadf (buf);
}

void cmd_oldshell (char *args)
{
# ifndef RESTRICTED_ACCESS
	shell_out (args);
# else
	msg ("-- Restricted Access: shell escape not available.");
# endif
}

void cmd_log (char *args)
{
char fbuf[SMABUF];
char rbuf[MAXSIZ];

# ifndef RESTRICTED_ACCESS
		log_file (args);
# else
	msg ("-- Restricted Access: logging not available.");
# endif
} 

/*
 * if no filename is given, then gb.log in $HOME is used.
 */
log_file (char *args)
{
long clk;
char *p;
char mode[SMABUF];
struct stat statbuf;

# ifdef RESTRICTED_ACCESS	
	msg ("-- Restricted Access: Logging not available.");
	return;
# else

	strcpy (mode, "a+");
	/* turn off logging, if appropriate */
	if (streq (args, "off") || *args == '\0' || streq (args, "off no msg")) {
		if (logfile.on)
			fclose (logfile.fd);
		logfile.on = FALSE;
		logfile.redirect = FALSE;
		logfile.level = LOG_OFF;
		/* noclobber sets name to null if error */
		if (*logfile.name == '\0') {
			if (!streq (args, "off no msg")) {
				msg ("-- Logging turned off.");
			}
		} else
			msg ("-- Log file %s closed.", logfile.name);
		*logfile.name = '\0';
		return;
	}	

	logfile.level = LOG_ALL;
	debug (1, "log str: %s", args);
	while (*args == '-') {
		args++;
		debug (1, "log hyphen: %s", args);
		while (!isspace (*args)) {
		debug (1, "log !space: %c", *(args));
			switch (*args++) {
			case 'a':
				strcpy (mode, "a+");
				break;
			case 'c':
				logfile.level = LOG_COMMUNICATION;
				break;
			case 'w':
				strcpy (mode, "w");
				break;
			default: /* error to be here */
				break;
			}
		}
		while (isspace (*args))
			args++;
	}

	/* else we have a new log file about to be opened,
	 * so close the old one */
	if (logfile.on && *logfile.name) {
		msg ("-- Log file %s closed.", logfile.name);
		fclose (logfile.fd);
	}

	/* if filename is on, use ~/gb.log */
	if (streq (args, "on")) {
		strcpy (args, "~/gb.log");
		expand_file (args);
	} else
		expand_file (args);
	strcpy (logfile.name, args);

	if (GET_BIT (options, NOCLOBBER) && *mode == 'w' &&
	    stat (logfile.name, &statbuf) == 0) {
		logfile.on = FALSE;
		msg ("-- Log: noclobber is set and file '%s' exists. Not writing.", logfile.name);
		*logfile.name = '\0';
		return;
	}

	if ((logfile.fd = fopen (logfile.name, mode)) == NULL) {
		msg ("-- Log: Could not open '%s' for writing(%s)",
			logfile.name, mode);
		logfile.on = FALSE;
		return;
	}

	clk = time (0);
	fprintf (logfile.fd, "GB II Log File: %s\n", ctime (&clk));
	msg ("-- Log file %s opened.", logfile.name);
	logfile.on = TRUE;
# endif
}

/*
 * If ~/ is the first part of the file string, it expands
 * to the users $HOME. Other wise it is left alone.
 */
expand_file (char *fname)
{
char *env;
char temp[BUFSIZ];

	if (fname[0] != '~')
		return;
	if (fname[1] == '/') {
		env = getenv ("HOME");
		if (env == NULL)
			temp[0] = '\0';
		else {
			strcpy (temp, env);
			strcat (temp, fname+1);
			strcpy (fname, temp);
		}
	}
}
