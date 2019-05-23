/*
 * util.c: parsing of socket output checking for lines to gag
 *         maintains the list of input commands (history and recall)
 *         handle the macro lists.
 *         handles the queuing of commands for sending to socket.
 *         handles the read news routines.
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
# include <malloc.h>

# include "args.h"
# include "str.h"
# include "vars.h"

extern int do_queue;

extern char builtship[];
extern char lotnum[];

/**** Variables for gags ****/
typedef struct gags {
  char *name;
  int indx;
  struct gags *next;
  struct gags *prev;
} Gag;

static Gag *gag_head = NULL;
static Gag *gag_tail = NULL;

Gag *find_gag (char *name);

/**** Variables for history and recall ****/
typedef struct node {
  char *line;
  int type;      /* for queuing. do we wait for prompt */
  int indx;
  struct node *next;
  struct node *prev;
} Node;

int max_history = DEFAULT_HISTORY;
static int num_hist = 0;      /* num of history kept */
static Node *hist_head = NULL;      /* head of history list */
static Node *hist_cur = NULL;      /* tail of history list */
static Node *hist_recall = NULL;    /* current loc of recall in
               history list  */
int max_recall = DEFAULT_RECALL;
static int num_recall = 0;      /* num of recall kept */
static Node *recall_head = NULL;    /* head of recall list */
static Node *recall_cur = NULL;      /* current loc in recall */

/**** Variables for macros ****/
typedef struct macro {
  char *name;
  char *action;
  int flag;
  int indx;
  struct macro *next;
  struct macro *prev;
} Macro;

static Macro *macro_head = NULL;

void cmd_undef (char *name);

/**** Variables for game ****/
Game *game_head = NULL;

Game *find_game (char *nick);

/**** Variables for queue ****/
Node *queue_head = NULL;    /* input/back of line */
Node *queue_tail = NULL;     /* output/front of line */
Node *sec_queue_head = NULL;    /* secondary for process */
Node *sec_queue_tail = NULL; 
int queue_sending = FALSE;  /* flag to send thru send_gb in socket code */
int queue_clear = FALSE;  /* clear queue upon return */

/**** Variables for news ****/
typedef struct rnode {
  char *line;
  char *date;
  int count;
  char ftime[10];
  char ltime[10];
  struct rnode *next;
  struct rnode *prev;
} RNode;

extern RNode *find_news (char *date, char *line);

RNode *rhead;
RNode *rlast;
int bulletin;

/**** Variables for aliases ****/
Macro *alias_head;

/**** Gag ROUTINES ****/


extern char *strncpy (char *, const char *, size_t);
extern int atoi (const char *);
extern int fprintf (FILE *, const char *, ...);
extern int sscanf (const char *, const char *, ...);
extern int strncmp (const char *, const char *, size_t);
int add_game (char *nick, char *host, char *port, char *type, char *racename, char *pripasswd, char *govname, char *secpasswd);
int add_queue (char *args, int wait);
int cmd_listdef (char *args);
int cmd_listgag (char *args);
int cmd_listgame (void);
int cmd_ungag (char *name);
int cmd_ungame (char *args);
int def_update_index (void);
int gag_update_index (void);
int game_update_index (void);
int remove_macro (char *name);

int match_gag (char *pat)
{
Gag *p;

  for (p = gag_head; p; p = p->next ) {
    if (MATCH (pat, p->name)) 
      return (1);
  }
  return (0);
}

cmd_gag (char *args)
{
Gag *p;
char temp[SMABUF];

  if (!*args) {
    cmd_listgag (NULL);
    return;
  }

  /* -gag_pattern will remove the gag */
  if (*args == '-') {
    cmd_ungag ((args+1));
    return;
  }

  if (find_gag (args))
    return;

  p = (Gag *) malloc (sizeof (Gag));
  if (!p) {
    msg ("-- Could not allocate memory for gag.");
    return;
  }
  if (!gag_head) {
    gag_head = p;
    gag_tail = p;
    p->next = NULL;
    p->prev = NULL;
  } else {
    gag_tail->next = p;
    p->prev = gag_tail;
    p->next = (Gag *) NULL;
    gag_tail = p;
  }
  p->name = string (args);
  gag_update_index ();
  msg ("-- Gag(#%d): added '%s'.", p->indx, p->name);
  sprintf (temp, "#%d", p->indx);
  add_assign ("pid", temp);
}

