/*
 * icomm.c: handles the protocol for issuing hidden/automatic commands
 *	    that the client uses for other things.
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1991, 1992, 1993
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"

# include <stdio.h>
# include <sys/types.h>
# include <malloc.h>
# include <math.h>                                  

# include "csp.h"
# include "str.h"
# include "vars.h"

# define SCALE 0.01                           

extern int end_msg;
extern int hide_msg;

typedef struct optplanets {
	char name[80];
	long pop;
	long c_inv;
	long n_inv;
	double c_gain;
	double a_gain;
	double der;
} Optplanet;

struct optstruct {
	int display_only;
	long number;
	long invest;
	long thresh;
	long invleft;
	long techgive;
	double technew;
	double techold;
	Optplanet *planets;
} opt = { 0, 0, 0, 0, 0, 0, 0.0, 0.0, (Optplanet *) NULL };


extern int sscanf (const char *, const char *, ...);
extern int strncmp (const char *, const char *, size_t);
extern long int atol (const char *);
int icomm_done_relation (void);
int icomm_done_status (void);
int icomm_issue_command (char *command, int flag);
int icomm_profile (char *s);
int icomm_relation (char *s);
int icomm_status (char *s);
int icomm_valid_csp (int num);
int icomm_valid_csp_end (int num);
int start_command (int val, int flag);
int type_relation (char *s);

init_icommand (void)
{
	icomm.num = 0;
}

init_start_commands (int flag)
{
# ifdef SMART_CLIENT
	start_command (C_PROFILE, 0);
	start_command (C_RELATION, 0);
	start_command (C_DONEINIT, flag);
# endif
}

int start_command (int val, int flag)
{
int indx;

	/* don't add if already on the list */
	for (indx = 0; indx < icomm.num; indx++) {
		if (icomm.list[indx].comm == val &&
		    icomm.list[indx].flag == flag)
			return (FALSE);
	}

	indx = icomm.num;
	if (icomm.num+1 > MAX_ICOMMANDS) {
		msg ("-- Icommands buffer full, could not issue command.");
		return (FALSE);
	}

	icomm.list[indx].comm = val;
	icomm.list[indx].flag = flag;
	icomm.list[indx].state = S_WAIT;
	icomm.list[indx].ignore = FALSE;
	icomm.list[indx].csp_start = 0;
	icomm.list[indx].csp_end = 0;
	switch (val) {
	case C_DONEINIT:
	case C_DONEPROC:
		icomm.list[indx].prompt = INTERNAL_PROMPT;
		break;
	case C_RNEWS:
		icomm_issue_command ("read news", 0);
		icomm.list[indx].prompt = LEVEL_PROMPT;
		break;
	case C_TELEGRAM:
		icomm_issue_command ("read", 0);
		icomm.list[indx].prompt = LEVEL_PROMPT;
		break;
# ifdef SMART_CLIENT
	case C_RWHO:
		if (rwho.on) {
			icomm_issue_command ("who", 0);
			icomm.list[indx].prompt = LEVEL_PROMPT;
			start_rwho ();
		} else {
			return (TRUE);
		}
		break;
	case C_PROFILE:
		icomm_issue_command ("profile", 0);
		icomm.list[indx].prompt = LEVEL_PROMPT;	
		icomm.list[indx].csp_start = CSP_PROFILE_INTRO;
		icomm.list[indx].csp_end = CSP_PROFILE_END;
		break;
	case C_RELATION:
		icomm_issue_command ("relation", 0);
		icomm.list[indx].prompt = LEVEL_PROMPT;
		icomm.list[indx].csp_start = CSP_RELATION_INTRO;
		icomm.list[indx].csp_end = CSP_RELATION_END;
		break;	
	case C_STATUS:
		icomm_issue_command ("status", 0);
		icomm.list[indx].prompt = LEVEL_PROMPT;
		break;
# endif
	default:
# ifdef CLIENT_DEVEL
		if (client_devel)
			msg (":: Unknown icomm: %d", val);
# endif
		return (FALSE);
		break;
	}
	icomm.num++;
	return (TRUE);
}

