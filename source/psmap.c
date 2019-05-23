/* psmap.c                                                                 */
/* produces a Postscript map from the output of the GB command 'stars'     */
/* Copyright: Andreas Girgensohn (andreasg@cs.colorado.edu)                */
/* Different arguments added by Samuli Suomi (Samuli.Suomi@hut.fi)         */
/* Modified to be part of the gbII client by mike@towerravens.com (7/6/01) */

#include "gb.h"

#include <stdio.h>
#include <time.h>

#include "types.h"
#include "csp.h"

extern char pbuf[];

/* #define PSVIEWER "/usr/openwin/bin/pageview" for SunOS */
#define PSVIEWER "/usr/X11R6/bin/ghostview" /* for Linux */
#define PSFILE "~/gbmap.ps"

struct star {
  int   x,
        y;
  int   dist;
  char  name[80];
};

static struct univ
{
  struct star stars[MAXSTARS];
  int nstars;
  int starptr;    /* pointer to current star */
  int max_dist;
  int ring_spacing;
  int highlight_home;
  int highlighted_star;  /* star that will be highlighted */
  int nrings;    /* number of distance rings around the highlighted star */
  char galaxyname[80];
} univ;


void cmd_psmap (char *);
void cspr_psmap (int, char *);
void scan_stars (void);
int produce_postscript (void);
int draw_star (FILE *, struct star *, int, int, double);
void process_psmap (void);
void display_psmap (void);
double myround (double);

void
cmd_psmap (char *args)
{
#ifndef RESTRICTED_ACCESS
  char str[MAXSIZ];
  int int1 = TRUE;
  int amt;

  univ.highlight_home = 1;
  univ.highlighted_star = -1;
  //univ.nrings = 20;
  univ.nrings = 12; // HUT value
  //univ.ring_spacing = 10000;
  univ.ring_spacing = 50000; // HUT value

  while (int1)
  {
    promptfor ("Highlight home planet? (y/n) ", pbuf, PROMPT_CHAR);

    if (YES (*pbuf))
    {
      univ.highlight_home = 1;
      int1 = FALSE;
    }
    else
    {
      univ.highlight_home = 0;
      int1 = FALSE;
    }
  }

  /* Don't need to prompt for this anymore, we scale it below
  while (int1)
  {
    promptfor ("Max distance? [999999] ", pbuf, PROMPT_STRING);
    amt = atoi(pbuf);

    if (amt <= 0)
    {
      univ.max_dist = 999999;
      int1 = FALSE;
    }
    else if (amt > 0 && amt < 1000000)
    {
      univ.max_dist = amt;
      int1 = FALSE;
    }
  } */

  int1 = TRUE;

  /* We print one ring every inch now, the scale is not printed on the map
  while (int1)
  {
    promptfor ("Ring spacing? [50000] ", pbuf, PROMPT_STRING);
    amt = atoi(pbuf);

    if (amt <= 0)
    {
      univ.ring_spacing = 50000;
      int1 = FALSE;
    }
    else if (amt < 10000)
    {
      univ.ring_spacing = 10000;
      int1 = FALSE;
    }
    else if (amt > 1000000)
    {
      univ.ring_spacing = 1000000;
      int1 = FALSE;
    }
    else
    {
      univ.ring_spacing = amt;
      int1 = FALSE;
    }
  } */

  msg ("-- Psmap: Gathering data. Please hold.");

  sprintf(str, "%s %d\n", CSP_SERVER, CSP_UNIVDUMP_COMMAND);
  send_gb (str, strlen (str));

  return;
#else
  msg ("-- Restricted Access: psmap not available.");
#endif
}

