/*
 * map.c: handles the map data (planet and star) from server
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
# include <string.h>
# include <sys/types.h>
# include <memory.h>

# include "str.h"
# include "option.h"
# include "term.h"
# include "vars.h"
# include "arrays.h"
# include "ansi.h"

char maplog[BUFSIZ];

extern char *strcat (char *, const char *);
extern char *strncat (char *, const char *, size_t);
extern int atoi (const char *);
extern int sscanf (const char *, const char *, ...);
int print_X (int Maxx);

plot_surface (char *t)
{
int x;
int y;
int show;
int Maxx;
int Maxy;
char *u;
char *v;
int inverse = 0;
char temp[BUFSIZ];
char convbuf[SMABUF];
char c; /* -mfw */
int want_color = 0; /* -mfw */
int colored = 0; /* -mfw */

#ifdef CLIENT_DEVEL
  if (client_devel)
      msg(":: plot_surface()");
#endif
  
  want_color = GET_BIT(options, DISP_ANSI);

  u = t + 1;      /* skip the marking char $ */
  v = strchr (u, ';');    /* get planet name */
  *v = '\0';      /* null term the name */
  msg ("     '%s'", u);  /* print out planet name */

  u = v + 1;      /* get X size same way */
  v = strchr (u, ';');
  *v = '\0';
  Maxx = atoi (u);

  u = v + 1;      /* get Y size same way */
  v = strchr (u, ';');
  *v = '\0';
  Maxy = atoi (u);

  u = v + 1;      /* get show, not used currently */
  v = strchr (u, ';');
  *v = '\0';
  show = atoi (u);

  print_X (Maxx);

  u = v + 1;
  for (y = 0; y < Maxy; y++) {
    sprintf (convbuf, "%c%c ", y /10 + '0', y % 10 + '0');
    strcpy (maplog, convbuf);
    for (x = 0; x < Maxx; x++) {
      if ((c = *u++) == '1') {
        if (!inverse) {
          strcat (maplog, INVERSE_CHAR_STR); //-mfw for testing
          inverse = 1;
        }
      }
      else if (c > '?' && want_color)
      {
        colored = (c % MAX_RCOLORS);
        strcat (maplog, race_colors[colored]);
        strcat (maplog, ANSI_FOR_BLACK);
      }
      else if (inverse)
      {
        strcat (maplog, INVERSE_CHAR_STR);
        inverse = 0;
      }
      else if (colored)
      {
        colored = 0;
        strcat (maplog, ANSI_NORMAL);
      }
      strncat (maplog, u, 1);
      u++;
    }
    if (inverse)
      //strcat (maplog, INVERSE_CHAR_STR);
      strcat (maplog, ANSI_NORMAL);
    inverse = 0;
    if (GET_BIT (options, MAP_DOUBLE)) {
      sprintf (temp, "%s%c%c",
        (GET_BIT (options, MAP_SPACE) ? " " : ""), 
        y /10 + '0', y % 10 + '0');  
      strcat (maplog, temp);
    }

    msg ("%s", maplog);
    add_recall (maplog, 0);
  }
  if (GET_BIT (options, MAP_DOUBLE)) {
    if (GET_BIT (options, MAP_SPACE))
      msg ("");  
    print_X (Maxx);
  }
  cursor_to_window ();
}

print_X (int Maxx)
{
int x;
char buf[MAXSIZ];
char temp[BUFSIZ];

  strcpy  (buf, "   ");
  for (x = 0; x < Maxx; x++) {
    sprintf (temp, "%d", x / 10);
    strcat (buf, temp);
  }
  msg (buf);
  strcpy (buf, "   ");
  for (x = 0; x < Maxx; x++) {
    sprintf (temp, "%d", x % 10);
    strcat (buf, temp);
  }
  msg (buf);
  if (GET_BIT (options, MAP_SPACE))
    msg ("");
}

plot_orbit (char *t)
{
char *p;
char *q;
char name[SMABUF];
char colbuf[SMABUF];
char symbol;
int want_color; /* -mfw */
int colored = 0; /* -mfw */
int array;
char stand1; /* converted from int to char to handle color -mfw */
char stand2;
int x;
int y;

#ifdef CLIENT_DEVEL
  if (client_devel)
      msg(":: plot_orbit()");
#endif

  want_color = GET_BIT(options, DISP_ANSI);

  term_clear_screen ();

  q = t + 1;
  while ((p = strchr (q, ';'))) {  /* new */
    *p = '\0';    /* new */
    sscanf (q, "%c %d %d %d %c %c %s", &stand1, &x, &y, &array,
           &symbol, &stand2, name);
    x = (int) x * Midx;
    y = (int) y * Midy;
    if (x <= S_X && y <= S_Y) {
# ifdef ARRAY
      if (symbol == 'M' && array > 0 &&
          array <= 8) { /* space mirror */
        DispArray (x, y, 9, 5, Mirror[array - 1], 1.0);
      } else if (symbol == '*' && array > 0 &&
        array <= 16) {    /* nova */
        DispArray (x, y, 11, 7, Novae[array - 1], 1.0);
      }
# endif
      // DEBUG in here somewhere, too sleepy, going to bed. -mfw 
      //
      if (stand1 > '?' && want_color)
      {
        // Do color here -mfw
        colored = (stand1 % MAX_RCOLORS);
        sprintf(colbuf, "%s%s%c%s", race_colors[colored], ANSI_FOR_BLACK,
				  symbol, ANSI_NORMAL);
        term_move_cursor (x, y);
        term_puts (colbuf, strlen (colbuf));
      }
      else if (stand1 == '1')
      {
        term_standout_on ();
        term_move_cursor (x, y);
        term_putchar (symbol);
      }
      else
      {
        term_move_cursor (x, y);
        term_putchar (symbol);
      }

      if (stand1 == '1')
        term_standout_off ();

      term_puts (" ", 1);

      if (stand2 > '?' && want_color)
      {
        // Do color here -mfw
        colored = (stand2 % MAX_RCOLORS);
        sprintf(colbuf, "%s%s%s%s", race_colors[colored], ANSI_FOR_BLACK,
				  name, ANSI_NORMAL);
        term_puts (colbuf, strlen (colbuf));
      }
      else if (stand2 == '1')
      {
        term_standout_on ();
         term_puts (name, strlen (name));
      }
      else
         term_puts (name, strlen (name));

      if (stand2 == '1')
        term_standout_off ();
    }
    q = p + 1;    /* new */
  }
  term_standout_off ();
  last_output_row = output_row;
}

# ifdef ARRAY
/*
 * display array on screen.  at the moment magnification is not implemented. 
 */
DispArray (x, y, maxx, maxy, array, mag)
int x;
int y;
int maxx;
int maxy;
char *array[];
float mag;
{
int x2;
int y2;
int curx;
int cury;

#ifdef CLIENT_DEVEL
  if (client_devel)
      msg(":: DispArray()");
#endif

  for (cury = y - maxy / 2, y2 = 0; y2 < maxy; cury++) {
    if (cury >= 0 && cury <= S_Y) {
      curx = x - maxx / 2;
      term_move_cursor (curx, cury);
      for (x2 = 0; x2 < maxx; curx++) {
        if (curx >= 0 && curx <= S_X)
            term_putchar (array[y2][x2]);
            /* one to right */
        x2++;
      }
    }
    y2++;
  }

}
# endif