/*
 * remove gag node from gag list by name
 */
cmd_ungag (char *name)      /* formerly remove_gag */
           
{
Gag *p;
int val = 0;

  if (*name == '#') {
    val = atoi (name+1);
    for (p = gag_head; p && p->indx != val; p = p->next)
      ;
  } else {
    p = find_gag (name);
  }

  if (!p) {
    msg ("-- Gag %s was not defined.", name);
    return;
  }
  /* head of list ? */
  if (!p->prev) {
    /* not sole node */
    if (p->next) {
      gag_head = p->next;
      gag_head->prev = (Gag *) NULL;
    } else {
      gag_head = (Gag *) NULL;
      gag_tail = (Gag *) NULL;
    }
  /* end of list */
  } else if (!p->next) {
    gag_tail = p->prev;
    p->prev->next = (Gag *) NULL;
  /* middle of list */
  } else {
    p->prev->next = p->next;
    p->next->prev = p->prev;
  }
  msg ("-- Gag %s removed.", name);
  strfree (p->name);
  free (p);
  gag_update_index ();
}

/*
 * list gags by name.
 */
cmd_listgag (char *args)
{
Gag *p;
int i = 1;

  if (!gag_head) {
    msg ("-- No gags defined.");
    return;
  }

  msg ("-- Gags:");
  for (p = gag_head; p; p = p->next) {
    p->indx = i;
    msg ("%3d) %s", i++, p->name);
  }
  msg ("-- End of gag list.");
}

gag_update_index (void)
{
Gag *p;
int i = 1;

  if (!gag_head)
    return;
  for (p = gag_head; p; p = p->next)
    p->indx = i++;
}

save_gags (FILE *fd)
{
Gag *p;

  if (!gag_head)
    return;
  fprintf (fd, "\n#\n# Gags\n#\n");
  for (p = gag_head; p; p = p->next)
    fprintf (fd, "gag %s\n", p->name);
}

/*
 * erase all nodes from the gag list
 */
cmd_cleargag (char *args)
{
Gag *p;

  for (p = gag_head; p; p = p->next ) {
    free (p->name);
    free (p);
  }
  gag_head = NULL;
  msg ("-- All gags cleared.");
}

/*
 * find the gag node in the list by name
 */
Gag *find_gag (char *name)
{
Gag *p;

  for (p = gag_head; p && (!streq (name, p->name)); p = p->next)
    ;
  return ((Gag *) p);
}

/**** HISTORY ROUTINES ****/

/*
 * adds one line of len to the history list.
 * removing a node if necessary
 */
add_history (char *line)
{

  if (*line == '\0')
    return;
  if (!hist_head) {
    hist_head = (Node *) malloc (sizeof (Node));
    hist_cur = hist_head;
    hist_cur->next = NULL;
    hist_cur->prev = NULL;
  } else {
    hist_cur->next = (Node *) malloc (sizeof (Node));
    hist_cur->next->prev = hist_cur;
    hist_cur = hist_cur->next;
    hist_cur->next = NULL;
  }
  hist_cur->line = string (line);
  num_hist++;
  while (num_hist > max_history) {
    Node *temp;
    temp = hist_head;
    hist_head = hist_head->next;
    if (hist_head != NULL)
      hist_head->prev = NULL;
    free (temp->line);
    free (temp);
    num_hist--;
  }
  hist_recall = NULL;
}

/*
 * recall one line forward in the history list
 * ctrl-n is the usual key.
 */
int recallf (char *line)
{
  if (!hist_head || !hist_recall || !hist_recall->next)
    return (FALSE);
  hist_recall = hist_recall->next;
  strcpy (line, hist_recall->line);
  return (TRUE);
}  
    
