/*
 * status.c: handles building up the status bar format.
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1991
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"

/*
# ifdef SYSV
# include <time.h>
# else
# include <sys/time.h>
# endif
 */

# include <sys/types.h>
# include <sys/stat.h>
# include <stdio.h>
# include <ctype.h>
# include <pwd.h>
# include <string.h>
# include <time.h>

# include "csp_types.h"
# include "str.h"
# include "term.h"
# include "option.h"
# include "vars.h"

extern int gb;
extern char pbuf[];
extern char last_prompt[];
extern long map_time;
extern char post_buf[];

# ifdef AIX
struct  tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
	char *tm_zone;
	long tm_gmtoff;
};
# endif

/* set it way high so the new mail notification does not occur at log in */
int mcnt = 9999999;

char last_update_buf[BUFSIZ];

extern char *getlogin (void);
extern char *strcat (char *, const char *);
extern int fclose (FILE *);
extern int strncmp (const char *, const char *, size_t);
extern time_t time (time_t *);
extern uid_t getuid (void);

void force_update_status (void);
int put_status (void);
char *print_time (long);

void
update_status (void)
{
long present;

	present = time (0);
	if ((status.last_time + 60) > present)
		return;
	force_update_status ();
}

void
force_update_status (void)
{
char *getbuf;
char *build_status (void);


	if (detached || client_stats == L_NOTHING)
		return;

	if (gb < 0)
		strcpy (last_prompt, "Not Connected");

	getbuf = build_status ();

	strcpy (status.current_buf, getbuf);
	if (!streq (status.last_buf, status.current_buf))
		put_status ();
}

put_status (void)
{
	term_move_cursor (0, num_rows - 2);
	write_string (status.current_buf, num_columns);
	term_normal_mode ();
	strcpy (status.last_buf, status.current_buf);
	cursor_to_window ();
}

int check_mail (void)
{
int cnt;
int oldmcnt = mcnt;
FILE *fmail;
char mpath[BUFSIZ];
static char *uname = NULL;
struct passwd *pwname;

# ifdef MAILPATH
	/* uname is static so this check is only made once instead of
	 * everytime thru here */
	if (!uname) {
		if ((uname = getlogin ()) == NULL) {
			pwname = getpwuid (getuid ());
			uname = pwname->pw_name;
		}
	}
		
	sprintf (mpath, "%s/%s", MAILPATH, uname);
	if ((fmail = fopen (mpath, "r")) == NULL) {
		return (-2);
	}

	cnt = 0;
	while (fgets (pbuf, BUFSIZ, fmail)) {
		if (strncmp (pbuf, MAIL_DELIMITER, MAIL_DELIMITER_LEN) == 0)
			cnt++;
	}
	fclose (fmail);
	if (cnt > oldmcnt && client_stats > L_NOTHING)
		msg ("-- You have new mail.");
	return (cnt);
# else
	return (-1);
# endif
}

char *print_time (long ptime)
{
static char temp[6];
struct tm *present_time;

	if (ptime == -1) 
		return ("--:--");

	present_time = localtime (&ptime);
	sprintf (temp, "%2d:%2d", present_time->tm_hour, present_time->tm_min);
	if (present_time->tm_hour < 10)
		temp[0] = '0';
	if (present_time->tm_min < 10)
		temp[3] = '0';
	temp[5] = '\0';

	return (temp);
}

