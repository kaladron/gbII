/*
 * types.h: used to define needed #defines as well as to set up
 *	    structures used throughout the client. They get declared
 *	    by gb.c (with the trick of including types.h first) and then
 *	    are forever after declared as externs by including vars.h
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

# ifndef _CLIENT_TYPES_H_
# define _CLIENT_TYPES_H_
# endif

# include "csp_types.h"

/* a yes macro for promptfor () */
# define YES(A)		((A) == 'Y' || (A) == 'y')

/* various defines to make my life easier */
# define REPEAT_SLEEP_TIME	60	/* time to sleep if doing repeats */
# define MORE_DELAY		15	/* in secs, time to trigger more */

/* number of loops thru select () til exiting. to allow the server
 * to catch up and display a good bye message */
# define GB_CLOSE_SOCKET_DELAY	15

# define MAX_NUM_PLAYERS	65	/* 1-64, and 1 for 0 is 65 */
# define MAX_NUM_STARS		256
# define MAX_NUM_PLANETS_STAR	10
# define MAX_NUM_PLANETS	(MAX_NUM_STARS * MAX_NUM_PLANETS_STAR)
# define MAX_POST_LEN 		500

/* rwho stuff */
# define RWHO_DELAY		60
# define RWHO
# define RWHO_OFF		0
# define RWHO_ON		1
# define RWHO_NOTIFIED		2

/* which type of game have we logged into */
# define GAME_NOTGB		-1
# define GAME_UNKNOWN	0
# define GAME_GB			1
# define GAME_HAP			2
# define GAME_GBDT		3

# define NOTGB()			(game_type == GAME_NOTGB)
# define UNKNOWN()			(game_type == GAME_UNKNOWN)
# define GB()				(game_type == GAME_GB)
# define HAP()				(game_type == GAME_HAP)
# define GBDT()				(game_type == GAME_GBDT)

/* types of end prompts */
# define INTERNAL_PROMPT	-1	/* for internal workings */
# define NOT_PROMPT		0	/* none */
# define END_PROMPT		1	/* generic,connect refused,help,doing */
# define PASS_PROMPT		2	/* please enter ... */
# define DOING_PROMPT		3	/* DOING */
# define ENDDOING_PROMPT	4
# define NODIS_PROMPT		5	/* finished */
# define FINISHED_PROMPT	6
# define LEVEL_PROMPT		7	/* ( [*] /* ) */

/* commands we are processing */
# define C_DONEINIT		-2	/* init and re-init */
# define C_DONEPROC		-1	/* any internal icomm process */
# define C_NONE			0
# define C_RNEWS		1
# define C_RWHO			2
# define C_PROFILE		3
# define C_RELATION		4
# define C_STATUS		5
# define C_TELEGRAM		6

/* state of processing */
# define S_NONES			0	/* not doing any processing */
# define S_WAIT 		1	/* waiting for the start sequence */
# define S_PROC 		2	/* processing */
# define S_DONE			3	/* done, waiting to cleanup/shutdown*/

/* levels for scope */
# define LEVEL_ERROR		-2
# define LEVEL_NONE		-1
# define LEVEL_UNIV		0
# define LEVEL_STAR		1
# define LEVEL_PLANET		2
# define LEVEL_SHIP		3	/* use this for if (x > LEVEL_SHIP) */

# define LEVEL_USHIP		5
# define LEVEL_SSHIP		6
# define LEVEL_PSHIP		7
# define LEVEL_MOTHERSHIP	8	/* use for if (x > LEVEL_MOTHERSHIP */
# define LEVEL_USSHIP		9	/* ship on ship on ship... at univ */
# define LEVEL_SSSHIP		10	/* ship on ship on ship... at star */
# define LEVEL_PSSHIP		11	/* ship on ship on ship... at planet */

/* buffer sizes */
# define SMABUF			200
# define NORMSIZ 		1024
# define MAXSIZ			8192

