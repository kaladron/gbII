/*
 * str.c: handles the string manipulation, as well as matching.
 *        Format output by wrapping around columns
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
# include <errno.h>
# include <malloc.h>
# include <stdarg.h>
# include <string.h>

# include "term.h"
# include "option.h"
# include "ansi.h"
# include "vars.h"

extern int errno;
/* extern int sys_nerr; using strerror() now -mfw */
/* extern char *sys_errlist[]; */
extern int kill_socket_output;
extern int pipe_output;
extern int pipe_running;

int kill_client_output = FALSE;
int refresh_next = 0;
int debug_level = 0;

char *client_prompt;
char *input_prompt;
char *output_prompt;

char column_maker_buf[BUFSIZ];		/* for building the columns */
int column_maker_pos;			/* indexing location in columns */
int column_maker_width;

/* Results for pattern matcher */
char pattern1[BUFSIZ], pattern2[BUFSIZ], pattern3[BUFSIZ], pattern4[BUFSIZ];
char pattern5[BUFSIZ], pattern6[BUFSIZ], pattern7[BUFSIZ], pattern8[BUFSIZ];
char pattern9[BUFSIZ], pattern10[BUFSIZ], pattern11[BUFSIZ], pattern12[BUFSIZ];
char pattern13[BUFSIZ], pattern14[BUFSIZ], pattern15[BUFSIZ], pattern16[BUFSIZ];
char pattern17[BUFSIZ], pattern18[BUFSIZ], pattern19[BUFSIZ], pattern20[BUFSIZ];
char *pattern[] = {
	pattern1, pattern2, pattern3, pattern4,
	pattern5, pattern6, pattern7, pattern8,
	pattern9, pattern10, pattern11, pattern12,
	pattern13, pattern14, pattern15, pattern16,
	pattern17, pattern18, pattern19, pattern20
};

char *skip_space (register char *s);
extern char *strncpy (char *, const char *, size_t);
extern int fflush (FILE *);
extern int fprintf (FILE *, const char *, ...);
extern int strncmp (const char *, const char *, size_t);
extern int toupper (int);
extern time_t time (time_t *);
int add_refresh_line (char *s, int cnt);
void display_msg (char *s);
int more (void);
int place_string_on_output_window (char *str, int len);
int set_column_maker (int width);
int write_string (char *s, int cnt);
void debug (int level, char *fmt, ...);
void msg (char *fmt, ...);
void msg_error (char *fmt, ...);
int has_esc_codes (char *str);

/*
 * returns a pointer to a static containg the first word of a string
 */
char *first (register char *str)
{
static char buf[BUFSIZ];
register char *s = buf;

	if (!str) return (NULL);

	str = skip_space (str);
	if (*str == '\"') {
		/* copy from " to " */
		str++;
		while (*str && *str != '\"')
			*s++ = *str++;
	} else {
		/* copy from here to first whitespace */
		while (*str && !isspace (*str))
			*s++ = *str++;
	}
	*s = '\0';

	return (s = buf);
}

/*
 * returns the rest of a string. ie, skips over first word.
 */
char *rest (register char *str)
{ 

	if (!str)
		return (NULL);

	str = skip_space (str);

  	if (!*str) 
		return (NULL);

	if (*str == '\"') {
		/* copy from " to " */
		str++;
		while (*str && *str != '\"')
			str++;
		str++;
	} else {
		/* copy from here to first whitespace */
		while (*str && !isspace (*str))
			str++;
	}

	str = skip_space (str);

  	if (!*str) 
		return (NULL);
  	return (str);
}

/*
 * Splits the string into first and rest buffers.
 * (Note: you must send valid sized buffers)
 */
split (char *s, char *fbuf, char *rbuf)
{
char *p;

	p = first (s);
	if (p)
		strcpy (fbuf, p);
	else
		*fbuf = '\0';
	p = rest (s);
	if (p)
		strcpy (rbuf, p);
	else
		*rbuf = '\0';
}

/*
 * This function skips the leading white space of a string.
 */
char *skip_space (register char *s)
{
char *str = s;

  	if (!str) 
		return (NULL);

  	while (*str && isspace (*str)) 
		str++;
	return (str);
}

