/*
 * imap.c: handles the input and out of the interactive map (Imap)
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

# include "csp.h"
# include "str.h"
# include "term.h"
# include "ansi.h"
# include "vars.h"

# define MIN_MAP_WIN_SIZE	8

# define NO_MAP		0
# define NEW_MAP	1
# define RE_MAP		2
# define HAVE_MAP	3

# ifdef IMAP

struct cursectstruct {
	int x;
	int y;
	int relx;
	int rely;
	int oldx;
	int oldy;
	int leftbound;
	int ping;
	int has_ships;
} cursect = { -1, -1, 0, 0, FALSE, FALSE };

Map imap_map = {
	0, 0, "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, TRUE, FALSE, FALSE, (Sector *) NULL };
long map_time = -1;
char cursor_sector = DEFAULT_CURSOR_SECTOR;

Sector *get_direction (char *c);
Sector *find_ship (int ship, int *pos);
# endif

extern int _filbuf (FILE *);
extern int atoi (const char *);
extern int sscanf (const char *, const char *, ...);
extern time_t time (time_t *);
int handle_map_mode (int nomsgs);
int ping_current_sector (void);
int ping_sector (int x, int y);
int process_client_survey (int cnum, char *s, Map *buildmap);
int redraw_map (void);
int redraw_map_window (void);
int redraw_sector (void);
int set_up_interactive_map (void);
int set_up_map_window (void);
int update_sector (Sector *ptr);

void imap_input (int comm_num, char *procbuf)
{
# ifdef IMAP
	if (!input_mode.map)
		return;

	switch (comm_num) {
	case CSP_SURVEY_INTRO:
		if (imap_map.ptr)
			free (imap_map.ptr);
		process_client_survey (comm_num, procbuf, &imap_map);
		imap_map.ptr = (Sector *) malloc ((sizeof (Sector) 
			* imap_map.maxx * imap_map.maxy));
		if (imap_map.ptr == (Sector *) NULL) {
			msg ("-- Imap: Could not malloc map space. Aborting.");
			handle_map_mode (1);
		}
		if (input_mode.map) {
			if (imap_map.maxx > (num_columns - 35)) {
				msg ("-- Imap: the planet is too big for your screen.");
				handle_map_mode (1);
				free (imap_map.ptr);
				imap_map.ptr = (Sector *) NULL;
				return;
			}	
			set_up_map_window ();
		}
		break;
	case CSP_SURVEY_SECTOR:
		if (!imap_map.ptr)
			return;
		process_client_survey (comm_num, procbuf, &imap_map);
		break;
	case CSP_SURVEY_END:
		if (input_mode.map) {
			if (imap_map.map == NEW_MAP ||
			    imap_map.map == RE_MAP) {
				set_up_interactive_map ();
				imap_map.map = HAVE_MAP;
			}
		}
		break;
	default:
		msg ("-- Unknown imap client_survey #%d", comm_num);
		break;
	}
# endif
}

int doing_imap_command (void)
{
	return (input_mode.map && !input_mode.say);
}

# ifdef IMAP
# define MAP
# endif
# ifdef POPN
# ifndef MAP
# define MAP
# endif
# endif

# ifdef MAP
process_client_survey (int cnum, char *s, Map *buildmap)
{
Sector p;
char *ptr;
char *qtr;
int i;

	switch (cnum) {
	case CSP_SURVEY_INTRO:
		sscanf (s, "%d %d %s %s %d %d %d %d %d %d %lf %d",
			&(buildmap->maxx), &(buildmap->maxy),
			buildmap->star, buildmap->planet,
			&(buildmap->res), &(buildmap->fuel), &(buildmap->des),
			&(buildmap->popn), &(buildmap->mpopn), &(buildmap->tox),
			&(buildmap->compat), &(buildmap->enslaved));
		break;
	case CSP_SURVEY_SECTOR:
		ptr = strchr (s, ';');
		if (ptr) {
			*ptr = '\0';
			ptr++;
		}
		sscanf (s, "%d %d %c %c %d %d %d %d %d %d %d %d %d %d",
			&p.x, &p.y, &p.sectc, &p.des, &p.wasted, &p.own,
			&p.eff, &p.frt, &p.mob, &p.xtal, &p.res, &p.civ,
			&p.mil, &p.mpopn);
		p.numships = 0;
		if (ptr) {
			i = 0;
			qtr = ptr;
			while (qtr && *qtr && (ptr = strchr (qtr, ';')) &&
				i < MAX_SHIPS_IN_SURVEY) {
				*ptr = '\0';
				sscanf (qtr, "%d %c %d", &p.ships[i].shipno,
					&p.ships[i].ltr, &p.ships[i].owner);
				i++;
				qtr = ptr + 1;
			}
			p.numships = i;
		}
		*(buildmap->ptr + p.x + (p.y * buildmap->maxx)) = p;
# ifdef IMAP
		if (input_mode.map && !input_mode.say) {
			if (cursect.ping && cursect.x == p.x &&
			  cursect.y == p.y) {
				redraw_sector ();
				cursect.ping = FALSE;
			}
		}
# endif
		break;
	case CSP_SURVEY_END:
		break;
	default:
		msg ("-- Unknown client_survey #%d", cnum);
		break;
	}
}
# endif
/* MAP */