/* modes of output */
# define NONE			0
# define SCREEN_ONLY		1
# define DO_SCREEN_ONLY		2
# define SERVER_ONLY		3
# define USER_DEFINED		4
# define ENCRYPTED		5
# define SERVER			6
# define NORM_ANNOUNCE		7
# define NORM_BROADCAST		8
# define GB_ANNOUNCE		9
# define GB_BROADCAST		10
# define GB_THINK		11
# define GB_EMOTE               12
# define HAP_ANNOUNCE		13
# define HAP_BROADCAST		14
# define HAP_THINK		15

/* modes for cursor */
# define EDIT_OVERWRITE		1
# define EDIT_INSERT		2

/* race types */
/* these are xref'ed to CSP_RELATION */
/* and will be obsolete soon */
# define RACE_UNKNOWN		0
# define RACE_MESO			1
# define RACE_NORM			2

/* defines for logging in and at which step */
# define L_NOTHING		0	/* pre init and not ready yet */
# define L_BOOTED		1	/* gbII has started itself after init */
# define L_CONNECTED		2	/* socket connection established */
# define L_PASSWORD		3	/* at password prompted */
# define L_LOGGEDIN		4	/* password entered and accepted */
# define L_SEGMENT		5	/* moveseg */
# define L_UPDATE		6	/* update */
# define L_INTERNALINIT		7	/* doing initial start commands */
# define L_REINIT		8	/* after update/segment reinit */
# define L_ACTIVE		9	/* ripping and rarin' to go */

/* wait_status flags */
# define WAIT_NONE		L_NOTHING
# define WAIT_BOOTED		L_BOOTED
# define WAIT_CONNECT		L_CONNECTED
# define WAIT_PASSWORD		L_PASSWORD
# define WAIT_LOGIN		L_LOGGEDIN
# define WAIT_SEGMENT		L_SEGMENT
# define WAIT_UPDATE		L_UPDATE


/* number of allowed queued Icommands */
# define MAX_ICOMMANDS		10

/* macros for manipulating Icommands */
# define ICOMM_DOING_COMMAND	(icomm.num)
# define ICOMM_INITIALIZE()	(icomm.num = 0)
# define ICOMM_IGNORE		icomm.list[0].ignore
# define ICOMM_PROMPT		icomm.list[0].prompt
# define ICOMM_STATE		icomm.list[0].state
# define ICOMM_COMM		icomm.list[0].comm
# define ICOMM_SIGNORE(n,x)	(icomm.list[(n)].ignore = (x))
# define ICOMM_SPROMPT(n,x)	(icomm.list[(n)].prompt = (x))
# define ICOMM_SSTATE(n,x)	(icomm.list[(n)].state = (x))
# define ICOMM_SCOMM(n,x)	(icomm.list[(n)].comm = (x))

/* types of relations */
/* these are now xref'ed to the CSP_RELATION values */
/* and will be obsolete soon */
# define RELATION_UNKNOWN	0
# define RELATION_ALLIED	1
# define RELATION_NEUTRAL	2
# define RELATION_ENEMY		3

/* types of bind modes */
# define BIND_ERR		-1
# define BIND_NORM		0
# define BIND_ESC		1
# define BIND_IMAP		2
# define BIND_MORE		3

/* default chars for more chars */
# define MORE_DEFAULT_QUITCH	'q'
# define MORE_DEFAULT_CLEARCH	'c'
# define MORE_DEFAULT_CANCELCH	'k'
# define MORE_DEFAULT_NONSTOPCH	'n'
# define MORE_DEFAULT_FORWARDCH	'f'
# define MORE_DEFAULT_ONELINECH	'\n'

/* values prompt_for take on in input_mode */
# define PROMPT_OFF		0
# define PROMPT_CHAR		1
# define PROMPT_STRING		2

