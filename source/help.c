/*
 * help.c: handles the help for the client
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991, 1992, 1993 
 *
 * See the COPYRIGHT file.
 */

/*
 * The help command expects a specific format. This format is as
 * follows:
 *	1) anything before the first line beginning with "-- " is
 *	   ignored.
 *	2) everything after a "-- " is considered a topic that is
 *	   to be check for a match to display the contents.
 *	3) If a match occurs, then everything from the current "-- "
 *	   up to the next line beginning with "-- " is displayed.
 *	4) Multiple levels are created by preceeding the command
 *	   with a *.
 * Example:
 * This is not displayed.
 * -- first
 *   A line of help regarding first.
 * -- second
 *   A line of help regarding second.
 */

# include "gb.h"

# include <stdio.h>
# include <sys/types.h>
# include <memory.h>

# include "str.h"
# include "vars.h"

# define HELP_INC		20	/* width of help columns */

# define MULTI_CHAR		'*'

/* various 'types' of help. done and need to exit; one specific
 * item requested; all items requested; set help requested; and
 * lastly bind help requested */
# define HELP_DONE		-1
# define HELP_ONE		0
# define HELP_ALL		1
# define HELP_MULTI		2

/* standard single level help entry and offset amount */
# define STR_NORM		"-- "
# define OFFSET_NORM		3

/* 2 level help entry and offset amount */
# define STR_MULTI		"-- *"
# define OFFSET_MULTI		4

extern char pbuf[];
extern int num_columns;

extern int strncmp (const char *, const char *, size_t);
extern int fclose (FILE *);

help (char *args, FILE *fdhelp)
{
int mode = HELP_ONE;		/* searching for command (0)/all commands (1) */
int found = FALSE;		/* find the entry */
int multi = FALSE;		/* multi level command ? */
int oldmulti = FALSE;		/* storage status variable */
char *ptr;
char multistr[200];		/* store current multi level name */
char buf[BUFSIZ];
char temp[SMABUF];

	if (!(*args) || !args || *args == '?') {
		mode = HELP_ALL;
		msg ("-- Client Help:");
		msg ("Entries with further topics are marked by (*). Do not type the (*).");
	}

	set_column_maker (HELP_INC);
	strcpy (multistr, "-=null=-");

	while (fgets (buf, BUFSIZ, fdhelp)) {
		if (*buf == '\n') {
			if (found)
				msg ("");
			continue;
		}
		if ((ptr = strchr (buf, '\n')))
			*ptr = '\0';
		if ((streqrn (buf, STR_MULTI) && (multi = TRUE)) ||
		     streqrn (buf, STR_NORM)) {  /* topic */
			if (found) {
				/* it's multi we've printed out multi info
				 * now we need to print out subtopics */
				if (mode == HELP_MULTI) {
					mode = HELP_DONE;
					found = FALSE;
				} else
					break; /* we are all done, so quit */
			}
			/* find what we are looking for? */
			if (mode ==  HELP_ONE) {
				/* find the multi we are looking for? */
				if (multi && streq (buf+OFFSET_MULTI, args)) {
					found = TRUE;
					mode = HELP_MULTI;
					sprintf (multistr, "-- %s",
						buf+OFFSET_MULTI);
					msg ("");
				} else if (streq (buf+OFFSET_NORM, args)) {
					found = TRUE;
					multi = FALSE;
					msg ("");
				}
			} else {
				/* we've printed all the multi there are */
				if (mode == HELP_DONE) {
			    		if (!streqrn (buf, multistr)) {
						found = TRUE;
						break;
					} else {
						do_column_maker (buf + strlen (multistr));
					}	
				}
				/* print specials (multi) only once */
				if (mode == HELP_ALL) {
					if (oldmulti) {
						if (streqrn (buf+OFFSET_NORM, multistr))
							continue;
						oldmulti = FALSE;
					}
					if (multi) {
						if (streqrn (buf+OFFSET_NORM, multistr)){
							continue;
						} else {
							sprintf (temp, "%s (*)",
								buf + OFFSET_MULTI);
							do_column_maker (temp);
							strcpy (multistr, buf+OFFSET_MULTI);
							oldmulti = TRUE;
							multi = FALSE;
						}
					} else
						do_column_maker (buf+OFFSET_NORM);
				}
			}	/* special indent */
		} else { /* not a topic line */
			if (found)	/* we found the entry, print to end */
				msg ("%s", buf);
		}
	}
	if (mode)
		flush_column_maker ();
	fclose (fdhelp);
	if (!found && !mode) {
		msg ("-- No help available on that topic");
		msg ("-- type: helpc for a list of client commands");
		msg ("-- type: helps for a list of server info");
		msg ("-- type: help to check the server help files");
		return;
	}
	msg ("-- end of help.");
	return;
}	
