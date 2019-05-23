/*
 * csp.c: handles special server/client communication.
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 *
 * Copyright(c) 1993
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"

# include <stdio.h>
# include <sys/types.h>
# include <ctype.h>
# include <malloc.h>
# include <math.h>
# include <memory.h>
# include <string.h>
# include <stdarg.h>

# include "csp.h"
# include "csparr.h"
# include "csp_types.h"  /* needs to be before vars.h */
# include "option.h"
# include "str.h"
# include "term.h"
# include "ansi.h"
# include "vars.h"
# include "arrays.h"

# ifdef XMAP
# include "xmap.h"
# endif

# define INIT      0
# define NEXT      1

extern int notify;
extern int socket_return;

extern int atoi (const char *);
extern int icomm_valid_csp (int);
extern int icomm_valid_csp_end (int);
extern int sscanf (const char *, const char *, ...);
extern void icomm_command_done (void);
extern void process_socket (char *);
void csp_msg (char *, ...);
void csp_profile_output (Profile *prof);

static int csp_kill_output = FALSE;
static int csp_last_comm_num = 0;

process_special (char *s)
{
int comm_num;
char temp[10];
char line[MAXSIZ];
static CSPReceiveVal *handler = NULL;

  split (s, temp, line);
  comm_num = atoi (temp);

  if (wait_csp.lo && wait_csp.lo <= comm_num && wait_csp.hi >= comm_num) {
    strcpy (wait_csp.buf, line);
    wait_csp.have = TRUE;
    socket_return = TRUE;
    debug (2, "proc_spec, wait_csp found!");
    return;
  }

  if (!handler || (handler->comm_num != comm_num)) {
    handler = cspr_binary_search (comm_num);
  }

  if (handler == NULL) {
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Unknown CSP command #%d - %s", comm_num, line);
    else
      msg ("Unknown CSP(rec): %d %s", comm_num, line);
# endif
    return; /* (FALSE) */
  }

  csp_kill_output = FALSE;
debug (1, "checking icomm/csp");
  if (icomm_valid_csp (comm_num)) {
debug (1, "CSP and icomm say to kill output");
    csp_kill_output = TRUE;
  }
  handler->cnt++;
  handler->func (comm_num, line);

  if (icomm_valid_csp_end (comm_num)) {
debug (1, "CSP and icomm say to do done");
    icomm_command_done ();
  }

  return; /* (TRUE) */
}

int cspr_qsort_cmp (const void *a, const void *b)
{
  return (((const CSPReceiveVal *)a)->comm_num -
    ((const CSPReceiveVal *)b)->comm_num);
}

int csps_qsort_cmp (const void *a, const void *b)
{
  return (((const CSPSendVal *)a)->comm_num -
    ((const CSPSendVal *)b)->comm_num);
}

void init_csp (void)
{
extern void qsort (void *, size_t, size_t, int (*)(const void *, const void *));

  qsort (csp_send_table, NUM_SEND_COMMANDS,
    sizeof (CSPSendVal), csps_qsort_cmp);
  qsort (csp_receive_table, NUM_RECEIVE_COMMANDS,
    sizeof (CSPReceiveVal), cspr_qsort_cmp);
}

/* CSP Receive functions */

/* update/segment/reset/backup */
void cspr_backup_end (int cnum, char *line)
{
  csp_msg ("BACKUP DONE... %s", line);
}

void cspr_update_end (int cnum, char *line)
{

  if (notify > 0)
    term_beep (notify);
  init_start_commands (1);
  client_stats = L_REINIT;
  csp_msg ("UPDATE DONE... %s", line);
}

void cspr_segment_end (int cnum, char *line)
{

  if (notify > 0)
    term_beep (notify);
  init_start_commands (2);
  client_stats = L_REINIT;
  csp_msg ("SEGMENT DONE... %s", line);
}

void cspr_reset_end (int cnum, char *line)
{

  if (notify > 0)
    term_beep (notify);
  init_start_commands (0);
  client_stats = L_REINIT;
  csp_msg ("RESET DONE... %s", line);
}

static struct map {
  int geo;      /* remove soon */
  int inverse;      /* remove soon */
  int double_digits;    /* remove soon */
  int color; /* for maps with ansi -mfw */
  int snum;
  char sname[SMABUF];
  int pnum;
  char pname[SMABUF];
  double compat;
  int tox;
  int enslaved;
  int x;
  int y;

  /* dynamic1 */
  int type;
  int sects;
  int guns;
  int mobpts;
  int res;
  int des;
  int fuel;
  int xtals;

  /* dynamic2 */
  int mob;
  int actmob;
  int popn;
  int mpopn;
  int totpopn;
  int mil;
  int totmil;
  int tax;
  int acttax;
  int deposits;
  int estprod;

  char aliens[BUFSIZ];

} map;

