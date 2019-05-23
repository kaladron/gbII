/*
 * socket.c: handles the socket code including reading from socket.
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"
# undef TRUE
# undef FALSE

# include <stdlib.h>
# include <stddef.h>

# include <fcntl.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <sys/errno.h>
# include <netdb.h>
# include <curses.h>
# include <stdio.h>
# include <ctype.h>
# include <memory.h>
# include <signal.h>
# include <string.h>
# include <time.h>

# include "csp.h"
# include "option.h"
# include "str.h"
# include "term.h"
# include "vars.h"

#ifdef XMAP
# include "xmap.h"
#endif

# ifndef TRUE
# define TRUE   1
# endif

# ifndef FALSE
# define FALSE   0
# endif

# define ENDHELP    "Help on that subject unavailable"
# define ENDDOINGSEGMENT  "DOING MOVEMENT..."
# define ENDDOINGUPDATE    "DOING UPDATE..."
# define ENDLOGIN    "Please enter your password"
# define ENDFINISH    "Finished."
# define GBMFINISH    "Segment finished"
# define GBUFINISH    "Update %d finished"
# define HAPMFINISH    "Movement finished."
# define HAPUFINISH    "Update finished."
# define ENDREFUSED    "Connection refused."

# define RACEGEN    "Galactic Bloodshed Race Generator"


# define NO_LOGOUT_DELAY  (600)    /* 10 minutes */

extern int gb;
extern char pbuf[];
extern int cursor_display;
extern int do_queue;      /* for queue */
extern int exit_now;
extern int kill_client_output;
extern int pipe_running;
extern int queue_sending;

BufferInfo gbsobuf;      /* socket buffer list ptrs */

char builtship[6] = "0";    /* contains # of last ship built */
char lotnum[6] = "0";      /* last lot # you put up for sale */

int command_has_output = FALSE;
int end_msg = 0;
int hide_msg = FALSE;      /* turn off kill_socket_output */
int kill_socket_output = FALSE;
int notify = -2;
int password_invalid = FALSE;    /* password prompt flag */
int server_help = FALSE;    /* doing a server help */

long last_no_logout_time;    /* for no_logout */

char last_prompt[BUFSIZ];    /* holds last prompt found */

extern char *check_crypt ();
extern char *match_action ();
extern int match_gag ();
char *build_scope (void);
void (*check4_endprompt) ();
void connect_prompts (char *s);
void null_func (void);
void oldcheck4_endprompts ();

extern int close (int);
extern int dup (int);
/* extern int send (int, const void *, int, unsigned int); */
extern long unsigned int inet_addr (/* ??? */);

void check_for_special_formatting (char *s, int type);
int close_gb (void);
int have_socket_output (void);
void loggedin (void);
int on_endprompt (int eprompt);
void parse_socket_output (char *s);
void process_gb (char *s);
void send_gb (char *s, int len);
int sendgb (char *buf, int len);
void set_no_logout (void);
void process_socket (char *s);
void socket_final_process (char *s, int type);
void chap_response (char *line);
void chap_abort (void);

/*
 * opens the socket and connects to specified host and port
 */
