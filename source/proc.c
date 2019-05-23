/*
 * proc.c: functions dealing with processes from within the client.
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"

# include <stdio.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <sys/types.h>
# include <sys/wait.h>		/* for WNOHANG */
# include <signal.h>

# ifdef AIX
# include <sys/m_wait.h>
# endif

# include "str.h"
# include "vars.h"

#define PIPEBUF	1024

# ifndef FD_ZERO
# define DESCR_MASK int
# define FD_ZERO(A)	(*(A) = 0)
# define FD_SET(A,B)	(*(B) |= ( 1 << (A)))
# define FD_ISSET(A,B) (*(B) & (1 << (A)))
# else
# define DESCR_MASK fd_set
# endif

extern int gb;
extern int end_msg;

typedef struct procstruct {
	char *cmd;
	int pid;
	int p_stdin;
	int p_stdout;
	int p_stderr;
	int on;
	int done;
	char outpipe[PIPEBUF+1];
	char errpipe[PIPEBUF+1];
	int redirect;
	int hide;
	char *string;
} Process;

static Process proc = { (char *) NULL, -1, -1, -1, -1, FALSE, FALSE, "", "",
	 SCREEN_ONLY, FALSE, (char *) NULL };

int pipe_running = FALSE;
int pipe_output = FALSE;

# define PIPE_WRITE	1
# define PIPE_READ	0

extern char *strcat (char *, const char *);
extern int close (int);
extern int dup2 (int, int);
extern int execl (const char *, const char *, ...);
extern int pipe (int *);
extern int read (int, void *, unsigned int);
extern int write (int, const void *, unsigned int);
extern pid_t fork (void);
extern pid_t getpgrp (int);
extern pid_t getpid (void);
extern pid_t setpgrp (int, int);
/* extern pid_t wait3 (union wait *, int, struct rusage *); */
extern pid_t wait3 (int *, int, struct rusage *);
extern unsigned int sleep (unsigned int);
extern void free (void *);
char *print_process_string (char *header, char *s);
int kill_process (void);
int process_eof (void);
void procmsg (char *header, char *args);
int send_process (char *s);
void close_up_process (void);
void flush_process_string (char *header, char *s);
void procmsg (char *header, char *args);

cmd_proc (char *args)
{
int pifd[2];
int pofd[2];
int pefd[2];
char *p;
char *q;

# ifndef RESTRICTED_ACCESS

	if (!args || !*args) {
		if (proc.on)
			msg ("-- proc: '%s' running, use 'proc kill' to stop it.", proc.cmd);
		else
			msg ("-- proc: usage: proc [-a|b|d|e|s|t][q] command_name");
		return;
	}

	if (streq (args, "eof")) {
		process_eof ();
		msg ("-- proc: eof done.");
		return;
	}
	if (streq (args, "kill")) {
		kill_process ();
		msg ("-- proc: killed.");
		proc.on = FALSE;
		if (!proc.done) {
			close_up_process ();
			return;
		}
		return;
	}

	if (proc.on) {
		msg ("-- proc: process still running. sending.");
		send_process (args);
		return;
	}

	proc.redirect = SCREEN_ONLY;
	if (*args == '-') {
		p = args + 1;
		args = rest (args);
		while (*p != ' ') {
			switch (*p++) {
			case 'a':
				proc.redirect = NORM_ANNOUNCE;
				break;
			case 'b':
				proc.redirect = NORM_BROADCAST;
				break;
			case 'e':
				proc.redirect = GB_EMOTE;
				break;
			case 't':
				proc.redirect = GB_THINK;
				break;
			case 'c':
				proc.redirect = ENCRYPTED;
				q = first(p);
				while (*p != ' ') p++;
				*p = '\0';
				proc.string = string (q);
				*p = ' ';
				break;
			case 'p':
				proc.redirect = USER_DEFINED;
				q = first(p);
				while (*p != ' ') p++;
				*p = '\0';
				proc.string = string (q);
				*p = ' ';
				break;
			case 's':
			case 'd':
				proc.redirect = SERVER;
				break;
			case 'x':
			case 'q':
				proc.hide = 1;
				break;
			default:
				break;
			}
		}
	} 

	if (!args || !*args)
		return;

	msg ("-- proc: starting command '%s'", args);
	proc.cmd = string (args);

	if (pipe (pifd) || pipe (pofd) || pipe (pefd)) {
		msg_error ("-- proc pipe: ");
		return;
	}

	switch ((proc.pid = fork ())) {
	case -1:
		msg ("-- proc: could not fork process. Sorry.");
		close (pifd[PIPE_READ]);
		close (pifd[PIPE_WRITE]);
		close (pofd[PIPE_READ]);
		close (pofd[PIPE_WRITE]);
		close (pefd[PIPE_READ]);
		close (pefd[PIPE_WRITE]);
		close_up_process ();
		return;
	case 0:
# ifdef CTIX
		setpgrp ();
# else
		setpgrp (0, getpid ());
# endif
		signal (SIGINT, SIG_IGN);
		dup2 (pifd[PIPE_READ], 0);
		dup2 (pofd[PIPE_WRITE], 1);
		dup2 (pefd[PIPE_WRITE], 2);
		close (pifd[PIPE_WRITE]);
		close (pifd[PIPE_READ]);
		close (pofd[PIPE_READ]);
		close (pofd[PIPE_WRITE]);
		close (pefd[PIPE_READ]);
		close (pefd[PIPE_WRITE]);
		if (logfile.on)
			fclose (logfile.fd);
		execl (shell, shell, shell_flags, 
			args, (char *) NULL);
		/* this should NEVER be reached */
		write (gb, "-- Error starting shell.\n", 26);
		close_up_process ();
		exit (0);
		break;
	default:
		close (pifd[PIPE_READ]);
		close (pofd[PIPE_WRITE]);
		close (pefd[PIPE_WRITE]);
		proc.p_stdin = pifd[PIPE_WRITE];
		proc.p_stdout = pofd[PIPE_READ];
		proc.p_stderr = pefd[PIPE_READ];
		proc.on = TRUE;
		*proc.outpipe = '\0';
		*proc.errpipe = '\0';
		return;
	}

# else
	msg ("-- Restricted Access: shell escapes not available.");
# endif
}