void cspr_map (int cnum, char *line)
{
char nums[3];
char outbuf1[BUFSIZ];
char outbuf2[BUFSIZ];
char fmt[SMABUF];
int pad;
int i;
static int y = 0;
char *p;
char *q;
struct sect {
  int geo_type;
  int symbol;
  int owner;
} sect;
int inverse;    /* currently on or not */
int colored = 0;    /* currently on or not */

char *get_map_info_buf (int, struct map *);

  switch (cnum) {
  case CSP_MAP_INTRO:  /* 601 */
    sscanf (line, "%d %s %d %s %lf %d %d %d %d %d %d %d %d",
      &map.snum, map.sname,
      &map.pnum, map.pname,
      &map.compat, &map.tox, &map.enslaved,
      &map.x, &map.y,
      &map.geo, &map.inverse, &map.double_digits, &map.color);
    y = 0;

#ifdef XMAP
    if (xmap_active)
      xmap_plot_surface(line);
#endif

/*
    Moved to "print header" below

    (void) get_map_info_buf (INIT, &map);
    p = get_map_info_buf (NEXT, &map);
    sprintf (outbuf1, "   %s(%d)/%s(%d)",
      map.sname, map.snum,
      map.pname, map.pnum);
    csp_msg ("");
    csp_msg ("%-40s%s", outbuf1, p);
*/
    break;
  case CSP_MAP_DYNAMIC_1: /* 602 */
#ifdef XMAP
    if (xmap_active)
      break;
#endif
    sscanf (line, "%d %d %d %d %d %d %d %d",
      &map.type, &map.sects,
      &map.guns, &map.mobpts,
      &map.res, &map.des,
      &map.fuel, &map.xtals);

    /* print header */
    (void) get_map_info_buf (INIT, &map);
    p = get_map_info_buf (NEXT, &map);
    sprintf (outbuf1, "   %s(%d)/%s(%d)",
      map.sname, map.snum,
      map.pname, map.pnum);
    csp_msg ("");
    csp_msg ("%-40s%s", outbuf1, p);
    strcpy (outbuf1, "   ");
    for (i = 0; i < map.x; i++) {
      sprintf (outbuf2, "%d", i / 10);
      strcat (outbuf1, outbuf2);
    }
    csp_msg ("%-40s", outbuf1);

    strcpy (outbuf1, "   ");
    for (i = 0; i < map.x; i++) {
      sprintf (outbuf2, "%d", i % 10);
      strcat (outbuf1, outbuf2);
    }
    p = get_map_info_buf (NEXT, &map);
    csp_msg ("%-40s%s", outbuf1, p);

    if (GET_BIT (options, MAP_SPACE)) {
      p = get_map_info_buf (NEXT, &map);
      csp_msg ("%-40s%s", "", p);
    }
    break;
  case CSP_MAP_DYNAMIC_2: /* 603 */
#ifdef XMAP
    if (xmap_active)
      break;
#endif
    sscanf (line, "%d %d %d %d %d %d %d %d %d %d %d",
      &map.mob, &map.actmob,
      &map.popn, &map.mpopn,
      &map.totpopn, &map.mil,
      &map.totmil, &map.acttax,
      &map.tax, &map.deposits,
      &map.estprod);
    break;
  case CSP_MAP_ALIENS:  /* 604 */
#ifdef XMAP
    if (xmap_active)
      break;
#endif
    strcpy (map.aliens, line);
    if (!*map.aliens || *map.aliens == '0')
      strcpy (map.aliens, "none");
    break;
  case CSP_MAP_DATA:   /* 605 */
#ifdef XMAP
    if (xmap_active)
      break;
#endif
    inverse = FALSE;
    pad = 0;
    *outbuf1 = '\0';
    p = line;
    for (i = 0; i < map.x; i++) {
      q = strchr (p, ';');
      *q = '\0';
      sect.geo_type = *p++ - '0';
      sect.symbol = *p++;
      sect.owner = atoi (p);
      /* determine sector output */
      if (map.color)
      {
        if (sect.owner > 0)
        {
          colored = (sect.owner % MAX_RCOLORS);
          strcat(outbuf1, ANSI_TRUNCATE);
          strcat(outbuf1, race_colors[colored]);
          strcat(outbuf1, ANSI_FOR_BLACK);
          pad += 15;
        }

        sprintf(outbuf2, "%c", sect.symbol);
        strcat (outbuf1, outbuf2);

        if (colored)
        {
          strcat (outbuf1, ANSI_NORMAL);
          colored = 0;
          pad += 4;
        }
      }
      else if (map.geo)
      {
        if (map.inverse && sect.owner==profile.raceid)
        {
          if (!inverse) {
            strcat (outbuf1, "");
            inverse = TRUE;
            pad++;
          }
        }
        else if (inverse)
        {
          strcat (outbuf1, "");
          inverse = FALSE;
          pad++;
        }
        strcat (outbuf1, SectorTypes[sect.geo_type]);
      } else { /* !map.geo */
        if (sect.owner==profile.raceid) {
          if (map.inverse) {
            if (!inverse) {
              strcat (outbuf1, "");
              inverse = TRUE;
              pad++;
            }
          sprintf (outbuf2, "%c", sect.symbol);
          } else {
            if (!map.double_digits || sect.owner < 10) {
              sprintf (outbuf2, "%d",
                sect.owner%10);
            } else {
              sprintf (outbuf2, "%d",
(i % 2) ? sect.owner / 10 : sect.owner % 10);
            }
          }
        } else { /* not owned by us */
          if (inverse) {
            strcat (outbuf1, "");
            inverse = FALSE;
            pad++;
          }
          if (sect.owner == 0) {
            sprintf (outbuf2, "%c", sect.symbol);
          } else if (sect.symbol == SectorTypesChar[sect.geo_type]) {
/* sector type == symbol type (ie, geo) so display digits */
            if (!map.double_digits || sect.owner < 10) {
              sprintf (outbuf2, "%d",
                sect.owner%10);
            } else {
              sprintf (outbuf2, "%d",
(i % 2) ? sect.owner / 10 : sect.owner % 10);
            }
          } else { /* non civ - display symbol */
            sprintf (outbuf2, "%c",
              sect.symbol);
          }
        }
        strcat (outbuf1, outbuf2);
      } /* !map.geo */
      p = q + 1;
    } /* for loop */

    if (inverse) {
      strcat (outbuf1, "");
      inverse = FALSE;
    }

    sprintf (nums, "%d%d ", y/10, y%10);
    y++;
    sprintf (outbuf2, "%s%s%s%s",
      nums,
      outbuf1, (GET_BIT (options, MAP_DOUBLE) &&
        GET_BIT (options, MAP_SPACE) ? " " : ""),
      (GET_BIT (options, MAP_DOUBLE) ? nums : ""));
    p = get_map_info_buf (NEXT, &map);
    if (p) {
      sprintf (fmt, "%%-%ds%%s", 40 + pad);
      csp_msg (fmt, outbuf2, p);
    } else
      csp_msg ("%-40s", outbuf2);
    break;
  case CSP_MAP_END:
#ifdef XMAP
    if (xmap_active)
      break;
#endif
    if (GET_BIT (options, MAP_DOUBLE)) {
      if (GET_BIT (options, MAP_SPACE)) {
        p = get_map_info_buf (NEXT, &map);
        if (p)
          csp_msg ("%-40s%s", "", p);
        else
          csp_msg ("");
      }

      strcpy (outbuf1, "   ");
      for (i = 0; i < map.x; i++) {
        sprintf (outbuf2, "%d", i / 10);
        strcat (outbuf1, outbuf2);
      }
      p = get_map_info_buf (NEXT, &map);
      if (p)
        csp_msg ("%-40s%s", outbuf1, p);
      else
        csp_msg ("%-40s", outbuf1);

      strcpy (outbuf1, "   ");
      for (i = 0; i < map.x; i++) {
        sprintf (outbuf2, "%d", i % 10);
        strcat (outbuf1, outbuf2);
      }
      p = get_map_info_buf (NEXT, &map);
      if (p)
        csp_msg ("%-40s%s", outbuf1, p);
      else
        csp_msg ("%-40s", outbuf1);
    }

    while (p = get_map_info_buf (NEXT, &map)) {
      csp_msg ("%-40s%s", "", p);
    }
    break;
  default:
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Bad CSP# in map: %d - %s", cnum, line);
# endif  
    break;
  }
}