void icomm_command_done (void)
{
int i;

	switch (icomm.list[0].comm) {
	case C_DONEINIT:
		if (client_stats == L_INTERNALINIT) {
# ifdef SMART_CLIENT
			msg ("-- Welcome to GB II client. I am fully initialized.");
			if (csp_server_vers)
				msg ("-- Server recognizes CSP Version %d.",
					csp_server_vers);
			msg ("-- You are [%2d,%2d] of %d players.",
				profile.raceid, profile.govid,
				cur_game.maxplayers);
# else
			msg ("-- Welcome to GB II client.");
# endif
			msg ("");
		} else if (client_stats == L_REINIT) {
			msg ("-- I am reinitialized from the %s.",
				(icomm.list[0].flag == 1 ? "update" :
				 icomm.list[0].flag == 2 ? "segment" :
				 "reset or unknown server action"));
		}
		client_stats = L_ACTIVE;
		break;
	case C_DONEPROC:
		break;
	case C_RNEWS:
		print_news ();
		break;
	case C_TELEGRAM:
		break;
# ifdef SMART_CLIENT
	case C_PROFILE:
		break;
	case C_RELATION:
		icomm_done_relation ();
		break;
	case C_RWHO:
		done_rwho ();
		break;
	case C_STATUS:	
		icomm_done_status ();
		break;
# endif
	default:
		msg ("-- BAD command value. Report %d to author.", 
			icomm.list[0].comm);
		break;
	}
	if (--icomm.num) {
		for (i = 1; i <= icomm.num; i++)
			icomm.list[i-1] = icomm.list[i];	
		if (icomm.list[0].comm < C_NONE)
			(void) icomm_command_done ();
	}
}

int icomm_valid_csp (int num)
{
debug (1, "icomm_vlaid_csp: start=%d, end=%d, num=%d",
	icomm.list[0].csp_start, icomm.list[0].csp_end, num);
	if (icomm.list[0].csp_start <= num && icomm.list[0].csp_end >= num)
		return (TRUE);
	return (FALSE);
}

int icomm_valid_csp_end (int num)
{
	return ((icomm.list[0].csp_end == num) ? TRUE : FALSE);
}


icomm_handling_command (char *s)
{
char temp[MAXSIZ];

	if (!s || !*s)
		return;

	debug (4, "icomm handle: %s", s);
	switch (icomm.list[0].comm) {
	case C_DONEPROC:
	case C_DONEINIT:
		break;
	case C_RNEWS:
		if (icomm.list[0].state == S_WAIT)
			check_news (s);
		else
			add_news (s);
		break;
	case C_TELEGRAM:
		if (icomm.list[0].state == S_WAIT) {
			if (streqrn (s, "Telegrams:")) {
				icomm.list[0].state = S_PROC;
				icomm.list[0].ignore = FALSE;
				msg_type = MSG_TELEGRAMS;
			}
		} else {
			/* nothing right now */
			msg_type = MSG_TELEGRAMS;
		}
		break;
# ifdef SMART_CLIENT
	case C_PROFILE:
		icomm_profile (s);
		break;
	case C_RELATION:
		icomm_relation (s);
		break;
	case C_STATUS:
		icomm_status (s);
		break;
# ifdef RWHO
	case C_RWHO:
		icomm_rwho (s);
		break;	
# endif
# endif
	default:
		msg ("Funny command in handling_command %d",
			icomm.list[0].comm);
		break;
	}
}

icomm_issue_command (char *command, int flag)
{
	send_gb (command, strlen (command));

	if (flag) {
		hide_msg++;
	}
}

