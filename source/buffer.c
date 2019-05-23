/*
 * buffer.c: handles the buffering of output on a lined list
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1993
 *
 * See the COPYRIGHT file.
 */

# include "gb.h"

# include <stdio.h>
# include <sys/types.h>
# include <malloc.h>

# include "option.h"
# include "str.h"
# include "vars.h"

extern char *strcat (char *, const char *);
extern time_t time (time_t *);

add_buffer (BufferInfo *infoptr, char *str, int partial)
{
char *q;
Buffer *p;
int len;

	/* partial lines so append incoming data */
	if (infoptr->partial) {
		debug (2, "add_buffer (partial): adding to line. This line is: %s", (partial ? "partial" : "done"));
		len = strlen (infoptr->tail->buf) + strlen (str);
		q = (char *) malloc (len + 1);
		if (!q) {
			msg ("-- Malloc: Could not allocate socket output!");
			return;
		}
		strcpy (q, infoptr->tail->buf);
		strcat (q, str);
		q[len] = '\0';
		strfree (infoptr->tail->buf);
		infoptr->tail->buf = q;
		/* are we newline terminated yet? */
		infoptr->partial = partial;
		return;
	}

	/* new data coming in, set up linked list */
	p = (Buffer *) malloc (sizeof (Buffer));
	if (!p) {
		msg ("-- Malloc: Could not allocate socket output!");
		return;
	}
	if (!infoptr->tail) {
		infoptr->tail = p;
		infoptr->head = p;
		p->next = NULL;
		p->prev = NULL;
	} else {
		p->next = infoptr->tail;
		infoptr->tail->prev = p;
		p->prev = NULL;
		infoptr->tail = p;
	}


	info.lines_read++;
	p->buf = maxstring (str);
	infoptr->partial = partial;
	debug (2, "add_buffer (newline): adding to line. This line is: %s", (partial ? "partial" : "done"));
}

char *remove_buffer (BufferInfo *infoptr)
{
Buffer *p = infoptr->head;
char *str;

	if (!infoptr->head) {
		return (NULL_STRING);
	}

	/* lop off the head node */
	if (p->prev)
		p->prev->next = NULL;
	/* one node, head/tail point to it */
	if (infoptr->head == infoptr->tail) {
		infoptr->head = infoptr->tail = NULL;
		infoptr->partial = FALSE;
	/* else move head back one node */
	} else
		infoptr->head = p->prev;

	debug (3, "remove_buffer: returning %s", p->buf);
	str = p->buf;
	free (p);
	return ((char *) str);
}


/*
 * Determines if there is any nodes on the buffer that
 * can be taken for processing. A partial head node is not
 * valid since all output is newline terminated.
 */
int have_buffer (BufferInfo *infoptr)
{
static long last_check = -1;

	if (!infoptr)
		return (FALSE);
	if (infoptr->tail == infoptr->head && infoptr->partial) {
		if (GET_BIT (options, PARTIAL_LINES)) {
			if (last_check == -1) {
				last_check = time (0);
			} else if (last_check < (time (0)/* - 1*/)) {
				infoptr->partial = FALSE;
				infoptr->is_partial = TRUE;
				last_check = -1;
				return (TRUE);
			}
		}
		return (FALSE);
	}
	if (infoptr->head) {
		last_check = -1;
		return (TRUE);
	}
	return (FALSE);
}