/*
 * fstring: prepares a string, generally for writing to a file,
 * by doubling up the backslashes. Generated so strings could be read
 * in correctly from the GBRC.
 */
char *fstring (char *str)
{
static char buf[MAXSIZ];
char *p;
char *q;

	for (p = str, q = buf; *p; *q++ = *p++) {
		if (*p == '\\') {
			*q++ = '\\';
		}
	}
	*q = '\0';
	return (buf);
}

/*
 * pattern_match: Given a string1 a string and a pattern containing one or
 * more embedded stars (*) (which match any number of characters)
 * return true if the match succeeds, and set pattern[i] to the
 * characters matched by the 'i'th *.
 */
int pattern_match (register char *string1, register char *string2, register char **pattern)
{ 
register char *star = 0;
register char *starend = NULL;
register char *resp = NULL;
int nres = 0;

  	while (1) {
  		if (*string2 == '*') {
    			star = ++string2; 	/* Pattern after * */
      			starend = string1; 	/* Data after * match */
      			resp = pattern[nres++]; /* Result string */
      			*resp = '\0'; 		/* Initially null */
    		} else if (*string1 == *string2) {	/* Characters match */
    			if (*string2 == '\0') 	/* Pattern matches */
				return (1);
      			string2++; 		/* Try next position */
      			string1++;
    		} else {
			if (*string1 == '\0') 	/* Pattern fails - */
				return (0); 	/* no more string1a */
      			if (star == 0) 		/* Pattern fails - no * */
				return (0); 	/* to adjust */
      			string2 = star;		/* Restart pat after * */
      			*resp++ = *starend; 	/* Copy char to result */
      			*resp = '\0'; 		/* null terminate */
      			string1 = ++starend; 	/* Rescan copied char */
    		}
  	} /* while */
}

/*
 * converts the given string to upper case, returning a pointer
 * to the static buffer. it leaves the original string unchanged.
 */
char *strtou (char *str)
{
static char upper[MAXSIZ];
register char *p = str;
register char *q = upper;

	while (*p) {
		*q++ = (islower (*p) ? toupper (*p) : *p);
		p++;
	}
	*q = '\0';
	return (upper);
}

/*
 * makes a string via malloc and does error checking.
 */
char *string (char *str)
{
char *s;

	if ((s = (char *) malloc (strlen (str) + 1))) {
		strcpy (s, str);
		return ((char *) s);
	} else
		quit_gb (-2, "-- Could not malloc memory in makestring.");
	return ((char *) NULL); /* redundent but shuts the compiler up */
}

char *maxstring (char *str)
{
char *s;
int len;

	len = (strlen (str) % MAXSIZ) + 1;
	if (s = (char *) malloc (len + 1)) {
		strcpy (s, str);
		return ((char *) s);
	} else
		quit_gb (-2, "-- Could not malloc memory in makestring.");
	return ((char *) NULL); /* redundent but shuts the compiler up */
}


/*
 * wrap a string to num_columns based on a spaces.
 */