read_process (fd_set rd)
{
char buf[PIPEBUF+1];
char *p;
char *q = buf;
int error;
int len;

	if (!proc.on)
		return;
	if (proc.p_stdout != -1 && FD_ISSET (proc.p_stdout, &rd)) {
		len = strlen (proc.outpipe);
		error = read (proc.p_stdout, buf, PIPEBUF - len);
		if (error < 0 && proc.done) {
			msg ("-- proc: received eof. %d", error);
			close_up_process ();
			return;
		} else if (error == 0) {
			flush_process_string (client_prompt, proc.outpipe);
			flush_process_string ("Proc (error):", proc.errpipe);
			msg ("-- proc: done.");
			close_up_process ();
		} else if (error < 0) {
			msg ("proc error is less than 0, report what you did please");
		} else {
			buf[error] = '\0';
			if (len) {
				strcat (proc.outpipe, buf);
				strcpy (buf, proc.outpipe);
			}
			p = print_process_string (client_prompt, buf);
			if (p)
				strcpy (proc.outpipe, p);
		}
	}
	q = buf;
	if (proc.p_stderr != -1 && FD_ISSET (proc.p_stderr, &rd)) {
		len = strlen (proc.errpipe);
		error = read (proc.p_stderr, buf, PIPEBUF - len);
		if (error < 0 && proc.done) {
			msg ("-- proc (error): received eof. %d", error);
			close_up_process ();
			return;
		} else if (error == 0) {
			flush_process_string (client_prompt, proc.outpipe);
			flush_process_string ("Proc (error):", proc.errpipe);
			msg ("-- proc: done.");
			close_up_process ();
		} else if (error < 0) {
			msg ("proc (error) is less than 0, report what you did please");
		} else {
			buf[error] = '\0';
			if (len) {
				strcat (proc.errpipe, buf);
				strcpy (buf, proc.errpipe);
			}
			p = print_process_string ("Proc (error):", buf);
			if (p)
				strcpy (proc.errpipe, p);
		}
	}
}

set_process (fd_set *rd)
{
	if (proc.on && proc.p_stdout > -1 && proc.p_stderr > -1) {
		FD_SET (proc.p_stdout, rd);
		FD_SET (proc.p_stderr, rd);
	}
}

/*
 * the process signals that it is done.
 */