# ifdef IMAP
set_up_map_window (void)
{
int old_orow = output_row;


	output_row = num_rows - MAX (imap_map.maxy+1, MIN_MAP_WIN_SIZE) - 4;
	last_output_row = output_row;
	/* -2  for status, -1 for map status, -1 to place correctly */
	more_val.num_rows = output_row - 1;
	if (output_row < 1) {
		msg ("-- Imap: not enough rows to display the planet.");
		handle_map_mode (1); /* toggle it off */
		return;
	}
	if (output_row < old_orow)
		term_scroll (0, old_orow, old_orow - output_row);

	cursect.rely = output_row+2;
	cursect.relx = 3;
	if (cursect.x == -1) {
		cursect.x = imap_map.maxx / 2;
		cursect.y = imap_map.maxy / 2;
	}
	cursect.leftbound = imap_map.maxx + 7;

	redraw_map_window ();
	cursect.ping = FALSE;
}

set_up_interactive_map (void)
{
char buf[MAXSIZ];

	redraw_map ();
	redraw_sector ();

	term_move_cursor (cursect.leftbound+2, num_rows-4);
	sprintf (buf, "/%4.4s/%-4.4s", imap_map.star, imap_map.planet);
	term_puts (buf, strlen (buf));

}

redraw_map_window (void)
{
int i;
int len;
char temp[BUFSIZ];

	sprintf (temp,
		"-R:%4d-F:%4d-D:%4d-Pop:%7d-^pop:%7d-Tox:%3d-Comp:%5.1lf",
		imap_map.res, imap_map.fuel, imap_map.des, imap_map.popn,
		imap_map.mpopn, imap_map.tox, imap_map.compat);
	term_move_cursor (0, output_row+1);
	len = strlen (temp);
	if (len > num_columns)
		len = num_columns;
	term_puts (temp, len);
	for (i = num_columns-len; i; i--)
		term_putchar ('-');
	len = 0;
	for (i = output_row +2; i < (num_rows-2); i++) {
		/* put row #'s on right side */
		if (len < imap_map.maxy) {
			term_move_cursor (0, i);
			sprintf (temp, "%d%d", len / imap_map.maxy,
				len % imap_map.maxy); 
			term_puts (temp, 2);
			len++;
		}
		term_move_cursor (imap_map.maxx+7, i);
		term_puts ("|                   |", 21);
	}
	/* bottom map row for #'s */
	term_move_cursor (3, (output_row+imap_map.maxy+2));
	for (i = 0; i < imap_map.maxx; i++) {
		sprintf (temp, "%d", i % 10);
		term_putchar (*temp);
	}
}