int wrap (char *line)
{
char s[MAXSIZ];
char temp[MAXSIZ];
char out[MAXSIZ];
char *place;
char ch;
char *p;
int i;
int first_line = TRUE;
int num_lines = 0;
int len;

	if (detached)
		return (0);

	strcpy (s, line);
	if (*s == '\n') {
		cursor_output_window ();
		scroll_output_window ();
		add_refresh_line ("", 1);
		return (0);
	}

	if (GET_BIT (options, BRACKETS))
		for (p = s; *p; p++) {
			switch (*p) {
			case '[':
			case '{':
				*p = '(';
				break;
			case ']':
			case '}':
				*p = ')';
				break;
			default:
				break;
			}
		}

	if (strlen (s) <= num_columns || has_esc_codes(s)) {
		if (more ())
			return (0);
		place_string_on_output_window (s, strlen (s));
		return (1);
	}
	strcpy(temp, s);
	do {
		num_lines++;
		p = temp + num_columns - 1;
		ch = *p;
		*p = '\0';	
		for (place = p - 1; !isspace (*place) && place != temp; place--)
			;
		if (place == temp)
			place = NULL;
		if (place == NULL) {	/* Can't do it, give up */
			p = s;
			i = strlen (p);
			while (i > num_columns - 1) {
				if (more ()) 
					return (0);
				if (!first_line) {
					*out = '+';
					strncpy (out+1, p, num_columns - 1);
				} else {
					first_line = FALSE;
					strncpy (out, p, num_columns);
				}
				place_string_on_output_window(out, num_columns);
				p = p + num_columns - 1;
				i = strlen (p);
			}
			strcpy (s, p);
			break;
		} else {
			if (more ())
				return (0);
			*p = ch;	
			*place = '\0';
			p = place+1;
			len = strlen (temp);
			if (!first_line) {
				*out = '+';
				strncpy (out+1, temp, len);
				len++;
			} else {
				first_line = FALSE;
				strncpy (out, temp, len);
			}
			out[len] = '\0';
			place_string_on_output_window (out, len);
		}
		p = skip_space (p);
		strcpy(temp, p);	/* Rest of string. */
		strcpy(s, temp);
	} while (strlen (s) > num_columns);

	len = strlen (s);
	if (len != 0) {
		if (more ())
			return (0);
		if (!first_line) {
			*out = '+';
			strncpy (out+1, s, len);
			len++;
		} else {
			first_line = FALSE;
			strncpy (out, s, len);
		}
		out[len] = '\0';
		place_string_on_output_window (out, len);
		num_lines++;
	}
	fflush (stdout);
	return (num_lines);
}

int more (void)
{
  long present_time;
  static char more_buf[] = "-- more --";
  char c;

  if (!more_val.on || !more_val.delay || more_val.non_stop ||
    kill_client_output || kill_socket_output || client_stats < L_BOOTED)
    return (0);

  /* if we are doing a (f)orward then skip if need be */
  if (more_val.forward)
  {
    if (more_val.forward < more_val.num_rows - 1)
    {
      more_val.forward++;
      return (1);
    }
    else
    {
      more_val.forward = FALSE;
      more_val.num_lines_scrolled = 0;
    }
  }

  /* too many lines in specified time? if not continue */
  present_time = time (0);

  if (present_time > more_val.last_line_time + more_val.delay)
  {
    more_val.num_lines_scrolled = 1;
    more_val.last_line_time = present_time;
    return (0);
  }

  /* not too many lines yet... so print out the line */
  more_val.num_lines_scrolled++;

  if (more_val.num_lines_scrolled < more_val.num_rows)
    return (0);

  /* oops.. too many lines... prompt the more */
  paused = TRUE;
  promptfor (more_buf, &c, PROMPT_CHAR);
  paused = FALSE;

  if (c == more_val.k_quit)
  {
    /* prevent problems if prompt triggers */
    kill_client_output++;
    kill_socket_output++;
    return (1);
  }
  else if (c == more_val.k_clear)
  {
    clear_screen ();
    more_val.num_lines_scrolled = 0;
  }
  else if (c == more_val.k_cancel)
  {
    more_val.on = FALSE;
  }
  else if (c == more_val.k_nonstop)
  {
    more_val.non_stop = TRUE;
  }
  else if (c == more_val.k_forward)
  {
    more_val.forward = TRUE;
    return (1);
  }
  else if (c == more_val.k_oneline)
  {
    more_val.num_lines_scrolled--;
  }
  else
  {
    more_val.num_lines_scrolled = 0;
  }

  refresh_input (0);
  more_val.last_line_time = time (0);
  return (0);
}

void msg (char *fmt, ...)
{
va_list vargs;
char buf[MAXSIZ];
char *p;

	if (!GET_BIT(options, DISPLAYING) || paused)
		return;

	va_start (vargs, fmt);
	(void) vsprintf (buf, fmt, vargs);
	va_end (vargs);

	if (!action_match_suppress) {
		p = buf;
		/* so '-- A' and 'A' don't match twice */
        	if (!strncmp (buf, "-- ", 3))
                	p += 3;

        	if (handle_action_matches (p)) {
                	return;
		}
	}

	if (kill_client_output) {
		if (logfile.redirect && logfile.on)
			fprintf (logfile.fd, "%s\n", buf);
		return;
	}
	display_msg (buf);
}


