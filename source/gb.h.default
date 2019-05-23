/*
 * gb.h: meta header for the software
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

/*
 * The VERSION is used to track differences between client versions.
 * This should only be changed if you make a mod, and should reflect
 * your changes. For example: 2.4.2-mymod2.2 or something. NOT
 * the same format that I use.
 * The GBSAYING is just the log off message which I periodically
 * use for what ever philosophical statement I have for the current
 * client release. It does not serve any important function.
 */
/* # define VERSION		"2.5.4 12/03/93" -mfw */
# define VERSION		"2.6.0 1/25/05"

# define GBSAYING		"-- We gladly feast on those who would subdue us -- Meso Motto"
/*
# define GBSAYING		"-- GB and reality are mutually exclusive!"
*/

/*
 * DEFAULT_GBPORT is the default port for connecting to games.
 * GBSERVER_VERS was used in the past but currently serves no
 * important function, but it may in the future.
 */
# define DEFAULT_GBPORT			2010
# define GBSERVER_VERS		60

/* these lines are set by the setup script and can NOT be removed */
/*
 * TERMIO causes the client to compile using termio instead of termcap.
 * SYSV forces client compilation for a System V environment.
 * CTIX is a unix like version used by a player.
 * INDEX is used by some systems over strchr along with a few other
 * functions. memcpy/bcopy, memset/bzero, strchar/index, strrchr/rindex.
 * XMAP compiles the client with the XMAP functions.
 * IMAP compiles the client with the Imap functions.
 * OPTTECH compiles the client with the code for opttech included
 * POPN compiles the client with the auto mover ability (popn) enacted
 * NOTE: not including xmap and/or imap will reduce client size.
 * RESTRICTED causes the client to limit access. See the Help file.
 * SMART_CLIENT is currently a beta test and to a normal user
 * serves no purpose.
 * CLIENT_DEVEL causes extra warnings to be displayed, for anyone
 * developing the client. This has to be manually set or change the
 * author variable in the setup script so it will be set automatically.
 *
 */

# define TERMIO
# define SYSV
# define CTIX

# define USE_INDEX

# define XMAP

# define ARRAY
# define IMAP
# define OPTTECH
# define POPN
# define RESTRICTED_ACCESS
# define SMART_CLIENT

# define CLIENT_DEVEL

/*
 * Locations of help files, for the client and server.
 */
# define HELP_CLIENT		"/usr/games/lib/Help"
# define HELP_SERVER		"/usr/games/lib/Help_server"

# define DEFAULT_GBRC_PATH	"~/.gbrc"	/* path to pre-init file */
# define DEFAULT_HISTORY	50		/* num to keep in history */
# define DEFAULT_RECALL		100		/* num to keep for recall */

/*
 * CLIENT_PROMPT is output generated from the client (like proc commands)
 * INPUT_PROMPT is in the input window.
 * OUTPUT_PROMPT is when you hit return and it is displayed in the output
 * window
 */
# define DEFAULT_CLIENT_PROMPT	"-> "
# define DEFAULT_INPUT_PROMPT	"command> "
# define DEFAULT_OUTPUT_PROMPT	"> "

/* the format for the status bar See the help file about the parameters */
# define DEFAULT_STATUS_BAR	"$c$S$c$t$M$c$P$R$E$c$m$c$c$T$c"
# define DEFAULT_STATUS_BAR_CHAR "-"		/* must be a string */

# define MAILPATH 		"/var/mail"	/* path to mail file */
# define MAIL_DELIMITER 	"From "			/* mail seperator */
# define MAIL_DELIMITER_LEN 	5			/* for strncmp check */

# define DEFAULT_CURSOR_SECTOR	'$'	/* use for current sector in Imap */

# define DEFAULT_SHELL		"/bin/csh"
# define DEFAULT_SHELL_FLAGS	"-cf"

# ifndef TRUE
# define TRUE 			1
# endif

# ifndef FALSE
# define FALSE 			0
# endif

# ifndef ERROR
# define ERROR			-1
# endif

#ifdef CLIENT_DEVEL
extern int client_devel;
#endif

#define ROBONAME  "Robby"
#define MAXSTARS  300

#define HYPER_DIST_FACTOR       200.0
#define HYPER_DRIVE_FUEL_USE    5.0
