/*
 * bind.c: handles the key bindings and interpretation
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1992
 *
 * See the COPYRIGHT file.
 */

# include "bind.h"

static int bind_changed = FALSE;
char more_ch;

char *display_char (char c, int mode);
char convert_string (char *s, int *mode);
int list_binding (int listkey, int mode);

extern int strncmp (const char *, const char *, size_t);
extern void free (void *);
extern int fprintf (FILE *, const char *, ...);

void cmd_bind (char *args)
{
char key[SMABUF];
int bindkey;
char rest[MAXSIZ];
int numnames;
int i;
int mode = 0;

	split (args, key, rest);
	if (!*key) {
		list_binding (-1, 0);
		return;
	}
	bindkey = convert_string (key, &mode);
	if (mode == -1) {
		msg ("-- Bind: unknown character '%s'", key);
		return;
	}
	if (!*rest) {
		list_binding (bindkey, mode);
		return;
	}
# ifdef IMAP
	if (streqrn (rest, "imap_") && mode != BIND_IMAP) {
		msg ("-- Bind: '%s' can only be bound to Imap keys.", rest);
		return;
	}
# endif
	if (streqrn (rest, "more_") && mode != BIND_MORE) {
		msg ("-- Bind: '%s' can only be bound to more keys.", rest);
		return;
	}

	if (streq (rest, "default")) {
		switch (mode) {
		case BIND_NORM:
			strcpy (rest, "default_for_input");
			break;
		case BIND_ESC:
			strcpy (rest, "default_for_escape");
			break;
		case BIND_IMAP:	
			strcpy (rest, "default_for_imap");
			break;
		case BIND_MORE:
			break;
		}
		i = -1;
	}

	numnames = sizeof (BindNames) / sizeof (BindName);
	for (i = 0; i < numnames; i++)
		if (streq (rest, BindNames[i].name))
			break;

	if (i >= numnames) {
		switch (mode) {
		case BIND_NORM:
			if (BindNormKeys[bindkey].cptr != (char *) NULL &&
			    BindNormKeys[bindkey].is_string)
				free (BindNormKeys[bindkey].cptr);
			BindNormKeys[bindkey].cptr = string (rest);
			BindNormKeys[bindkey].is_string = TRUE;
			BindNormKeys[bindkey].has_changed = TRUE;
			bind_changed = TRUE;
			msg ("-- Bind: '%s' bound to key '%s'",
				rest, display_char (bindkey, mode));
			break;
		case BIND_ESC:
			if (BindEscKeys[bindkey].cptr != (char *) NULL && 
			    BindEscKeys[bindkey].is_string)
				free (BindEscKeys[bindkey].cptr);
			BindEscKeys[bindkey].cptr = string (rest);
			BindEscKeys[bindkey].is_string = TRUE;
			BindEscKeys[bindkey].has_changed = TRUE;
			bind_changed = TRUE;
			msg ("-- Bind: '%s' bound to key '%s'",
				rest, display_char (bindkey, mode));
			break;
		case BIND_IMAP:	
		case BIND_MORE:
			msg ("-- Bind: '%s' is not a valid binding function.",
				rest);
			break;
		}
		return;
	}

	switch (mode) {
	case BIND_NORM:
		if (BindNormKeys[bindkey].cptr != (char *) NULL &&
		    BindNormKeys[bindkey].is_string) {
			free (BindNormKeys[bindkey].cptr);
		}
		BindNormKeys[bindkey].func = BindNames[i].func;
		BindNormKeys[bindkey].cptr = BindNames[i].name;
		BindNormKeys[bindkey].is_string = FALSE;
		BindNormKeys[bindkey].has_changed = TRUE;
		bind_changed = TRUE;
		break;
	case BIND_ESC:
		if (BindEscKeys[bindkey].cptr != (char *) NULL && 
		    BindEscKeys[bindkey].is_string) {
			free (BindEscKeys[bindkey].cptr);
		}
		BindEscKeys[bindkey].func = BindNames[i].func;
		BindEscKeys[bindkey].cptr= BindNames[i].name;
		BindEscKeys[bindkey].is_string = FALSE;
		BindEscKeys[bindkey].has_changed = TRUE;
		bind_changed = TRUE;
		break;
	case BIND_IMAP:
# ifdef IMAP
		BindImapKeys[bindkey].func = BindNames[i].func;
		BindImapKeys[bindkey].cptr= BindNames[i].name;
		BindImapKeys[bindkey].has_changed = TRUE;
		bind_changed = TRUE;
# endif
		break;
	case BIND_MORE:
		more_ch = bindkey;
		BindNames[i].func ();
		bind_changed = TRUE;
		msg ("-- Bind: key '%s' bound to more function '%s'",
			display_char (bindkey, mode), BindNames[i].name);
		return;
	}
	msg ("-- Bind: key '%s' bound to function '%s'",
		display_char (bindkey, mode), BindNames[i].name);
}