/*
 * recall one line back in the history list
 * ctrl-p is the usual key.
 */
int recallb (char *line)
{
 
  if (!hist_head)
    return (FALSE);
  if (!hist_recall)
    hist_recall = hist_cur;
  else {
    if (!hist_recall->prev)
      return (FALSE);
    hist_recall = hist_recall->prev;
  }
  strcpy (line, hist_recall->line);
  return (TRUE);
}

/**** RECALL ROUTINES ****/

/*
 * recalls n lines from the recall buffer
 */
recall (int n, int type)
{
Node *temp;

  n--;
  for (temp = recall_cur; temp && n; temp = temp->prev) {
    if (type) {
      if (temp->type) {
        n--;
      }
    } else {
      n--;
    }
  }
  if (!temp)
    temp = recall_head;
  for ( ; temp; temp = temp->next) {
    if (type) {
      if (temp->type) {
        msg ("%s", temp->line);
      }
    } else {
      msg ("%s", temp->line);
    }
  }
  msg ("-- end of %s", (type ? "convo" : "recall"));
}

/*
 * recalls lines n to m from recall buffer
 */
recall_n_m (int n, int m, int type)
{
int dist;
Node *p;

  dist = m - n;
  if (dist <= 0) {
    msg ("-- recall must be non-zero.");
    return;
  }
  if (n < 0 || m < 0) {
    msg ("-- recall arguements must be positive.");
    return;
  }

  for (p = recall_head; p && n; p = p->next) {
    if (type) {
      if (p->type) {
        n--;
      }
    } else {
      n--;
    }
  }

  for ( ; p && dist; p = p->next, dist--) {
    if (type) {
      if (p->type) {
        msg ("%s", p->line);
      }
    } else {
      msg ("%s", p->line);
    }
  }
  msg ("-- end of %s", (type ? "convo" : "recall"));
}

/*
 * adds line to the recall list and maintains num_recall nodes.
 */
add_recall (char *line, int type)
           
             /* 0 is normal, 1/2 is broad/announce */
{

  if (*line == '\n' || *line == '\0' || !max_recall)
    return;

  if (!recall_head) {
    recall_head = (Node *) malloc (sizeof (Node));
    recall_cur = recall_head;
    recall_cur->next = NULL;
    recall_cur->prev = NULL;
  } else {
    recall_cur->next = (Node *) malloc (sizeof (Node));
    recall_cur->next->prev = recall_cur;
    recall_cur = recall_cur->next;
    recall_cur->next = NULL;
  }
  recall_cur->line = string (line);
  recall_cur->type = type;
  num_recall++;
  while (num_recall > max_recall) {
    Node *temp;
    temp = recall_head;
    recall_head = recall_head->next;
    recall_head->prev = NULL;
    if (temp->line/* && *temp->line */)
      free (temp->line);
    free (temp);
    num_recall--;
  }
}

/*
 * recalls all nodes from recall list matching <args>
 * utilizes the MATCH function.
 */
recall_match (char *args, int type)
{
Node *p;

  for (p = recall_head; p; p = p->next) {
    if (type && !p->type) continue;
    if (MATCH (p->line, args))
      msg ("%s", p->line);
  }
  msg ("-- end of %s", (type ? "convo" : "recall"));
}

/*
 * this does the UNIX style ^xx^yy replace search.
 * only affects last command and displays "Modifier failed."
 * on a no match.
 */
history_sub (char *args)
{
char *p;
char buf[BUFSIZ];
char pat[BUFSIZ];

  p = strchr (args, '^');
  if (!p) {
    msg ("-- No specified modifier.");
    return;
  }

  *p = '\0';
  sprintf (pat, "*%s*", args);
  if (!MATCH (hist_cur->prev->line, pat)) {
    msg ("-- Modifier failed.");
    return;
  } else {
    sprintf (buf, "%s%s%s", pattern1, p+1, pattern2);
    msg ("%s", buf);
    process_key (buf, FALSE);
    add_history (buf);
  }
}

/**** Macro ROUTINES ****/