clear_map_window (void)
{
int i;

	for (i = MAX (MIN_MAP_WIN_SIZE, imap_map.maxy) + 1; i; i--) {
		term_move_cursor (0, num_rows-2-i);
		term_clear_to_eol ();
	}
}

refresh_map (void)
{
char buf[MAXSIZ];

	redraw_map_window ();
	redraw_map ();
	redraw_sector ();

	if (!input_mode.map)
		return;
	term_move_cursor (cursect.leftbound+2, num_rows-3);
	sprintf (buf, "/%4.4s/%-4.4s", imap_map.star, imap_map.planet);
	term_puts (buf, strlen (buf));
}

redraw_map (void)
{
int y;
int x;
Sector *p = imap_map.ptr;
int inverse = FALSE;
int colored = FALSE;

	if (!input_mode.map)
		return;
	if (imap_map.maxx > (num_columns - 35)) {
		msg ("Imap: planet is too big for your screen.");
		handle_map_mode (1);
		return;
	}

	for (y = 0; y < imap_map.maxy; y++) {
		term_move_cursor (cursect.relx, cursect.rely+y);
		for (x = 0; x < imap_map.maxx; x++, p++)
		{
			if (imap_map.ansi && p->own > 0)
			{
				colored = (p->own % MAX_RCOLORS);
				term_puts(race_colors[colored], 5);
				term_puts(ANSI_FOR_BLACK, 5);
			}

			if (imap_map.geo)
			{
				/* doing geography only */
				if (imap_map.inverse &&
				    p->own == profile.raceid) { /* do inverse */
					if (!inverse) {
						inverse = TRUE;
						term_standout_on ();
					}
				}
				term_putchar (p->sectc);
			}
			else
			{
				/* not showing geography only */
				if (p->own == profile.raceid)
				{
					/* my sect */
					if (imap_map.inverse && !inverse && !colored) {
						inverse = TRUE;
						term_standout_on ();
					}

					if (p->numships) {
						term_putchar (p->ships[0].ltr);
					} else if (p->wasted) {
						term_putchar ('%');
					} else if (p->xtal) {
						term_putchar ('x');
					} else {
						term_putchar (p->sectc);
					}
				} else if (!p->own) {
					if (inverse) {
						term_standout_off ();
						inverse = FALSE;
					}
					if (p->numships) {
						term_putchar (p->ships[0].ltr);
					} else if (p->wasted) {
						term_putchar ('%');
					} else if (p->xtal) {
						term_putchar ('x');
					} else {
						term_putchar (p->sectc);
					}
				} else { /* owned by someone but me */
					if (inverse) {
						term_standout_off ();
						inverse = FALSE;
					}
					if (p->numships) {
						term_putchar (p->ships[0].ltr);
					} else {
						term_putchar ((p->own%10+'0'));
					}
				} /* else owned by someone but me */
			} /* else not showing geo only */

			if (colored)
			{
				term_puts(ANSI_NORMAL, strlen(ANSI_NORMAL));
				colored = FALSE;
			}
		} /* for x loop */
	} /* for y loop */
	if (inverse)
		term_standout_off ();
}