char *get_map_info_buf (int cnt, struct map *map)
{
static int pos = 0;
static char xtrabuf[40];
char enslavebuf[SMABUF];


  if (cnt == INIT) {
    pos = 0;
    return ((char *) NULL);    /* to hush compiler */
  }

  pos++;
  switch (pos) {
  case 1:
    sprintf (xtrabuf, " %s    Tox:%3d Compat:%3.0f",
      PlanetTypes[map->type], map->tox, map->compat);
    break;
  case 2:
    sprintf (xtrabuf, " R:%7d   Sects:%3d (%3d) x=%-2d",
      map->res, map->sects, map->x * map->y, map->xtals);
    break;
  case 3:
    sprintf (xtrabuf, " D:%7d    Guns:%5d (%5d)",
      map->des, map->guns, map->mobpts);
    break;
  case 4:
    sprintf (xtrabuf, " F:%7d     Mil:%5d (%5d)",
      map->fuel, map->mil, map->totmil);
    break;
  case 5:
    sprintf (xtrabuf, " Mob:%3d/%-3d   Dep:%6d(%5d)",
      map->actmob, map->mob, map->deposits, map->estprod);
    break;
  case 6:
    sprintf (xtrabuf, " Tax:%3d/%-3d", map->acttax, map->tax);
    break;
  case 7:
    sprintf (xtrabuf, " Pop:%7d/%-8d (%8d)",
      map->popn, map->mpopn, map->totpopn);
    break;
  case 8:
    if (map->enslaved) {
      sprintf (enslavebuf, "ENSLAVED(%d)", map->enslaved);
    } else {
      *enslavebuf = '\0';
    }
    sprintf (xtrabuf, " %sAliens: %s", enslavebuf, map->aliens);
    break;
  default:
    return ((char *) NULL);
    break;
  }

  return (xtrabuf);
}

void cspr_ping (int cnum, char *line)
{
  csp_send_request (CSP_PING, "");
# ifdef CLIENT_DEVEL
  if (client_devel)
    msg (":: Ping!!!!");
# endif
}

void cspr_pause (int cnum, char *line)
{
  char c;

  paused = TRUE;
  promptfor("-- paused --", &c, PROMPT_CHAR);
  paused = FALSE;
}  

void cspr_survey (int cnum, char *line)
{

# ifdef POPN
  if (doing_popn_command ()) {
    popn_input (cnum, line);
    return;
  }
# endif
# ifdef IMAP
  if (doing_imap_command ()) {
    imap_input (cnum, line);
    return;
  }
# endif
# ifdef XMAP
  if (doing_xmap_command ()) {
    xmap_input (cnum, line);
    return;
  }
# endif
}

void cspr_updates_suspended (int cnum, char *line)
{
  csp_msg ("== Updates/Segments have been suspended.");
  servinfo.updates_suspended = TRUE;
}

void cspr_updates_resumed (int cnum, char *line)
{
  csp_msg ("== Updates/Segments have resumed.");
  servinfo.updates_suspended = FALSE;
}

/* receiving a list of those commands that we can _send_ */
void cspr_knowledge (int cnum, char *line)
{
char *p;
char *q = line;
CSPSendVal *handler;
char known[MAXSIZ];
char unknown[MAXSIZ];
extern char *entry_quote;

  /* send entry quote now so data will be ready when we leave */
  if (entry_quote && *entry_quote) {
    send_gb (entry_quote, strlen (entry_quote));
  }

  *known = '\0';
  *unknown = '\0';
  while ((p = strchr (q, ' '))) {
    *p = '\0';
    handler = csps_binary_search (atoi (q));
    if (handler) {
      handler->know = TRUE;
      strcat (known, handler->name);
      strcat (known, " ");
    } else {
      strcat (unknown, q);
      strcat (unknown, " ");
    }
    q = p + 1;
  }
  handler = csps_binary_search (atoi (q));
  if (handler) {
    handler->know = TRUE;
    strcat (known, handler->name);
    strcat (known, " ");
  } else {
    strcat (unknown, q);
    strcat (unknown, " ");
  }
# ifdef CLIENT_DEVEL
  if (client_devel)
  {
    msg (":: Server knows the following CSP commands:");
    msg (":: %s", known);
    msg (":: Unknown: %s", unknown);
  }
# endif
}

void cspr_err (int cnum, char *line)
{
int err;

  err = atoi (line);
  if (err == CSP_ERR_UNKNOWN_COMMAND &&
     csp_last_comm_num == CSP_KNOWLEDGE) { /* old server */
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Old style server. Does not know CSP_KNOWLEDGE!");
# endif
    return;
  }
  msg ("CSP Error: %d %s", cnum, line);
}

/* CSP Send functions */

CSPSendVal *csps_binary_search (int cnum)
{
int bottom = 0;
int top = NUM_SEND_COMMANDS - 1;
int mid; 
int value;

  while (bottom <= top) {
    mid = bottom + ((top - bottom) / 2);
    value = cnum - csp_send_table[mid].comm_num;
    if (value == 0) 
      return (&csp_send_table[mid]);
    else if (value < 0) 
      top = mid - 1;
    else 
      bottom = mid + 1;
  }
  return ((CSPSendVal *) NULL);
}

CSPReceiveVal *cspr_binary_search (int cnum)
{
int bottom = 0;
int top = NUM_RECEIVE_COMMANDS - 1;
int mid; 
int value;

  while (bottom <= top) {
    mid = bottom + ((top - bottom) / 2);
    value = cnum - csp_receive_table[mid].comm_num;
    if (value == 0) 
      return (&csp_receive_table[mid]);
    else if (value < 0) 
      top = mid - 1;
    else 
      bottom = mid + 1;
  }
  return ((CSPReceiveVal *) NULL);
}

waitfor (char *buf, int lo, int hi)
{
  if (!csp_server_vers) {
    debug (2, "waitfor (csp off however): %s %d %d", buf, lo, hi);
    *buf = '\0';
    return;
  }

  wait_csp.lo = lo;
  wait_csp.hi = hi;
  gbs ();
  strcpy (buf, wait_csp.buf);
  wait_csp.lo = 0;
  wait_csp.have = FALSE;
  socket_return = FALSE;
}

int csp_send_request (int comm_num, char *buf)
{
CSPSendVal *handler;
char *p;
char str[MAXSIZ];

  /* if not active, then return with a 0 (fail) status */
  if (!csp_server_vers && comm_num != CSP_LOGIN_COMMAND) {
    debug (2, "send_csp (off however): %d %s", comm_num, buf);
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: send_csp (off however): %d %s", comm_num, buf);
# endif
    return (0);
  }
  
  handler = csps_binary_search (comm_num);

  if (handler == NULL) {
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Unknown CSP(send) %s %d %s",
        CSP_SERVER, comm_num, buf);
    else
      msg ("-- CSP(send) unknown: %s %d %s",
        CSP_SERVER, comm_num, buf);
# endif
    return (0); /* (FALSE) */
  }

  handler->cnt++;
  p = (char *) handler->func (buf);

  if (p)
    sprintf (str, "%s %d %s\n", CSP_SERVER, comm_num, p);
  else
    sprintf (str, "%s %d\n", CSP_SERVER, comm_num);
  send_gb (str, strlen (str));
  csp_last_comm_num = handler->comm_num;
  return (1); /* (TRUE) */
}