/* The individual routines called by the previous routing functions. */
# ifdef SMART_CLIENT
icomm_profile (char *s)
{

	if (ICOMM_STATE == S_WAIT) {
		if (streqrn (s, "--==** Racial profile for ")) {
			ICOMM_STATE = S_PROC;
			ICOMM_IGNORE = TRUE;
			profile.player_type = 0;
			profile.capital = 0;
		}
		return;
	}
	if (streqrn (s, "Default Scope:")) {
		strcpy (profile.defscope, s+15);
	} else if (streqrn (s, "*** Diety Status ***")) {
		profile.player_type = 1;
	} else if (streqrn (s, "NO DESIGNATED CAPITAL!!")) {
		profile.capital = 0;
		sscanf (s,
			"NO DESIGNATED CAPITAL!!\t\t\tRanges:     guns:   %d",
			&profile.ranges.guns);
	} else if ((sscanf (s,
		"Designated Capital: #%d\t\t\tRanges:     guns:   %d",
		&profile.capital, &profile.ranges.guns) == 2)) {
	} else if ((sscanf (s,
		"Morale: %6d\t\t\t\t\t    space:  %d", 
		&profile.raceinfo.morale, &profile.ranges.space) == 2)) {
	} else if ((sscanf (s,
		"Updates active: %d\t\t\t\t    ground: %d",
		&profile.updates_active, &profile.ranges.ground) == 2)) {
	} else if ((sscanf (s,
		"Up%*s active: %d\t\t\t\t    ground: %d",
		&profile.updates_active, &profile.ranges.ground) == 2)) {
	} else if (streqrn (s, "Mesomorphic Race") ||
	  streqrn (s, "Biomorph Race") ||
	  streqrn (s, "Metamorphic Race")) {
                profile.raceinfo.racetype = RACE_MESO;
        } else if (streqrn (s, "Normal Race")) {
                profile.raceinfo.racetype = RACE_NORM;
        } else if ((sscanf (s, "\t\t\t  Temp:\t%d", &profile.planet.temp) == 1)) {
	} else if ((sscanf (s, "Fert: %d%%", &profile.raceinfo.fert) == 1)) {
        } else if ((sscanf (s,
                "Rate:    %lf\t\t  methane  %5d%%\t     ocean    . %d%%",
                &profile.raceinfo.birthrate, &profile.planet.methane, &profile.sector.ocean) == 3)) {
		sector_type[SECTOR_OCEAN].sectc = '.';
		sector_type[SECTOR_OCEAN].compat = profile.sector.ocean;
        } else if ((sscanf (s,
                "Mass:    %lf\t\t  oxygen   %5d%%\t      gaseous  ~ %d%%",
                &profile.raceinfo.mass, &profile.planet.oxygen, &profile.sector.gas) == 3)) {
		sector_type[SECTOR_GAS].sectc = '~';
		sector_type[SECTOR_GAS].compat = profile.sector.gas;
        } else if ((sscanf (s,                                              
                "Fight:   %d\t\t  helium   %5d%%\t      ice      # %d%%",
                &profile.raceinfo.fight, &profile.planet.helium, &profile.sector.ice) == 3)) {
		sector_type[SECTOR_ICE].sectc = '#';
		sector_type[SECTOR_ICE].compat = profile.sector.ice;
        } else if ((sscanf (s,                                       
                "Metab:   %lf\t\t  nitrogen   %5d%%\t      mountain ^ %d%%",
                &profile.raceinfo.metab, &profile.planet.nitrogen, &profile.sector.mtn) == 3)) {
		sector_type[SECTOR_MTN].sectc = '^';
		sector_type[SECTOR_MTN].compat = profile.sector.mtn;
        } else if ((sscanf (s,                                       
                "Sexes:   %d\t\t  CO2        %5d%%\t      land     * %d%%",
                &profile.raceinfo.sexes, &profile.planet.co2, &profile.sector.land) == 3)) {   
		sector_type[SECTOR_LAND].sectc = '*';
		sector_type[SECTOR_LAND].compat = profile.sector.land;
        } else if ((sscanf (s,                                       
                "Explore: %d %%\t\t  hydrogen   %5d%%\t      desert   - %d%%",
                &profile.raceinfo.explore, &profile.planet.hydrogen, &profile.sector.desert) == 3)) {
		sector_type[SECTOR_DESERT].sectc = '-';
		sector_type[SECTOR_DESERT].compat = profile.sector.desert;
        } else if ((sscanf (s,                                       
                "Avg Int: %lf\t\t  sulfer     %5d%%\t      forest   \" %d%%",
                &profile.raceinfo.iq, &profile.planet.sulfur, &profile.sector.forest) == 3)) {   
		sector_type[SECTOR_FOREST].sectc = '"';
		sector_type[SECTOR_FOREST].compat = profile.sector.forest;
        } else if ((sscanf (s,                                       
                "Tech:    %lf\t\t  other      %5d%%\t      plated   o %d%%",
                &profile.raceinfo.tech, &profile.planet.other, &profile.sector.plated) == 3)) {   
		sector_type[SECTOR_PLATED].sectc = 'o';
		sector_type[SECTOR_PLATED].compat = profile.sector.plated;
	} else if (streqrn (s, "Personal: ") || 
		streqrn (s, "Discoveries:")) {
		return;
	} else {
		msg ("-- craziness in profile: %s", s);
	}
} /* end handle_profile */
# endif

