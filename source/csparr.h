/*
 * csparr.h: Contains the table for Client-Server Protocol (CSP)
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1993
 *
 * See the COPYRIGHT file.
 */

/* for receive binary search */
void cspr_backup_end (int cnum, char *line);
void cspr_client_off (int cnum, char *line);
void cspr_client_on (int cnum, char *line);
void cspr_explore (int cnum, char *line);
void cspr_err (int cnum, char *line);
void cspr_event (int cnum, char *line);
void cspr_knowledge (int cnum, char *line);
void cspr_map (int cnum, char *line);
void cspr_orbit (int cnum, char *line);
void cspr_pause (int cnum, char *line);
void cspr_ping (int cnum, char *line);
void cspr_print (int cnum, char *line);
void cspr_profile (int cnum, char *line);
void cspr_relation (int cnum, char *line);
void cspr_reset_end (int cnum, char *line);
void cspr_scope_prompt (int cnum, char *line);
void cspr_segment_end (int cnum, char *line);
void cspr_survey (int cnum, char *line);
void cspr_update_end (int cnum, char *line);
void cspr_updates_suspended (int cnum, char *line);
void cspr_updates_resumed (int cnum, char *line);
void cspr_who (int cnum, char *line);
void cspr_psmap (int cnum, char *line);
void cspr_fuse (int cnum, char *line);

/* for send binary search */
char *csps_null (char *s);
char *csps_explore (char *s);
char *csps_knowledge (char *s);
char *csps_login (char *s);
char *csps_map (char *s);
char *csps_ping (char *s);
char *csps_relation (char *s);
char *csps_scope (char *s);
char *csps_survey (char *s);
char *csps_version (char *s);
char *csps_who (char *s);

typedef struct cspsendval_struct {
	char *name;
	int comm_num;
	char *(*func)();
	int cnt;
	int know;
} CSPSendVal;

typedef struct cspreceiveval_struct {
	int comm_num;
	void (*func)();
	int cnt;
	int know;
} CSPReceiveVal;

CSPSendVal *csps_binary_search (int cnum);
CSPReceiveVal *cspr_binary_search (int cnum);