/*
 * find the macro node in the list by name or by index #.
 * To request a macro by index # the string must be #number.
 */
Macro *find_macro (char *name)
{
Macro *p;
int val;

  if (*name == '#') {
    val = atoi (name+1);
    for (p = macro_head; p && p->indx != val; p = p->next)
      ;
  } else {
    for (p = macro_head; p; p = p->next) {
      val = strcmp (name, p->name);
      if (!val)
        return ((Macro *) p);
      else if (val < 0)
        return ((Macro *) NULL);
    }
    return ((Macro *) NULL);
  }
  return ((Macro *) p);
}

/*
 * place a macro on the list with corresponding action.
 * a null action will erase the name macro from the list
 */
cmd_def (char *args)
           
/*
char *name;
char *action;
*/
{
Macro *new;
Macro *p;
char *q;
char *r;
char *name;
char *action;
char fmtact[MAXSIZ];
int carg = 1;
int edit = FALSE;

  name = get_args (carg, 0);
  if (!*name) {
    cmd_listdef (NULL);
    return;
  }

  if (streq (name, "edit")) {
    msg ("-- alias: %s is an invalid macro name since it is an option.", name);
    strfree (name);
    return;
  /* -macro will delete the macro from the list */
  } else if (*name == '#') {
    msg ("-- alias: macros may not start with %c due to their use in indexes.", *name);
    strfree (name);
    return;
  } else if (*name == '-') {
    if (streq (name+1, "edit")) {
      carg++;
      edit = TRUE;
      strfree (name);
      name = get_args (carg, 0);
    } else {
      (void) cmd_undef (name+1);
      strfree (name);
      return;
    }
  }

  action = get_args (carg+1, 100);
  p = find_macro (name);

  if (edit) {
    if (p) {
      sprintf (fmtact, "def %s %s", p->name,  p->action);
      set_edit_buffer (fmtact);
    } else 
      msg ("-- Alias: '%s' not found.", name);
    strfree (name);
    strfree (action);
    return;
  }

  if (!*action) {
    if (p == NULL) 
      msg ("-- Macros need a corresponding action.");
    else
      msg ("-- def: %3d) %s = %s",
        p->indx, p->name, p->action);
    strfree (name);
    strfree (action);
    return;
  }

  if (p) {
    remove_macro (name);
    p = NULL;
  }

  new = (Macro *) malloc (sizeof (Macro));
  if (!new) {
    msg ("-- Could not allocate memory for macro.");
    return;
  }
  if (!macro_head) {
    macro_head = new;
    new->next = NULL;
    new->prev = NULL;
  } else {
  /* add in, in strcmp order */
    for (p = macro_head; p->next && (strcmp (name, p->name) > 0);
        p = p->next)
      ;
    if (!p->next && (strcmp (name, p->name) > 0)) {/* goes on end */
        p->next = new;
        new->prev = p;
        new->next = (Macro *) NULL;
    } else { /* goes in front of p */
      new->next = p;
      new->prev = p->prev;
      if (p->prev && p->prev->next)
        p->prev->next = new;
      p->prev = new;
      if (macro_head == p)
        macro_head = new;
    }
  }

  /* put spaces around semi-colons to prevent arg problems later */
  for (r = fmtact, q = action; *q; q++) {
    if (*q == ';') {
      if (*(q-1) != ' ')
        *r++ = ' ';
      *r++ = ';';
      if (*(q+1) != ' ')
        *r++ = ' ';
    } else {
      *r++ = *q;
    }
  }
  *r = '\0';

  strcpy (action, fmtact);
  new->name = name;
  new->action = action;
  new->flag = FALSE;
  def_update_index ();
  msg ("-- Alias(#%d): '%s' '%s'", new->indx, new->name, new->action);
  sprintf (fmtact, "#%d", new->indx);
  add_assign ("pid", fmtact);
}

void cmd_undef (char *name)
{
Macro *p;
int val = 0;

  p = find_macro (name);

  if (p == NULL)
    msg ("-- No such macro (%s) defined.", name);
  else {
    msg ("-- Removed macro: %s", name);
    remove_macro (p->name);
  }
}