redraw_sector (void)
{
char sectbuf[20];
Sector *p = imap_map.ptr;
Sector *q = imap_map.ptr;
int i;
	if (!input_mode.map)
		return;

	p += cursect.x + (cursect.y * imap_map.maxx);

	term_move_cursor (cursect.leftbound+2, output_row+2);
	sprintf (sectbuf, "%2d,%-2d %c", p->x, p->y, p->sectc);
	term_puts (sectbuf, strlen (sectbuf));
	if (imap_map.enslaved) {
		sprintf (sectbuf, "ENSLAVED-%d", imap_map.enslaved);
		term_puts (sectbuf, strlen (sectbuf));
	}

	term_move_cursor (cursect.leftbound+2, output_row+3);
	if (p->wasted || p->xtal) {
		sprintf (sectbuf, "%s%s",
			(p->xtal ? "CRYSTAL " : ""),
			(p->wasted ? "WASTED" : ""));
		term_puts (sectbuf, strlen (sectbuf));
	} else
		term_puts ("              ", 15);

	term_move_cursor (cursect.leftbound+2, output_row+4);
	sprintf (sectbuf, "own:%3d ^pop:%4d", p->own, p->mpopn);
	term_puts (sectbuf, strlen (sectbuf));

	term_move_cursor (cursect.leftbound+2, output_row+5);
	sprintf (sectbuf, "mil:%3d civ:%5d", p->mil, p->civ);
	term_puts (sectbuf, strlen (sectbuf));

	term_move_cursor (cursect.leftbound+2, output_row+6);
	sprintf (sectbuf, "res:%3d frt:%5d", p->res, p->frt);
	term_puts (sectbuf, strlen (sectbuf));

	term_move_cursor (cursect.leftbound+2, output_row+7);
	sprintf (sectbuf, "eff:%3d mob:%5d", p->eff, p->mob);
	term_puts (sectbuf, strlen (sectbuf));

	if (p->numships) {
		cursect.has_ships = TRUE;
		for (i = 0; (i < MAX (imap_map.maxy, MIN_MAP_WIN_SIZE))
			&& i < MAX_SHIPS_IN_SURVEY; i++) {
			term_move_cursor (cursect.leftbound+22, i+output_row+2);
			if (i < p->numships) {
				sprintf (sectbuf, "%2d %c %4d",
					p->ships[i].owner,
					p->ships[i].ltr,
					p->ships[i].shipno);
				term_puts (sectbuf, strlen (sectbuf));
			} else
				term_clear_to_eol ();
		}
	} else {
		if (cursect.has_ships) {
			for (i = 0; (i < MAX (imap_map.maxy, MIN_MAP_WIN_SIZE))
				&& i < MAX_SHIPS_IN_SURVEY; i++) {
				term_move_cursor (cursect.leftbound+22,
					i+output_row+2);
				term_clear_to_eol ();
			}
			cursect.has_ships = FALSE;
		}
	}

	/* remove old highlight */
	if (cursect.oldx >= 0) {
		q += cursect.oldx + (cursect.oldy * imap_map.maxx);
		update_sector (q);
	}

	/* reset old sect values */
	cursect.oldx = cursect.x;
	cursect.oldy = cursect.y;

	term_move_cursor (cursect.relx+p->x, cursect.rely+p->y);
	term_putchar (cursor_sector);

	map_time = -1;
}

update_sector (Sector *ptr)
{

	term_move_cursor (cursect.relx+ptr->x, cursect.rely+ptr->y);

	if (imap_map.geo) {
		if (imap_map.inverse && ptr->own == profile.raceid) {
			term_standout_on ();
			term_putchar (ptr->sectc);
			term_standout_off ();
		} else {
			term_putchar (ptr->sectc);
		}
	} else { /* not geo */
		if (ptr->own == profile.raceid) { /* my sect */
			if (imap_map.inverse) {
				term_standout_on ();
			}
			if (ptr->numships) {
				term_putchar (ptr->ships[0].ltr);
			} else if (ptr->wasted) {
				term_putchar ('%');
			} else if (ptr->xtal) {
				term_putchar ('x');
			} else {
				term_putchar (ptr->sectc);
			}
			if (imap_map.inverse) {
				term_standout_off ();
			}
		} else if (!ptr->own) {
			if (ptr->numships) {
				term_putchar (ptr->ships[0].ltr);
			} else if (ptr->wasted) {
				term_putchar ('%');
			} else if (ptr->xtal) {
				term_putchar ('x');
			} else {
				term_putchar (ptr->sectc);
			}
		} else { /* owned by someone but me */
			if (ptr->numships) {
				term_putchar (ptr->ships[0].ltr);
			} else {
				term_putchar ((ptr->own%10+'0'));
			}
		} /* else owned by someone but me */
	} /* else not showing geo only */
}