char convert_string (char *s, int *mode)
{
char *p;
char *q;

	p = strtou (s);
	*mode = 0;
	if (((streqrn (p, "E-") || streqrn (p, "ESC-")) &&
		(*mode = BIND_ESC)) ||
	    ((streqrn (p, "I-") || streqrn (p, "IMAP-")) &&
		(*mode = BIND_IMAP)) ||
	    ((streqrn (p, "M-") || streqrn (p, "MORE-")) &&
		(*mode = BIND_MORE))) {
		q = strchr (s, '-');
		p = q;
		p++;
		if (*p == '\0') {
			*mode = -1;
			return (0);
		}
	} else
		p = s;

	if (*p == '\\' && *(p+1) == 'n')
		return ('\n');
	
	if ((*p == '^' && (p++)) ||
	    (streqrn (p, "C-") && (p += 2)) ||
	    (streqrn (p, "CTRL-") && (p += 5))) {
		q = strtou (p);
		p = q;
		if (*p <= '_') {	/* highest ctrl char char value */
			return (*p - 64);
		} else {
			return (-1);
		}
	} else
		return (*p);
}

char *display_char (char c, int mode)
{
static char ret[10];
int start = 0;
	
	if (mode == BIND_ESC) {
		strcpy (ret, "ESC-");
		start += 4;
	} else if (mode == BIND_IMAP) {
		strcpy (ret, "IMAP-");
		start += 5;
	} else if (mode == BIND_MORE) {
		strcpy (ret, "MORE-");
		start += 5;
	}
	if (c == '\n') {
		ret[start++] = '\\';
		ret[start++] = 'n';
		ret[start] = '\0';
	} else if (c <= 32) { /* ctrl char */
		ret[start++] = '^';
		ret[start++] = c + 64;
		ret[start] = '\0';
	} else if (c == 127) {
		strcpy (ret+start, "^?");
	} else {
		ret[start++] = c;
		ret[start] = '\0';
	}
	return (ret);
}

bind_translate_char (char c, int mode)
{
char buf[MAXSIZ];

	switch (mode) {
	case BIND_NORM:
		if (BindNormKeys[c].cptr != (char *) NULL && 
		    BindNormKeys[c].is_string) {
			msg ("--> %s", BindNormKeys[c].cptr);
			strcpy (buf, BindNormKeys[c].cptr);
			process_key (buf, FALSE);
		} else {
			(void) BindNormKeys[c].func (c);
		}
		break;
	case BIND_ESC:
		if (BindEscKeys[c].cptr != (char *) NULL &&
		    BindEscKeys[c].is_string) {
			msg ("--> %s", BindEscKeys[c].cptr);
			strcpy (buf, BindEscKeys[c].cptr);
			process_key (buf, FALSE);
		} else {
			(void) BindEscKeys[c].func (c);
		}
		break;
# ifdef IMAP
	case BIND_IMAP:
		(void) BindImapKeys[c].func (c);
# endif
		break;
	default:
		msg ("-- Funny mode(%d) in bind_translate.. report this please.", mode);
		break;
	}
}