void cspr_psmap (int cnum, char *line)
{
  int i;
  int starnum, tmpx, tmpy, dist;
  char sname[SMABUF];

  switch (cnum)
  {
    case CSP_UNIVDUMP_INTRO:

      /* Initialize stars array */
      for (i = 0; i < MAXSTARS; i++)
      {
        strcpy(univ.stars[i].name, "");
        univ.stars[i].x = 0;
        univ.stars[i].y = 0;
      }

#ifdef CLIENT_DEVEL
      if (client_devel)
        msg (":: CSP_UNIVDUMP_INTRO - %s", line);
#endif

      sscanf(line, "%d %d %s", &univ.nstars, &univ.max_dist, univ.galaxyname);

      break;

    case CSP_UNIVDUMP_STAR:

#ifdef CLIENT_DEVEL
      if (client_devel)
        msg (":: CSP_UNIVDUMP_STAR - %s", line);
#endif

      sscanf(line, "%d %s %d %d %d", &starnum, sname,
        &tmpx, &tmpy, &dist);

      strcpy(univ.stars[starnum].name, sname);
      univ.stars[starnum].dist = dist;
      univ.stars[starnum].x = tmpx;
      univ.stars[starnum].y = tmpy;

      break;

    case CSP_UNIVDUMP_END:

#ifdef CLIENT_DEVEL
      if (client_devel)
        msg (":: CSP_UNIVDUMP_END - %s", line);
#endif

      /* Call star draw function */
      process_psmap ();

      break;

    default:

      msg (":: Unknown CSP command #%d - %s", cnum, line);
  }
}

void
process_psmap (void)
{
  if (univ.nstars == 0)
  {
    msg ("No stars.\n");
  }
  else
  {
    scan_stars ();
    if (produce_postscript ())
      display_psmap();
  }
}

void
scan_stars (void)
{
  int in_stars = 0;
  int i;

  for (i = 0; i < univ.nstars; i++)
  {
    if (univ.highlight_home && univ.stars[i].dist == 0)
    {
      univ.highlighted_star = i;

      in_stars = 1;
    }

    if (in_stars)
      break;
  }
}