/* special characters */
# define SEND_QUOTE_CHAR	'^'
# define SEND_QUOTE_PHRASE	"GBCS"
# define SEND_OLD_QUOTE_PHRASE	"gbII_client_special"
# define BELL_CHAR		'\007'
# define BOLD_CHAR		'\002'
# define INVERSE_CHAR		'\022'
# define UNDERLINE_CHAR		'\025'

# define BELL_CHAR_STR		"\007"
# define BOLD_CHAR_STR		"\002"
# define INVERSE_CHAR_STR	"\022"
# define UNDERLINE_CHAR_STR	"\025"

# define VAR_CHAR		'$'	/* variable char denote */

/* msg_type levels */
# define MSG_BROADCAST		1
# define MSG_ANNOUNCE		2
# define MSG_THINK		3
# define MSG_TELEGRAMS		4

# define MSG_COMMUNICATION	5	/* cut off point */

# define MSG_NEWS		6
# define MSG_EXTRA		7

# define MSG_PLANET		8
# define MSG_ORBIT		10	/* can't use currently */

# define MSG_ALL		11
# define MSG_NONE		12

/* log levels */
# define LOG_OFF		0
# define LOG_COMMUNICATION	MSG_COMMUNICATION
# define LOG_ALL		MSG_ALL

/* modes for imap settings */
# define IMAP_GEO		1
# define IMAP_NORMAL		2
# define IMAP_INVERSE		3

/* parsing modes */
# define PARSE_SLASH		1
# define PARSE_SLASH_NOTNL	2
# define PARSE_VARIABLES	3
# define PARSE_LOOP		4
# define PARSE_FOR_LOOP		5
# define PARSE_ALL		6

/* maximum amount of arguments in parsing line arguments */
# define MAX_NUM_ARGS		20

/* sector indexes */
# define SECTOR_OCEAN		1
# define SECTOR_GAS		2
# define SECTOR_ICE		3
# define SECTOR_MTN		4
# define SECTOR_LAND		5
# define SECTOR_DESERT		6
# define SECTOR_FOREST		7
# define SECTOR_PLATED		8
# define SECTOR_WORM       9
# define SECTOR_MAX		10

/* check_for_special_formatting types */
# define FORMAT_NORMAL		1
# define FORMAT_DEFAULT		1
# define FORMAT_HELP		2		/* server help format */

/* macro for secret input */
# define SECRET(B,S,T)	{ hide_input++; promptfor ((B), (S), (T)); \
			hide_input = 0; }

/* imap/popn defines */
# define MAX_SHIPS_IN_SURVEY	10

/* STRUCTS and other VARIABLE DECLARATIONS */

/* extern declared in vars.h, initialized in gb.c */
struct logstruct {
	FILE *fd;
	char name[BUFSIZ];
	int on;
	int redirect;
	int level;
};

typedef struct icommunitstruct {
	int comm;		/* which command are we doing */
	int csp_start;
	int csp_end;
	int state;		/* waiting/processing */
	int prompt;		/* what prompt to wait for */
	int ignore;		/* don't print line or not */
	int flag;		/* a flag for passing extra info */
} IcommUnit;

typedef struct icommstruct {
	int num;
	IcommUnit list[MAX_ICOMMANDS];
} Icomm;

typedef struct shipstruct {
	int shipno;
	char ltr;
	int owner;
	struct shipstruct *next;
	struct shipstruct *prev;
} Ship;

struct scopestruct {
	int aps;
	int starnum;
	int planetnum;
	int numships;
	int ship;
	Ship *motherlist;
	char star[200];
	char planet[200];
	char shipc[20];		/* for old GB games */
	char mothership[20];	/* for old GB games */
/*
	int level;
*/
	enum LOCATION level;
};

# ifdef RWHO
struct rwhostruct {
	long last_time;
	int on;
	struct rwhoplayers {
		char name[200];
		int id;
		long last_on;
		long last_spoke;
		char last_name[200];
		long changed_names;
		int on;
		int watch4;
	} info[MAX_NUM_PLAYERS];
};
# endif /* RWHO */