static CSPReceiveVal csp_receive_table[] = {
/* Name		 		Function	      Cnt, Know */
{ CSP_BACKUP_START,		cspr_event,		0, 0 },
{ CSP_BACKUP_END,		cspr_backup_end,	0, 0 },
{ CSP_CLIENT_ON,		cspr_client_on,		0, 0 },
{ CSP_CLIENT_OFF,		cspr_client_off,	0, 0 },
{ CSP_EXPLORE_INTRO,		cspr_explore,		0, 0 },
{ CSP_EXPLORE_STAR,		cspr_explore,		0, 0 },
{ CSP_EXPLORE_STAR_DATA,	cspr_explore,		0, 0 },
{ CSP_EXPLORE_STAR_ALIENS,	cspr_explore,		0, 0 },
{ CSP_EXPLORE_STAR_END,		cspr_explore,		0, 0 },
{ CSP_EXPLORE_END,		cspr_explore,		0, 0 },
{ CSP_KNOWLEDGE,		cspr_knowledge,		0, 0 },
{ CSP_MAP_INTRO,		cspr_map,		0, 0 },
{ CSP_MAP_DYNAMIC_1,		cspr_map,		0, 0 },
{ CSP_MAP_DYNAMIC_2,		cspr_map,		0, 0 },
{ CSP_MAP_ALIENS,		cspr_map,		0, 0 },
{ CSP_MAP_DATA,			cspr_map,		0, 0 },
{ CSP_MAP_END,			cspr_map,		0, 0 },
{ CSP_ORBIT_OUTPUT_INTRO,		cspr_orbit,		0, 0 },
{ CSP_ORBIT_STAR_DATA,		cspr_orbit,		0, 0 },
{ CSP_ORBIT_UNEXP_PL_DATA,		cspr_orbit,		0, 0 },
{ CSP_ORBIT_EXP_PL_DATA,		cspr_orbit,		0, 0 },
{ CSP_ORBIT_SHIP_DATA,		cspr_orbit,		0, 0 },
{ CSP_ORBIT_OUTPUT_END,		cspr_orbit,		0, 0 },
{ CSP_PAUSE,			cspr_pause,		0, 0 },
{ CSP_PING,			cspr_ping,		0, 0 },
{ CSP_PROFILE_INTRO,		cspr_profile,		0, 0 },
{ CSP_PROFILE_PERSONAL,		cspr_profile,		0, 0 },
{ CSP_PROFILE_DYNAMIC,		cspr_profile,		0, 0 },
{ CSP_PROFILE_DYNAMIC_OTHER,	cspr_profile,		0, 0 },
{ CSP_PROFILE_RACE_STATS,	cspr_profile,		0, 0 },
{ CSP_PROFILE_PLANET,		cspr_profile,		0, 0 },
{ CSP_PROFILE_SECTOR,		cspr_profile,		0, 0 },
{ CSP_PROFILE_DISCOVERY,	cspr_profile,		0, 0 },
{ CSP_PROFILE_END,		cspr_profile,		0, 0 },
{ CSP_RELATION_INTRO,		cspr_relation,		0, 0 },
{ CSP_RELATION_DATA,		cspr_relation,		0, 0 },
{ CSP_RELATION_END,		cspr_relation,		0, 0 },
{ CSP_RESET_START,		cspr_event,		0, 0 },
{ CSP_RESET_END,		cspr_reset_end,		0, 0 },
{ CSP_SCOPE_PROMPT,		cspr_scope_prompt,	0, 0 },
{ CSP_SEGMENT_START,		cspr_event,		0, 0 },
{ CSP_SEGMENT_END,		cspr_segment_end,	0, 0 },
{ CSP_SHIPDUMP_GEN,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_STOCK,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_STATUS,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_WEAPONS,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_FACTORY,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_DEST,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_PTACT_GEN,	cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_PTACT_PDIST,	cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_STACT_PDIST,	cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_PTACT_INFO,	cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_STACT_INFO,	cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_ORDERS,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_THRESH,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_SPECIAL,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_HYPER,		cspr_fuse,		0, 0 },
{ CSP_SHIPDUMP_END,		cspr_fuse,		0, 0 },
{ CSP_SURVEY_INTRO,		cspr_survey,		0, 0 },
{ CSP_SURVEY_SECTOR,		cspr_survey,		0, 0 },
{ CSP_SURVEY_END,		cspr_survey,		0, 0 },
{ CSP_UNIVDUMP_INTRO,		cspr_psmap,		0, 0 },
{ CSP_UNIVDUMP_STAR,		cspr_psmap,		0, 0 },
{ CSP_UNIVDUMP_END,		cspr_psmap,		0, 0 },
{ CSP_UPDATE_START,		cspr_event,		0, 0 },
{ CSP_UPDATE_END,		cspr_update_end,	0, 0 },
{ CSP_UPDATES_RESUMED,		cspr_updates_resumed,	0, 0 },
{ CSP_UPDATES_SUSPENDED,	cspr_updates_suspended,	0, 0 },
{ CSP_VERSION_INFO,		cspr_print,		0, 0 },
{ CSP_VERSION_OPTIONS,		cspr_print,		0, 0 },
{ CSP_WHO_INTRO,		cspr_who,		0, 0 },
{ CSP_WHO_DATA,			cspr_who,		0, 0 },
{ CSP_WHO_COWARDS,		cspr_who,		0, 0 },
{ CSP_WHO_END,			cspr_who,		0, 0 },
{ CSP_ERR,			cspr_err,		0, 0 }

};

static CSPSendVal csp_send_table[] = {
/* Name		Number	 			Function            Cnt, Know */
{ "explore",	CSP_EXPLORE_COMMAND,		csps_null,		0, 0 },
{ "login",	CSP_LOGIN_COMMAND,		csps_login,		0, 0 },
{ "knowledge",	CSP_KNOWLEDGE,			csps_knowledge,		0, 0 },
{ "map",	CSP_MAP_COMMAND,		csps_null,		0, 0 },
{ "orbit",	CSP_ORBIT_COMMAND,		csps_null,		0, 0 },
{ "ping",	CSP_PING,			csps_null,		0, 0 },
{ "relation",	CSP_RELATION_COMMAND,		csps_relation,		0, 0 },
{ "scope",	CSP_SCOPE_COMMAND,		csps_null,		0, 0 },
{ "survey",	CSP_SURVEY_COMMAND,		csps_null,		0, 0 },
{ "version",	CSP_VERSION_COMMAND,		csps_null,		0, 0 },
{ "who",	CSP_WHO_COMMAND,		csps_null,		0, 0 }
};

#define NUM_RECEIVE_COMMANDS (sizeof(csp_receive_table) / sizeof(CSPReceiveVal))
#define NUM_SEND_COMMANDS (sizeof (csp_send_table) / sizeof (CSPSendVal))