# ifdef SMART_CLIENT
icomm_relation (char *s)
{
int id;
int meso;
int know;
char name[200];
char yours[20];
char theirs[20];
char *p;

	if (ICOMM_STATE == S_WAIT) {
		if (streqrn (s, "              Racial Relations Report for ")) {
			ICOMM_STATE = S_PROC;
			ICOMM_IGNORE = TRUE;
		}
		return;
	}

	if (streqrn (s, " # ") || streqrn (s, " - ")) 
		return;

	if ((sscanf (s, "%2d  Morph (%3d%%)%[^:]: %s %s",
		&id, &know, name, yours, theirs) == 5) ||
	   (sscanf (s, "%2d  Biom (%3d%%)%[^:]: %s %s",
		&id, &know, name, yours, theirs) == 5))
		meso = RACE_MESO;
	else if ((sscanf (s, "%2d  Unknown (%3d%%)%[^:]: %s %s",
		&id, &know, name, yours, theirs) == 5))
		meso = RACE_UNKNOWN;
	else if ((sscanf (s, "%2d  Normal (%3d%%)%[^:]: %s %s",
		&id, &know, name, yours, theirs) == 5))
		meso = RACE_NORM;
	/* old format */
	else if ((sscanf (s, "%2d       (%3d%%)%[^:]: %s %s",
		&id, &know, name, yours, theirs) == 5))
		meso = RACE_NORM;
	else {
		msg ("-- craziness in relation: %s", s);
		return;
	}
	name[strlen (name) - 1] = '\0';
	p = skip_space (name);
	strcpy (races[id].name, p);
	races[id].you_to_them = type_relation (yours);
	races[id].them_to_you = type_relation (theirs);
	races[id].type = meso;
	cur_game.maxplayers = id;
}

icomm_done_relation (void)
{
        if (profile.raceid > cur_game.maxplayers)
                cur_game.maxplayers = profile.raceid;
        if (cur_game.maxplayers > MAX_NUM_PLAYERS) {
                msg ("-- WARNING: This game has more players (%d) than the client (%d) is compiled to handle. Adjusting to client amount.", cur_game.maxplayers, MAX_NUM_PLAYERS);
                cur_game.maxplayers = MAX_NUM_PLAYERS;
        }
}

int type_relation (char *s)
{

	if (streq (s, "ALLIED"))
		return (RELATION_ALLIED);
	else if (streq (s, "WAR"))
		return (RELATION_ENEMY);
	else
		return (RELATION_NEUTRAL);
}
# endif

# ifdef SMART_CLIENT
/* process incoming lines */
icomm_status (char *s)
{
	if (ICOMM_STATE == S_WAIT) {
		if (streqrn (s, "             ========== Technology Report ==========")) {
			ICOMM_STATE = S_PROC;
			ICOMM_IGNORE = TRUE;
		}
		return;
	}

	if (streqrn (s, "Tech:") || streqrn (s, "       Total Popn:") ||
	   streqrn (s, "       Planet          popn    invest    gain   ^gain"))
	    return;

	if (sscanf (s, "%79s %d %d %lf %lf",
	   opt.planets[opt.number].name,
	   &(opt.planets[opt.number].pop),
	   &(opt.planets[opt.number].c_inv),
	   &(opt.planets[opt.number].c_gain),
	   &(opt.planets[opt.number].a_gain))== 5) {
		if (opt.planets[opt.number].name[strlen (opt.planets[opt.number].name)-1] == '*')
		opt.planets[opt.number].name[strlen (opt.planets[opt.number].name)-1] = '\0';
		opt.number++;
	}

}
# endif

# ifdef OPTTECH
double find_derivative (long int pop, long int inv)
{
double temp;
double der_res;

	temp = (double) pop / 10000.0;
    	der_res = (double) temp / ((inv * temp + 1));
    	return (der_res);
}

void switch_planets (long int a, long int b)
{
Optplanet t;

    	t = opt.planets[a];
    	opt.planets[a] = opt.planets[b];
    	opt.planets[b] = t;
}
# endif 



