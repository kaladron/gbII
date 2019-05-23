/* fuse.c
 * I don't know who wrote the original command-line fuse.c, I used it as a
 * starting point for this program. Obviously, integrated with gbII, this
 * does quite a bit more. -Michael Wilkinson (12/3/96) */

#include <stdio.h>
#include <math.h>

#include "gb.h"
#include "csp.h"
#include "types.h"

/* #define PLOTTER "/usr/local/bin/gnuplot -mono" */
#define PLOTTER "/usr/bin/gnuplot -mono"
#define FUEL_USE 0.02
#define PAUSE 30

extern char pbuf[];

void cmd_fuse (char *);
void cspr_fuse (int cnum, char *line);
void proc_fuse (void);
void plot_fuse (void);
double calc_fuse (double);
double max_range (double);

struct fuse {
  int shipno;
  char shipname[80];
  int hashyper;
  double tech;
  double mass;
  int maxspeed;
  int speed;
  float fuel;
  long maxfuel;
  int dist;
  int xtal;
  int plot;
  int xterm;
} fuse;

int doing_fuse = FALSE;

void
cmd_fuse(char *args)
{
  int int1 = TRUE;
  int shipno;
  char str[MAXSIZ];

  fuse.speed = 0;

        while (int1)
        {
                promptfor ("Ship Number? ", pbuf, PROMPT_STRING);
                shipno = atoi(pbuf);

                if (shipno > 0)
                {
                        int1 = FALSE;
                }
        }

#ifndef RESTRICTED_ACCESS
  promptfor ("Do you want a plot (y/n) ", pbuf, PROMPT_CHAR);

  if (YES (*pbuf))
  {
    fuse.plot = TRUE;

    promptfor ("Use xterm for display (y/n) ", pbuf, PROMPT_CHAR);

    if (YES (*pbuf))
      fuse.xterm = TRUE;
    else
      fuse.xterm = FALSE;
  }
  else
  {
    fuse.plot = FALSE;
  }

#else
  fuse.plot = FALSE;
#endif

  int1 = TRUE;

  while (int1 && !fuse.plot)
        {
                promptfor ("Distance to Travel? ", pbuf, PROMPT_STRING);
                fuse.dist = atoi(pbuf);

                if (fuse.dist > 0)
                {
                        int1 = FALSE;
                }
        }

  int1 = TRUE;

  while (fuse.dist < 10000 && int1 && !fuse.plot)
        {
    msg("-- Fuse: Short distance, calculating sub-hyper journey.");
                promptfor ("Ship's Speed? ", pbuf, PROMPT_STRING);
                fuse.speed = atoi(pbuf);

                if (fuse.speed > 0 && fuse.speed < 10)
                {
                        int1 = FALSE;
                }
        }

  doing_fuse = TRUE;

        msg ("-- Fuse: Gathering data. Please hold.");

        sprintf(str, "%s %d %d\n", CSP_SERVER, CSP_SHIPDUMP_COMMAND, shipno);
        send_gb (str, strlen (str));

  return;
}

void cspr_fuse (int cnum, char *line)
{
  int d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12; /* dummy variables */
  char *junk;
  int i,j;

  switch (cnum)
  {
    case CSP_SHIPDUMP_GEN:

#ifdef CLIENT_DEVEL
      if (client_devel)
        msg (":: CSP_SHIPDUMP_GEN - %s", line);
#endif

      sscanf(line, "%d %d %d %d %d %d %d %d %s %s",
        &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8,
        junk, fuse.shipname);

      /* PostScript doesn't like the quote marks around the ship name
       * so this little routine removes them. */
		if (!strncmp(fuse.shipname, "\"\"", 2))
      {
        /* delete the name if it was only quote marks */
        strcpy(fuse.shipname, "");
      }
      else
      {
		  for (i = 0; i < strlen(fuse.shipname); i++)
        {
          if (fuse.shipname[i] == '"')
          {
            for (j = i; j < strlen(fuse.shipname); j++)
            {
              fuse.shipname[j] = fuse.shipname[j+1];
            }
          }
        }
        fuse.shipname[j] = '\0';
      }
          
      break;

    case CSP_SHIPDUMP_STATUS:

#ifdef CLIENT_DEVEL
      if (client_devel)
        msg (":: CSP_SHIPDUMP_STATUS - %s", line);
#endif

      sscanf(line, "%d %d %d %d %d %d %lf %d %d %lf %d",
        &fuse.shipno, &fuse.hashyper, &d2, &fuse.xtal, &d3, &d4,
        &fuse.tech, &fuse.maxspeed, &d6, &fuse.mass, &d7);

      break;

    case CSP_SHIPDUMP_STOCK:

#ifdef CLIENT_DEVEL
      if (client_devel)
        msg (":: CSP_SHIPDUMP_STOCK - %s", line);
#endif

      sscanf(line, "%d %d %d %d %d %ld %d %ld %f %ld %d %d %d %d",
        &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &fuse.fuel, &fuse.maxfuel,
        &d9, &d10, &d11, &d12);

      break;

    case CSP_SHIPDUMP_END:

#ifdef CLIENT_DEVEL
      if (client_devel)
        msg (":: CSP_SHIPDUMP_END - %s", line);
#endif

      if (doing_fuse)
        proc_fuse ();

      break;

    default:

#ifdef CLIENT_DEVEL
      if (client_devel)
                                msg (":: CSP_SHIPDUMP %d - %s", cnum, line);
#endif

      break;
  }
}

