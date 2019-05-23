/* command.h -- client commands.  
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1991, 1992, 1993 
 *
 * See the COPYRIGHT file.
 *
 */

/* functions for binary search */
extern void cmd_action ();
extern void cmd_assign ();
extern void cmd_bind ();
extern void cmd_cleardef ();
extern void cmd_cleargag ();
extern void cmd_connect ();
extern void cmd_cr ();
extern void cmd_crypt ();
extern void cmd_def ();
extern void cmd_do_macro ();
extern void cmd_for ();
extern void cmd_fuse ();
extern void cmd_gag ();
extern void cmd_game ();
extern void cmd_if ();
extern void cmd_listassign ();
extern void cmd_listcrypt ();
extern void cmd_listdef ();
extern void cmd_listgag ();
extern void cmd_listgame ();
extern void cmd_listloop ();
extern void cmd_loadf ();
extern void cmd_log ();
extern void cmd_loop ();
# ifdef XMAP
extern void cmd_map ();
extern void cmd_xmap ();
# endif

# ifdef OPTTECH
extern void cmd_opttech ();
# endif
extern void cmd_ping ();
extern void cmd_proc ();
extern void cmd_uncrypt ();
extern void cmd_removeloop ();
extern void cmd_savef (char *args);
extern void cmd_set ();
extern void cmd_source ();
extern void cmd_talk ();
extern void cmd_oldshell ();
extern void cmd_popn ();
extern void cmd_undef ();
extern void cmd_ungame ();
extern void cmd_ungag ();
extern void cmd_wait (char *args);
extern void cmd_psmap ();

extern void internal_pipe_off ();

void cmd_cd (char *args);
void cmd_clear (char *args);
void cmd_convo (char *args);
void cmd_detach (char *args);
void cmd_echo (char *args);
void cmd_help (char *args);			/* server command */
void cmd_helpc (char *args);
void cmd_helps (char *args);
void cmd_hide (char *args);
void cmd_info (char *args);
void cmd_internal (char *args);
void cmd_post (char *args);
void cmd_quit (char *args);
void cmd_quote (char *args);
void cmd_read (char *args);			/* server command */
void cmd_recall (char *args);
void cmd_repeat (char *args);
void cmd_msg (void);
void cmd_queue (char *args);
void cmd_sleep (char *args);
void cmd_version (char *args);
void cmd_watch4 (char *args);

/* for binary search */
typedef struct command_struct {
	char *name;
	void (*func)();
	int parse_var;
	int echo_command;
	int send_to_socket;
	int cnt;
} Command;

Command *binary_search (char *cmd);

/* echo/noecho on output window */
# define ECHOS 			1
# define NOECHOS			0

/* parse/no parse thru parse_variables */
# define PAR			1
# define NOPAR			0

/* send/nosend 'args' to send_gb */
# define SEND			1
# define NOSEND			0