list_binding (int listkey, int mode)
{
int i;
BindKey *bind;

	if (listkey != -1) {
		switch (mode) {
		case BIND_NORM:
			bind = BindNormKeys;
			break;
		case BIND_ESC:
			bind = BindEscKeys;
			break;
# ifdef IMAP
		case BIND_IMAP:
			bind = BindImapKeys;
			break;
# endif
		case BIND_MORE:
			break;
		default:
			return;
		}

		msg ("-- Bind: '%s' = '%s' (%s)",
			display_char (listkey, mode),
			(bind[listkey].cptr ? bind[listkey].cptr : "default"),
			(bind[listkey].is_string ? "string" :
				"function"));
		return;
	}

	for (i = 0; i < 127; i++)
		if (BindNormKeys[i].has_changed)
			msg ("-- Bind: '%s' = '%s' (%s)",
				display_char (i, BIND_NORM),
				(BindNormKeys[listkey].cptr ?
					BindNormKeys[listkey].cptr : "default"),
				(BindNormKeys[i].is_string ? "string" :
					"function"));
	for (i = 0; i < 127; i++)
		if (BindEscKeys[i].has_changed)
			msg ("-- Bind: '%s' = '%s' (%s)",
				display_char (i, BIND_ESC),
				(BindEscKeys[i].cptr ?
					BindEscKeys[i].cptr : "default"),
				(BindEscKeys[i].is_string ? "string" :
					"function"));
# ifdef IMAP
	for (i = 0; i < 127; i++)
		if (BindImapKeys[i].has_changed)
			msg ("-- Bind: '%s' = '%s' (%s)",
				display_char (i, BIND_IMAP),
				(BindImapKeys[i].cptr ?
					BindImapKeys[i].cptr : "default"),
				(BindImapKeys[i].is_string ? "string" :
					"imap function"));
# endif
	if (more_val.k_quit != MORE_DEFAULT_QUITCH)
		msg ("-- Bind: more_quit bound to 'MORE-%c'",
			more_val.k_quit);
	if (more_val.k_clear != MORE_DEFAULT_CLEARCH)
		msg ("-- Bind: more_clear bound to 'MORE-%c'",
			more_val.k_clear);
	if (more_val.k_cancel != MORE_DEFAULT_CANCELCH)
		msg ("-- Bind: more_cancel bound to 'MORE-%c'",
			more_val.k_cancel);
	if (more_val.k_nonstop != MORE_DEFAULT_NONSTOPCH)
		msg ("-- Bind: more_nonstop bound to 'MORE-%c'",
			more_val.k_nonstop);
	if (more_val.k_forward != MORE_DEFAULT_FORWARDCH)
		msg ("-- Bind: more_forward bound to 'MORE-%c'",
			more_val.k_forward);
	if (more_val.k_oneline != MORE_DEFAULT_ONELINECH)
		msg ("-- Bind: more_oneline bound to 'MORE-%c'",
			more_val.k_oneline);
}

save_binds (FILE *fd)
{
int i;

	if (!bind_changed)
		return;
	fprintf (fd,
		"#\n# Bindings which are different than client's default\n#\n");
	for (i = 0; i < 127; i++)
		if (BindNormKeys[i].has_changed)
			fprintf (fd, "bind %s %s\n",
				display_char (i, BIND_NORM),
				fstring (BindNormKeys[i].cptr));
	for (i = 0; i < 127; i++)
		if (BindEscKeys[i].has_changed) {
			fprintf (fd, "bind %s %s\n",
				display_char (i, BIND_ESC),
				fstring (BindEscKeys[i].cptr));
		}
# ifdef IMAP
	for (i = 0; i < 127; i++)
		if (BindImapKeys[i].has_changed)
			fprintf (fd, "bind %s %s\n",
				display_char (i, BIND_IMAP),
				BindImapKeys[i].cptr);
# endif
	if (more_val.k_quit != MORE_DEFAULT_QUITCH)
		fprintf (fd, "bind MORE-%c more_quit\n", more_val.k_quit);
	if (more_val.k_clear != MORE_DEFAULT_CLEARCH)
		fprintf (fd, "bind MORE-%c more_clear\n", more_val.k_clear);
	if (more_val.k_cancel != MORE_DEFAULT_CANCELCH)
		fprintf (fd, "bind MORE-%c more_cancel\n", more_val.k_cancel);
	if (more_val.k_nonstop != MORE_DEFAULT_NONSTOPCH)
		fprintf (fd, "bind MORE-%c more_nonstop\n", more_val.k_nonstop);
	if (more_val.k_forward != MORE_DEFAULT_FORWARDCH)
		fprintf (fd, "bind MORE-%c more_forward\n", more_val.k_forward);
	if (more_val.k_oneline != MORE_DEFAULT_ONELINECH)
		fprintf (fd, "bind MORE-%c more_oneline\n", more_val.k_oneline);
}

void more_quit (void)
{
	more_val.k_quit = more_ch;
}

void more_clear (void)
{
	more_val.k_clear = more_ch;
}

void more_cancel (void)
{
	more_val.k_cancel = more_ch;
}

void more_nonstop (void)
{
	more_val.k_nonstop = more_ch;
}

void more_forward (void)
{
	more_val.k_forward = more_ch;
}

void more_oneline (void)
{
	more_val.k_oneline = more_ch;
}

void bind_underline (void)
{
	input_ch_into_buf (UNDERLINE_CHAR);
}

void bind_boldface (void)
{
	input_ch_into_buf (BOLD_CHAR);
}

void bind_bell (void)
{
	input_ch_into_buf (BELL_CHAR);
}

void bind_inverse (void)
{
	input_ch_into_buf (INVERSE_CHAR);
}