void cspr_profile (int cnum, char *line)
{
char *p;
char *q;
int i;
static Profile prof;

  switch (cnum) {
  case CSP_PROFILE_INTRO: /* 301 */
    strcat (line, "|");
    sscanf (line, "%d %d %[^|]|",
      &prof.raceid, &prof.player_type, prof.racename);
    break;
  case CSP_PROFILE_PERSONAL: /* 302 */
    strcpy (prof.personal, line);
    break;
  case CSP_PROFILE_DYNAMIC: /* 303 */
    sscanf (line, "%d %d %d %d %d %d %d %s",
      &prof.updates_active, &prof.know,
      &prof.capital, &prof.raceinfo.morale,
      &prof.ranges.guns, &prof.ranges.space,
      &prof.ranges.ground, prof.defscope);
    break;
  case CSP_PROFILE_DYNAMIC_OTHER: /* 304 */
    sscanf (line, "%d %d %d %d %d %s",
      &prof.know, &prof.raceinfo.morale,
      &prof.ranges.guns, &prof.ranges.space,
      &prof.ranges.ground, prof.sect_pref);
    break;
  case CSP_PROFILE_RACE_STATS: /* 305 */
    sscanf (line, "%d %d %lf %lf %d %lf %d %d %lf %lf",
      &prof.raceinfo.racetype, &prof.raceinfo.fert,
      &prof.raceinfo.birthrate, &prof.raceinfo.mass,
      &prof.raceinfo.fight, &prof.raceinfo.metab,
      &prof.raceinfo.sexes, &prof.raceinfo.explore,
      &prof.raceinfo.tech, &prof.raceinfo.iq);
    break;
  case CSP_PROFILE_PLANET: /* 306 */
    sscanf (line, "%d %d %d %d %d %d %d %d %d",
      &prof.planet.temp, &prof.planet.methane,
      &prof.planet.oxygen, &prof.planet.co2,
      &prof.planet.hydrogen, &prof.planet.nitrogen,
      &prof.planet.sulfur, &prof.planet.helium,
      &prof.planet.other);
    break;
  case CSP_PROFILE_SECTOR: /* 307 */
    sscanf (line, "%d %d %d %d %d %d %d %d",
      &prof.sector.ocean,
      &prof.sector.land,
      &prof.sector.mtn,
      &prof.sector.gas,
      &prof.sector.ice,
      &prof.sector.forest,
      &prof.sector.desert,
      &prof.sector.plated);
      break;
  case CSP_PROFILE_DISCOVERY: /* 308 */
    strcpy (prof.discovery, line);
    break;
  case CSP_PROFILE_END: /* 309 */
    csp_profile_output (&prof);
    /* update our race profile */
    if (profile.raceid == prof.raceid)
      profile = prof;
    break;
  default:
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Bad CSP# in profile: %d - %s", cnum, line);
# endif  
    break;
  }
}

void csp_profile_output (Profile *prof)
{
char buf[MAXSIZ];
char *p;
int i;

  csp_msg ("   Racial Profile for %s[%d]%s",
    prof->racename, prof->raceid,
    (prof->player_type == CSPD_GUEST ? "  **GUEST**" :
    (prof->player_type == CSPD_DIETY ? "  **DIETY**" :
    "")));
  csp_msg ("");
  csp_msg ("Personal: %s", prof->personal);

if (prof->raceid == profile.raceid || profile.player_type == CSPD_DIETY) {
  csp_msg ("Default Scope: %-20s Capital: #%-d",
    prof->defscope, prof->capital);
  csp_msg ("Morale: %-28d  Ranges:   guns   space   ground",
    prof->raceinfo.morale);
  csp_msg ("Updates Active: %-18d            %6d %7d %8d",
    prof->updates_active, prof->ranges.guns,
    prof->ranges.space, prof->ranges.ground);
  csp_msg ("");
csp_msg ("%s Race%s            Planet Conditions         Sector Preferences",
  RaceType[prof->raceinfo.racetype],
  RaceTypePad[prof->raceinfo.racetype]);
  csp_msg ("Fert: %6d%%           Temp: %9d",
    prof->raceinfo.fert, prof->planet.temp);
csp_msg ("Rate: %6.1lf            methane %6d%%           ocean    . %3d%%",
  prof->raceinfo.birthrate, prof->planet.methane, prof->sector.ocean);
csp_msg ("Mass: %6.2lf            oxygen  %6d%%           gaseous  ~ %3d%%",
  prof->raceinfo.mass, prof->planet.oxygen, prof->sector.gas);
csp_msg ("Fight: %5d            helium %7d%%           ice      # %3d%%",
  prof->raceinfo.fight, prof->planet.helium, prof->sector.ice);
csp_msg ("Metab: %5.2lf            nitrogen %5d%%           mountain ^ %3d%%",
  prof->raceinfo.metab, prof->planet.nitrogen, prof->sector.mtn);
csp_msg ("Sexes: %5d            CO2 %10d%%           land     * %3d%%",
  prof->raceinfo.sexes, prof->planet.co2, prof->sector.land);
csp_msg ("Explore: %3d%%           hydrogen %5d%%           desert   - %3d%%",
  prof->raceinfo.explore, prof->planet.hydrogen, prof->sector.desert);
csp_msg ("Avg Int:%4.0lf            sulfur %7d%%           forest   \" %3d%%",
  prof->raceinfo.iq, prof->planet.sulfur, prof->sector.forest);
csp_msg ("Tech: %6.2lf            other %8d%%           plated   o %3d%%",
  prof->raceinfo.tech, prof->planet.other, prof->sector.plated);

  i = 0;                                                          
  *buf = '\0';                                                    
  p = strtok (prof->discovery, " ");   
  while (p) {               
    if (atoi (p)) {                               
      strcat (buf, Discoveries[i]);  
    }                                     
    i++;                                       
    p = strtok (NULL, " ");                     
  }                                          
  csp_msg ("Discoveries: %s", buf);

} else {
csp_msg ("");
csp_msg ("%s Race%s            Planet Conditions",
  RaceType[prof->raceinfo.racetype],
  RaceTypePad[prof->raceinfo.racetype]);
  csp_msg ("Fert: %6d%%           Temp: %9d",
    prof->raceinfo.fert, prof->planet.temp);
csp_msg ("Rate: %6.1lf            methane %6d%%           Ranges:",
  prof->raceinfo.birthrate, prof->planet.methane, prof->sector.ocean);
csp_msg ("Mass: %6.2lf            oxygen  %6d%%             guns:%8d",
  prof->raceinfo.mass, prof->planet.oxygen, prof->ranges.guns);
csp_msg ("Fight: %5d            helium %7d%%             space:%7d",
  prof->raceinfo.fight, prof->planet.helium, prof->ranges.space);
csp_msg ("Metab: %5.2lf            nitrogen %5d%%             ground:%6d",
  prof->raceinfo.metab, prof->planet.nitrogen, prof->ranges.ground);
csp_msg ("Sexes: %5d            CO2 %10d%%",
  prof->raceinfo.sexes, prof->planet.co2);
csp_msg ("Explore: %3d%%           hydrogen %5d%%",
  prof->raceinfo.explore, prof->planet.hydrogen);
csp_msg ("Avg Int:%4.0lf            sulfur %7d%%",
  prof->raceinfo.iq, prof->planet.sulfur);
csp_msg ("Tech: %6.2lf            other %8d%%           Morale: %7d",
  prof->raceinfo.tech, prof->planet.other, prof->raceinfo.morale);

csp_msg ("Sector Type Preference: %s", prof->sect_pref);
} /* else */
} /* csp_profile_output */

