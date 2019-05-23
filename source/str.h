/*
 * str.h: header for string manipulations.
 *
 *
 * Written By Evan D. Koffler <evank@netcom.com>
 *
 * Copyright(c) 1990, 1991 
 *
 * See the COPYRIGHT file.
 */

# ifndef MAX
# define MAX(A,B)	(((A) < (B)) ? (B) : (A))
# endif

# ifndef MIN
# define MIN(A,B)	(((A) > (B)) ? (B) : (A))
# endif

/* string equal */
# define streq(A,B)	(!strcmp ((A), (B)))

/* string equal for N characters */
# define streqn(A,B,N)	(!strncmp ((A), (B), (N)))

/* string equal to the N characters of first parameter */
# define streqln(A,B)	(!strncmp ((A), (B), (strlen ((A)))))

/* string equal to the N characters of the second parameter */
# define streqrn(A,B)	(!strncmp ((A), (B), (strlen ((B)))))

/* string matching */
# define MATCH(D,P)	pattern_match ((D),(P),pattern)

/* a null string to be used anywhere in the client */
# define NULL_STRING	""

/* results for pattern matcher */
extern char pattern1[], pattern2[], pattern3[], pattern4[];
extern char pattern5[], pattern6[], pattern7[], pattern8[];
extern char pattern9[], pattern10[], pattern11[], pattern12[];
extern char pattern13[], pattern14[], pattern15[], pattern16[];
extern char pattern17[], pattern18[], pattern19[], pattern20[];
extern char *pattern[];

extern char *client_prompt;
extern char *input_prompt;
extern char *output_prompt;

extern char *first ();
extern char *fstring ();
extern char *maxstring ();
extern char *rest ();
extern char *skip_space ();
extern char *string ();
extern char *strtou ();
extern char *strfree ();
