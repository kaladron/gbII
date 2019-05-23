/*
 * crypt.c: encryption routines. Got the encoding from some
 *          magazine article a prof had. No known author.
 * NOTE: The current encryption scheme is NOT secure.
 *       Several cracking programs exist. You have been warned.
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
# include <sys/file.h>
# include <malloc.h>
# include <memory.h>

# include "str.h"
# include "option.h"
# include "vars.h"

# define CRYPT_HEADER		"CRYPT"
# define CRYPT_HEADER_LEN 	5
# define MOD1			95

extern char pbuf[];

typedef struct cryptrecallstruct {
	char *key;
	struct cryptrecallstruct *next;
	struct cryptrecallstruct *prev;
} CryptRecall;

CryptRecall *cryptrecall = (CryptRecall *) NULL;
CryptRecall *cryptcur = (CryptRecall *) NULL;

char crypt_values[100];

/*
 * Crypt: the crypt list structure,  consists of the nickname, and the
 * encryption key 
 */
typedef struct cryptstruct {
    	char *nick;
    	char *key;
	int indx;
    	struct cryptstruct *next;
	struct cryptstruct *prev;
} Crypt;

static Crypt *crypt_list = NULL;

int cmd_listcrypt (void);
int cmd_uncrypt (char *nick);
int crypt_update_index (void);
int encode (char *str, char *key);
int add_crypt_recall (char *key);

extern int atoi (const char *);
extern int strncmp (const char *, const char *, size_t);
extern int fprintf (FILE *, const char *, ...);

/*
 * find_crypt: looks up nick in the crypt list and returns the Crypt
 * structure if found, NULL otherwise.
 */
Crypt *find_crypt (char *id)
{
Crypt *p = NULL;

    	for (p = crypt_list; p && !streq (id, p->nick); p = p->next) 
		;
    	return (p);
}

/*
 * cmd_crypt: adds the nickname and key pair to the crypt_list.  If the
 * nickname is already in the list, then the key is changed to the supplied
 * key. 
 */
void cmd_crypt (char *args)
{
Crypt *p;
char nick[BUFSIZ];
char key[BUFSIZ];

	if (!*args) {
		cmd_listcrypt ();
		return;
	}

	/* if a - flag, then remove the crypt from the list */
	if (*args == '-') {
		cmd_uncrypt ((args+1));
		return;
	}

	split (args, nick, key);

	if (!*nick || !*key) {
		msg ("-- Usage: crypt <pattern> <key>");
		return;
	}
    	p = find_crypt (nick);
    	if (!p) {
		p = (Crypt *) malloc (sizeof (Crypt));
		if (!crypt_list) {
                        crypt_list = p;                          
                        p->next = NULL;                                        
                        p->prev = NULL;                  
                } else {                                             
                        p->next = crypt_list;                                   
                        crypt_list->prev = p;                                 
                        p->prev = NULL;   
                        crypt_list = p;
                }
		p->key = NULL;
		p->nick = string (nick);
    	} else
		free (p->key);
	p->key = string (key);
	crypt_update_index ();
}

/*
 * Removes the node that contains nick from the crypt list.
 */
cmd_uncrypt (char *nick)
{
Crypt *p;
int val = 0;

	if (*nick == '#') {
		val = atoi (nick+1);
		for (p = crypt_list; p && p->indx != val; p = p->next)
			;
	} else {
    		p = find_crypt (nick);
	}

	if (!p) {
		msg ("-- No such crypt %s defined.", nick);
		return;
	}
	msg ("-- Removed crypt: %s", nick);
	if (!p->prev) {
		if (p->next) {
			crypt_list = p->next;
			crypt_list->prev = NULL;
		} else {
			crypt_list = NULL;
		}
	} else if (!p->next) {
		p->prev->next = NULL;
	} else {
		p->prev->next = p->next;
		p->next->prev = p->prev;
	}
	free (p->key);
	free (p->nick);
	free (p);
	crypt_update_index ();
}

/*
 * Prints out the entire crypt list.
 */
cmd_listcrypt (void)
{
Crypt *p;
int i = 1;

	if (!crypt_list) {
		msg ("-- No crypts defined.");
		return;
	}

	msg ("-- Crypts:");
	for (p = crypt_list; p; p = p->next) {
		p->indx = i;
		msg ("%3d) %s <%s>", i++, p->nick, p->key);
	}
	msg ("-- end of list");
}	

crypt_update_index (void)
{
Crypt *p;
int i = 1;

	if (!crypt_list)
		return;
	for (p = crypt_list; p; p = p->next)
		p->indx = i++;
}
/*
 * looks up nick in the crypt_list and returns the encryption key
 * if found in the list.  If not found in the crypt_list, null is returned. 
 */
char *is_crypted (char *nick)
{
Crypt *p;

    	if (crypt_list) {
		if (p = find_crypt (nick))
	    		return (p->key);
    	}
    	return ((char *) NULL);
}


/*
 * Takes the socket input and checks to see if it is supposed
 * to be decrypted or whatever.
 */