void cspr_relation (int cnum, char *line)
{
char *p;
char buf[BUFSIZ];
char name[SMABUF];
int id;
int know;
int race_type;
int them_to_you;
int you_to_them;

  switch (cnum) {
  case CSP_RELATION_INTRO:
    p = strchr (line, ' ');  /* first word is race id */
    *p++ = '\0';      /* null term, and move to char after */
    id = atoi (line);
    /* just updating the name since we have it here */
    if (id == profile.raceid) {
      strcpy (profile.racename, p);
    } else {
      strcpy (races[id].name, p);
    }
    csp_msg ("");
    csp_msg ("Race Relations for %s[%d]", p, id);
    csp_msg ("");
    csp_msg ("%2s        %-7s   %-35s %-10s %-10s",
      " #", " know", "Race Name", "Yours", "Theirs");
    csp_msg ("%2s        %-7s   %-35s %-10s %-10s",
      "--", "------", "---------", "-------", "-------");
    break;
  case CSP_RELATION_DATA:
    strcat (line, "|");
    p = strchr (line, ' ');  /* first word is race id */
    *p++ = '\0';    /* null term, and move to char after */
    id = atoi (line);
    sscanf (p, "%d %d %d %d %[^|]|",
      &race_type, &know, &you_to_them, &them_to_you, name);
    cur_game.maxplayers = id;
    csp_msg ("%2d %-6s (%3d%%)    %-35s %-10s %-10s",
      id, RaceType[race_type], know, name,
      Relation[you_to_them], Relation[them_to_you]);
    break;
  case CSP_RELATION_END:
    break;
  default:
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Bad CSP# in relation: %d - %s", cnum, line);
# endif  
    break;
  }
}

void cspr_print (int cnum, char *line)
{
  msg ("%s", line);
}

void cspr_client_on (int cnum, char *line)
{
int rc;

  rc = sscanf (line, "%d %d %d",
    &profile.raceid, &profile.govid,
    &servinfo.updates_suspended);
  if (rc < 3) { 
    servinfo.updates_suspended = 0;
  }
# ifdef CLIENT_DEVEL
  if (client_devel)
  {
    msg (":: Server recognizes CSP.");
    msg (":: You are [%d,%d].", profile.raceid, profile.govid);
  }
# endif
  if (servinfo.updates_suspended)
    msg ("== Updates/Segments are currently suspended .");
  csp_server_vers = TRUE;
  csp_send_request (CSP_KNOWLEDGE, NULL);
  game_type = GAME_GBDT;
}

void cspr_client_off (int cnum, char *line)
{
  csp_server_vers = 0;
}

void cspr_event (int cnum, char *line)
{

  msg ("DOING %s... %s",
    (cnum == CSP_UPDATE_START ? "UPDATE" :
    (cnum == CSP_SEGMENT_START ? "SEGMENT" :
    (cnum == CSP_RESET_START ? "RESET" :
    (cnum == CSP_BACKUP_START? "BACKUP" : "UNKNOWN")))),
    line);
  end_prompt = DOING_PROMPT;
}

/* CSP_SCOPE_PROMPT */
void cspr_scope_prompt (int cnum, char *line)
{
char buf[SMABUF];
char rest[MAXSIZ];
int i;
char *p;
Ship *mptr;
Ship *tptr;

  scope.starnum = -1;
  *scope.star = '\0';
  scope.planetnum = -1;
  *scope.planet = '\0';
  scope.ship = 0;

  strcpy (rest, line);
  p = strtok (rest, " ");
  scope.level = atoi (p);

  p = strtok (NULL, " ");
  scope.numships = atoi (p);

  p = strtok (NULL, " ");
  scope.aps = atoi (p);

  if (scope.level == CSPD_STAR || scope.level == CSPD_PLAN) {
    p = strtok (NULL, " ");
    scope.starnum = atoi (p);
    strcpy (scope.star, strtok (NULL, " "));
  }

  if (scope.level == CSPD_PLAN) {
    p = strtok (NULL, " ");
    scope.planetnum = atoi (p);
    p = strtok (NULL, " ");
    strcpy (scope.planet, p);
  }

  if (scope.numships) {
    /* we have at least 1 ship */
    p = strtok (NULL, " ");
    scope.ship = atoi (p);

    /* now to get mother ships in reverse order
     * and push them on the list, overwriting old
     * data and malloc'ing new nodes as needed */
    for (i = 1; i < scope.numships; i++) {
      if (i == 1) {
        if (!scope.motherlist) {
          scope.motherlist = (Ship *)
            malloc (sizeof (Ship));
          scope.motherlist->next = NULL;
          scope.motherlist->prev = NULL;
        }
        mptr = scope.motherlist;
      } else {
        if (!mptr->next) {
          mptr->next = (Ship *)
            malloc (sizeof (Ship));
          mptr->next->next = NULL;
          mptr->next->prev = mptr;
        }
        mptr = mptr->next;
      }
    }
    for (i = 1; i < scope.numships;
         mptr = mptr->prev, i++) {
      p = strtok (NULL, " ");
      if (!p)  /* being cautious here */
        break;
      mptr->shipno = atoi (p);
    }
  }
  end_prompt = LEVEL_PROMPT;
  add_assign ("scope", build_scope ());
  sprintf (buf, "%d", scope.aps);
  add_assign ("aps", buf);
  sprintf (buf, "%d", scope.level);
  add_assign ("scope_level", buf);
  add_assign ("star", scope.star);
  add_assign ("planet", scope.planet);
  sprintf (buf, "%d", scope.ship);
  add_assign ("ship", buf);
  if (scope.motherlist) {
    sprintf (buf, "%d", scope.motherlist->shipno);
    add_assign ("mothership", buf);
  }

  if (on_endprompt (end_prompt))
    return;

  /* else display the scope prompt */
  msg (" %s", build_scope_prompt ());
}