void imap_test (void)
{
		
}

handle_map_mode (int nomsgs)
{
	if (!input_mode.map) {
		if (scope.level != LEVEL_PLANET) {
			msg ("-- You must be at planet scope for interactive map.");
			return;
		}
		input_mode.map = TRUE;
		input_mode.say = FALSE;
		imap_map.map = NEW_MAP;
		cursect.x = -1;
		cursect.oldx = -1;
		if (GBDT ())
			csp_send_request (CSP_SURVEY_COMMAND, "-");
		else
			send_gb ("client_survey -\n", 16);
		msg ("-- Interactive map activated.");
	} else {
		input_mode.map = FALSE;
		output_row = num_rows - 3;
		last_output_row = output_row;
		cursect.x = -1;
		cursect.oldx = -1;
		more_val.num_rows = output_row;
		if (!nomsgs)
			msg ("-- Interactive map turned off.");
		scroll_output_window ();
	}
	force_update_status ();
}

cursor_to_map (void)
{
	term_move_cursor (cursect.relx+cursect.x, cursect.rely+cursect.y);
}

void imap_map_mode (void)
{
	handle_map_mode (0);
}

void imap_say_mode (void)
{

	if (input_mode.map) {
		if (!input_mode.say) {
			input_mode.say = TRUE;
			msg ("-- Say mode activated");
		} else {
			input_mode.say = FALSE;
			msg ("-- Say mode deactivated");
		}
		ping_current_sector ();
		force_update_status ();
	}
}

map_move_down (void)
{
	if (cursect.y+1 == imap_map.maxy)
		cursect.y = 0;
	else
		cursect.y++;
}

map_move_up (void)
{
	if (cursect.y == 0)
		cursect.y = imap_map.maxy - 1;
	else
		cursect.y--;
}

map_move_right (void)
{
	if (cursect.x == imap_map.maxx - 1)
		cursect.x = 0;
	else
		cursect.x++;
}

map_move_left (void)
{
	if (cursect.x == 0)
		cursect.x = imap_map.maxx - 1;
	else
		cursect.x--;
}

void imap_mover (void)
{
Sector *p = imap_map.ptr;
Sector *q;
char buf[MAXSIZ];
char c;
double apcost;
int amt;

	p += cursect.x + (cursect.y * imap_map.maxx); /* from pos */
	if (p->own != profile.raceid) {
		msg ("You don't own that sector!");
		return;
	}
	q = get_direction (&c);
	if (q == NULL)
		return;

	promptfor ("# of people? ", buf, PROMPT_STRING);
	amt = atoi (buf);

	if (!amt)
		return;

	if (p->civ >= amt) {
		if (p->own && q->own && p->own != q->own)
			apcost = (int)log (1.0 + (double)amt) + 2;
		else
			apcost = (int)log (1.0 + (double)amt) + 1;
		if (apcost > scope.aps) {
			msg ("You don't have enough APs. Need %.lf",
				apcost);
			return;
		}
		if (amt < 0) {
			q->civ = p->civ + amt;
			p->civ = -amt;
		} else {
			p->civ -= amt;
			q->civ += amt;
		}
		sprintf (buf, "move %d,%-d %c %d\n", p->x, p->y, c, amt);
		send_gb (buf, strlen (buf));
		if (!q->own)
			q->own = p->own;
		update_sector (q);
		if (!p->mil && !p->civ)
			p->own = 0;
		redraw_sector ();
	} else {
		msg ("Not enough people for me to move.");
		return;
	}
	
}
void imap_deploy (void)
{
char buf[MAXSIZ];
char c;
int amt;
double apcost;
Sector *p = imap_map.ptr;
Sector *q;

	p += cursect.x + (cursect.y * imap_map.maxx); /* from pos */
	if (p->own != profile.raceid) {
		msg ("You don't own that sector!");
		return;
	}
	q = get_direction (&c);
	if (q == NULL)
		return;

	promptfor ("# of troops? ", buf, PROMPT_STRING);
	amt = atoi (buf);

	if (!amt)
		return;

	if (p->mil >= amt) {
		if (p->own && q->own && p->own != q->own)
			apcost = (int)log10 (1.0 + (double) amt) + 2;
		else
			apcost = (int)log10 (1.0 + (double) amt) + 1;
		if (apcost > scope.aps) {
			msg ("You don't have enough APs. Need %.lf",
				apcost);
			return;
		}
		if (amt < 0) {
			q->mil = p->mil + amt;
			p->mil = -amt;
		} else {
			p->mil -= amt;
			q->mil += amt;
		}
		if (!p->mil && !p->civ)
			p->own = 0;
		sprintf (buf, "deploy %d,%-d %c %d\n", p->x, p->y, c, amt);
		send_gb (buf, strlen (buf));
		redraw_sector ();
	} else {
		msg ("Not enough troops for me to move.");
		return;
	}
	
}