static Command command_table[] =
{
/* Name		 Function	Sub	Echo	Socket Cnt */
{ "action",	 cmd_action,	PAR,	NOECHOS,	NOSEND, 0 },
{ "addgame",     cmd_game,	PAR,	ECHOS,	NOSEND, 0 },
{ "assign",      cmd_assign,	PAR,	ECHOS,	NOSEND, 0 },
{ "bind",	 cmd_bind,	PAR,	ECHOS,	NOSEND, 0 },
{ "cd",		 cmd_cd,	PAR,	ECHOS,	SEND,   0 },
{ "clear",       cmd_clear,	PAR,	NOECHOS,	NOSEND, 0 },
{ "cleardef",    cmd_cleardef,	PAR,	ECHOS,	NOSEND, 0 },
{ "cleargag",    cmd_cleargag,	PAR,	ECHOS,	NOSEND, 0 },
{ "connect",     cmd_connect,	PAR,	ECHOS,	NOSEND, 0 },
{ "convo",       cmd_convo,	PAR,	NOECHOS,	NOSEND, 0 },
{ "cr",          cmd_cr,	PAR,	ECHOS,	SEND,   0 },
{ "crypt",       cmd_crypt,	PAR,	ECHOS,	NOSEND, 0 },
{ "def",         cmd_def,	NOPAR,	ECHOS,	NOSEND, 0 },
{ "detach",	 cmd_detach,	PAR,	ECHOS,	NOSEND, 0 },
{ "echo",        cmd_echo,	PAR,	NOECHOS,	NOSEND, 0 },
{ "for",	 cmd_for,	NOPAR,	ECHOS,	NOSEND, 0 },
{ "fuse",	 cmd_fuse,	PAR,	ECHOS,	NOSEND, 0 },
{ "gag",         cmd_gag,	PAR,	ECHOS,	NOSEND, 0 },
{ "game",        cmd_game,	PAR,	ECHOS,	NOSEND, 0 },
{ "help",        cmd_help,	NOPAR,	ECHOS,	SEND,   0 },
{ "helpc",       cmd_helpc,	PAR,	NOECHOS,	NOSEND, 0 },
{ "helps",       cmd_helps,	PAR,	ECHOS,	NOSEND, 0 },
{ "hide",	 cmd_hide,	PAR,	ECHOS,	NOSEND, 0 },
{ "if",	 	 cmd_if,	NOPAR,	ECHOS,	NOSEND, 0 },
{ "info",	 cmd_info,	PAR,	NOECHOS,	NOSEND, 0 },
{ "internal",	 cmd_internal,	PAR,	ECHOS,	NOSEND, 0 },
{ "internal_pipe_off", internal_pipe_off, NOPAR, NOECHOS, NOSEND, 0 },
{ "last",        cmd_recall,	PAR,	NOECHOS,	NOSEND, 0 },
{ "listassign",  cmd_listassign, PAR,	NOECHOS,	NOSEND, 0 },
{ "listcrypt",   cmd_listcrypt,	PAR,	NOECHOS,	NOSEND, 0 },
{ "listdef",     cmd_listdef,	PAR,	NOECHOS,	NOSEND, 0 },
{ "listgag",     cmd_listgag,	PAR,	NOECHOS,	NOSEND, 0 },
{ "listgame",    cmd_listgame,	PAR,	NOECHOS,	NOSEND, 0 },
{ "listloop",    cmd_listloop,	PAR,	NOECHOS,	NOSEND, 0 },
{ "loadf",       cmd_loadf,	PAR,	ECHOS,	NOSEND, 0 },
{ "log",         cmd_log,	PAR,	NOECHOS,	NOSEND, 0 },
{ "loop",        cmd_loop,	PAR,	ECHOS,	NOSEND, 0 },
# ifdef XMAP
{ "map",	 cmd_map,	PAR,	ECHOS,	SEND,   0 },
# endif
{ "msg",	 cmd_msg,	PAR,	NOECHOS,	NOSEND, 0 },
{ "oldsh",	 cmd_oldshell,	PAR,	ECHOS,	NOSEND, 0 },
# ifdef OPTTECH
{ "opttech",	 cmd_opttech,	NOPAR,	NOECHOS,	NOSEND, 0 },
# endif
{ "ping",	 cmd_ping,	PAR,	ECHOS,	NOSEND, 0 },
{ "popn",	 cmd_popn,	PAR,	ECHOS,	NOSEND, 0 },
{ "post", 	 cmd_post,	PAR,	ECHOS,	NOSEND, 0 },
{ "proc",	 cmd_proc,	PAR,	NOECHOS,	NOSEND, 0 },
{ "psmap",	 cmd_psmap,	PAR,	ECHOS,	NOSEND, 0 },
{ "queue",	 cmd_queue,	PAR,	ECHOS,	NOSEND, 0 },
{ "quit",        cmd_quit,	PAR,	NOECHOS,	NOSEND, 0 },
{ "quote",       cmd_quote,	PAR,	ECHOS,	NOSEND, 0 },
{ "rea",	 cmd_read,	NOPAR,	ECHOS,	NOSEND, 0 },
{ "read",	 cmd_read,	NOPAR,	ECHOS,	NOSEND, 0 },
{ "recall",      cmd_recall,	PAR,	NOECHOS,	NOSEND, 0 },
{ "removeloop",  cmd_removeloop, PAR,	ECHOS,	NOSEND, 0 },
{ "repeat",      cmd_repeat,	PAR,	ECHOS,	NOSEND, 0 },
{ "savef",	 cmd_savef,	PAR,	ECHOS,	NOSEND, 0 },
{ "set",         cmd_set,	NOPAR,	NOECHOS,	NOSEND, 0 },
{ "sh",          cmd_proc,	PAR,	NOECHOS,	NOSEND, 0 },
{ "shell",       cmd_proc,	PAR,	NOECHOS,	NOSEND, 0 },
{ "sleep",       cmd_sleep,	PAR,	ECHOS,	NOSEND, 0 },
{ "source",      cmd_source,	PAR,	ECHOS,	NOSEND, 0 },
{ "talk",	 cmd_talk,	PAR,	NOECHOS,	NOSEND, 0 },
{ "uncrypt",     cmd_uncrypt,	PAR,	ECHOS,	NOSEND, 0 },
{ "undef",       cmd_undef,	PAR,	ECHOS,	NOSEND, 0 },
{ "ungag",       cmd_ungag,	PAR,	ECHOS,	NOSEND, 0 },
{ "ungame",      cmd_ungame,	PAR,	ECHOS,	NOSEND, 0 },
{ "unloop",      cmd_removeloop, PAR,	ECHOS,	NOSEND, 0 },
{ "version",     cmd_version,	PAR,	NOECHOS,	NOSEND, 0 },
{ "wait",	 cmd_wait,	PAR,	ECHOS,	NOSEND, 0 },
{ "watch4",      cmd_watch4,	PAR,	NOECHOS,	NOSEND, 0 }
#ifdef XMAP
,{"xmap",        cmd_xmap,      NOPAR,  NOECHOS,NOSEND, 0}
#endif
};

#define NUM_COMMANDS (sizeof (command_table) / sizeof (Command))