void
display_msg (char *s)
{
char store[MAXSIZ];

	if (!GET_BIT(options, DISPLAYING) || paused)
		return;
	if (pipe_running && !pipe_output)
		strcpy (store, s);
	if (logfile.on && logfile.level >= msg_type)
		fprintf (logfile.fd, "%s\n", s);
	wrap (s);
	term_normal_mode ();
	if (pipe_running && !pipe_output)
		send_process (store);
}

display_bold_communication (char *s)
{
char *p;
char ch;
char temp[MAXSIZ];

	if ((p = strchr (s, '>')) || (p = strchr (s, ':')) ||
	    (p = strchr (s, '=')) || (p = strchr (s, '!'))) {
		ch = *p;
		p--;
		*p = '\0';
		sprintf (temp, "%c%s%c %c%s", BOLD_CHAR, s, BOLD_CHAR,
			ch, (p+2));
		display_msg (temp);
	} else
		display_msg (s);
}

void msg_error (char *fmt, ...)
{
va_list vargs;
char buf[MAXSIZ];
char *p;

	if (!GET_BIT(options, DISPLAYING) || paused)
		return;

	va_start (vargs, fmt);
	(void) vsprintf (buf, fmt, vargs);
	va_end (vargs);

	/* -mfw
	if (errno < sys_nerr)
		msg ("%s %s", buf, sys_errlist[errno]);
	else
		msg ( "%s - Unknown error. Report Error #%d.", buf, errno);
	*/
	msg ("%s %s", buf, strerror(errno));
}

/*
 * debugging function
 * if the current debug level is equal to or greater than the
 * level of the given debug statement, then display it else
 * do nothing.
 *
 * Current debug levels are:
 *	1 - Minor statements indicating where and what the client is doing.
 *	2 - display introduction and exit values
 *	3 - detailed internal information of functions.
 *	4 - very specific internal information.
 */
void debug (int level, char *fmt, ...)
{
va_list vargs;
char buf[MAXSIZ];

	if (debug_level && debug_level >= level) {
		va_start (vargs, fmt);
		(void) vsprintf (buf, fmt, vargs);
		wrap (buf);
		term_normal_mode ();
		va_end (vargs);
	}

}

do_column_maker (char *s)
{
int len;

	len = strlen (s);
	if ((len + column_maker_pos + 1) > num_columns) {
		column_maker_buf[num_columns] = '\0';
		msg (column_maker_buf);
		set_column_maker (column_maker_width);
	}
	strcpy (column_maker_buf+column_maker_pos, s);
	*(column_maker_buf+column_maker_pos+len) = ' ';
	column_maker_pos += column_maker_width * (len / column_maker_width + 1);
}

set_column_maker (int width)
{
int i;
	column_maker_pos = 0;
	column_maker_width = width;
	for (i = 0; i < num_columns; i++)
		*(column_maker_buf+i) = ' ';
}

flush_column_maker (void)
{
	if (column_maker_pos)
		msg (column_maker_buf);
}

# define OCT1989	623303940
# define SECOND		1
# define MINUTES	(60 * SECOND)
# define HOURS		(60 * MINUTES)
# define DAYS		(24 * HOURS)

char *time_dur (long int dur)
{
long cnt = dur;
long yr = 0;                          
char *units = "seconds";
char *s;                          
char ybuf[200];
char dbuf[200];
static char tdbuf[400];

	if (dur < OCT1989) {
		strcpy (tdbuf, "a long time");
    		return (s = tdbuf);
  	}

	dur = time (0) - dur;
  	if (dur > (365 * DAYS + 20952)) {
		yr = dur / (365 * DAYS + 20952);    /* Years */
    		dur -= yr * (365 * DAYS + 20952);   /* Portion of a year */
  	}

  	if (dur < 101) {
		cnt = dur;
		units = "second";
  	} else if (dur < 6001) {
		cnt = dur/MINUTES;
		units = "minute";
  	} else if (dur < 120000) {
		cnt = dur/HOURS;
		units = "hour";
  	} else if (dur < 1200000) {
		cnt = dur/DAYS;
		units = "day";
  	} else if (dur < 13000000)       {
		cnt = dur/(7*DAYS);
		units = "week";
  	} else {
		cnt = dur/((365/12)*DAYS);
		units = "month";
	}

  	if (yr == 0)
		strcpy (ybuf, "");
  	else {
  		sprintf (ybuf, "%d year%s", yr, (yr == 1) ? "" : "s");
	}

  	if (cnt == 0)
		strcpy (dbuf, "");
  	else
		sprintf (dbuf, "%ld %s%s", cnt, units, (cnt == 1) ? "" : "s");

  	if (yr == 0 && cnt == 0)
		sprintf (tdbuf, "an instant");
  	else if (yr > 0 && cnt > 0)
		sprintf (tdbuf, "%s and %s", ybuf, dbuf);
  	else if (yr > 0)
		strcpy (tdbuf, ybuf);
  	else
  		strcpy (tdbuf, dbuf);

  	return (s = tdbuf);
}