Sector *get_direction (char *c)
{
Sector *q = imap_map.ptr;
int newx;
int newy;

	promptfor ("direction? ", c, PROMPT_CHAR);

	switch (*c) {
	case 'b': *c = '1';
	case '1':
		newx = -1;
		newy = 1;
		break;
	case 'j': *c = '2';
	case '2':
		newx = 0;
		newy = 1;
		break;
	case 'n': *c = '3';
	case '3':
		newx = 1;
		newy = 1;
		break;
	case 'h': *c = '4';
	case '4':
		newx = -1;
		newy = 0;
		break;
	case ' ':
	case '\n':
	case '5':
		return ((Sector *) NULL);
	case 'l': *c = '6';
	case '6':
		newx = 1;
		newy = 0;
		break;
	case 'y': *c = '7';
	case '7':
		newx = -1;
		newy = -1;
		break;
	case 'k': *c = '8';
	case '8':
		newx = 0;
		newy = -1;
		break;
	case 'u': *c = '9';
	case '9':
		newx = 1;
		newy = -1;
		break;
	default:
		msg ("Invalid direction. Aborting. |%c|", *c);
		return ((Sector *) NULL);
	}
	if ((cursect.y + newy) == -1 || (cursect.y + newy) == imap_map.maxy) {
		msg ("Invalid move. Aborting.");
		return ((Sector *) NULL);
	}
	if ((cursect.x + newx) == imap_map.maxx) {
		q += ((cursect.y + newy) * imap_map.maxx);
	} else if ((cursect.x + newx) == -1) {
		q += (imap_map.maxx - 1) + ((cursect.y + newy) * imap_map.maxx);
	} else
		q += (cursect.x + newx) + ((cursect.y + newy) * imap_map.maxx);
	return (q);
}

void imap_zoom_sector (void)
{
int x;
int y;
char buf[MAXSIZ];

	promptfor ("Goto to where(x,y)? ", buf, PROMPT_STRING);
	if (sscanf (buf, "%d,%d", &x, &y) != 2)
		return;

	if ((x >= 0 && x < imap_map.maxx) && ( y >= 0 && y < imap_map.maxy)) {
		cursect.oldx = cursect.x;
		cursect.oldy = cursect.y;
		cursect.x = x;
		cursect.y = y;
		redraw_sector ();
	} else
		msg ("Invalid coordinates.");
}

void imap_capture_ship (void)
{
int ship;
int boarders;
char type;
Sector *p;
int pos;
char buf[MAXSIZ];

	promptfor ("ship number: ", buf, PROMPT_STRING);
	ship = atoi (buf);

	if (!ship)
		return;

	p = find_ship (ship, &pos);
	if (!p && pos == -1) {
		msg ("-- No such ship #%d on planet. Imap may need updating.",
			ship);
		return;
	}

	if (p->own != profile.raceid) {
		msg ("-- You don't own the sector ship #%d is landed on.",
			ship);
		return;
	}

	promptfor ("number of boarders? ", buf, PROMPT_STRING);
	boarders = atoi (buf);	

	if (boarders < 0) {
		msg ("-- Capture: aborting.");
		return;
	}

	promptfor ("(c)iv/(m)il? ", &type, PROMPT_CHAR);

	if (type != 'c' && type != 'm') {
		msg ("-- Capture: Invalid type for boarders-%c. Aborting.",
			type);
		return;
	}

	sprintf (buf, "capture %d %d %s\n",
		ship, boarders, (type == 'c' ? "civ" : "mil"));
	send_gb (buf, strlen (buf));
	ping_sector (p->x, p->y);
}