int
produce_postscript (void)
{
  int min_x, max_x, min_y, max_y, i;
  double  scale, nscale;
  char fname[BUFSIZ];
  FILE *fd;
  time_t *now;
  struct tm *today;
  char datestr[80];

  time(now); 
  today = localtime(now);
  sprintf(datestr, (today->tm_year > 99 && today->tm_year < 110 ? "%d/%d/0%d" : "%d/%d/%d"), (today->tm_mon + 1), today->tm_mday, (today->tm_year >= 100 ? (today->tm_year - 100) : (today->tm_year)));

  min_x = max_x = univ.stars[0].x;
  min_y = max_y = univ.stars[0].y;

  for (i = 1; i < univ.nstars; i++)
  {
    if (univ.stars[i].x < min_x)
      min_x = univ.stars[i].x;

    if (univ.stars[i].x > max_x)
      max_x = univ.stars[i].x;

    if (univ.stars[i].y < min_y)
      min_y = univ.stars[i].y;

    if (univ.stars[i].y > max_y)
      max_y = univ.stars[i].y;
  }

  /* max map size: 8.5in x 11in sheet, 0.5in borders, */
  /* 0.5in on the right for star names */
  /* 72 points = 1in */
  scale = 7.0 * 72 / (max_x - min_x);
  nscale = 10.0 * 72 / (max_y - min_y);

  if (nscale < scale)
    scale = nscale;

  /* Recalculate scale so inches will land on the nearest 10000 mark */
  scale = 72 / (myround(((72 / scale) / 10000)) * 10000);

  strcpy(fname, PSFILE);
  expand_file(fname);

  if ((fd = fopen (fname, "w")) == NULL)
  {
                msg ("-- Psmap: Could not open %s for writing.", fname);
                return 0;
  }
  else
  {
    msg ("-- Psmap: Saving Postscript map to %s", fname);
  }

  fprintf (fd, "%%!PS-Adobe-2.1\n");
  fprintf (fd, "%%%%Title: GB Map\n");
  fprintf (fd, "%%%%Creator: psmap\n");
  fprintf (fd, "%%%%DocumentFonts: Times-Roman Times-Bold\n");
  fprintf (fd, "%%%%Pages: 1\n");
  fprintf (fd, "%%%%EndComments\n");
  fprintf (fd, "%%%%BeginProcSet: psmap\n\n");

  fprintf (fd, "%%\n");
  fprintf (fd, "%% Convert inches->points (1/72)\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "/inch { 72 mul } bind def\n\n");

  fprintf (fd, "%%\n");
  fprintf (fd, "%% Fonts\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "/TitleFont /Helvetica-Bold findfont 36 scalefont def\n");
  fprintf (fd, "/LegendFont /Helvetica findfont 15 scalefont def\n");
  fprintf (fd, "/LegendFontB /Helvetica-Bold findfont 15 scalefont def\n");
  fprintf (fd, "/StarFont /Helvetica findfont 8 scalefont def\n");
  fprintf (fd, "/StarFontB /Helvetica-Bold findfont 9 scalefont def\n\n");

  fprintf (fd, "%%\n");
  fprintf (fd, "%% Circle, used for rings\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "/drawcircle\n");
  fprintf (fd, "{\n");
  fprintf (fd, "    newpath\n");
  fprintf (fd, "      0 360 arc\n");
  fprintf (fd, "    stroke\n");
  fprintf (fd, "}\n");
  fprintf (fd, "bind def\n\n");

  fprintf (fd, "%%\n");
  fprintf (fd, "%% Single side of a star\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "/starside\n");
  fprintf (fd, "{\n");
  fprintf (fd, "    6 0 lineto\n");
  fprintf (fd, "    currentpoint translate\n");
  fprintf (fd, "    -144 rotate\n");
  fprintf (fd, "}\n");
  fprintf (fd, "def\n\n");

  fprintf (fd, "%%\n");
  fprintf (fd, "%% Draw star position with name\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "/drawstar\n");
  fprintf (fd, "{\n");
  fprintf (fd, "  /starname exch def\n");
  fprintf (fd, "  /ypos exch def\n");
  fprintf (fd, "  /xpos exch def\n\n");
  fprintf (fd, "  gsave\n");
  fprintf (fd, "    newpath\n");
  fprintf (fd, "    %% Adjust for star offset\n");
  fprintf (fd, "    xpos 3 sub ypos 1 add moveto\n");
  fprintf (fd, "    currentpoint translate\n");
  fprintf (fd, "    4 {starside} repeat\n");
  fprintf (fd, "    closepath\n");
  fprintf (fd, "    fill\n");
  fprintf (fd, "  grestore\n\n");
  fprintf (fd, "  4 xpos add ypos moveto\n");
  fprintf (fd, "  starname show\n");
  fprintf (fd, "}\n");
  fprintf (fd, "def\n\n");

  /* Comment out the above PostScript operator and put this one
     back in if you would rather have points to represent stars -mfw 
  fprintf (fd, "/drawstar\n");
  fprintf (fd, "{\n");
  fprintf (fd, "  /starname exch def\n");
  fprintf (fd, "  /ypos exch def\n");
  fprintf (fd, "  /xpos exch def\n");
  fprintf (fd, "  xpos ypos 2 drawcircle\n");
  fprintf (fd, "  4 xpos add ypos moveto\n");
  fprintf (fd, "  starname show\n");
  fprintf (fd, "}\n");
  fprintf (fd, "def\n\n"); */

  fprintf (fd, "%%\n");
  fprintf (fd, "%% Draw the legend box\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "/drawlegend\n");
  fprintf (fd, "{\n");
  fprintf (fd, "  newpath\n");
  fprintf (fd, "    5.5 inch 12 add -9 inch moveto\n");
  fprintf (fd, "    7.5 inch -9 inch 7.5 inch -10 inch 12 arcto\n");
  fprintf (fd, "    7.5 inch -10 inch 5.5 inch -10 inch 12 arcto\n");
  fprintf (fd, "    5.5 inch -10 inch 5.5 inch -9 inch 12 arcto\n");
  fprintf (fd, "    5.5 inch -9 inch 7.5 inch -9 inch 12 arcto\n");
  fprintf (fd, "  closepath\n");
  fprintf (fd, "  gsave\n");
  fprintf (fd, "    .8 setgray\n");
  fprintf (fd, "    fill\n");
  fprintf (fd, "  grestore\n");
  fprintf (fd, "  stroke\n\n");
  fprintf (fd, "  0 setgray\n\n");
  fprintf (fd, "  5.5 inch 5 add -9 inch -20 add moveto\n");
  fprintf (fd, "  LegendFontB setfont\n");
  fprintf (fd, "  (Galaxy: ) show\n");
  fprintf (fd, "  LegendFont setfont\n");
  fprintf (fd, "  (%s) show\n\n", univ.galaxyname);
  fprintf (fd, "  5.5 inch 5 add -9 inch -40 add moveto\n");
  fprintf (fd, "  LegendFontB setfont\n");
  fprintf (fd, "  (Date: ) show\n");
  fprintf (fd, "  LegendFont setfont\n");
  fprintf (fd, "  (%s) show\n\n", datestr);
  fprintf (fd, "  5.5 inch 5 add -9 inch -60 add moveto\n");
  fprintf (fd, "  LegendFontB setfont\n");
  fprintf (fd, "  (1 in = ) show\n");
  fprintf (fd, "  LegendFont setfont\n");
  fprintf (fd, "  (%.0lf su) show\n", (72 / scale));
  fprintf (fd, "}\n");
  fprintf (fd, "bind def\n\n");

  fprintf (fd, "%%\n");
  fprintf (fd, "%% Draw the title\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "/drawtitle\n");
  fprintf (fd, "{\n");
  fprintf (fd, "  TitleFont setfont\n");
  fprintf (fd, "  0 inch -9 inch -30 add moveto\n");
  fprintf (fd, "  (Galactic Bloodshed) show\n");
  fprintf (fd, "  LegendFont setfont\n");
  fprintf (fd, "  0 inch -9 inch -50 add moveto\n");
  fprintf (fd, "  (gbII 2.6.0 psmap) show\n");
  fprintf (fd, "}\n");
  fprintf (fd, "bind def\n\n");

  fprintf(fd, "%%%%EndProcSet\n");
  fprintf(fd, "%%%%EndProlog\n");
  fprintf(fd, "%%%%Page: 1 1\n\n");

  /* 0,0 is in the topleft corner */
  fprintf (fd, "0.5 inch 10.5 inch translate\n\n");

  fprintf (fd, "drawtitle\n\n");

  fprintf (fd, "drawlegend\n\n");

  fprintf (fd, "0 setlinewidth\n");
  fprintf (fd, "newpath -10 10 moveto 7.5 inch 10 add 10 lineto\n");
  fprintf (fd, "7.5 inch 10 add %d lineto -10 %d lineto closepath clip\n\n",
    (int) ((min_y - max_y) * scale) - 10,
    (int) ((min_y - max_y) * scale) - 10);

  if (univ.highlighted_star >= 0 && univ.highlighted_star < univ.nstars)
  {
    fprintf (fd, "%%\n");
    fprintf (fd, "%% Plot our home star\n");
    fprintf (fd, "%%\n");
    fprintf (fd, "StarFontB setfont\n");

    draw_star (fd, univ.stars + univ.highlighted_star,
      min_x, min_y, scale);

    fprintf (fd, "\n%%\n");
    fprintf (fd, "%% Draw the distance rings\n");
    fprintf (fd, "%%\n");

    for (i = 1; i <= univ.nrings; i++)
    {
      fprintf (fd, "%d %d %d drawcircle\n",
      (int)((univ.stars[univ.highlighted_star].x - min_x)
        * scale),
      (int)((min_y - univ.stars[univ.highlighted_star].y)
        * scale),
      /* Print one every inch instead
      (int) (i * univ.ring_spacing * scale)); */
      (int) (i * 72));
    }
  }

  fprintf (fd, "%%\n");
  fprintf (fd, "%% Plot the other stars\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "StarFont setfont\n");

  for (i = 0; i < univ.nstars; i++)
  {
    if (i != univ.highlighted_star)
      draw_star (fd, univ.stars + i, min_x, min_y, scale);
  }

  fprintf (fd, "\n%%\n");
  fprintf (fd, "%% Print the page\n");
  fprintf (fd, "%%\n");
  fprintf (fd, "showpage\n\n");

  fprintf (fd, "%%%%Trailer\n");

  fclose(fd);

  return 1;
}

int
draw_star (FILE *fd, struct star *star, int min_x, int min_y, double scale)
{
  fprintf (fd, "%d %d (%s) drawstar\n", (int) ((star -> x - min_x)
      * scale),
    (int) ((min_y - star -> y) * scale), star -> name);

  return 1;
}

void
display_psmap (void)
{
  char mbuf[SMABUF];
  char obuf[SMABUF];
  char fname[BUFSIZ];

  msg ("-- Psmap: Finished.");

  sprintf (obuf, "Display using %s? (y/n) ", PSVIEWER);

  promptfor (obuf, mbuf, PROMPT_CHAR);

  if (!YES (*mbuf))
    return;

  strcpy(fname, PSFILE);
  expand_file(fname);
  
  sprintf(mbuf, "%s %s", PSVIEWER, fname);

  cmd_proc(mbuf);
}

/* I had to code this up because round() was not working on Redhat 9 */
double myround (double value)
{
  double foo;
  long bar;

  bar = (long) value;
  foo = value - (double) bar;

  if (foo < 0.5)
    return (double) bar;
  else
    return (double) (bar + 1);
}