check_process (void)
{
# ifndef CTIX
int pid;
/*union wait status;*/
int status;

	pid = wait3 (&status, WNOHANG, (struct rusage *) NULL);
	if (pid > 0 && pid == proc.pid)
		kill_process ();
# endif
}

send_process (char *s)
{
	if (proc.p_stdin != -1) {
		write (proc.p_stdin, s, strlen (s));
		write (proc.p_stdin, "\n", 1);
	} else if (!pipe_running)
		msg ("-- proc: process stdin is closed.");
}

process_eof (void)
{
	close (proc.p_stdin);
	proc.p_stdin = -1;
}

int process_running (void)
{
	return (proc.on);
}

/*
 * sends a term signal to allow the process group to clean up nicely
 * if this is not enough, then it sends a kill signal. Process group
 * should be -1 if the pid is invalid and is used to check.
 * proc.done is set to true, since the process is no longer valid
 * and needs to be marked for clean up elsewhere.
 */
kill_process (void)
{
	if (proc.on) {
		if (!proc.done && getpgrp (proc.pid) > -1) {
# ifdef CTIX
			kill (-proc.pid, SIGTERM);
# else	
			kill (-getpgrp (proc.pid), SIGTERM);
# endif
			sleep (2);
			if (getpgrp (proc.pid) > 0)
# ifdef CTIX
				kill (-proc.pid, SIGKILL);
# else
				kill (-getpgrp (proc.pid), SIGKILL);
# endif
		}
		if (getpgrp (proc.pid) == -1)
			proc.done = TRUE;
	}
}

proc_test (void)
{
msg ("-- client pgrp = %d.", getpgrp (getpid ()));
msg ("-- proc pgrp = %d.", getpgrp (proc.pid));
}

char *print_process_string (char *header, char *s)
{
char *p;
char *q = s;

	while ((p = strchr (q, '\n'))) {
		*p = '\0';
	 	procmsg (header, q);
		q = p + 1;
	}
	if ((q == s) && (strlen (s) == PIPEBUF)) {
		procmsg (header, q);
		return ((char *) NULL);
	}
	return (q);
}

void flush_process_string (char *header, char *s)
{

	if (s && *s) {
		procmsg (header, s);
		*s = '\0';
	}
}

void close_up_process (void)
{
	proc.on = FALSE;
	proc.done = FALSE;
	close (proc.p_stdin);
	close (proc.p_stdout);
	close (proc.p_stderr);
	proc.p_stdin = -1;
	proc.p_stdout = -1;
	proc.p_stderr = -1;
	proc.redirect = SCREEN_ONLY;
	proc.hide = FALSE;
	free (proc.cmd);
	if (proc.string) {
		free (proc.string);
		proc.string = NULL;
	}
}

signal_pipe (void)
{
	close_up_process ();
}

void procmsg (char *header, char *args)
{
char buf[MAXSIZ];
char buf2[MAXSIZ];


		switch (proc.redirect) {
		case NORM_ANNOUNCE:
			sprintf (buf2, "announce %s", args);
			break;
		case NORM_BROADCAST:
			sprintf (buf2, "broadcast %s", args);
			break;
		case GB_EMOTE:
			sprintf (buf2, "emote %s", args);
			break;
		case GB_THINK:
			sprintf (buf2, "think %s", args);
			break;
		case SCREEN_ONLY:
		case SERVER:
			strcpy (buf2, args);
			break;
		case ENCRYPTED:
			sprintf (buf, "%s %s", proc.string, args);
			cmd_cr (buf);
			sprintf (buf2, "cr %s %s", proc.string, args);
			break;
		case USER_DEFINED:
			sprintf (buf2, "%s %s", proc.string, args);
			break;
		default:
			msg ("-- proc: error - unknown redirect");
			break;
		}
		if (!proc.hide) {
			pipe_output = TRUE;
			msg ("%s %s", header, buf2);
			pipe_output = FALSE;
		}
		if (proc.redirect != SCREEN_ONLY &&
		    proc.redirect != ENCRYPTED) {
			end_msg++;
			add_queue (buf2);
		}
	if (proc.redirect != SCREEN_ONLY)
		end_msg--;
}
void internal_pipe_off (void)
{
extern int pipe_running;

	if (pipe_running) {
		process_eof ();
		pipe_running = FALSE;
	}
}