char *build_status (void)
{
static char new_status_buf[BUFSIZ];
char end_buf[BUFSIZ];
char *p = status.format;
char *q = new_status_buf;
long present;
char temp[SMABUF];
int len;
char *r;


	present = time (0);
        status.last_time = present;
	*end_buf = '\0';
	while (*p) {
		if (*p == '$') {
			switch (*++p) {
			case 'B':
				strcpy (q, print_time (boot_time));
				q += 5; /* HH:MM is 5 */
				break;
			case 'C':
				strcpy (q, print_time (connect_time));
				q += 5; /* HH:MM is 5 */
				break;
			case 'E':
				sprintf (temp, "(%c)",
					(input_mode.edit == EDIT_OVERWRITE ? 'O' : 'I'));
				strcpy (q, temp);
				q += strlen (temp);
				break;
			case 'H':
				if (*cur_game.game.host) {
					strcpy (q, cur_game.game.host);
					q += strlen (cur_game.game.host);
				}
				break;
			case 'I':
				sprintf (temp, "%d", profile.raceid);
				strcpy (q, temp);
				q += strlen (temp);
				break;
			case 'M':
				if (input_mode.map) {
					sprintf (temp, "(%c)",
						(input_mode.say ? 'S' : 'M'));
					strcpy (q, temp);
					q += strlen (temp);
				}
				break;
			case 'N':
				strcpy (q, profile.racename);
				q += strlen (profile.racename);
				break;
			case 'P':
				if (input_mode.post) {
					sprintf (temp, "(%d)", MAX_POST_LEN - strlen (post_buf)-1);
					strcpy (q, temp);
					q += strlen (temp);
				}
				break;
			case 'R':
				*q = '\0';
				q = end_buf;
				p++;
				continue;
				break;
			case 'S':
				strcpy (q, last_prompt);
				if (GET_BIT (options, BRACKETS)) {
					r = strchr (q, '[');
					*r = '(';
					r = strchr (q, ']');
					*r = ')';
				}
				q += strlen (last_prompt);
				break;
			case 'T':
        			if (GET_BIT (options, SHOW_CLOCK)) {
					strcpy (q, print_time (present));
					q += 5;
				}
				break;
			case 'c':
				*q++ = *status.schar;
				break;
			case 'g':
				if (cur_game.game.nick && *cur_game.game.nick) {
					strcpy (q, cur_game.game.nick);
					q += strlen (cur_game.game.nick);
				}
				break;
			case 'i':
				sprintf (temp, "%d", profile.govid);
				strcpy (q, temp);
				q += strlen (temp);
				break;
			case 'm':
				if (GET_BIT (options, SHOW_MAIL) &&
				   ((mcnt = check_mail ()) > 0)) {
                			sprintf (temp, "Mail: %d", mcnt);
					strcpy (q, temp);
					q += strlen (temp);
				}
				break;
			case 'n':
				strcpy (q, profile.govname);
				q += strlen (profile.govname);
				break;
			case 't':
				if (in_talk_mode()) {
					strcpy (q, "(T)");
					q += 3;
				}
				break;
			default:
				*q++ = '$';
				*q++ = *p;
				break;
			} /* switch */
			p++;
		} else
			*q++ = *p++;
	}
	*q = '\0';
	
	len = num_columns - strlen (new_status_buf) - strlen (end_buf);
	for ( ; len > 0; len--)
		strcat (new_status_buf, status.schar);
	strcat (new_status_buf, end_buf);
	new_status_buf[num_columns] = '\0';
	return (new_status_buf);
}

char *build_scope_prompt (void)
{
static char sbuf[BUFSIZ];
char shipbuf[BUFSIZ];
char temp[SMABUF];
int i;
Ship *s;


	*shipbuf = '\0';
	if (scope.numships == -1) { /* old style scope */
		switch (scope.level) {
		case LEVEL_USSHIP: scope.level = CSPD_UNIV;
		case LEVEL_SSSHIP: scope.level = CSPD_STAR;
		case LEVEL_PSSHIP: scope.level = CSPD_PLAN;
			sprintf (shipbuf, "/../#%s", scope.shipc);
			break;
		case LEVEL_USHIP: scope.level = CSPD_UNIV;
		case LEVEL_SSHIP: scope.level = CSPD_STAR;
		case LEVEL_PSHIP: scope.level = CSPD_PLAN;
			sprintf (shipbuf, "/#%s", scope.shipc);
			break;
		default:
			break;
		}
	} else if (scope.numships) {
		for (s = scope.motherlist, i = 1; i < scope.numships;
		     s = s->next, i++) {
			sprintf (temp, "/#%d", s->shipno);
			strcat (shipbuf, temp);
		}
		sprintf (temp, "/#%d", scope.ship);
		strcat (shipbuf, temp);
	}

	switch (scope.level) {
	case CSPD_UNIV:
		sprintf (sbuf, "( [%d] /%s )", scope.aps, shipbuf);
		break;
	case CSPD_STAR:
		sprintf (sbuf, "( [%d] /%-s%s )", scope.aps, scope.star,
			shipbuf);
		break;
	case CSPD_PLAN:
		sprintf (sbuf, "( [%d] /%-s/%-s%s )", scope.aps,
			scope.star, scope.planet, shipbuf);
		break;
	default:
		break;
	}

	return (sbuf);
}