/*
 * remove macro node from macro list by name
 */
remove_macro (char *name)
{
Macro *p;
int val = 0;

  p = find_macro (name);

  if (!p) {
    msg ("-- Macro %s was not defined.", name);
    return;
  }
  if (!p->prev) {
    if (p->next) {
      macro_head = p->next;
      macro_head->prev = NULL;
    } else {
      macro_head = NULL;
    }
  } else if (!p->next) {
    p->prev->next = NULL;
  } else {
    p->prev->next = p->next;
    p->next->prev = p->prev;
  }
  free (p->action);
  free (p->name);
  free (p);
  def_update_index ();
}

/*
 * upon matching name, do the corresponding action.
 */
int do_macro (char *str)
{
char named[BUFSIZ];
char args[MAXSIZ];
Macro *macro_ptr;
register char *r;
register char *d;
register char *p;
int silent = 0;
char *name = named;

  split (str, named, args);
  if (*name == '-') {  /* silent macro */
    silent = TRUE;
    name++;
  }
  if (!(macro_ptr = find_macro (name))) {
    return (FALSE);
  }

  r = parse_macro_args (macro_ptr->action, args);
  d = r;

  if (silent)
    add_queue ("set display off", 1);
  while (p = strchr (d, ';')) {
    *p = '\0';
    add_queue (d, 1);
    d = p + 1;
  }
  add_queue (d, 1);
  if (silent)
    add_queue ("set display on", 1);
  strfree (r);

  return (TRUE);

} /* end do_macro */

/*
 * list macros by name.
 */
cmd_listdef (char *args)
{
Macro *p;
int i = 1;

  if (!macro_head) {
    msg ("-- No aliases defined.");
    return;
  }

  msg ("-- Aliases:");
  for (p = macro_head; p; p = p->next) {
      p->indx = i;
      msg ("%3d) %s = %s", i++, p->name, p->action);
  }
  msg ("-- End of aliases listing.");
}

def_update_index (void)
{
Macro *p;
int i = 1;

  if (!macro_head)
    return;

  for (p = macro_head; p; p = p->next)
      p->indx = i++;
}

save_defs (FILE *fd)
{
Macro *p;

  if (!macro_head)
    return;
  fprintf (fd, "\n#\n# Macros\n#\n");
  for (p = macro_head; p; p = p->next) 
    fprintf (fd, "def %s %s\n", p->name, p->action);
}

/*
 * erase all nodes from the macro list
 */
cmd_cleardef (void)
{
Macro *p;

  for (p = macro_head; p; p = p->next ) {
    free (p->name);
    free (p->action);
    free (p);
  }
  macro_head = NULL;
}

/**** Game ROUTINES ****/

/*
 * preps a line for possible adding to game list.
 */
cmd_game (char *args)
{
  char nick[BUFSIZ];
  char host[BUFSIZ];
  char port[BUFSIZ];
  char type[BUFSIZ];
  char sub1[BUFSIZ];
  char sub2[BUFSIZ];
  char sub3[BUFSIZ];
  char sub4[BUFSIZ];
  int cnt;

  if (!*args) {
    cmd_listgame ();
    return;
  }

  if (*args == '-') {
    cmd_ungame ((args+1));
    return;
  }

  cnt = sscanf (args, "%s %s %s %s %s %s %s %s",
    nick, host, port, type, sub1, sub2, sub3, sub4);

  if (cnt > 3)
  {
    if (streq(type, "plain"))
    {
      switch (cnt)
      {
        case 5:
          *sub2 = '\0';
        case 6:
          *sub3 = '\0';
          *sub4 = '\0';
          /* args: nick, host, port, type, racename, passwd, govname, passwd */
          add_game (nick, host, port, type, sub3, sub1, sub4, sub2);
          break;
        default:
          msg ("-- Error in add_game, plain auth.");
          break;
      }  

      return;
    }
    else if (streq(type, "chap"))
    {
      if (cnt == 8)
      {
          add_game (nick, host, port, type, sub1, sub2, sub3, sub4);
      }
      else
      {
          msg ("-- Error in add_game, chap auth.");
      }

      return;
    }
    else
    {
      msg("-- Error in game line %s: login type should be 'plain' or 'chap'.",
          nick);
    }

    return;
  }
  else if (cnt == 3)
  {
    strcpy(type, "none"); /* no authentication -mfw */

    add_game (nick, host, port, type, sub1, sub2, sub3, sub4);

    return;
  }

  msg("-- Error in game line: incorrect number of args.");
}
  
