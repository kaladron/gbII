/*
 * vars.h: contains a few generic changes for various systems
 * 	   and how they like to handle functions. (ie names and
 *	   parameter listing).
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

# ifdef SYSV
# define TERMIO
# endif

# ifdef CTIX
# define signal	sigset
# endif

# ifndef _CLIENT_TYPES_H_
# include "types.h"

extern struct input_modestruct input_mode;
extern struct logstruct logfile;
extern struct morestruct more_val;
extern struct profilestruct profile;
extern struct racestruct races[MAX_NUM_PLAYERS];
extern struct scopestruct scope;
extern struct sector_typestruct sector_type[SECTOR_MAX];
extern struct statusstruct status;
extern struct waitforstruct wait_csp;

extern Game *find_game (char *nick);

extern CurGame cur_game;
extern Info info;
extern Icomm icomm;
extern ServInfo servinfo;

# ifdef RWHO
extern struct rwhostruct rwho;
# endif

extern int action_match_suppress;
extern int client_stats;
extern int csp_server_vers;
extern int detached;
extern int end_prompt;
extern int game_type;
extern int gb_close_socket;
extern int hide_input;
extern int input_file;
extern int msg_type;
extern int paused;
extern int quit_all;
extern int racegen;
extern int wait_status;

extern long boot_time;
extern long connect_time;

extern char macro_char;

extern char *Discoveries[];
extern char *PlanetTypes[];
extern char *RaceType[];
extern char *RaceTypePad[];
extern char *Relation[];
extern char *SectorTypes[];
extern char SectorTypesChar[];

extern char gbrc_path[];

extern char *help_client;
extern char *progname;
extern char *shell;
extern char *shell_flags;

extern char **refresh_line;

extern char race_name[];
extern char govn_name[];
extern char race_pass[];
extern char govn_pass[];
extern int password_failed;

extern char *race_colors[];

# endif
/* _CLIENT_TYPES_H_ */