void
remove_space_at_end (char *s)
{
char *p = s;

	if (strlen (s) == 1)
		return;
	p += strlen (s) - 1; /* get past the null term */
	while (*p == ' ')
		p--;
	*++p = '\0';		/* increment past last non space and null term */
}

/*
 * strfree checks the given ptr to see if it is NULL, if not.. it frees it.
 * strfree always returns NULL
 */
char *strfree (char *ptr)
{
	if (ptr)
		free (ptr);
	return ((char *) NULL);
}

place_string_on_output_window (char *str, int len)
{
	if (GET_BIT (options, FULLSCREEN)) {
		scroll_output_window ();
	}
	cursor_output_window ();
	write_string (str, len);
	last_output_row++;
	if (!GET_BIT (options, FULLSCREEN)) {
		scroll_output_window ();
	}
	add_refresh_line (str, len);
}

write_string (char *s, int cnt)
{
int i;
char *p;

	for (i = 0, p = s; i <= cnt; i++) {
		switch (*p) {
		case BELL_CHAR:
			if (GET_BIT (options, DO_BELLS)) {
				term_beep (1);
			} else {
				if (!term_standout_status ()) {
					term_standout_on ();
					term_putchar (*p + 'A' - 1);
					term_standout_off ();
				} else {
					term_putchar (*p + 'A' - 1);
				}
			}
			p++;
			break;
		case BOLD_CHAR:
			term_toggle_boldface ();
			p++;
			break;
		case INVERSE_CHAR:
			term_toggle_standout ();
			p++;
			break;
		case UNDERLINE_CHAR:
			term_toggle_underline ();
			p++;
			break;
		default:
			term_putchar (*p);
			p++;
			break;
		}
	}
}

init_refresh_lines (void)
{
int i;

	refresh_next = 0;
	refresh_line = (char **) malloc ((output_row + 1) * sizeof (char *));
	for (i = 0; i < (output_row+1); i++)
		refresh_line[i] = (char *) NULL;
}

add_refresh_line (char *s, int cnt)
{
        char cbuf[MAXSIZ]; // I increased the buffer size for ansi codes. -mfw

	strncpy (cbuf, s, cnt);
	cbuf[cnt] = '\0';

	if (refresh_line[refresh_next] != (char *) NULL)
		free (refresh_line[refresh_next]);
	refresh_line[refresh_next++] = string (cbuf);
	refresh_next %= (output_row+1);
}

int start_refresh_line_index (int *start_pos)
{
int i = refresh_next;
int oldrn = refresh_next;

	while (!refresh_line[i]) {
		i++;
		if (i > output_row)
			i = 0;
		if (i == oldrn)
			break;
	}
	/* don't have a full table yet */
	if (i == refresh_next)
		*start_pos = output_row;
	else
		*start_pos = refresh_next - 1;
	return (i);
}

/*
 * Clear the lines, but keep table intact
 */
clear_refresh_line (void)
{
int i;

	for (i = 0; i < (output_row+1); i++)
		if (refresh_line[i] != (char *) NULL)
			refresh_line[i] = strfree (refresh_line[i]);
	refresh_next = 0;
}

/*
 * Delete the refresh table. Lines need to be freed
 * via clear_refresh_line (). Reset via init_refresh_lines ().
 */
clear_refresh_line_node (void)
{
	if (refresh_line != (char **) NULL)
		free (refresh_line);
	refresh_next = 0;
}

int has_esc_codes(char *str)
{
	char *p;

	for (p = str; *p; *p++)
	{
		if (*p == ESC_CHAR)
		{
			return 1;
		}
	}
	return 0;
}