int connectgb (char *gbhost, char *charport, char *s)
{
struct sockaddr_in sin;
struct hostent *hp;
int gbport;
int fd;

  gbport = atoi (charport);

  memset ((char *) &sin, 0, sizeof (sin));
  
    sin.sin_port = htons (gbport);

    if (isdigit(*gbhost)) {
    sin.sin_addr.s_addr = inet_addr (gbhost);
    if (sin.sin_addr.s_addr == (unsigned long) -1) {
      msg ("-- %s Unknown IP host.", s);
      return -1;
    }
        sin.sin_family = AF_INET;
    } else {
        if ((hp = gethostbyname (gbhost)) == 0) {
      msg ("-- %s Unknown host(gethost) '%s'", s, gbhost);
      return (-1);
    }
    memcpy ((char *) &sin.sin_addr, hp->h_addr, hp->h_length);
        sin.sin_family = hp->h_addrtype;
    }

    fd = socket (AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
    msg_error ("-- %s", s);
    return (-1);
    }

    if (connect (fd, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
    msg_error ("-- %s", s);
    close (fd);
    return (-1);
    }
    return (fd);
}

/*
 * Returns true if socket is active
 */
int is_connected (void)
{
  if (gb >= 0)
    return (TRUE);
  return (FALSE);
}

/*
 * Handles the reading from the buffer and the processing of socket output
 */
void
get_socket (void)
{
  char *gbbuf;
  char *remove_buffer ();

  gbbuf = remove_buffer (&gbsobuf);

  if (gbsobuf.is_partial)
  {
    if (racegen)
    {
    /* fancy prompt stuff */
      update_input_prompt (gbbuf);
      refresh_input ();
      strfree (gbbuf);
      gbsobuf.is_partial = FALSE;
      return;
    }
    gbsobuf.is_partial = FALSE;
  }
  process_gb (gbbuf);
  strfree (gbbuf);
}

/*
 * Take the given string and process it as socket output
 */
void
process_gb (char *s)
{
  char *p;

  if (GET_BIT (options, RAWMODE))
  {
    display_msg (s); /* no more needed here */
    return;
  }

  end_prompt = NOT_PROMPT;
  check4_endprompt (s);  /* dynamic call here */

  if (on_endprompt (end_prompt))
    return;

  switch (*s)
  {
    case '#': 
# ifndef XMAP
      plot_orbit (s);
# else
      if (xmap_active)
      {
        xmap_plot_orbit (s);
      }
      else
      {
        plot_orbit (s);
      }
# endif
      break;

    case '$':
# ifndef XMAP
      plot_surface (s);
# else
      if (xmap_active)
      {
        xmap_plot_surface (s);
      }
      else
      {
        plot_surface (s);
      }
# endif
      break;

    case CSP_CLIENT:
      process_special (s+2);

      if (wait_csp.have)
        return;

      break;

# ifdef TEST
    case 255:
      msg ("TELNET - %s", s+1);
      break;
# endif

    default:
      /* so CSP can catch end prompts this has to be here */
      if (ICOMM_DOING_COMMAND)
      {
        icomm_handling_command (s);

        if (ICOMM_IGNORE)
          return;
      }

      process_socket (s);
      break;
  }
}

void process_socket (char *s)
{
int type = NONE;    /* to signal for announce/broadcast */
char *ptr;
char *p;
char racename[BUFSIZ];
char govname[BUFSIZ];
char raceid[BUFSIZ];
char govid[BUFSIZ];
char actualmsg[MAXSIZ];

  if (match_gag (s))
    return;

  if (kill_socket_output) {
    if (logfile.redirect && logfile.on)
      fprintf (logfile.fd, "%s\n", s);
    if (pipe_running)
      send_process (s);
    return;
  }
  if (end_prompt == PASS_PROMPT) {
    client_stats = L_PASSWORD;
    if (GET_BIT (options, AUTOLOGIN))
    {
      if (!password_invalid)
        send_password ();
    }
    return;
  } else {
    if (client_stats < L_PASSWORD &&
        GET_BIT (options, LOGINSUPPRESS)) {
      return;
    }
  }

  if (server_help) {
    check_for_special_formatting (s, FORMAT_HELP);
  }

  parse_socket_output (s);      /* for simple matches */
  
  if ((MATCH (s, "* \"*\" [*,*] *> *") && (type = GB_BROADCAST))) {
    msg_type = MSG_COMMUNICATION;
    strcpy (racename, pattern1);
    strcpy (govname, pattern2);
    strcpy (raceid, pattern3);
    strcpy (govid, pattern4);
    strcpy (actualmsg, pattern6);
  } else
  if ((MATCH (s, "* \"*\" [*,*] > *") && (type = GB_BROADCAST)) ||
      (MATCH (s, "* \"*\" [*,*] = *") && (type = GB_THINK)) ||
      (MATCH (s, "* \"*\" [*,*] : *") && (type = GB_ANNOUNCE)) ||
      (MATCH (s, "* [*,*] * *") && (type = GB_EMOTE))) {
    msg_type = MSG_COMMUNICATION;
    strcpy (racename, pattern1);
    strcpy (govname, pattern2);
    strcpy (raceid, pattern3);
    strcpy (govid, pattern4);
    strcpy (actualmsg, pattern5);
  } else if (MATCH (s, "(* [*.*]) *")) {
    type = HAP_THINK;
    msg_type = MSG_COMMUNICATION;
    strcpy (racename, profile.racename);
    strcpy (govname, pattern1);
    strcpy (raceid, pattern2);
    strcpy (govid,pattern3);
    strcpy (actualmsg, pattern4);
  } else if ((MATCH (s, "*:* [*.*] > *") && (type = HAP_BROADCAST)) ||
      (MATCH (s, "*:* [*.*] : *") && (type = HAP_ANNOUNCE))) {
    msg_type = MSG_COMMUNICATION;
    strcpy (racename, pattern1);
    strcpy (govname, pattern2);
    strcpy (raceid, pattern3);
    strcpy (govid, pattern4);
    strcpy (actualmsg, pattern5);
  } else if ((MATCH (s, "* [*] > *") && (type = NORM_BROADCAST)) ||
    (MATCH (s, "* [*] : *") && (type = NORM_ANNOUNCE))) {
    msg_type = MSG_COMMUNICATION;
    strcpy (racename, pattern1);
    *govname = '\0';
    strcpy (raceid, pattern2);
    *govid = '\0';
    strcpy (actualmsg, pattern3);
  }
  check_for_special_formatting (s, FORMAT_NORMAL);
  if (type != NONE) {
    check_for_special_formatting (actualmsg, FORMAT_NORMAL);
    debug (3, "Crypting! Sending: %s", actualmsg);
        ptr = check_crypt (skip_space (actualmsg), type);
    debug (3, "Crypting! Receive: %s", ptr);
    if (ptr) {
      if (type == GB_EMOTE)
        p = s + strlen (s) - strlen (actualmsg);
      else
        p = s + strlen (s) - strlen (actualmsg) - 2;
      strcpy (p, ptr);
      if (match_gag (s))
        return;
    }
  } else
    command_has_output = TRUE;
  if (GET_BIT (options, BEEP) && !end_prompt)
    term_beep (1);
  if (type != NONE) {
    process_spoken (racename, govname, atoi (raceid),
      atoi (govid), actualmsg);
  }

  socket_final_process (s, type);
  msg_type = MSG_ALL;    /* reset for next loop - must be here */
}

void socket_final_process (char *s, int type)
{

  if (handle_action_matches (s)) {
    return;
  }

  add_recall (s, type);
  if (type != NONE && GET_BIT (options, BOLD_COMM))
    display_bold_communication (s);
  else
    display_msg (s);    /* write to screen */
  if (type)
    cursor_to_window ();
}
      
/*
 * used for finding simple matches and setting the appropriate markers.
 */
void
parse_socket_output (char *s)
{
char dmbuf[MAXSIZ];
char dmbuf2[SMABUF];
int dmint;
int dmint2;
char dmch;

  if (!s && !*s)
    return;
  *dmbuf = '\0';
  *dmbuf2 = '\0';
  if ((sscanf (s, "Logged in as %[^:]:%[^[][%d.%d].",
      dmbuf, dmbuf2, &dmint, &dmint2) == 4) &&
     client_stats == L_PASSWORD) {
    remove_space_at_end (dmbuf);
    remove_space_at_end (dmbuf2);
    strcpy (profile.racename, dmbuf);
    strcpy (profile.govname, dmbuf2);
    add_assign ("racename", dmbuf);
    add_assign ("govname", dmbuf2);
    profile.raceid = dmint;
    profile.govid = dmint2;
    sprintf (dmbuf, "%d", profile.raceid);
    add_assign ("raceid", dmbuf);
    sprintf (dmbuf, "%d", profile.govid);
    add_assign ("govid", dmbuf);
    loggedin ();
    game_type = GAME_HAP;
    add_assign ("game_type", "HAP");
    password_invalid = FALSE;
  } else if (((sscanf (s, "%[^\"]\"%[^\"]\" [%d,%d] logged on.",
    dmbuf, dmbuf2, &dmint, &dmint2) == 4) ||
    (sscanf (s, "%[^\"]\"\" [%d,%d]", dmbuf, &dmint, &dmint2) == 3))  &&
    client_stats == L_PASSWORD) {
    remove_space_at_end (dmbuf);
    remove_space_at_end (dmbuf2);
    strcpy (profile.racename, dmbuf);
    strcpy (profile.govname, dmbuf2);
    add_assign ("racename", dmbuf);
    add_assign ("govname", dmbuf2);
    profile.raceid = dmint;
    profile.govid = dmint2;
    sprintf (dmbuf, "%d", profile.raceid);
    add_assign ("raceid", dmbuf);
    sprintf (dmbuf, "%d", profile.govid);
    add_assign ("govid", dmbuf);
    loggedin ();
    password_invalid = FALSE;
    on_endprompt (LEVEL_PROMPT);
  } else if ((sscanf (s,
    "Name changed to `%[^']'.", dmbuf) == 1)) { 
    strcpy (profile.racename, dmbuf);
    add_assign ("racename", dmbuf);
  } else if (MATCH (s, "Ship #* built at a cost*")) {
    strcpy (builtship, pattern1);
    add_assign ("builtship", builtship);
  } else if ((sscanf (s, "%c%d built at a cost of %d resources.",
    &dmch, &dmint, &dmint2) == 3)) {
    sprintf (builtship, "%d", dmint);
    add_assign ("builtship", builtship);
  } else if ((sscanf (s, "%c%d [%d] built at a cost of %d resources.",
    &dmch, &dmint, &dmint2, &dmint2) == 4)) {
    sprintf (builtship, "%d", dmint);
    add_assign ("builtship", builtship);
  } else if (MATCH (s, "Lot #* - *")) {
    strcpy (lotnum, pattern1);
    add_assign ("lotnum", lotnum);
  } else if (streq (s, "Diety has kicked you off")) {
    close_gb ();
  }
}

void oldcheck4_endprompt (char *s)
{
char *p;
int dummyint;
char dummybuf[SMABUF];
  
  if (!s || !*s) {
    end_prompt = NOT_PROMPT;
    return;
  }

  debug (4, "In oldcheck4 on a prompt: %s", s);

  if ((*(s+1) == '(') && (*(s+3) == '[')) {  /* we have a prompt */
    *scope.star = '\0';
    *scope.planet = '\0';
    *scope.shipc = '\0';
    *scope.mothership = '\0';
    if (strchr (s, '#')) {   /* we are at ship scope somewhere */
      scope.numships = -1; /* flag old style ship scope */
      if ((sscanf (s, " ( [%d] / /../#%[^/]/#%s )",
          &scope.aps, scope.mothership, scope.shipc) == 3) ||
          (sscanf (s, " ( [%d] /#%[^/]/#%s )",
          &scope.aps, scope.mothership, scope.shipc) == 3))
        scope.level = LEVEL_USSHIP;
      else if ((sscanf (s,
          " ( [%d] /%[^/]/%[^/]/ /../#%[^/]/#%s )",
          &scope.aps, scope.star, scope.planet,
          scope.mothership, scope.shipc) == 5) || 
          (sscanf (s, " ( [%d] /%[^/]/%[^/]/#%[^/]/#%s )",
          &scope.aps, scope.star, scope.planet,
          scope.mothership, scope.shipc) == 5))
        scope.level = LEVEL_PSSHIP;
      else if ((sscanf (s, " ( [%d] /%[^/]/ /../#%[^/]/#%s )",
          &scope.aps, scope.star,
          scope.mothership, scope.shipc) == 4) ||
          (sscanf (s, " ( [%d] /%[^/]/#%[^/]/#%s )",
          &scope.aps, scope.star, scope.mothership,
          scope.shipc) == 4))
        scope.level = LEVEL_SSSHIP;
      else if (sscanf (s, " ( [%d] /%[^/]/%[^/]/#%s )",
          &scope.aps, scope.star, scope.planet,
          scope.shipc) == 4)
        scope.level = LEVEL_PSHIP;
      else if (sscanf (s, " ( [%d] /%[^/]/#%s )",
          &scope.aps, scope.star, scope.shipc) == 3)
        scope.level = LEVEL_SSHIP;
      else if (sscanf (s, " ( [%d] /#%s )",
          &scope.aps, scope.shipc) == 2)
        scope.level = LEVEL_USHIP;
      else {
        scope.level = LEVEL_ERROR;
      }
    } else {
      if (sscanf (s, " ( [%d] /%[^/]/%s )", &scope.aps,
        scope.star, scope.planet) == 3)
        scope.level = LEVEL_PLANET;
      else {
        p = strchr (s, '/');
        if (!p) {
          end_prompt = NOT_PROMPT;
          return;
        }
        if (*(p+1) == ' ') {
          *scope.star = '\0';
          scope.level = LEVEL_UNIV;
        } else {
          if (sscanf (s, " ( [%d] /%s )",
            &scope.aps, scope.star) == 2)
            scope.level = LEVEL_STAR;
        }
      }
    }
    if (NOTGB ()) {
      game_type = GAME_UNKNOWN;
      add_assign ("game_type", "UNKNOWN");
    }
    end_prompt = LEVEL_PROMPT;
    add_assign ("scope", build_scope ());
    sprintf (dummybuf, "%d", scope.aps);
    add_assign ("aps", dummybuf);
    sprintf (dummybuf, "%d", scope.level);
    add_assign ("scope_level", dummybuf);
    add_assign ("star", scope.star);
    add_assign ("planet", scope.planet);
    add_assign ("ship", scope.shipc);
    add_assign ("mothership", scope.mothership);  
  }
  
  else if (streqrn (s, ENDREFUSED)) end_prompt = END_PROMPT;
  else if (streqrn (s, ENDHELP))    end_prompt = END_PROMPT;
  else if (streqrn (s, GBMFINISH) ||
     (sscanf (s, GBUFINISH, &dummyint) == 1) ||
     streqrn (s, HAPMFINISH) ||
     streqrn (s, HAPUFINISH)) {
    end_prompt = ENDDOING_PROMPT;  
    if (client_stats == L_SEGMENT) {
      if (notify > 0)
        term_beep (notify);
      init_start_commands (2);
      client_stats = L_REINIT;
    } else if (client_stats == L_UPDATE) {
      if (notify > 0)
        term_beep (notify);
      init_start_commands (1);
      client_stats = L_REINIT;
    }
  } else if (streqrn (s, ENDDOINGSEGMENT)) {
    end_prompt = DOING_PROMPT;
    client_stats = L_SEGMENT;
  } else if (streqrn (s, ENDDOINGUPDATE)) {
    end_prompt = DOING_PROMPT;
    client_stats = L_UPDATE;
  } else if (streqrn (s, ENDFINISH)) {
    end_prompt = FINISHED_PROMPT;
    server_help = FALSE;
  } else if (streqrn (s, ENDLOGIN) ||
     MATCH (s, "Please * password*") ||
         MATCH (s, "please * password*") ||
         MATCH (s, "Please * Password*") ||
     MATCH (s, "Password:*")) {
    end_prompt = PASS_PROMPT;
  } else if (streq (s, "Enter Password <race> <gov>:")) {
    end_prompt = PASS_PROMPT;
    game_type = GAME_GBDT;
    add_assign ("game_type", "GB+");
    CLR_BIT (options, LOGINSUPPRESS);
    if (!GET_BIT (options, AUTOLOGIN)) {
      msg ("%s", s);
      SECRET ("Password> ", dummybuf, PROMPT_STRING);
      send_gb (dummybuf, strlen (dummybuf));
    }
  } else if ((streq (s, "Invalid:  Bad Password.") ||
    streq (s, "Invalid:  Player already logged on!")) &&
    client_stats == L_PASSWORD) {
    end_prompt = NODIS_PROMPT;
    client_stats = L_CONNECTED;
    password_invalid = TRUE;
    if (GBDT ()) {
      CLR_BIT (options, LOGINSUPPRESS);
      CLR_BIT (options, AUTOLOGIN);
      CLR_BIT (options, CONNECT);
    }
    msg ("%s", s);
    msg ("-- Please enter a new password or 'quit' to disconnect from the server.");
    SECRET ("Password> ", dummybuf, PROMPT_STRING);
      if (streq (dummybuf, "quit")) {
        cmd_quote (dummybuf);
      } else {
        send_gb (dummybuf, strlen (dummybuf));
      }
  } else if (strstr (s, RACEGEN)) {
    racegen = TRUE;
    msg ("-- Client is in RACEGEN mode.");
    strcpy (last_prompt, "[ GB Racegen ]");
    SET_BIT (options, PARTIAL_LINES);    
    CLR_BIT (options, AUTOLOGIN);
    CLR_BIT (options, LOGINSUPPRESS);
    force_update_status();
  } else {
    end_prompt = NOT_PROMPT;
  }
}

void connect_prompts (char *s)
{
  char buf[SMABUF];

  debug (4, "In connect_prompts on a prompt: %s", s);

  if (streqrn (s, ENDLOGIN) ||
    MATCH (s, "Please * password*") ||
    MATCH (s, "please * password*") ||
    MATCH (s, "Please * Password*") ||
    MATCH (s, "Password:*"))
  {
    end_prompt = PASS_PROMPT;
  }
  else if (streq (s, "Enter Password <race> <gov>:"))
  {
    end_prompt = PASS_PROMPT;
    game_type = GAME_GBDT;
    add_assign ("game_type", "GB+");

    CLR_BIT (options, LOGINSUPPRESS);

    if (!GET_BIT (options, AUTOLOGIN))
    {
      msg ("%s", s);
      SECRET ("Password> ", buf, PROMPT_STRING);

      // Allow the user to quit -mfw
      if (streqn (buf, "quit", 4))
      {
        cmd_quote (buf);
      }
      else
      {
        send_gb (buf, strlen (buf));
      }
    }
  }
  else if ((streq (s, "Invalid:  Bad Password.") ||
    streq (s, "Invalid:  Player already logged on!")) &&
    client_stats == L_PASSWORD)
  {
    end_prompt = NODIS_PROMPT;
    // It seems to me after looking at the code that this should be set
    // to L_PASSWORD. I'm commenting out L_CONNECTED for now -mfw
    //client_stats = L_CONNECTED;
    client_stats = L_PASSWORD;
    password_invalid = TRUE;

    if (GBDT ())
    {
      CLR_BIT (options, LOGINSUPPRESS);
      CLR_BIT (options, AUTOLOGIN);
      CLR_BIT (options, CONNECT);
    }

    msg ("%s", s);
    strcpy(s, ""); // To blank out the message -mfw

    // Hmm, the quit command isn't working -mfw
    //msg ("-- Please enter a new password or 'quit' to disconnect from the server.");
    msg ("-- Please enter a new password or 'ctrl-c' to quit.");

    SECRET ("Password> ", buf, PROMPT_STRING);

    if (streqn (buf, "quit", 4))
    {
      cmd_quote (buf);
    }
    else
    {
      send_gb (buf, strlen (buf));
    }
  }
  else if (streqn (s, "CHAP CHALLENGE", 14))
  {
    /* Mike's CHAP login hack -mfw */
    end_prompt = PASS_PROMPT;
    game_type = GAME_GBDT;
    add_assign ("game_type", "GB+");

    if (GET_BIT (options, AUTOLOGIN))
    {
      if (streq(cur_game.game.type, "chap"))
      {
        strcpy(race_name, cur_game.game.racename);
        strcpy(govn_name, cur_game.game.govname);
        strcpy(race_pass, cur_game.game.pripassword);
        strcpy(govn_pass, cur_game.game.secpassword);
      }
      else
      {
        msg ("-- autologin failed, server requires CHAP login method.");
      }
    }

    chap_response (s);
  }
  else if (streq (s, "CHAP FAILURE"))
  {
    end_prompt = INTERNAL_PROMPT;
    client_stats = L_PASSWORD;
    password_failed = 1;

    strcpy(s, ""); // To blank out the message -mfw
    msg ("-- CHAP login failed, try again or 'ctrl-c' to quit.");

    if (GBDT ())
    {
      CLR_BIT (options, LOGINSUPPRESS);
      CLR_BIT (options, AUTOLOGIN);
      CLR_BIT (options, CONNECT);
    }
  }
  else if (streq(s, "CHAP SUCCESS"))
  {
    strcpy(s, ""); // To blank out the message -mfw
  }
  else if (strstr (s, RACEGEN))
  {
    racegen = TRUE;
    msg ("-- Client is in RACEGEN mode.");
    strcpy (last_prompt, "[ GB Racegen ]");
    SET_BIT (options, PARTIAL_LINES);    
    CLR_BIT (options, AUTOLOGIN);
    CLR_BIT (options, LOGINSUPPRESS);
    force_update_status();
  }
  else
  {
    end_prompt = NOT_PROMPT;
  }
}

void init_endprompt_connect (void)
{
  check4_endprompt = connect_prompts;
}

/*
 * send to the socket
 */
void
send_gb (char *s, int len)
{
int error;
int i;
char *p;
char outbuf[MAXSIZ];
char *q = s;
int olen = 0;
int splen;

  if ((!queue_sending && have_queue ()) ||
      client_stats != L_ACTIVE && !GET_BIT (options, PARTIAL_LINES) &&
      !(client_stats == L_PASSWORD ||
       client_stats == L_INTERNALINIT ||
       client_stats == L_REINIT)) {
    add_queue (s);
    return;
  }

  if (GET_BIT (options, NO_LOGOUT))
    set_no_logout ();

  do_queue = FALSE;

  if (gb < 0)
    return;

  splen = 0;
  for (p = outbuf, q = s; *q; ) {
    switch (*q) {
    case BELL_CHAR:
    case BOLD_CHAR:
    case INVERSE_CHAR:
    case UNDERLINE_CHAR:
      *p++ = SEND_QUOTE_CHAR;
      strcpy (p, SEND_QUOTE_PHRASE);
      splen = strlen (SEND_QUOTE_PHRASE);
      p += splen;
      *p++ = (*q + 'A' - 1);
      olen += 2 + splen;
      q++;
      break;
    case '\n':
      info.lines_sent++;
      *p++ = *q++;
      olen++;
      break;
    default:
      *p++ = *q++;
      olen++;
      break;
    }
  }

  outbuf[olen] = '\0';
  debug (4, "send_gb: %s", outbuf);

  if (sendgb (outbuf,olen) == -1)
  {
    msg ("Send to gb failed -- %s", s);
  }
  else
    info.bytes_sent += olen;
}

/*
 * Final raw sending of the data to the server.
 * Returns a -1 upon failure. Otherwise #bytes sent.
 */
int sendgb (char *buf, int len)
{
  buf[len++]='\x0d';
  buf[len++]='\x0a';
  buf[len]='\0';

  debug(4, "sending: %s", buf);

  return (send (gb, buf, len, 0));
}

cursor_output_window (void)
{
  if (GET_BIT (options, DISPLAY_TOP))
    term_move_cursor (0, last_output_row);
  else
    term_move_cursor (0, output_row);
  cursor_display = FALSE;
}

scroll_output_window (void)
{
  if (last_output_row >= output_row || !GET_BIT (options, DISPLAY_TOP)) {
    term_scroll (0, output_row, 1);
    cursor_display = FALSE;
    if (last_output_row > output_row)
      last_output_row--;
  }
}

void
cmd_connect (char *s)
{
  Game *p = (Game *) NULL;
  char host_try[BUFSIZ];
  char port_try[BUFSIZ];
  int fd;
  int unknown = FALSE;
  int dup_game = FALSE;
  int cnt;

  if (!*s)
   {
    msg ("Usage: connect [nick | host] [port]");
    return;
  }

  p = find_game (s);

  if (p)
  {
    if (cur_game.game.host && /*added this, was segfaulting on next line -mfw*/
        streq (cur_game.game.host, p->host) &&
        streq (cur_game.game.port, p->port) &&
        streq (cur_game.game.nick, p->nick) &&
        streq (cur_game.game.pripassword, p->pripassword) &&
        streq (cur_game.game.secpassword, p->secpassword))
    {
      dup_game = TRUE;
    }

    strcpy (host_try, p->host);
    strcpy (port_try, p->port);
  }
  else
   {
    split (s, host_try, port_try);

    if (!*port_try)
# ifdef DEFAULT_GBPORT
      sprintf (port_try, "%d", DEFAULT_GBPORT);
# else
      strcpy (port_try, "2010");
# endif
    unknown = TRUE;
  }

  msg ("-- Trying new host (%s %s)", host_try, port_try);
  fd = connectgb (host_try, port_try, "Connect: ");

  debug (1, "cmd_connect() fd: %d gb: %d", fd, gb);

  if (fd > 0)
  {
    if (gb > 0)
    {
      close_gb ();
      gb = dup (fd);
      close (fd);
    }
    else
      gb = fd;

    /* connecting to same site and race? */
    /* if so, don't re-init everything */
    if (!dup_game)
    {
      init_assign (TRUE);
    }

    cur_game.game.pripassword = strfree (cur_game.game.pripassword);
    cur_game.game.secpassword = strfree (cur_game.game.secpassword);
    cur_game.game.host = strfree (cur_game.game.host);
    cur_game.game.port = strfree (cur_game.game.port);
    cur_game.game.nick = strfree (cur_game.game.nick);
    cur_game.game.type = strfree (cur_game.game.type);
    cur_game.game.racename = strfree (cur_game.game.racename);
    cur_game.game.govname = strfree (cur_game.game.govname);

    cur_game.game.host = string (host_try);
    cur_game.game.port = string (port_try);

    if (!unknown)
    {
      cur_game.game.pripassword = string (p->pripassword);
      cur_game.game.secpassword = string (p->secpassword);
      cur_game.game.nick = string (p->nick);
      cur_game.game.type = string (p->type);
      cur_game.game.racename = string (p->racename);
      cur_game.game.govname = string (p->govname);
    }
    else
    {
      cur_game.game.pripassword = string ("");
      cur_game.game.secpassword = string ("");
      cur_game.game.nick = string ("unknown");
      cur_game.game.type = string ("unknown");
      cur_game.game.racename = string ("");
      cur_game.game.govname = string ("");
    }

    add_assign ("game_nick", cur_game.game.nick);
    add_assign ("host", cur_game.game.host);
    add_assign ("port", cur_game.game.port);
    add_assign ("type", cur_game.game.type);
    add_assign ("racename", cur_game.game.racename);
    add_assign ("pripassword", cur_game.game.pripassword);
    add_assign ("govname", cur_game.game.govname);
    add_assign ("secpassword", cur_game.game.secpassword);

    if (GET_BIT (options, AUTOLOGIN_STARTUP))
      SET_BIT (options, AUTOLOGIN);

    if (GET_BIT (options, LOGINSUPPRESS_STARTUP))
      SET_BIT (options, LOGINSUPPRESS);

    msg ("-- Connected to new host.");
    client_stats = L_CONNECTED;
    connect_time = time (0);
  }
   else
   {
    msg ("-- Could not reach new host.");
  }
}

set_values_on_end_prompt (void)
{
  more_val.non_stop = FALSE;
  more_val.forward = FALSE;
  if (!hide_msg)
    kill_socket_output = FALSE;
  kill_client_output = FALSE;
}

void
set_no_logout (void)
{
  last_no_logout_time = time (0);
}

void
check_no_logout (void)
{
long now;
char tbuf[BUFSIZ];

  if (!GET_BIT (options, NO_LOGOUT))
    return;
  now = time (0);
  if (now - NO_LOGOUT_DELAY < last_no_logout_time)
    return;
  last_no_logout_time = now;
  sprintf (tbuf, "cs %s\n", build_scope ());
  send_gb (tbuf, strlen (tbuf));
}

char *build_scope (void)
{
static char scopebuf[BUFSIZ];

  if (scope.level >= LEVEL_SHIP) {
    if (scope.numships == -1)
      sprintf (scopebuf, "#%s", scope.shipc);
    else
      sprintf (scopebuf, "#%d", scope.ship);
  } else if (scope.level == LEVEL_UNIV)
    sprintf (scopebuf, "/");
  else if (scope.level == LEVEL_STAR)
    sprintf (scopebuf, "/%s", scope.star);
  else if (scope.level == LEVEL_PLANET)
    sprintf (scopebuf, "/%s/%s", scope.star, scope.planet);
  return (scopebuf);
}

void
check_for_special_formatting (char *s, int type)
{
char *p;
char outbuf[MAXSIZ];
char *q;
char ch;

  if (!*s)
    return;
  for (p = s, q = outbuf; *p; p++, q++) {
    if (*p == SEND_QUOTE_CHAR && type == FORMAT_NORMAL) {
      p++;
      if (streqrn (p, SEND_QUOTE_PHRASE)) {
        p += strlen (SEND_QUOTE_PHRASE);
        if (*p == SEND_QUOTE_CHAR)
          *q = SEND_QUOTE_CHAR;
        else
          *q = (*p - 64);
      } else if (streqrn (p, SEND_OLD_QUOTE_PHRASE)) {
        p += strlen (SEND_OLD_QUOTE_PHRASE);
        *q = (*p - 64);
      } else {
        p--;
        *q = *p;
      }
    } else if (type == FORMAT_HELP) {
      switch (*p) {
      case '{':
      case '}':
        ch = *p++;
        if (*p == ch) {
          *q = BOLD_CHAR;
        } else {
          *q++ = ch;
          *q = *p;
        } 
        break;
      case '_':
        p++;
        if (*p == '_') {
          *q = UNDERLINE_CHAR;
        } else {
          *q++ = '_';
          *q = *p;
        } 
        break;
      default:
        *q = *p;
        break;
      }
    } else {
      *q = *p;
    }
  }
  *q = '\0';
  strcpy (s, outbuf);
}

/*
 * close the socket and clean up variables.
 */
close_gb (void)
{
  while (have_socket_output () && !paused)
    get_socket ();
  client_stats = L_BOOTED;
  password_invalid = FALSE;
  game_type = GAME_NOTGB;
  csp_server_vers = 0;
  init_assign (FALSE);
  ICOMM_INITIALIZE ();
  if (gb > 0) {
    msg ("-- Game closed.");
    close (gb);
    strcpy (last_prompt, "Not Connected");
    force_update_status();
  }
  gb = -1;
}

/*
 * Read a chunk from the socket and separate on newlines
 * and null terminate after removing the newline
 */
int read_socket (void)
{
int cnt;
char buf[MAXSIZ];
char *p;
char *q;

  /* read a chunk */
  if ((cnt = recv (gb, buf, MAXSIZ-1, 0)) > 0) {
  } else if (cnt == 0) {
    close_gb ();
  } else {
    msg_error ("-- Read_socket: ");
    return (-1);
  }
  buf[cnt] = '\0';  /* line isn't null'ed unless we do it */
  info.bytes_read += cnt;

  debug (3, "read_socket buf is:");
  debug (3, "%s", buf);
  debug (3, "end read_socket buf");

  q = buf;
  /* break into newlines if possible */
  while ((p = strchr (q, '\n'))) {
    *p = '\0';
    add_buffer (&gbsobuf, q, 0);
    q = p + 1;
  }
  /* we ended on a newline so we are fine, otherwise
   * need to put the partial part on the buffer */
  if ((q - buf) != cnt) {
    add_buffer (&gbsobuf, q, 1);
  }
  return (cnt);

}

int have_socket_output (void)
{
  return (have_buffer (&gbsobuf));
}

void loggedin (void)
{
extern char *entry_quote;

  client_stats = L_LOGGEDIN;
  connect_time = time (0);
  if (GBDT ()) {
    /* arg of 0 for color off, 1 for color on */
    csp_send_request (CSP_LOGIN_COMMAND, NULL);
    if (csp_server_vers <= 1)
      check4_endprompt = oldcheck4_endprompt;
    else  
      check4_endprompt = null_func;
  } else {
    game_type = GAME_GB;
    add_assign ("game_type", "GB");
    check4_endprompt = oldcheck4_endprompt;
  }
  if (GET_BIT (options, CONNECT_STARTUP))
    SET_BIT (options, CONNECT);
  if (GET_BIT (options, AUTOLOGIN_STARTUP))
    SET_BIT (options, AUTOLOGIN);
  if (GET_BIT (options, LOGINSUPPRESS_STARTUP))
    SET_BIT (options, LOGINSUPPRESS);

  init_start_commands (0); /* set client_stats below */

  if ( !GBDT () && entry_quote) {
    send_gb (entry_quote, strlen (entry_quote));
  }

  /* for init_start_comm, but after entry_quote */
  client_stats = L_INTERNALINIT;
  more_val.num_lines_scrolled = 0;
}

void
cmd_ping (char *s)
{
Game *p = (Game *) NULL;
char host_try[BUFSIZ];
char port_try[BUFSIZ];
int fd;

  if (!*s) {
    msg ("Usage: ping [nick | host] [port]");
    return;
  }

  p = find_game (s);
  if (p) {
    strcpy (host_try, p->host);
    strcpy (port_try, p->port);
  } else  {
    split (s, host_try, port_try);
    if (!*port_try)
# ifdef DEFAULT_GBPORT
      sprintf (port_try, "%d", DEFAULT_GBPORT);
# else
      sprintf (port_try, "%d", 2010);
# endif
  }

  msg ("-- Ping: (%s %s)", host_try, port_try);
  fd = connectgb (host_try, port_try, "Ping: ");
  if (fd > 0) {
    msg ("-- Ping: Host (%s %s) reached.", host_try, port_try);
    close (fd);
  }
}

void null_func (void)
{
}

int on_endprompt (int eprompt)
{
char *p;
extern char *build_scope_prompt ();

  if (!eprompt)
    return (FALSE);

  if (ICOMM_DOING_COMMAND && ICOMM_STATE == S_PROC &&
      ICOMM_PROMPT == eprompt) {
    icomm_command_done ();
  }
  set_values_on_end_prompt ();
  do_queue = TRUE;  /* special case.. must be socket*/
  if (eprompt == LEVEL_PROMPT) {
    if (client_stats == L_PASSWORD &&
        !GET_BIT (options, PARTIAL_LINES)) {
      msg ("-- WARNING: The client does NOT know your race id number.");
      while (client_stats != L_LOGGEDIN
             && client_stats >= L_PASSWORD) {
        promptfor ("Race Id #? ", pbuf,
          PROMPT_STRING);
        if (isdigit (*pbuf)) {
          profile.raceid = atoi (pbuf);
          if (profile.raceid > 0)
            loggedin ();
        }
      }
    }
    if (!command_has_output) {
    } else
      command_has_output = FALSE;
# ifdef IMAP
    if (input_mode.map && scope.level == LEVEL_PLANET) {
      map_prompt_force_redraw ();
    }
# endif
  }
  if (hide_msg) {
    hide_msg--;
    if (!hide_msg)
      kill_socket_output = FALSE;
    else
      return (TRUE); /* should this be false? */
  }
  if (end_msg) {
    end_msg--;  
    return (TRUE);
  }
  if (eprompt == FINISHED_PROMPT) {
    server_help = FALSE;
  }

  if (end_prompt == LEVEL_PROMPT) {
    p = build_scope_prompt ();
    if (!streq (last_prompt, p)) {
      strcpy (last_prompt, p);
      force_update_status ();
    }
  }

  if (eprompt >= NODIS_PROMPT &&
      GET_BIT (options, HIDE_END_PROMPT)) {
    cursor_to_window ();
    return (TRUE);
  }

  return (FALSE);
}

void get_pass_info (void)
{
  promptfor("Race Name: ", pbuf, PROMPT_STRING);
  strcpy(race_name, pbuf);

  if (!strncasecmp(race_name, "quit", 4))
  {
    chap_abort();
    return;
  }

  SECRET("Race Password: ", pbuf, PROMPT_STRING);
  strcpy(race_pass, pbuf);

  promptfor("Governor Name: ", pbuf, PROMPT_STRING);
  strcpy(govn_name, pbuf);

  if (!strncasecmp(govn_name, "quit", 4))
  {
    chap_abort();
    return;
  }

  SECRET("Governor Password: ", pbuf, PROMPT_STRING);
  strcpy(govn_pass, pbuf);

  return;
}

void chap_response (char *line)
{
  char auth_string[80];
  char client_hash[80];
  char pass_cat[80];
  char key[80];
  char garb1[80];
  char garb2[80];

  debug(2, "Received CHAP Challenge.");

  sscanf(line, "%s %s %s", &garb1, &garb2, &key);

  debug(4, "Recieved key: %s", key);

  if (!race_name[0] || !govn_name[0] || !race_pass[0] || !govn_pass[0]
    || password_failed)
  {
    CLR_BIT (options, LOGINSUPPRESS);
    get_pass_info();
  }

  sprintf(pass_cat, "%s%s%s", race_pass, govn_pass, key);

  MD5String(pass_cat, client_hash);

  sprintf(auth_string, "CHAP RESPONSE %s %s %s", race_name, govn_name,
    client_hash);

  debug(3, "chap_response sending: %s", auth_string);

  send_gb(auth_string, strlen(auth_string));

  return;
}

void chap_abort (void)
{
  char abort[22];
  char c;

  strcpy(abort, "CHAP ABORT");

  debug(3, "chap_abort sending abort string");

  sendgb(abort, 10);

  promptfor ("Quit client (y/n)? ", &c, PROMPT_CHAR);

  if (YES (c))
    quit_all = TRUE;
  else
    update_input_prompt (input_prompt);
}