struct morestruct {
	char k_quit;
	char k_clear;
	char k_cancel;
	char k_nonstop;
	char k_forward;
	char k_oneline;
	int num_rows;
	int delay;
	int on;
	long last_line_time;
	int num_lines_scrolled;
	int non_stop;
	int forward;
	int doing;
};

struct statusstruct {
	long last_time;
	char last_buf[BUFSIZ];
	char current_buf[BUFSIZ];
	char format[BUFSIZ];
	char schar[2];
};

typedef struct profilestruct {
	enum PLAYER_TYPE player_type;	/* normal/guest/diety */
	char defscope[SMABUF];
	char personal[BUFSIZ];
	char sect_pref[SMABUF];

	char racename[SMABUF];
	char govname[SMABUF];
	int raceid;
	int govid;
	int capital;
	int updates_active;
	int know;
	char discovery[SMABUF];

	struct ranges {
		int guns;
		int space;
		int ground;
	} ranges;

	struct raceinfo {
		enum RACE_TYPE racetype;
		int morale;
		int fert;
		double birthrate;
		double mass;
		int fight;
		double metab;
		int sexes;
		int explore;
		double tech;
		double iq;
	} raceinfo;

	struct planetinfo {
		int temp;
		int methane;
		int oxygen;
		int helium;
		int nitrogen;
		int co2;
		int hydrogen;
		int sulfur;
		int other;
	} planet;

	struct sectorinfo {
		int ocean;
		int gas;
		int ice;
		int mtn;
		int land;
		int desert;
		int forest;
		int plated;
	} sector;
} Profile;

struct sector_typestruct {
	char sectc;
	int compat;
};

struct racestruct {
	char name[200];
	int type;
	enum RELATION you_to_them;
	enum RELATION them_to_you;
};

/**** for world/game construct *****/
typedef struct gamestruct {
	char *nick;
	char *host;
	char *port;
	char *type;        /* CHAP -mfw */
	char *racename;    /* CHAP -mfw */
	char *pripassword;
	char *govname;     /* CHAP -mfw */
	char *secpassword;
	int indx;
	struct gamestruct *next;
	struct gamestruct *prev;
} Game;	

typedef struct curgamestruct {
	Game game;
	int maxplayers;
} CurGame;

struct input_modestruct {
	int edit;
	int map;
	int say;
	int post;
	int promptfor;
	int offset;
};

/* csp waitfor */
struct waitforstruct {
	int lo;
	int hi;
	int have;
	char buf[MAXSIZ];
};

typedef struct sectorstruct {
	int x;
	int y;
	char sectc;
	char des;
	int wasted;
	int own;
	int eff;
	int frt;
	int mob;
	int xtal;
	int res;
	int civ;
	int mil;
	int mpopn;
	int numships;
	Ship ships[MAX_SHIPS_IN_SURVEY];
	int sect_status;		/* used by popn */
} Sector;


typedef struct mapstruct {
	int maxx;
	int maxy;
	char star[200];
	char planet[200];
	int res;
	int fuel;
	int des;
	int popn;
	int mpopn;
	int tox;
	double compat;
	int enslaved;
	int map;
	int inverse;	/* inverse on/off */
	int geo;	/* geo on/off */
	int ansi;	/* ansi on/off */
	Sector *ptr;
} Map;

/* buffer output structs */
typedef struct bufferstruct {
	char *buf;
	struct bufferstruct *next;
	struct bufferstruct *prev;
} Buffer;

typedef struct bufferinfostruct {
	Buffer *head;
	Buffer *tail;
	int partial;
	int is_partial;
} BufferInfo;

typedef struct infostruct {
	unsigned long bytes_read;
	unsigned long bytes_sent;
	unsigned long lines_sent;
	unsigned long lines_read;
} Info;

typedef struct serverinfo {
	int updates_suspended;
	int version;
} ServInfo;