void cspr_explore (int cnum, char *line)
{
char name[SMABUF];
char nbuf[SMABUF];
static char aliens[MAXSIZ];
int compat;
int aps;
int autorep;
int enslaved;
int explored;
int id;
int nsects;
int stab;
int tox;
int totsects;
int deposits;
int xtal_sects;
int type;

  switch (cnum) {
  case CSP_EXPLORE_INTRO: /* 501 */
    aps = atoi (line);
csp_msg ("         ========== Exploration Report ==========");
csp_msg ("Global APs: [%d]", aps);
csp_msg ("#    Star[AP](stability)");
csp_msg ("     #  Planet             PlanType Deposit(Compat) [ Explored? Inhab(#sect) Race#]");
    break;
  case CSP_EXPLORE_STAR: /* 502 */
    sscanf (line, "%d %s %d %d",
      &id, name, &stab, &aps);
    csp_msg ("");
    csp_msg ("#%-3d %s[%3d](%d)", id, name, aps, stab);
    break;
  case CSP_EXPLORE_STAR_ALIENS: /* 503 */
    if (*line) {
      strcpy (aliens, line);
    } else {
      *aliens = '\0';
    }
    break;
  case CSP_EXPLORE_STAR_DATA: /* 504 */
    sscanf (line, "%d %s %d %d %d %d %d %d %d %d %d %d",
      &id, &name, &explored, &nsects, &autorep,
      &enslaved, &tox, &compat, &totsects, &deposits,
      &type);
    if (explored) {
      sprintf (nbuf, "Inhab(%d) ", nsects);
      csp_msg ("    %2d) %-18s %-9s %6d (%3d%%) [ Ex %s%s ]",
        id, name, PlanetTypes[type], deposits,
        compat, (nsects ? nbuf : ""), aliens);
    } else {
      csp_msg ("    %2d) %-18s ????????? ?????? (???%%) [ No Data ]", id, name);
    }
    break;
  case CSP_EXPLORE_END:
    break;
  default:
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Bad CSP# in explore: %d - %s", cnum, line);
# endif  
    break;
  }
}

void cspr_who (int cnum, char *line)
{
char rname[SMABUF];
char gname[SMABUF];
int rid;
int gid;
int idle;
int gag;
int invis;
int rc;
char scope[SMABUF];

  switch (cnum) {
   case CSP_WHO_INTRO:
    csp_msg ("Current Players: %s", line);
    break;
   case CSP_WHO_DATA:
    *scope = '\0';
    rc = sscanf (line, "%[^\"] \"%[^\"]\" %d %d %d %d %d %s",
      rname, gname, &rid, &gid, &idle, &gag, &invis, scope);
    if (rc == 8) {
      scope[MAX_SCOPE_LTRS] = '\0';
    }
    csp_msg ("        %s \"%s\" [%2d,%2d] %d seconds idle %4s%s%s",
      remove_space_at_end (rname), gname, rid, gid, idle, 
      (*scope ? scope : ""),
      (gag ? " GAG" : ""),
      (invis ? " INVISIBLE" : ""));
    break;
   case CSP_WHO_COWARDS:
    idle = atoi (line);
    if (idle != -1) {
      csp_msg ("And %d coward%s.",
        idle, ((idle == 0  || idle > 1) ? "s" : ""));
    }
    break;
  default:
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Bad CSP# in who: %d - %s", cnum, line);
# endif  
    break;
  }
}

/* CSP Send functions */
char *csps_relation (char *s)
{
  return (s);
}

char *csps_knowledge (char *s)
{
static char mbuf[MAXSIZ];
char num[10];
int i;

  *mbuf = '\0';
  /* we are sending the list of those commands we can _receive_ */
  for (i = 0; i < NUM_RECEIVE_COMMANDS; i++) {
    sprintf (num, "%d ", csp_receive_table[i].comm_num);
    strcat (mbuf, num);
  }
# ifdef CLIENT_DEVEL
  if (client_devel)
    msg (":: CSP List: %s", mbuf);
# endif
  return (mbuf);
}

char *csps_login (char *s)
{
  return ("0");
}

char *csps_map (char *s)
{
  return ((char *) NULL);
}

char *csps_ping (char *s)
{
  return ((char *) NULL);
}

char *csps_survey (char *s)
{
  return ((char *) NULL);
}

char *csps_null (char *s)
{
static char buf[BUFSIZ];
  if (s == NULL)
    return ((char *) NULL);
  strcpy (buf, s);
  return (buf);
}

void csp_msg (char *fmt, ...)
{
va_list vargs;
char buf[MAXSIZ];

  if (csp_kill_output)
    return;

  va_start (vargs, fmt);
  (void) vsprintf (buf, fmt, vargs);
  process_socket (buf);
  va_end (vargs);
}

struct orbstar {
  int snum;
  double x;
  double y;
  int explored;
  int inhabited;
  int numplan;
  int stability;
  int novastage;
  int tempclass;
  double gravity;
  char name[SMABUF];
} obstar;

struct orbplanet {
  int snum;
  int pnum;
  double x;
  double y;
  int explored;
  int type;
  double compat;
  int owned;
  char name[SMABUF];
} orbplanet;

struct orbship {
  int num;
  int owner;
  char type;
  double x;
  double y;
  double xt;
  double yt;
  int array;
} orbship;

struct orbit {
  int scope;
  int univsize;
  int syssize;
  int plorbsize;
  int scnt;
  int pcnt;
  int bcnt;
  struct orbstar star[100];
  struct orbplanet planet[15];
  struct orbship ship[100];
  int inverse;
  int color;
  int scale;
  float lastx;
  float lasty;
  float zoom;
  int nostars;
  int noplanets;
  int noships;
  int stars;
  int gltype[SMABUF];
  int glname[SMABUF];
} orbit;
  