/*
 * adds a game entry to the list.
 */
add_game (char *nick, char *host, char *port, char *type,
  char *racename, char *pripasswd, char *govname, char *secpasswd)
{
  Game *p;

  p = find_game (nick);

  if (!p)
  {
    p = game_head;

    if (!game_head)
    {
      game_head = (Game *) malloc (sizeof (Game));
      game_head->next = NULL;
      game_head->prev = NULL;
      p = game_head;
    }
    else
    {
      for ( ; p->next; p = p->next)
        ;
      p->next = (Game *) malloc (sizeof (Game));
      p->next->prev = p;
      p = p->next;
      p->next = NULL;
    }
  }
  else
  {
    p->nick = strfree (p->nick);
    p->host = strfree (p->host);
    p->port = strfree (p->port);
    p->type = strfree (p->type);
    p->racename = strfree (p->racename);
    p->pripassword = strfree (p->pripassword);
    p->govname = strfree (p->govname);
    p->secpassword = strfree (p->secpassword);
  }

  p->nick = string (nick);
  p->host = string (host);
  p->port = string (port);
  p->type = string (type);
  p->racename = string (racename);
  p->pripassword = string (pripasswd);
  p->govname = string (govname);
  p->secpassword = string (secpasswd);
  
  if (streq(type, "chap"))
  {
    msg ("-- Game: %s added. %s %s with CHAP authentication.",
      p->nick, p->host, p->port);
  }
  else if (streq(type, "plain"))
  {
    /*msg ("-- Game: %s added. %s %s with password: %s %s",
      p->nick, p->host, p->port, p->pripassword, p->secpassword);*/
    msg ("-- Game: %s added. %s %s with plain-text authentication.",
      p->nick, p->host, p->port);
  }
  else if (streq(type, "none"))
  {
    msg ("-- Game: %s added. %s %s with no authentication method.",
      p->nick, p->host, p->port);
  }

  game_update_index ();
}

cmd_ungame (char *args)
{
Game *p;
int val = 0;

  p = find_game (args);

  if (!p) {
    msg ("-- No such game nick %s found.", args);
    return;
  }

  if (!p->prev) {
    if (p->next) {
      game_head = p->next;
      game_head->prev = NULL;
    } else {
      game_head = NULL;
    }
  } else if (!p->next) {
    p->prev->next = NULL;
  } else {
    p->prev->next = p->next;
    p->next->prev = p->prev;
  }

  msg ("-- Game %s removed.", args);
  free (p->nick);
  free (p->host);
  free (p->port);
  free (p->type);
  free (p->racename);
  free (p->pripassword);
  free (p->govname);
  free (p->secpassword);
  free (p);
  game_update_index ();
}

/*
 * attempts to find the specified game. returns NULL if not found
 */
Game *find_game (char *nick)
{
Game *gp = game_head;

  for ( ; gp; gp = gp->next) {
    if (streq (gp->nick, nick))
      return (gp);
  }
  return ((Game *) NULL);
}

/*
 * lists the games in the list
 */
cmd_listgame (void)
{
Game *p = game_head;
int i = 1;

  if (!game_head) {
    msg ("-- No games defined.");
    return;
  }

  msg ("-- Games:");
  for ( ; p; p = p->next) {
    p->indx = i;
    msg ("%2d) %s %s %s %s %s", i++, p->nick, p->host, 
      p->port, p->pripassword, p->secpassword);
  }
  msg ("-- End games list.");
}