void
proc_fuse(void)
{
  double fused;

  msg("-- Fuse: Ship: %d, Techlevel: %.2f, Mass: %.2f, Crystal: %s",
  fuse.shipno,fuse.tech,fuse.mass,(fuse.xtal?"mounted.":"unmounted."));

  doing_fuse = FALSE;

  if (fuse.plot)
  {
    plot_fuse();
    return;
  }

  fused = calc_fuse(fuse.dist);

  if (fused > fuse.maxfuel)
    msg("-- Fuse: WARNING - Fuel required exceeds ship's total fuel capacity.");
  else if (fused > fuse.fuel)
    msg("-- Fuse: WARNING - Fuel required exceeds ship's current fuel load.");

  if (fuse.speed)
  {
    msg("-- Fuse: Divide fuel usage by segments and multiply by 2 for evasive manuevers.");
    msg("-- Fuse: Distance to Travel: %d, Speed: %d, Fuel Usage: %.2lf", fuse.dist, fuse.speed, fused);
  }
  else
  {
    msg("-- Fuse: Distance to Travel: %d, Speed: Hyper, Fuel Usage: %.2lf", fuse.dist, fused);
  }
}

void
plot_fuse(void)
{
  double  fuel;
  double  dist;
  double  distfac;
  char obuf[MAXSIZ];
  char tbuf[1024];
  FILE *pfd;
  int foo; double bar;

  if (strlen(fuse.shipname))
    sprintf(obuf, "set title \"Fuel Prediction for %s (#%d)\"\n",
      fuse.shipname, fuse.shipno);
  else
    sprintf(obuf, "set title \"Fuel Prediction for Ship #%d\"\n",
      fuse.shipno);

  if (!fuse.xterm)
    strcat(obuf, "set term dumb feed 79 20\n");

  strcat(obuf, "set key left top\n");
  strcat(obuf, "set xlabel \"Distance\"\n");
  strcat(obuf, "set ylabel \"Fuel Consumed\"\n");
  strcat(obuf, "set data style linespoints\n");
  strcat(obuf, "set grid\n");
//strcat(obuf, "set terminal postscript portrait\n");
//strcat(obuf, "set output \"fuse.ps\"\n");
  sprintf(tbuf, "plot %lf title \"current fuel\", \"-\" using 1:2 title \"%s crystal\"\n",
    fuse.fuel, (fuse.xtal ? "with" : "without"));
  strcat(obuf, tbuf);

  distfac = HYPER_DIST_FACTOR * (fuse.tech + 100.0);

  for (fuel = 0; fuel <= fuse.maxfuel; fuel++)
  {
    dist = max_range(fuel);

    //msg("-- Fuse: dist: %lf, fuel: %lf", dist, fuel);

    sprintf(tbuf, "%.0lf %.2lf\n", dist, fuel);
    strcat(obuf, tbuf);
  }

  strcat(obuf, "e\n");

  if (fuse.xterm)
    sprintf(tbuf, "pause %d\n", PAUSE);
  else
    sprintf(tbuf, "pause 5\n");
  strcat(obuf, tbuf);

  pfd = popen(PLOTTER, "w");

  if (!pfd)
  {
    msg("-- Fuse: cannot open %s, aborting.", PLOTTER);
    return;
  }

  fprintf(pfd, "%s", obuf);
  fflush(pfd);

  if (fuse.xterm)
    msg ("-- Fuse: Displaying plot for %d seconds.", PAUSE);

  pclose(pfd);

  return;
}

double
calc_fuse (double dist)
{
  double distfac;

  if (fuse.speed)
  {
    return (0.5 * pow(fuse.speed, 1.5) * fuse.mass * FUEL_USE);
  }

  distfac = HYPER_DIST_FACTOR * (fuse.tech + 100.0);

  if (fuse.xtal && dist > distfac)
  {
    return (HYPER_DRIVE_FUEL_USE * sqrt(fuse.mass) *
      pow((dist / distfac), 2));
  }
  else
  {
    return (HYPER_DRIVE_FUEL_USE * sqrt(fuse.mass) *
      pow((dist / distfac), 3));
  }
}

double max_range (double fuel)
{
  double dist, distfac;

  if (fuse.speed)
  {
    return pow(fuse.speed, 1.5);
  }

  distfac = HYPER_DIST_FACTOR * (fuse.tech + 100.0);

  if (fuse.xtal && dist > distfac)
  {
    dist = distfac * sqrt((fuel / HYPER_DRIVE_FUEL_USE) / sqrt(fuse.mass));
  }
  else
  {
    dist = distfac * cbrt((fuel / HYPER_DRIVE_FUEL_USE) / sqrt(fuse.mass));
  }

  return dist;
}