char *check_crypt (char *message, int type)
{
char *key;
char returnfmt[SMABUF];
char buf[MAXSIZ];
static char buf2[MAXSIZ];
char pat[BUFSIZ];
char *p;

	if (strncmp (message, CRYPT_HEADER, CRYPT_HEADER_LEN)) {
		debug (4, "NO crypt header. Returning null");
		return ((char *) NULL);
	}

	p = strchr (message, '|');
	if (!p || !*p) {
		return ((char *) NULL);
	}

	*p = '\0';
	strcpy (buf, p+1);
	strcpy (pat, message+CRYPT_HEADER_LEN);
	*p = '|';	/* replace back for strlen in socket.c */

	debug (4, "pattern: '%s'", pat);
	debug (4, "buffer:  '%s'", buf);

	switch (type) {
	case NORM_BROADCAST:
	case GB_BROADCAST:
	case HAP_BROADCAST:
		sprintf (returnfmt, "%s >>", pat);
		break;
	case NORM_ANNOUNCE:
	case GB_ANNOUNCE:
	case HAP_ANNOUNCE:
		sprintf (returnfmt, "%s ::", pat);
		break;
	case GB_THINK:
		sprintf (returnfmt, "%s ==", pat);
		break;
	case HAP_THINK:
		sprintf (returnfmt, "%s ))", pat);
		break;
        case GB_EMOTE:
		sprintf (returnfmt, "%s ))", pat);
                break;
	default:
		*returnfmt = '\0';
		break;
	}

	key = is_crypted (pat);

	if (!key) {
		debug (3, "Crypt: no key for message");
		if (GET_BIT (options, ENCRYPT)) {
			sprintf (buf2, "%s [encrypted]", returnfmt);
			return (buf2);
		} else 
			return ((char *) NULL);
	}
	debug (4, "Key is: %s", key);
	encode (buf, key);
	sprintf (buf2, "%s %s", returnfmt, buf);
	return (buf2);
}	

/*
 * takes the given input and creates the appropriate output
 * for sending to the socket.
 */
#define MOD2 95
cmd_cr (char *args)		/* formerly build_crypt */
           
{
char pat[BUFSIZ];
int type = NORM_BROADCAST;
char *p;
char *key;
char buf[MAXSIZ];

	if (*args == '-') {
		switch (*(args+1)) {
		case 'a': type = NORM_ANNOUNCE; break;
		case 'b': type = NORM_BROADCAST; break;
		case 'e': type = GB_EMOTE; break;
		case 't': type = GB_THINK; break;
		default: type = NORM_BROADCAST; break;
		}
		p = rest (args);
		if (p)
			strcpy (args, p);
	}

	p = first (args);
	if (p)
		strcpy (pat, p);
	else {
		msg ("-- Usage: cr <pat> <message>");
		return;
	}
	p = rest (args);
	if (p)
		strcpy (args, p);
	else {
		msg ("-- Usage: cr <pat> <message>");
		return;
	}

	key = is_crypted (pat);
	if (!key) {
		msg ("-- Pattern %s does not have a key.", pat);
		return;
	}

	debug (4, "cr buffer before encode: '%s'", args);
	encode (args, key);
	debug (4, "cr buffer after encode:  '%s'", args);

	sprintf (buf, "%s %s%s|%s\n",
		(type == NORM_BROADCAST ? "broadcast" : 
		(type == NORM_ANNOUNCE ? "announce" : 
                (type == GB_EMOTE ? "emote" : "think"))), 
		CRYPT_HEADER, pat, args);
	strcpy (args, buf);
	add_crypt_recall (pat);
}

/*
 * Fill the crypt array with the ascii values for use by the 
 * encoding process.
 */
init_crypt (void)
{
int i;
int j = 127;

	for(i = 0; i < 96; i++)
		crypt_values[i] = --j;
}

/*
 * This is a double code. Encoding, and then encoding again creates
 * the original string. So it is the encryption and decryption
 * process.
 */
encode (char *str, char *key)
{
char *kptr = key;	
int klen;
int slen;

	if(!str || !*str)
	return;

	klen = strlen(key) - 1;
	slen = strlen(str) - 1;

   	while(*str) {
     		*str = crypt_values[(int) ((*str - 32) + (int) ((*kptr + 
   		         ((klen + key[klen] + slen) % MOD1)) % MOD1)) % MOD2]; 
		str++;
		if(!*++kptr)
 			kptr = key;
   	}
}

/*
 * saves the current crypts to the file descriptor provided.
 * in a format that can be reloaded via loadf
 */
save_crypts (FILE *fd)
{
Crypt *p;

	if (!crypt_list)
		return;
	fprintf (fd, "\n#\n# Crypt keys\n#\n");
	for (p = crypt_list; p; p = p->next)
		fprintf (fd, "crypt %s %s\n", p->nick, p->key);
}	

int do_crypt_recall (char *str)
{
	if (cryptrecall) {
		if (!cryptcur)
			cryptcur = cryptrecall;
		sprintf (str, "cr %s ", cryptcur->key);
		if (cryptcur->next)
			cryptcur = cryptcur->next;
		else
			cryptcur = cryptrecall;
		return (TRUE);
	} else
		return (FALSE);
}

add_crypt_recall (char *key)
{
CryptRecall *new = cryptrecall;

	cryptcur = (CryptRecall *) NULL;

	/* try to find the crypt first, if it exists, remove to front */
	for ( ; new && !streq (key, new->key); new = new->next)
		;

	if (new) { /* key exists already */
		if (new == cryptrecall) /* found 1st node, leave alone */
			return;
		if (new->next)
			new->next->prev = new->prev;
		if (new->prev)
			new->prev->next = new->next;
	} else { /* does not exist, create */
		new = (CryptRecall *) malloc (sizeof (CryptRecall));
		if (!new) {
			msg ("-- cryptrecall: could not malloc memory needed.");
			return;
		}
		new->key = string (key);
	}

	new->next = cryptrecall;
	if (cryptrecall)
		cryptrecall->prev = new;
	new->prev = (CryptRecall *) NULL;
	cryptrecall = new;
}

void crypt_test (void)
{
int i;

	for(i = 0; i < 96; i++)
		msg ("%c = %c", i, crypt_values[i]);
}