# ifdef SMART_CLIENT
/* done with status */
icomm_done_status (void)
{
long i;
long j;
long maxnum;
long maxpop = 0;
char out[BUFSIZ];


# ifdef OPTTECH
	/* initiate first derivatives */
	for (i = 0; i < opt.number; i++)
		opt.planets[i].der = find_derivative (opt.planets[i].pop, 1);

	i = 0;

	while (i < opt.number) {
		for(j = i; j < opt.number; j++) {
			if (opt.planets[j].pop > maxpop) {
				maxpop = opt.planets[j].pop;
				maxnum = j;
			}
		}
		switch_planets (i, maxnum);
		maxpop = 0;
		i++;
	}

	for(opt.invleft = opt.invest; opt.invleft > 0; opt.invleft -= opt.techgive) {
		opt.techgive = opt.invleft / opt.number > 1 ?
			opt.invleft / opt.number : 1;
		opt.planets[0].n_inv += opt.techgive;
		opt.planets[0].der = find_derivative (opt.planets[0].pop,
					opt.planets[0].n_inv);
		j = 0;
		while (opt.planets[j].der < opt.planets[j+1].der) {
			switch_planets (j, j+1);
	    		j++;
		}
    	}
    
	msg ("-- opttech report (%d planets):", opt.number);
	for (i = 0; i < opt.number; i++) {
		if (!opt.planets[i].pop) {
			if (!opt.display_only) {
				msg (" [%d] /%s - no popn. ignoring", i,
					opt.planets[i].name);
			}
		} else if (opt.planets[i].n_inv < opt.thresh) {
			if (!opt.display_only) {
				msg ("  [%d] /%s - below threshhold", i,
					opt.planets[i].name);
			}
		} else if (opt.planets[i].n_inv != opt.planets[i].c_inv) {
			if (!opt.display_only) {
				msg ("  [%d] /%s - %ld", i, opt.planets[i].name,
					opt.planets[i].n_inv);
				sprintf (out, "cs /%s\ntech %d\n",
					opt.planets[i].name,
					opt.planets[i].n_inv);
				send_gb (out, strlen (out));
				end_msg += 2;
			}
		} else if (!opt.planets[i].n_inv) {
			if (!opt.display_only)  {
				msg ("  [%d] /%s - no investment", i,
					opt.planets[i].name);
			}
		} else {
			if (!opt.display_only) {
				msg ("  /%s - no change", opt.planets[i].name);
			}
		}
	}

	opt.technew = 0;
	opt.techold = 0;
	for (i = 0; i < opt.number; i++) {
		if (opt.planets[i].n_inv > opt.thresh)
			opt.technew = (double) (opt.technew + SCALE *
				log10 ((double) opt.planets[i].pop *
					(double) opt.planets[i].n_inv
					/ 10000.0 + 1.0));
			opt.techold = (double) (opt.techold + SCALE *
				log10 ((double) opt.planets[i].pop *
					(double) opt.planets[i].c_inv
					/ 10000.0 + 1.0));
	}
	if (end_msg)
		end_msg--;
	msg ("  Old Tech: %2.4f", opt.techold);
	msg ("  New Tech: %2.4f", opt.technew);
	msg ("-- opttech: done");
	free (opt.planets);
# endif
}
# endif

# ifdef OPTTECH
void cmd_opttech (char *s)
{
char fbuf[SMABUF];
char rbuf[BUFSIZ];
char temp[BUFSIZ];
int i;

# ifdef SMART_CLIENT
	split (s, fbuf, rbuf);

	if (!*fbuf) {
		msg ("-- Usage: opttech [-t[otal]] investment [threshhold]");
		return;
	}
	if (streqrn (fbuf, "-total") || streqrn (fbuf, "-t")) {
		opt.display_only = TRUE;
		strcpy (temp, rbuf);
		split (temp, fbuf, rbuf);
	} else
		opt.display_only = FALSE;
	opt.invest = atol (fbuf);
	if (opt.invest < 0) {
		msg ("-- opttech: a positive investment might be useful");
		return;
	}

	opt.number = 0;
	opt.thresh = 0;
	opt.invleft = 0;
	opt.techgive = 0;
	opt.technew = 0;
	opt.techold = 0;

	if (*rbuf) {
		opt.thresh = atol (rbuf);
		if (opt.thresh < 0)
			opt.thresh = 0;
	}


	opt.planets = (Optplanet *) malloc (sizeof (Optplanet) * MAX_NUM_PLANETS);
	if (opt.planets == (Optplanet *) NULL) {
		msg ("-- opttech: Could not malloc memory needed. Abort.");
		return;
	}

	for (i = 0; i < MAX_NUM_PLANETS; i++) {
		opt.planets[i].pop = 0;
		opt.planets[i].c_inv = 0;
		opt.planets[i].n_inv = 0;
	}

	msg ("-- opttech: investment = %ld; threshhold = %ld",
		opt.invest, opt.thresh);
	start_command (C_STATUS, 0);
# else
	msg ("-- opttech requires the client to be compiled with OPTTECH and SMART_CLIENT available");
# endif
}
# endif