game_update_index (void)
{
Game *p;
int i = 1;

  if (!game_head)
    return;
  for (p = game_head; p; p = p->next)
    p->indx = i++;
}

/*
 * Saves the current listing of games in a readable format by loadf
 * to the file descriptor provided to the function.
 */
save_games (FILE *fd)
{
Game *p = game_head;

  if (!game_head)
    return;
  fprintf (fd, "\n#\n# List of Current Games\n#\n");
  for ( ; p; p = p->next)
    fprintf (fd, "game %s %s %s %s %s\n", p->nick, p->host, 
      p->port, p->pripassword, p->secpassword);
}

/*
 * send the primary and secondary passwords as needed
 */
send_password (void)
{
  char pass[BUFSIZ];

  debug(1, "send_password() type: %s", cur_game.game.type);

  if (streq(cur_game.game.type, "plain"))
  {
    sprintf (pass, "%s %s\n", cur_game.game.pripassword,
      cur_game.game.secpassword);
    send_gb (pass, strlen (pass));
  }
  else if (streq(cur_game.game.type, "chap"))
  {
    /* Do nothing, handled in connect_prompts() -mfw */
  }
  else
  {
    msg("-- Error in send_password");
  }
}

/**** QUEUE ROUTINES ****/

/*
 * adds a line to the queue list for later processing.
 */
add_queue (char *args, int wait)
{
Node *p;
char *ptr = args;

  if (*args == '\n' || streq (args, "")) {
    return;
  }

  p = (Node *) malloc (sizeof (Node));
  if (!p) {
    quit_gb (-2, "-- Fatal. Could not allocate memory for queue.");
  }
  if (!queue_head) {
    queue_head = p;
    queue_tail = p;
    p->next = NULL;
    p->prev = NULL;
  } else {
    p->next = queue_head;
    queue_head->prev = p;
    p->prev = NULL;
    queue_head = p;
  }
  p->line = string (args);
  p->type = wait;
  debug (3, "add_queue: '%s' added.", p->line);
}

/*
 * removes the first item in the queue and returns it in args
 */
remove_queue (char *args)
{
Node *p = queue_tail;
char dest[MAXSIZ+1];
char *s;

  if (!queue_head) {
    *args = '\0';
    return;
  }

  if (p->prev)
    p->prev->next = NULL;
  /* no more on queue, lock queue for ctrl-c
     it gets reset in process_queue () */
  if (queue_tail == queue_head) {
    debug (2, "remove_queue: secondary queue is empty");
    queue_tail = queue_head = NULL;
  } else
    queue_tail = p->prev;

  debug (2, "remove_queue: returning %s", p->line);
  strcpy (args, p->line);
  free (p->line);
  free (p);
}

process_queue (char *s)
{

  sec_queue_tail = queue_tail;
  sec_queue_head = queue_head;
  queue_head = NULL;  

  debug (3, "process_queue: %s", s);
  queue_sending = TRUE;
  process_key (s, FALSE);
  queue_sending = FALSE;

  /* old queue exists, place back in line, q is front of old */
  if (sec_queue_tail)
    sec_queue_tail->next = queue_head;
  /* new queue exists, link back to front of old queue */
  if (queue_head)
    queue_head->prev = sec_queue_tail;
  /* p is back of old. it exists and therefore is end of total queue */
  if (sec_queue_head)
    queue_head = sec_queue_head;
}  

/*
 * returns TRUE if the queue exists, otherwise FALSE
 * returns TRUE also if we are not connected to a gb server
 * so that the queue doesn't block output on other connections
 * (or lack of connections).
 */
int check_queue (void)
{
extern int is_connected ();

  if (!queue_head)      /* no queue */
    return (FALSE);

  /* is socket connected? if no, true.
   * is it gb, if no, then true.
   * are we doing queue, if not, then true
   */
  if (is_connected () && !NOTGB () && !do_queue)
    return (FALSE);
  return (TRUE);        /* else do queue */
}

int have_queue (void)
{
  if (queue_head || sec_queue_head)
    return (TRUE);
  return (FALSE);
}