void cspr_orbit(int cnum, char *line)
{
  int x, y, i, j, colored, want_color;
  double dx, dy, dt;
  char colbuf[SMABUF];
  char ptypes[] = "@oO#~.\"-0(";

  switch (cnum)
  {
    case CSP_ORBIT_OUTPUT_INTRO:

      sscanf(line, "%d %d %d %d %d %f %f %f %d %d %d %d %d %d %s %s",
        &orbit.scope, &orbit.univsize, &orbit.syssize, &orbit.plorbsize,
        &orbit.scale, &orbit.lastx, &orbit.lasty, &orbit.zoom,
        &orbit.nostars, &orbit.noplanets, &orbit.noships,
         &orbit.inverse, &orbit.color,
        &orbit.stars, orbit.gltype, orbit.glname);

      orbit.scnt = orbit.pcnt = orbit.bcnt = 0;

      break;

    case CSP_ORBIT_STAR_DATA:

      sscanf(line, "%d %lf %lf %d %d %d %d %d %d %lf %s",
        &orbit.star[orbit.scnt].snum,
        &orbit.star[orbit.scnt].x,
        &orbit.star[orbit.scnt].y,
        &orbit.star[orbit.scnt].explored,
        &orbit.star[orbit.scnt].inhabited,
        &orbit.star[orbit.scnt].numplan,
        &orbit.star[orbit.scnt].stability,
        &orbit.star[orbit.scnt].novastage,
        &orbit.star[orbit.scnt].tempclass,
        &orbit.star[orbit.scnt].gravity,
        orbit.star[orbit.scnt].name);

      orbit.scnt++;

      break;

    case CSP_ORBIT_UNEXP_PL_DATA:

      sscanf(line, "%d %d %lf %lf %s",
        &orbit.planet[orbit.pcnt].snum,
        &orbit.planet[orbit.pcnt].pnum,
        &orbit.planet[orbit.pcnt].x,
        &orbit.planet[orbit.pcnt].y,
        orbit.planet[orbit.pcnt].name);

      orbit.planet[orbit.pcnt].explored = 0;

      orbit.pcnt++;

      break;

    case CSP_ORBIT_EXP_PL_DATA:

      sscanf(line, "%d %d %lf %lf %d %lf %d %s",
        &orbit.planet[orbit.pcnt].snum,
        &orbit.planet[orbit.pcnt].pnum,
        &orbit.planet[orbit.pcnt].x,
        &orbit.planet[orbit.pcnt].y,
        &orbit.planet[orbit.pcnt].type,
        &orbit.planet[orbit.pcnt].compat,
        &orbit.planet[orbit.pcnt].owned,
        orbit.planet[orbit.pcnt].name);

      orbit.planet[orbit.pcnt].explored = 1;

      orbit.pcnt++;

      break;

    case CSP_ORBIT_SHIP_DATA:

      sscanf(line, "%d %d %c %lf %lf %lf %lf",
        &orbit.ship[orbit.bcnt].num,
        &orbit.ship[orbit.bcnt].owner,
        &orbit.ship[orbit.bcnt].type,
        &orbit.ship[orbit.bcnt].x,
        &orbit.ship[orbit.bcnt].y,
        &orbit.ship[orbit.bcnt].xt,
        &orbit.ship[orbit.bcnt].yt);

      orbit.bcnt++;

      break;

    case CSP_ORBIT_OUTPUT_END:

      term_clear_screen();

      // STARS
      for (i = 0; i < orbit.scnt; i++)
      {
				if (orbit.nostars)
					break;

        if (orbit.scope == LEVEL_UNIV)
        {
          x = (int)(orbit.scale + ((orbit.scale *
              (orbit.star[i].x - orbit.lastx)) /
              (orbit.univsize * orbit.zoom)));
          y = (int)(orbit.scale + ((orbit.scale *
              (orbit.star[i].y - orbit.lasty)) /
              (orbit.univsize * orbit.zoom)));
        }
        else if (orbit.scope == LEVEL_STAR)
        {
          x = (int)(orbit.scale + ((orbit.scale *
              (-orbit.lastx)) /
              (orbit.syssize * orbit.zoom)));
          y = (int)(orbit.scale + ((orbit.scale *
              (-orbit.lasty)) /
              (orbit.syssize * orbit.zoom)));
        }

        x *= Midx;
        y *= Midy;

        if (x >= 0 && y >= 1 && x <= S_X && y <= S_Y)
        {
#ifdef ARRAY
          if (orbit.star[i].novastage > 0 && orbit.star[i].novastage <= 16 &&
            orbit.scope == LEVEL_STAR)
          {
            /* nova */
            DispArray (x, y, 11, 7, Novae[orbit.star[i].novastage - 1], 1.0);
          }
#endif
          term_move_cursor(x, y);

          if (orbit.star[i].explored && orbit.inverse)
          {
            term_standout_on();
            term_putchar('*');
            term_standout_off();
          }
          else
          {
            term_putchar('*');
          }

          term_putchar(' ');
        
          sprintf(colbuf, "%s", orbit.star[i].name);

          if (orbit.star[i].inhabited && orbit.inverse)
          {
            term_standout_on();
            term_puts(colbuf, strlen(colbuf));
            term_standout_off();
          }
          else
          {
            term_puts(colbuf, strlen(colbuf));
          }
        }
      }

      // PLANETS
      for (i = 0; i < orbit.pcnt; i++)
      {
				if (orbit.noplanets)
					break;

        if (orbit.scope == LEVEL_STAR)
        {
          x = (int)(orbit.scale + ((orbit.scale *
              (orbit.planet[i].x - orbit.lastx)) /
              (orbit.syssize * orbit.zoom)));
          y = (int)(orbit.scale + ((orbit.scale *
              (orbit.planet[i].y - orbit.lasty)) /
              (orbit.syssize * orbit.zoom)));
        }
        else if (orbit.scope == LEVEL_PLANET)
        {
          x = (int)(orbit.scale + ((orbit.scale *
              (-orbit.lastx)) /
              (orbit.plorbsize * orbit.zoom)));
          y = (int)(orbit.scale + ((orbit.scale *
              (-orbit.lasty)) /
              (orbit.plorbsize * orbit.zoom)));
        }

        x *= Midx;
        y *= Midy;

        if (x >= 0 && y >= 1 && x <= S_X && y <= S_Y)
        {
          term_move_cursor(x, y);

          if (orbit.planet[i].explored && orbit.inverse)
          {
            term_standout_on();
            term_putchar(ptypes[orbit.planet[i].type]);
            term_standout_off();
          }
          else
          {
            term_putchar('?');
          }

          term_putchar(' ');
        
          sprintf(colbuf, "%s", orbit.planet[i].name);

          if (orbit.planet[i].owned && orbit.inverse)
          {
            term_standout_on();
            term_puts(colbuf, strlen(colbuf));
            term_standout_off();
          }
          else
          {
            term_puts(colbuf, strlen(colbuf));
          }
        }
      }

      // SHIPS
      for (i = 0; i < orbit.bcnt; i++)
      {
				if (orbit.noships)
					break;

        if (orbit.scope == LEVEL_UNIV)
        {
          x = (int)(orbit.scale + ((orbit.scale *
              (orbit.ship[i].x - orbit.lastx)) /
              (orbit.univsize * orbit.zoom)));
          y = (int)(orbit.scale + ((orbit.scale *
              (orbit.ship[i].y - orbit.lasty)) /
              (orbit.univsize * orbit.zoom)));
        }
        else if (orbit.scope == LEVEL_STAR)
        {
          x = (int)(orbit.scale + ((orbit.scale *
              (orbit.ship[i].x - orbit.star[0].x - orbit.lastx)) /
              (orbit.syssize * orbit.zoom)));
          y = (int)(orbit.scale + ((orbit.scale *
              (orbit.ship[i].y - orbit.star[0].y - orbit.lasty)) /
              (orbit.syssize * orbit.zoom)));
        }
        else if (orbit.scope == LEVEL_STAR)
        {
          // DEBUG: We're not actually receiving the star data, so this is
          // going to come our wrong; need to fix. -mfw
          x = (int)(orbit.scale + ((orbit.scale *
              (orbit.ship[i].x - (orbit.star[0].x + orbit.planet[0].x) -
               orbit.lastx)) / (orbit.syssize * orbit.zoom)));
          y = (int)(orbit.scale + ((orbit.scale *
              (orbit.ship[i].y - (orbit.star[0].y + orbit.planet[0].y) -
               orbit.lasty)) / (orbit.syssize * orbit.zoom)));
        }

        x *= Midx;
        y *= Midy;

        if (x >= 0 && y >= 1 && x <= S_X && y <= S_Y)
        {
#ifdef ARRAY
          // The array variable is not sent from the server, so this
          // won't work. -mfw
          if (orbit.ship[i].type == 'M' && orbit.ship[i].array <= 8 &&
            orbit.ship[i].array > 0 && orbit.scope == LEVEL_PLANET)
          {
            /* mirror */
            DispArray (x, y, 9, 5, Mirror[orbit.ship[i].array - 1], 1.0);
          }
#endif
          term_move_cursor(x, y);

          if (orbit.color && want_color)
          {
            colored = (orbit.ship[i].owner % MAX_RCOLORS);
            sprintf(colbuf, "%s%s%c%s", race_colors[colored],
              ANSI_FOR_BLACK, orbit.ship[i].type, ANSI_NORMAL);
            term_puts(colbuf, strlen(colbuf));
          }
          else if (orbit.ship[i].owner == profile.raceid && orbit.inverse)
          {
            term_standout_on();
            term_putchar(orbit.ship[i].type);
            term_standout_off();
          }
          else
          {
            term_putchar(orbit.ship[i].type);
          }

          term_putchar(' ');
        
          sprintf(colbuf, "#%d", orbit.ship[i].num);

          term_puts(colbuf, strlen(colbuf));
        }
      }

      // Print survey info in a nice little box
      if (orbit.scope == LEVEL_UNIV)
      {
				term_move_cursor(60, 1);
				sprintf(colbuf, "| Galaxy: %.10s", orbit.glname);
        term_puts(colbuf, strlen(colbuf));

				term_move_cursor(60, 2);
				sprintf(colbuf, "| Type: %.10s", orbit.gltype);
        term_puts(colbuf, strlen(colbuf));

				term_move_cursor(60, 3);
				sprintf(colbuf, "| Size: %d", orbit.univsize);
        term_puts(colbuf, strlen(colbuf));

				term_move_cursor(60, 4);
				sprintf(colbuf, "| Stars: %d", orbit.stars);
        term_puts(colbuf, strlen(colbuf));

				term_move_cursor(60, 5);
				sprintf(colbuf, "X-------------------");
        term_puts(colbuf, strlen(colbuf));
      }
      else if (orbit.scope == LEVEL_STAR)
      {
				term_move_cursor(60, 1);
				sprintf(colbuf, "| Star: %.10s", orbit.star[0].name);
        term_puts(colbuf, strlen(colbuf));

				/* Only ever shows 0,0 -mfw
				term_move_cursor(60, X);
				sprintf(colbuf, "| Locn: %.0f,%.0f", orbit.star[0].x, orbit.star[0].y);
        term_puts(colbuf, strlen(colbuf));
				*/

				term_move_cursor(60, 2);
				sprintf(colbuf, "| Grav: %.2lf", orbit.star[0].gravity);
        term_puts(colbuf, strlen(colbuf));

				if (orbit.star[0].novastage)
				{
				  term_move_cursor(60, 3);
				  sprintf(colbuf, "| Nova: %d (1-16)", orbit.star[0].novastage);
          term_puts(colbuf, strlen(colbuf));
				}
				else
				{
				  term_move_cursor(60, 3);
				  sprintf(colbuf, "| Stab: %d%%", (100 - orbit.star[0].stability));
          term_puts(colbuf, strlen(colbuf));
				}

				term_move_cursor(60, 4);
				sprintf(colbuf, "| Temp: %d (1-10)", orbit.star[0].tempclass);
        term_puts(colbuf, strlen(colbuf));

				term_move_cursor(60, 5);
				sprintf(colbuf, "| %d Planets:", orbit.star[0].numplan);
        term_puts(colbuf, strlen(colbuf));

				for (j = 0; j < orbit.star[0].numplan; j++)
				{
					dx = pow((orbit.planet[j].x - orbit.star[0].x), 2);
					dy = pow((orbit.planet[j].y - orbit.star[0].y), 2);
					dt = sqrt((dx + dy));

				  term_move_cursor(60, (6 + j));
				  sprintf(colbuf, "| %5.0f %s", dt, orbit.planet[j].name);
          term_puts(colbuf, strlen(colbuf));
				}

				term_move_cursor(60, (6 + j));
				sprintf(colbuf, "X-------------------");
        term_puts(colbuf, strlen(colbuf));
      }
      else if (orbit.scope == LEVEL_PLANET)
      {
				term_move_cursor(60, 1);
				sprintf(colbuf, "| Planet: %.10s", orbit.planet[0].name);
        term_puts(colbuf, strlen(colbuf));

				term_move_cursor(60, 2);
				sprintf(colbuf, "| Compat: %.2f%%", orbit.planet[0].compat);
        term_puts(colbuf, strlen(colbuf));

				term_move_cursor(60, 3);
				sprintf(colbuf, "X-------------------");
        term_puts(colbuf, strlen(colbuf));
      }

      break;

    default:
# ifdef CLIENT_DEVEL
    if (client_devel)
      msg (":: Bad CSP# in orbit: %d - %s", cnum, line);
# endif  
      break;
  }
}