void imap_launch_ship (void)
{
int pos;
int ship;
Sector *p;
char buf[MAXSIZ];

	promptfor ("ship number: ", buf, PROMPT_STRING);
	ship = atoi (buf);

	if (!ship)
		return;

	p = find_ship (ship, &pos);
	if (!p && pos == -1) {
		msg ("-- Can't find ship #%d to launch.", ship);
		ping_current_sector ();
		return;
	}
	if (p->ships[pos].owner != profile.raceid) {
		msg ("-- You don't own ship #%d.", p->ships[pos].shipno);
		return;
	}

	sprintf (buf, "undock %d\n", ship);
	send_gb (buf, strlen(buf));
}

void imap_fire (void)
{
int tship;
int fship;
int str;
char buf[MAXSIZ];

	promptfor ("fire ship#: ", buf, PROMPT_STRING);
	tship = atoi (buf);

	while (getchar () != '\n') ;
	if (tship <= 0)
		return;

	promptfor ("at ship#: ", buf, PROMPT_STRING);
	fship = atoi (buf);

	if (fship <= 0)
		return;

	promptfor ("#guns: ", buf, PROMPT_STRING);
	str = atoi (buf);

	while (getchar () != '\n') ;
	if (str == 0)
		return;

	if (str > 0)
		sprintf (buf, "fire %d %d %d\n", tship, fship, str);
	else
		sprintf (buf, "fire %d %d\n", tship, fship);
	send_gb (buf, strlen (buf));
}

map_bombard_or_defend (int mode)
{
int ship;
int str;
char buf[MAXSIZ];

	promptfor ("ship#: ", buf, PROMPT_STRING);
	ship = atoi (buf);

	if (ship <= 0)
		return;

	promptfor ("#guns: ", buf, PROMPT_STRING);
	str = atoi (buf);

	if (str == 0)
		return;

	if (str > 0)
		sprintf (buf, "%s %d %d,%-d %d\n",
			(mode ? "defend" : "bombard"), ship,
			cursect.x, cursect.y, str);
	else
		sprintf (buf, "%s %d %d,%-d\n",
			(mode ? "defend" : "bombard"), ship,
			cursect.x, cursect.y);
	send_gb (buf, strlen (buf));
}

void imap_land_ship (void)
{
int ship;
char buf[MAXSIZ];

	promptfor ("ship#: ", buf, PROMPT_STRING);
	ship = atoi (buf);

	sprintf (buf, "land %d %d,%-d\n", ship, cursect.x, cursect.y);
	send_gb (buf, strlen (buf));
	ping_sector (cursect.x, cursect.y);
}

ping_current_sector (void)
{
	if (scope.level != LEVEL_PLANET)
		return;
	ping_sector (cursect.x, cursect.y);
	cursect.ping = TRUE;
}

ping_sector (int x, int y)
{
char pingbuf[25];

	if (scope.level != LEVEL_PLANET)
		return;
	if (GBDT ()) {
		sprintf (pingbuf, "%d,%-d", x, y);
		csp_send_request (CSP_SURVEY_COMMAND, pingbuf);
	} else {
		sprintf (pingbuf, "client_survey %d,%-d\n", x, y);
		send_gb (pingbuf, strlen (pingbuf));
	}
}