int do_clear_queue (void)
{
  if (queue_head && queue_clear)
    return (TRUE);
  return (FALSE);
}

/*
 * removes all nodes from the queue
 */
clear_queue (void)
{
char buf[MAXSIZ];

  while (queue_head)
    remove_queue (buf);
  queue_clear = FALSE;
}

/**** NEWS ROUTINES ****/


/*
 * Check to see if news command has started, and build first node
 */
check_news (char *s)
{

  bulletin = 0;
  if (streq (s, "The Galactic News") ||
     (*s == '-' || streq (s, "<Output Flushed>") && (bulletin = 1))) {
    ICOMM_STATE = S_PROC;
    ICOMM_IGNORE = TRUE;
  }

  /* This is here in case the News name changes like it has
   * on some games, then at least hopefully news will be parsed
   * correctly */
  rhead = (RNode *) malloc (sizeof (RNode));
  rlast = rhead;
  rlast->next = NULL;
  rlast->prev = NULL;
  rlast->line = string (s);
  rlast->date = string ("-");
  rlast->count = 1;
}

add_news (char *s)
{
RNode *p;
char date[10];
char time[9];
char line[MAXSIZ];
int idummy;

  *date = '\0';
  *time = '\0';
  if (*s == '\0')
    return (1);
  if (ICOMM_STATE != S_PROC)
    return (0);
  if (*s == '-') {
    strcpy (date, "-");
    strncpy (line, s, MAXSIZ);
    bulletin++;
  } else {
    if (sscanf (s, "%2d/ %d %2d:%2d:%2d %*s", &idummy,
      &idummy, &idummy, &idummy, &idummy) < 5) {
      *date = 'c';
      strncpy (line, s, MAXSIZ);
    } else {
      strncpy (date, s, 5);  /* 5 cause, MM/DD */
      date[5] = '\0';    /* just in case */
      strncpy (time, s+6, 5);
      time[5] = '\0';
      strncpy (line, s+15, MAXSIZ);
    }
  }
  line[MAXSIZ - 1] = '\0';

  p = find_news (date, line);
  if ((bulletin != 4 || streqrn (line, "Server") ||
     streqrn (line, "Shutdown")) && p) {
    (p->count)++;
    strcpy (p->ltime, time);
    return (1);
  }
  rlast->next = (RNode *) malloc (sizeof (RNode));
  if (!rlast->next) {
    msg ("-- malloc error in read news. aborting.");
    return (0);
  }

  rlast->next->prev = rlast;
  rlast = rlast->next;
  rlast->next = NULL;
  rlast->line = string (line);
  rlast->date = string (date);
  strcpy (rlast->ftime, time);
  strcpy (rlast->ltime, time);
  rlast->count = 1;
  return (1);
}

RNode *find_news (char *date, char *line)
{
RNode *p;

  for (p = rhead; p && (!streq (date, p->date) ||
      !streq (line, p->line)); p = p->next)
    ;
  return (p);
}

print_news (void)
{
RNode *p;
char buf[BUFSIZ];

  for (p = rhead; p; p = p->next)  {
    if (*p->date == '-') {
      sprintf (buf, "%s", p->line);
      msg_type = MSG_NEWS;
      process_socket (buf);
      msg_type = MSG_NEWS;
      process_socket ("");
    } else if (*p->date == 'c') {
      check_for_special_formatting (buf);
      msg_type = MSG_NEWS;
      process_socket (p->line);
    } else {
      if (p->count > 1) {
        sprintf (buf, "%s (%s-%s) %s [%d times]",
          p->date, p->ftime, p->ltime,
          p->line, p->count);
      } else {
        sprintf (buf, "%s (%s      ) %s",
          p->date, p->ftime, p->line);
      }
      check_for_special_formatting (buf);
      msg_type = MSG_NEWS;
      process_socket (buf);
    }
    free (p->date);
    free (p->line);
  }
  for (p = rhead; p; ) {
    p = p->next;
    free (rhead);
    rhead = p;
  }
}