map_complex_move (int mode)
{
char mbuf[BUFSIZ];
char buf[MAXSIZ];

	promptfor ("Move: ", mbuf, PROMPT_STRING);

	if (mode)
		sprintf (buf, "move %d,%-d %s\n", cursect.x, cursect.y, mbuf);
	else
		sprintf (buf, "move %s\n", mbuf);
	send_gb (buf, strlen (buf));
}

Sector *find_ship (int ship, int *pos)
{
int j;
int i;
Sector *p = imap_map.ptr;

	j = 0;
	i = imap_map.maxx * imap_map.maxy;
	for ( ; i && (p->ships[j].shipno != ship); i--, p++)
		for (j = 0; j < MAX_SHIPS_IN_SURVEY &&
		    (p->ships[j].shipno != ship); j++)
			;
	if (i == 0 && j == MAX_SHIPS_IN_SURVEY) {
		*pos = -1;
		return ((Sector *) NULL);
	}
	*pos = j;
	return (p);
}

int invalid_map_screen_sizes (void)
{

int leny;
int lenx;

	if ((MAX (imap_map.maxy, MIN_MAP_WIN_SIZE) >= output_row) ||
	    (imap_map.maxx > (num_columns - 35))) {
		msg ("-- Imap: screen sizes will not work for the planet.");
		return (1);
	}
	return (0);
}

void imap_complex_move (void)
{
	map_complex_move (0);
	if (GBDT ())
		csp_send_request (CSP_SURVEY_COMMAND, "-");
	else
		send_gb ("client_survey -\n", 16);
}

void imap_default (void)
{
	 msg ("-- Imap: map mode - Hit <ESC> s to type");
}

void imap_bombard (void)
{
	map_bombard_or_defend (0);
}

void imap_defend (void)
{
	map_bombard_or_defend (1);
}

void imap_move_sw (void)
{
	map_move_down ();
	map_move_left ();
	map_time = time (0);
}

void imap_move_s (void)
{
	map_move_down ();
	map_time = time (0);
}

void imap_move_se (void)
{
	map_move_down ();
	map_move_right ();
	map_time = time (0);
}

void imap_move_e (void)
{
	map_move_right ();
	map_time = time (0);
}

void imap_move_ne (void)
{
	map_move_up ();
	map_move_right ();
	map_time = time (0);
}

void imap_move_n (void)
{
	map_move_up ();
	map_time = time (0);
}

void imap_move_nw (void)
{
	map_move_up ();
	map_move_left ();
	map_time = time (0);
}

void imap_move_w (void)
{
	map_move_left ();
	map_time = time (0);
}

void imap_toggle_inverse (void)
{
	imap_map.inverse = (imap_map.inverse + 1) % 2;
	msg ("-- Imap: inverse mode %s", (imap_map.inverse ? "on" : "off"));
	redraw_map ();
	redraw_sector ();
}

void imap_toggle_geography (void)
{
	imap_map.geo = (imap_map.geo + 1) % 2;
	msg ("-- Imap: geography mode %s", (imap_map.geo ? "on" : "off"));
	redraw_map ();
	redraw_sector ();
}

void imap_toggle_ansi (void)
{
	imap_map.ansi = (imap_map.ansi + 1) % 2;
	msg ("-- Imap: ansi color %s", (imap_map.ansi ? "on" : "off"));
	redraw_map ();
	redraw_sector ();
}

void imap_force_redraw (void)
{
	if (scope.level != LEVEL_PLANET) {
		msg ("-- You are NOT at planet scope.");
		return;
	}
	clear_map_window ();
	imap_map.map = RE_MAP;
	cursect.x = -1;
	if (GBDT ())
		csp_send_request (CSP_SURVEY_COMMAND, "-");
	else
		send_gb ("client_survey -\n", 16);
}

map_prompt_force_redraw (void)
{
	if (streq (scope.star, imap_map.star) &&
	    streq (scope.planet, imap_map.planet))
		return;
	(void) imap_force_redraw ();
}

void imap_ping_sector (void)
{
	ping_current_sector ();
}

# endif
