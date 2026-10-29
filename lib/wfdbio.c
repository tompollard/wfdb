/* file: wfdbio.c	G. Moody	18 November 1988
                        Last revised:     11 April 2022       wfdblib 10.7.0
Low-level I/O functions for the WFDB library

_______________________________________________________________________________
wfdb: a library for reading and writing annotated waveforms (time series data)
Copyright (C) 1988-2013 George B. Moody

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Library General Public License for more
details.

You should have received a copy of the GNU Library General Public License along
with this library; if not, see <http://www.gnu.org/licenses/>.

You may contact the author by e-mail (wfdb@physionet.org) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

This file contains a large number of functions related to input and output.
To make them easier to find, the functions are arranged in several groups as
outlined below;  they appear in the body of the file in the same order that
their names appear in these comments.

This file contains definitions of the following WFDB library functions:
 getwfdb		(returns the database path string)
 setwfdb		(sets the database path string)
 resetwfdb [10.5.7]	(restores the database path to its initial value)
 wfdbquiet		(suppresses WFDB library error messages)
 wfdbverbose [4.0]	(enables WFDB library error messages)
 wfdberror [4.5]	(returns the most recent WFDB library error message)
 wfdbfile [4.3]		(returns the complete pathname of a WFDB file)
 wfdbmemerr [10.4.6]    (set behavior on memory errors)

These functions expose config strings needed by the WFDB Toolkit for Matlab:
 wfdbversion [10.4.20]  (return the string defined by VERSION)
 wfdbldflags [10.4.20]  (return the string defined by LDFLAGS)
 wfdbcflags [10.4.20]   (return the string defined by CFLAGS)
 wfdbdefwfdb [10.4.20]  (return the string defined by DEFWFDB)
 wfdbdefwfdbcal [10.4.20] (return the string defined by DEFWFDBCAL)

These functions, also defined here, are intended only for the use of WFDB
library functions defined elsewhere:

 wfdb_me_fatal [10.4.6] (indicates if memory errors are fatal)
 wfdb_g16		(reads a 16-bit integer)
 wfdb_g32		(reads a 32-bit integer)
 wfdb_p16		(writes a 16-bit integer)
 wfdb_p32		(writes a 32-bit integer)
 wfdb_getline		(reads a line from a text file)
 wfdb_free_path_list [10.0.1] (frees data structures assigned to the path list)
 wfdb_parse_path [10.0.1] (splits WFDB path into components)
 wfdb_export_config [10.3.9] (puts the WFDB path, etc. into the environment)
 wfdb_getiwfdb [6.2]	(sets WFDB from the contents of a file)
 wfdb_addtopath [6.2]	(adds path component of string argument to WFDB path)
 wfdb_vasprintf		(allocates and formats a message)
 wfdb_asprintf		(allocates and formats a message)
 wfdb_error		(produces an error message)
 wfdb_fprintf [10.0.1]	(like fprintf, but first arg is a WFDB_FILE pointer)
 wfdb_open		(finds and opens database files)
 wfdb_checkname		(checks record and annotator names for validity)
 wfdb_striphea [10.4.5] (removes trailing '.hea' from a record name, if present)
 wfdb_setirec [9.7]	(saves current record name)
 wfdb_getirec [10.5.12]	(gets current record name)
 wfdb_isarchive (checks whether a record name refers to an archive)
 wfdb_archive_split (splits a file name into archive and member)
 wfdb_archive_open (opens a file within an archive for reading)
 wfdb_archive_name (returns the archive containing an open file)
 wfdb_archive_close_all (writes all archives opened for output)

(Numbers in brackets in the lists above indicate the first version of the WFDB
library that included the corresponding function.  Functions not so marked
have been included in all published versions of the WFDB library.)

The next two groups of functions, which together enable input from remote
(http and ftp) files, were first implemented in version 10.0.1 by Michael
Dakin.  Thanks, Mike!

These functions, defined here if WFDB_NETFILES is non-zero, are intended only
for the use of the functions in the next group below (their definitions are not
visible outside of this file):
 www_parse_passwords	(load username/password information)
 www_userpwd		(get username/password for a given url)
 wfdb_wwwquit		(shut down libcurl cleanly)
 www_init		(initialize libcurl)
 www_perform_request    (request a url and check the response code)
 www_get_cont_len	(find length of data for a given url)
 www_get_url_range_chunk (get a block of data from a given url)
 www_get_url_chunk	(get all data from a given url)
 nf_delete		(free data structures associated with an open netfile)
 nf_new			(associate a netfile with a url)
 nf_get_range		(get a block of data from a netfile)
 nf_feof		(emulates feof, for netfiles)
 nf_eof			(TRUE if netfile pointer points to EOF)
 nf_fopen		(emulate fopen, for netfiles; read-only access)
 nf_fclose		(emulates fclose, for netfiles)
 nf_fgetc		(emulates fgetc, for netfiles)
 nf_fgets		(emulates fgets, for netfiles)
 nf_fread		(emulates fread, for netfiles)
 nf_fseek		(emulates fseek, for netfiles)
 nf_ftell		(emulates ftell, for netfiles)
 nf_ferror		(emulates ferror, for netfiles)
 nf_clearerr		(emulates clearerr, for netfiles)
 nf_fflush		(emulates fflush, for netfiles) [stub]
 nf_fwrite		(emulates fwrite, for netfiles) [stub]
 nf_putc		(emulates putc, for netfiles) [stub]
 nf_vfprintf		(emulates fprintf, for netfiles) [stub]

In the current version of the WFDB library, output to remote files is not
implemented;  for this reason, several of the functions listed above are
stubs (placeholders) only, as noted.

These functions, also defined here, are compiled only if WFDB_NETFILES is non-
zero; they permit access to remote files via http or ftp (using libcurl) as
well as to local files (using the standard C I/O functions).  The functions in
this group are intended primarily for use by other WFDB library functions, but
may also be called directly by WFDB applications that need to read remote
files. Unlike other private functions in the WFDB library, the interfaces to
these are not likely to change, since they are designed to emulate the
similarly-named ANSI/ISO C standard I/O functions:
 wfdb_clearerr		(emulates clearerr)
 wfdb_feof		(emulates feof)
 wfdb_ferror		(emulates ferror)
 wfdb_fflush		(emulates fflush, for local files only)
 wfdb_fgets		(emulates fgets)
 wfdb_fread		(emulates fread)
 wfdb_fseek		(emulates fseek)
 wfdb_ftell		(emulates ftell)
 wfdb_fwrite		(emulates fwrite, for local files only)
 wfdb_getc		(emulates getc)
 wfdb_putc		(emulates putc, for local files only)
 wfdb_fclose		(emulates fclose)
 wfdb_fopen		(emulates fopen, but returns a WFDB_FILE pointer)

(If WFDB_NETFILES is zero, wfdblib.h defines all but the last two of these
functions as macros that invoke the standard I/O functions that they would
otherwise emulate.  The implementations of wfdb_fclose and wfdb_fopen are
below;  they include a small amount of code compiled only if WFDB_NETFILES
is non-zero.  All of these functions are new in version 10.0.1.)

Finally, this file includes several miscellaneous functions needed only in
certain environments:
 strtok		(parses strings into tokens, for old C libraries that need it)
 LibMain        (initializes 16-bit MS-Windows DLL version of this library)
 WEP		(cleans up on exit from 16-bit MS-Windows DLL)
 wgetenv	(replacement for getenv, for use with MS-Windows 16-bit DLLs)
 DllMain	(initialize/cleanup 32-bit MS-Windows DLL)

Functions in signal.c and calib.c use the C library function strtok() to parse
lines into tokens;  this function (and its associated header file <string.h>)
may not be available in certain older C libraries (e.g., UNIX version 7 and BSD
4.2).  This file includes a portable implementation of strtok(), which can be
obtained if necessary by defining the symbol NOSTRTOK when compiling this
module.
*/

#include "wfdblib.h"
#include <limits.h>
#include <time.h>

/* WFDB library functions */

/* getwfdb is used to obtain the WFDB path, a list of places in which to search
for database files to be opened for reading.  In most environments, this list
is obtained from the shell (environment) variable WFDB, which may be set by the
user (typically as part of the login script).  A default value may be set at
compile time (DEFWFDB in wfdblib.h); this is necessary for environments that do
not support the concept of environment variables, such as MacOS9 and earlier.
If WFDB or DEFWFDB is of the form '@FILE', getwfdb reads the WFDB path from the
specified (local) FILE (using wfdb_getiwfdb); such files may be nested up to
10 levels. */

static char *wfdbpath = NULL, *wfdbpath_init = NULL;

static const char *wfdb_getiwfdb(char **p);

/* resetwfdb is called by wfdbquit, and can be called within an application,
to restore the WFDB path to the value that was returned by the first call
to getwfdb (or NULL if getwfdb was not called). */

FVOID resetwfdb(void)
{
    SSTRCPY(wfdbpath, wfdbpath_init);
}

FSTRING getwfdb(void)
{
    if (wfdbpath == NULL) {
	const char *p = getenv("WFDB");

	if (p == NULL) p = DEFWFDB;
	SSTRCPY(wfdbpath, p);
	p = wfdb_getiwfdb(&wfdbpath);
	SSTRCPY(wfdbpath_init, wfdbpath);
	wfdb_parse_path(p);
    }
    return (wfdbpath);
}

/* setwfdb can be called within an application to change the WFDB path. */

FVOID setwfdb(const char *p)
{
    void wfdb_export_config(void);

    if (p == NULL && (p = getenv("WFDB")) == NULL) p = DEFWFDB;
    SSTRCPY(wfdbpath, p);
    wfdb_export_config();

    SSTRCPY(wfdbpath, p);
    p = wfdb_getiwfdb(&wfdbpath);
    wfdb_parse_path(p);
}

/* wfdbquiet can be used to suppress error messages from the WFDB library. */

static int error_print = 1;

FVOID wfdbquiet(void)
{
    error_print = 0;
}

/* wfdbverbose enables error messages from the WFDB library. */

FVOID wfdbverbose(void)
{
    error_print = 1;
}

static char *wfdb_filename;

/* wfdbfile returns the pathname or URL of a WFDB file. */

FSTRING wfdbfile(const char *s, char *record)
{
    WFDB_FILE *ifile;

    if (s == NULL && record == NULL)
	return (wfdb_filename);

    /* Remove trailing .hea, if any, from record name. */
    wfdb_striphea(record);

    if ((ifile = wfdb_open(s, record, WFDB_READ))) {
	(void)wfdb_fclose(ifile);
	return (wfdb_filename);
    }
    else return (NULL);
}

/* Determine how the WFDB library handles memory allocation errors (running
out of memory).  Call wfdbmemerr(0) in order to have these errors returned
to the caller;  by default, such errors cause the running process to exit. */

static int wfdb_mem_behavior = 1;

FVOID wfdbmemerr(int behavior)
{
    wfdb_mem_behavior = behavior;
}

/* Functions that expose configuration constants used by the WFDB Toolkit for
   Matlab. */

#ifndef VERSION
#define VERSION "VERSION not defined"
#endif

#ifndef LDFLAGS
#define LDFLAGS "LDFLAGS not defined"
#endif

#ifndef CFLAGS
#define CFLAGS  "CFLAGS not defined"
#endif

FCONSTSTRING wfdbversion(void)
{
   return VERSION;
}

FCONSTSTRING wfdbldflags(void)
{
   return LDFLAGS;
}

FCONSTSTRING wfdbcflags(void)
{
   return CFLAGS;
}

FCONSTSTRING wfdbdefwfdb(void)
{
   return DEFWFDB;
}

FCONSTSTRING wfdbdefwfdbcal(void)
{
   return DEFWFDBCAL;
}

/* Private functions (for the use of other WFDB library functions only). */

FINT wfdb_me_fatal()	/* used by the MEMERR macro defined in wfdblib.h */
{
    return (wfdb_mem_behavior);
}

/* The next four functions read and write integers in PDP-11 format, which is
common to both MIT and AHA database files.  The purpose is to achieve
interchangeability of binary database files between machines which may use
different byte layouts.  The routines below are machine-independent; in some
cases (notably on the PDP-11 itself), taking advantage of the native byte
layout can improve the speed.  For 16-bit integers, the low (least significant)
byte is written (read) before the high byte; 32-bit integers are represented as
two 16-bit integers, but the high 16 bits are written (read) before the low 16
bits. These functions, in common with other WFDB library functions, assume that
a byte is 8 bits, a "short" is 16 bits, an "int" is at least 16 bits, and a
"long" is at least 32 bits.  The last two assumptions are valid for ANSI C
compilers, and for almost all older C compilers as well.  If a "short" is not
16 bits, it may be necessary to rewrite wfdb_g16() to obtain proper sign
extension. */

/* read a 16-bit integer in PDP-11 format */
int wfdb_g16(WFDB_FILE *fp)
{
    int x;

    x = wfdb_getc(fp);
    return ((int)((short)((wfdb_getc(fp) << 8) | (x & 0xff))));
}

/* read a 32-bit integer in PDP-11 format */
long wfdb_g32(WFDB_FILE *fp)
{
    long x, y;

    x = wfdb_g16(fp);
    y = wfdb_g16(fp);
    return ((x << 16) | (y & 0xffff));
}

/* write a 16-bit integer in PDP-11 format */
void wfdb_p16(unsigned int x, WFDB_FILE *fp)
{
    (void)wfdb_putc((char)x, fp);
    (void)wfdb_putc((char)(x >> 8), fp);
}

/* write a 32-bit integer in PDP-11 format */
void wfdb_p32(long x, WFDB_FILE *fp)
{
    wfdb_p16((unsigned int)(x >> 16), fp);
    wfdb_p16((unsigned int)x, fp);
}

/* Read a line of text, allocating a buffer large enough to hold the
result.  Note that unlike the POSIX getline function, this function
returns zero at end of file. */
size_t wfdb_getline(char **buffer, size_t *buffer_size, WFDB_FILE *fp)
{
    size_t n = (*buffer == NULL ? 0 : *buffer_size), i = 0;
    char *s = *buffer;
    int c;

    do {
	if (i + 1 >= n) {
	    n += 256;
	    SREALLOC(s, n, 1);
	    if (s == NULL)
		break;
	    *buffer = s;
	    *buffer_size = n;
	}
	c = wfdb_getc(fp);
	if (c == EOF)
	    break;
	s[i++] = c;
    } while (c != '\n');

    if (i > 0)
	(*buffer)[i] = 0;
    return (i);
}

struct wfdb_path_component {
    char *prefix;
    struct wfdb_path_component *next, *prev;
    int type;		/* WFDB_LOCAL or WFDB_NET */
};
static struct wfdb_path_component *wfdb_path_list;

/* wfdb_free_path_list clears out the path list, freeing all memory allocated
   to it. */
void wfdb_free_path_list(void)
{
   struct wfdb_path_component *c0 = NULL, *c1 = wfdb_path_list;

    while (c1) {
	c0 = c1->next;
	SFREE(c1->prefix);
	SFREE(c1);
	c1 = c0;
    }
    wfdb_path_list = NULL;
}

/* Operating system and compiler dependent code

All of the operating system and compiler dependencies in the WFDB library are
contained within the following section of this file.  There are three
significant types of platforms addressed here:

   UNIX and variants
     This includes GNU/Linux, FreeBSD, Solaris, HP-UX, IRIX, AIX, and other
     versions of UNIX, as well as Mac OS/X and Cygwin/MS-Windows.
   MS-DOS and variants
     This group includes MS-DOS, DR-DOS, OS/2, and all versions of MS-Windows
     when using the native MS-Windows libraries (as when compiling with MinGW
     gcc).
   MacOS 9 and earlier
     "Classic" MacOS.

Differences among these platforms:

1. Directory separators vary:
     UNIX and variants (including Mac OS/X and Cygwin) use '/'.
     MS-DOS and OS/2 use '\'.
     MacOS 9 and earlier uses ':'.

2. Path component separators also vary:
     UNIX and variants use ':' (as in the PATH environment variable)
     MS-DOS and OS/2 use ';' (also as in the PATH environment variable;
       ':' within a path component follows a drive letter)
     MacOS uses ';' (':' is a directory separator, as noted above)
   See the notes above wfdb_open for details about path separators and how
   WFDB file names are constructed.

3. By default, MS-DOS files are opened in "text" mode.  Since WFDB files are
   binary, they must be opened in binary mode.  To accomplish this, ANSI C
   libraries, and those supplied with non-ANSI C compilers under MS-DOS, define
   argument strings "rb" and "wb" to be supplied to fopen(); unfortunately,
   most other non-ANSI C versions of fopen do not recognize these as legal.
   The "rb" and "wb" forms are used here for ANSI and MS-DOS C compilers only,
   and the older "r" and "w" forms are used in all other cases.

4. Before the ANSI/ISO C standard was adopted, there were at least two
   (commonly used but incompatible) ways of declaring functions with variable
   numbers of arguments (such as printf).  The ANSI/ISO C standard defined
   a third way, incompatible with the other two.  For this reason, the only two
   functions in the WFDB library (wfdb_error and wfdb_fprintf) that need to use
   variable argument lists are included below in three different versions (with
   additional minor variations noted below).

OS- and compiler-dependent definitions:

   For MS-DOS/MS-Windows compilers.  Note that not all such compilers
   predefine the symbol MSDOS;  for those that do not, MSDOS is usually defined
   by a compiler command-line option in the "make" description file. */ 
#ifdef MSDOS
#define DSEP	'\\'
#define PSEP	';'
#define AB	"ab"
#define RB	"rb"
#define WB	"wb"
#else

/* For other ANSI C compilers.  Such compilers must predefine __STDC__ in order
   to conform to the ANSI specification. */
#ifdef __STDC__
#ifdef MAC	/* Macintosh only.  Be sure to define MAC in 'wfdblib.h'. */
#define DSEP	':'
#define PSEP	';'
#else		/* Other ANSI C compilers (UNIX and variants). */
#define DSEP	'/'
#define PSEP	':'
#endif
#define AB	"ab"
#define RB	"rb"
#define WB	"wb"

/* For all other C compilers, including traditional UNIX C compilers. */
# else
#define DSEP	'/'
#define PSEP	':'
#define AB	"a"
#define RB	"r"
#define WB	"w"
#endif
#endif

/* wfdb_parse_path constructs a linked list of path components by splitting
its string input (usually the value of WFDB). */

int wfdb_parse_path(const char *p)
{
    const char *q;
    int current_type, slashes, found_end;
    struct wfdb_path_component *c0 = NULL, *c1 = wfdb_path_list;
    static first_call = 1;

    /* First, free the existing wfdb_path_list, if any. */
    wfdb_free_path_list();

    /* Do nothing else if no path string was supplied. */
    if (p == NULL) return (0);

    /* Register the cleanup function so that it is invoked on exit. */
    if (first_call) {
	atexit(wfdb_free_path_list);
	first_call = 0;
    }
    q = p;

    /* Now construct the wfdb_path_list from the contents of p. */
    while (*q) {
	/* Find the beginning of the next component (skip whitespace). */
	while (*q == ' ' || *q == '\t' || *q == '\n' || *q == '\r')
	    q++;
	p = q--;
	current_type = WFDB_LOCAL;
	/* Find the end of the current component. */
	found_end = 0;
	slashes = 0;
	do {
	    switch (*++q) {
	    case ':':	/* might be a component delimiter, part of '://',
			   a drive suffix (MS-DOS), or a directory separator
			   (Mac) */
		if (*(q+1) == '/' && *(q+2) == '/') current_type = WFDB_NET;
#if PSEP == ':'
		/* Allow colons within the authority portion of the URL.
		   For example, "http://[::1]:8080/database:/usr/database"
		   is a database path with two components. */
		else if (current_type != WFDB_NET || slashes > 2)
		    found_end = 1;
#endif
		break;
	    case ';':	/* definitely a component delimiter */
	    case ' ':
	    case '\t':
	    case '\n':
	    case '\r':
	    case '\0':
		found_end = 1;
		break;
	    case '/':
		slashes++;
		break;
	    }
	} while (!found_end);

	/* current component begins at p, ends at q-1 */
	SUALLOC(c1, 1, sizeof(struct wfdb_path_component));
	SALLOC(c1->prefix, q-p+1, sizeof(char));
	memcpy(c1->prefix, p, q-p);
	c1->type = current_type;
	c1->prev = c0;
	if (c0) c0->next = c1;
	else wfdb_path_list = c1;
	c0 = c1;
	if (*q) q++;
    }
    return (0);
}	


/* wfdb_getiwfdb reads a new value for WFDB from the file named by the second
through last characters of its input argument.  If that value begins with '@',
this procedure is repeated, with nesting up to ten levels.

Note that the input file must be local (it is accessed using the standard C I/O
functions rather than their wfdb_* counterparts).  This limitation is
intentional, since the alternative (to allow remote files to determine the
contents of the WFDB path) seems an unnecessary security risk. */

#ifndef SEEK_END
#define SEEK_END 2
#endif

static const char *wfdb_getiwfdb(char **p)
{
    FILE *wfdbpfile;
    int i = 0;
    long len;

    for (i = 0; i < 10 && *p != NULL && **p == '@'; i++) {
	if ((wfdbpfile = fopen((*p) + 1, RB)) == NULL) **p = 0;
	else {
	    if (fseek(wfdbpfile, 0L, SEEK_END) == 0)
		len = ftell(wfdbpfile);
	    else len = 255;
	    SALLOC(*p, 1, len+1);
	    if (*p == NULL) {
		fclose(wfdbpfile);
		break;
	    }
	    rewind(wfdbpfile);
	    len = fread(*p, 1, (int)len, wfdbpfile);
	    while ((*p)[len-1] == '\n' || (*p)[len-1] == '\r')
		(*p)[--len] = '\0';
	    (void)fclose(wfdbpfile);
	}
    }	
    if (*p != NULL && **p == '@') {
	wfdb_error("getwfdb: files nested too deeply\n");
	**p = 0;
    }
    return (*p);
}

/* wfdb_export_config is invoked from setwfdb to place the configuration
   variables into the environment if possible. */

#ifndef HAS_PUTENV
#define wfdb_export_config()
#else
static char *p_wfdb, *p_wfdbcal, *p_wfdbannsort, *p_wfdbgvmode;

/* wfdb_free_config frees all memory allocated by wfdb_export_config.
   This function must be invoked before exiting to avoid a memory leak.
   It must not be invoked at any other time, since pointers passed to
   putenv must be maintained by the caller, according to POSIX.1-2001
   semantics for putenv.  */
void wfdb_free_config(void)
{
    static char n_wfdb[] = "WFDB=";
    static char n_wfdbcal[] = "WFDBCAL=";
    static char n_wfdbannsort[] = "WFDBANNSORT=";
    static char n_wfdbgvmode[] = "WFDBGVMODE=";
    if (p_wfdb) putenv(n_wfdb);
    if (p_wfdbcal) putenv(n_wfdbcal);
    if (p_wfdbannsort) putenv(n_wfdbannsort);
    if (p_wfdbgvmode) putenv(n_wfdbgvmode);
    SFREE(p_wfdb);
    SFREE(p_wfdbcal);
    SFREE(p_wfdbannsort);
    SFREE(p_wfdbgvmode);
    SFREE(wfdbpath);
    SFREE(wfdbpath_init);
    SFREE(wfdb_filename);
}

void wfdb_export_config(void)
{
    static int first_call = 1;
    char *envstr = NULL;

    /* Register the cleanup function so that it is invoked on exit. */
    if (first_call) {
	atexit(wfdb_free_config);
	first_call = 0;
    }
    SALLOC(envstr, 1, strlen(wfdbpath)+6);
    if (envstr) {
	sprintf(envstr, "WFDB=%s", wfdbpath);
	putenv(envstr);
	SFREE(p_wfdb);
	p_wfdb = envstr;
    }
    if (getenv("WFDBCAL") == NULL) {
	SALLOC(p_wfdbcal, 1, strlen(DEFWFDBCAL)+9);
	if (p_wfdbcal) {
	    sprintf(p_wfdbcal, "WFDBCAL=%s", DEFWFDBCAL);
	    putenv(p_wfdbcal);
	}
    }
    if (getenv("WFDBANNSORT") == NULL) {
	SALLOC(p_wfdbannsort, 1, 14);
	if (p_wfdbannsort) {
	    sprintf(p_wfdbannsort, "WFDBANNSORT=%d", DEFWFDBANNSORT == 0 ? 0 : 1);
	    putenv(p_wfdbannsort);
	}
    }
    if (getenv("WFDBGVMODE") == NULL) {
	SALLOC(p_wfdbgvmode, 1, 13);
	if (p_wfdbgvmode) {
	    sprintf(p_wfdbgvmode, "WFDBGVMODE=%d", DEFWFDBGVMODE == 0 ? 0 : 1);
	    putenv(p_wfdbgvmode);
	}
    }
}
#endif

/* wfdb_addtopath adds the path component of its string argument (i.e.
everything except the file name itself) to the WFDB path, inserting it
there if it is not already in the path.  If the first component of the WFDB
path is '.' (the current directory), the new component is moved to the second
position; otherwise, it is moved to the first position.

wfdb_open calls this function whenever it finds and opens a file.

Since the files comprising a given record are most often kept in the
same directory, this strategy improves the likelihood that subsequent
files to be opened will be found in the first or second location wfdb_open
checks.

If the current directory (.) is at the head of the WFDB path, it remains there,
so that wfdb_open will continue to find the user's own files in preference to
like-named files elsewhere in the path.  If this behavior is not desired, the
current directory should not be specified initially as the first component of
the WFDB path.
 */

void wfdb_addtopath(const char *s)
{
    const char *p;
    int i, len;
    struct wfdb_path_component *c0, *c1;

    if (s == NULL || *s == '\0') return;

    /* Start at the end of the string and search backwards for a directory
       separator (accept any of the possible separators). */
    for (p = s + strlen(s) - 1; p >= s &&
	      *p != '/' && *p != '\\' && *p != ':'; p--)
	;

    /* A path component specifying the root directory must be treated as a
       special case;  normally the trailing directory separator is not
       included in the path component, but in this case there is nothing
       else to include. */

    if (p == s && (*p == '/' || *p == '\\' || *p == ':')) p++;

    if (p < s) return;		/* argument did not contain a path component */

    /* If p > s, then p points to the first character following the path
       component of s. Search the current WFDB path for this path component. */
    if (wfdbpath == NULL) (void)getwfdb();
    for (c0 = c1 = wfdb_path_list, i = p-s; c1; c1 = c1->next) {
	if (strncmp(c1->prefix, s, i) == 0) {
	    if (c0 == c1 || (c1->prev == c0 && strcmp(c0->prefix, ".") == 0))
		return; /* no changes needed, quit */
	    /* path component of s is already in WFDB path -- unlink its node */
	    if (c1->next) (c1->next)->prev = c1->prev;
	    if (c1->prev) (c1->prev)->next = c1->next;
	    break;
	}
    }
    if (!c1) {
	/* path component of s not in WFDB path -- make a new node for it */
 	SUALLOC(c1, 1, sizeof(struct wfdb_path_component));
	SALLOC(c1->prefix, p-s+1, sizeof(char));
	memcpy(c1->prefix, s, p-s);
	if (strstr(c1->prefix, "://")) c1->type = WFDB_NET;
	else c1->type = WFDB_LOCAL;
    }
    /* (Re)link the unlinked node. */
    if (strcmp(c0->prefix, ".") == 0) {  /* skip initial "." if present */
	c1->prev = c0;
	if ((c1->next = c0->next) != NULL)
	    (c1->next)->prev = c1;
	c0->next = c1;
    }	
    else { /* no initial ".";  insert the node at the head of the path */ 
	wfdb_path_list = c1;
	c1->prev = NULL;
	c1->next = c0;
	c0->prev = c1;
    }
    return;
}

#include <stdarg.h>
#ifdef va_copy
# define VA_COPY(dst, src) va_copy(dst, src)
# define VA_END2(ap)       va_end(ap)
#else
# ifdef __va_copy
#  define VA_COPY(dst, src) __va_copy(dst, src)
#  define VA_END2(ap)       va_end(ap)
# else
#  define VA_COPY(dst, src) memcpy(&(dst), &(src), sizeof(va_list))
#  define VA_END2(ap)       (void) (ap)
# endif
#endif

/* wfdb_vasprintf formats a string in the same manner as vsprintf, and
allocates a new buffer that is sufficiently large to hold the result.
The original buffer, if any, is freed afterwards (meaning that, unlike
vsprintf, it is permissible to use the original buffer as a '%s'
argument.) */
int wfdb_vasprintf(char **buffer, const char *format, va_list arguments)
{
    char *oldbuffer;
    int length, bufsize;
    va_list arguments2;

    /* make an initial guess at how large the buffer should be */
    bufsize = 2 * strlen(format) + 1;

    oldbuffer = *buffer;
    *buffer = NULL;

    while (1) {
	/* do not use SALLOC; avoid recursive calls to wfdb_error */
	if (*buffer)
	    free(*buffer);
	*buffer = malloc(bufsize);
	if (!(*buffer)) {
	    if (wfdb_me_fatal()) {
		fprintf(stderr, "WFDB: out of memory\n");
		exit(1);
	    }
	    length = 0;
	    break;
	}

	VA_COPY(arguments2, arguments);
#ifdef _WINDOWS
	length = _vsnprintf(*buffer, bufsize, format, arguments2);
#else
	length = vsnprintf(*buffer, bufsize, format, arguments2);
#endif
	VA_END2(arguments2);

	/* some pre-standard versions of 'vsnprintf' return -1 if the
	   formatted string does not fit in the buffer; in that case,
	   try again with a larger buffer */
	if (length < 0)
	    bufsize *= 2;
	/* standard 'vsnprintf' returns the actual length of the
	   formatted string */
	else if (length >= bufsize)
	    bufsize = length + 1;
	else
	    break;
    }

    if (oldbuffer)
	free(oldbuffer);
    return (length);
}

/* wfdb_asprintf formats a string in the same manner as sprintf, and
allocates a new buffer that is sufficiently large to hold the
result. */
int wfdb_asprintf(char **buffer, const char *format, ...)
{
    va_list arguments;
    int length;

    va_start(arguments, format);
    length = wfdb_vasprintf(buffer, format, arguments);
    va_end(arguments);

    return (length);
}

/* The wfdb_error function handles error messages, normally by printing them
on the standard error output.  Its arguments can be any set of arguments which
would be legal for printf, i.e., the first one is a format string, and any
additional arguments are values to be filled into the '%*' placeholders
in the format string.  It can be silenced by invoking wfdbquiet(), or
re-enabled by invoking wfdbverbose().

The function wfdberror (without the underscore) returns the most recent error
message passed to wfdb_error (even if output was suppressed by wfdbquiet).
This feature permits programs to handle errors somewhat more flexibly (in
windowing environments, for example, where using the standard error output may
be inappropriate).
*/

static int error_flag;
static char *error_message;

#ifndef WFDB_BUILD_DATE
#define WFDB_BUILD_DATE __DATE__
#endif

FSTRING wfdberror(void)
{
    if (!error_flag)
	wfdb_asprintf(&error_message,
		      "WFDB library version %d.%d.%d (%s).\n",
		      WFDB_MAJOR, WFDB_MINOR, WFDB_RELEASE, WFDB_BUILD_DATE);
    if (error_message)
	return (error_message);
    else
	return ("WFDB: cannot allocate memory for error message");
}

FVOID wfdb_error(const char *format, ...)
{
    va_list arguments;

    error_flag = 1;
    va_start(arguments, format);
    wfdb_vasprintf(&error_message, format, arguments);
    va_end(arguments);

#if 1				/* standard variant: use stderr output */
    if (error_print) {
	(void)fprintf(stderr, "%s", wfdberror());
	(void)fflush(stderr);
    }
#else				/* MS Windows variant: use message box */
    if (error_print)
	MessageBox(GetFocus(), wfdberror(), "WFDB Library Error",
		   MB_ICONASTERISK | MB_OK);
#endif
}

/* The wfdb_fprintf function handles all formatted output to files.  It is
used in the same way as the standard fprintf function, except that its first
argument is a pointer to a WFDB_FILE rather than a FILE. */

#if WFDB_NETFILES
static int nf_vfprintf(netfile *nf, const char *format, va_list ap)
{
    /* no support yet for writing to remote files */
    errno = EROFS;
    return (0);
}
#endif

static int am_readonly(WFDB_FILE *wp);

int wfdb_fprintf(WFDB_FILE *wp, const char *format, ...)
{
    int ret;
    va_list args;

    va_start(args, format);
#if WFDB_NETFILES
    if (wp->type == WFDB_NET)
	ret = nf_vfprintf(wp->netfp, format, args);
    else
#endif
    if (am_readonly(wp))
	ret = -1;		/* opened for reading only */
    else
	ret = vfprintf(wp->fp, format, args);
    va_end(args);
    return (ret);
}

#define spr1(S, RECORD, TYPE)   ((*TYPE == '\0') ? \
				 wfdb_asprintf(S, "%s", RECORD) : \
				 wfdb_asprintf(S, "%s.%s", RECORD, TYPE))
#ifdef FIXISOCD
# define spr2(S, RECORD, TYPE)  ((*TYPE == '\0') ? \
				 wfdb_asprintf(S, "%s;1", RECORD) : \
				 wfdb_asprintf(S, "%s.%.3s;1",RECORD,TYPE))
#else
# define spr2(S, RECORD, TYPE)  ((*TYPE == '\0') ? \
				 wfdb_asprintf(S, "%s.", RECORD) : \
				 wfdb_asprintf(S, "%s.%.3s", RECORD, TYPE))
#endif

static char irec[WFDB_MAXRNL+1]; /* current record name, set by wfdb_setirec */

/* wfdb_open is used by other WFDB library functions to open a database file
for reading or writing.  wfdb_open accepts two string arguments and an integer
argument.  The first string specifies the file type ("hea", "atr", etc.),
and the second specifies the record name.  The integer argument (mode) is
either WFDB_READ or WFDB_WRITE.  Note that a function which calls wfdb_open
does not need to know the filename itself; thus all system-specific details of
file naming conventions can be hidden in wfdb_open.  If the first argument is
"-", or if the first argument is "hea" and the second is "-", wfdb_open
returns a file pointer to the standard input or output as appropriate.  If
either of the string arguments is null or empty, wfdb_open takes the other as
the file name.  Otherwise, it constructs the file name by concatenating the
string arguments with a "." between them.  If the file is to be opened for
reading, wfdb_open searches for it in the list of directories obtained from
getwfdb(); output files are normally created in the current directory.  By
prefixing the record argument with appropriate path specifications, files can
be opened in any directory, provided that the WFDB path includes a null
(empty) component.

Beginning with version 10.0.1, the WFDB library accepts whitespace (space, tab,
or newline characters) as path component separators under any OS.  Multiple
consecutive whitespace characters are treated as a single path component
separator.  Use a '.' to specify the current directory as a path component when
using whitespace as a path component separator.

If the WFDB path includes components of the forms 'http://somewhere.net/mydata'
or 'ftp://somewhere.else/yourdata', the sequence '://' is explicitly recognized
as part of a URL prefix (under any OS), and the ':' and '/' characters within
the '://' are not interpreted further.  Note that the MS-DOS '\' is *not*
acceptable as an alternative to '/' in a URL prefix.  To make WFDB paths
containing URL prefixes more easily (human) readable, use whitespace for path
component separators.

WFDB file names are usually formed by concatenating the record name, a ".", and
the file type, using the spr1 macro (above).  If an input file name, as
constructed by spr1, does not match that of an existing file, wfdb_open uses
spr2 to construct an alternate file name.  In this form, the file type is
truncated to no more than 3 characters (as MS-DOS does).  When searching for
input files, wfdb_open tries both forms with each component of the WFDB path
before going on to the next path component.

If the record name is empty, wfdb_open swaps the record name and the type
string.  If the type string (after swapping, if necessary) is empty, spr1 uses
the record name as the literal file name, and spr2 uses the record name with an
appended "." as the file name.

In environments in which ISO 9660 version numbers are visible in CD-ROM file
names, define the symbol FIXISOCD.  This causes spr2 to append the
characters ";1" (the version number for an ordinary file) to the file names
it generates.  This feature is needed in order to read post-1992 CD-ROMs
with pre-5.0 versions of the Macintosh "ISO 9660 File Access" software,
with some versions of HP-UX, and possibly in other environments as well.

Pre-10.0.1 versions of this library that were compiled for environments other
than MS-DOS used file names in the format TYPE.RECORD.  This file name format
is no longer supported. */

WFDB_FILE *wfdb_open(const char *s, const char *record, int mode)
{
    char *wfdb, *p, *q, *r, *buf = NULL;
    int rlen;
    struct wfdb_path_component *c0;
    int bufsize, len, ireclen;
    WFDB_FILE *ifile;

    /* If the type (s) is empty, replace it with an empty string so that
       strcmp(s, ...) will not segfault. */
    if (s == NULL) s = "";

    /* If the record name is empty, use s as the record name and replace s
       with an empty string. */
    if (record == NULL || *record == '\0') {
	if (*s) { record = s; s = ""; }
	else return (NULL);	/* failure -- both components are empty */
    }

    /* Check to see if standard input or output is requested. */
    if (strcmp(record, "-") == 0)
	if (mode == WFDB_READ) {
	    static WFDB_FILE wfdb_stdin;

	    wfdb_stdin.type = WFDB_LOCAL;
	    wfdb_stdin.fp = stdin;
	    return (&wfdb_stdin);
	}
        else {
	    static WFDB_FILE wfdb_stdout;

	    wfdb_stdout.type = WFDB_LOCAL;
	    wfdb_stdout.fp = stdout;
	    return (&wfdb_stdout);
	}

    /* If the record name ends with '/', expand it by adding another copy of
       the final element (e.g., 'abc/123/' becomes 'abc/123/123'). */
    rlen = strlen(record);
    p = (char *)(record + rlen - 1);
    if (rlen > 1 && *p == '/') {
	for (q = p-1; q > record; q--)
	    if (*q == '/') { q++; break; }
	if (q < p) {
	    SUALLOC(r, rlen + p-q + 1, 1); /* p-q is length of final element */
	    strcpy(r, record);
	    strncpy(r + rlen, q, p-q);
	}
	else {
	    SUALLOC(r, rlen + 1, 1);
	    strcpy(r, record);
	}
    }
    else {
	SUALLOC(r, rlen + 1, 1);
	strcpy(r, record);
    }

    /* If the file is to be opened for output, use the current directory.
       An output file can be opened in another directory if the path to
       that directory is the first part of 'record'. */
    if (mode == WFDB_WRITE) {
	spr1(&wfdb_filename, r, s);
	SFREE(r);
	return (wfdb_fopen(wfdb_filename, WB));
    }
    else if (mode == WFDB_APPEND) {
	spr1(&wfdb_filename, r, s);
	SFREE(r);
	return (wfdb_fopen(wfdb_filename, AB));
    }

    /* Parse the WFDB path if not done previously. */
    if (wfdb_path_list == NULL) (void)getwfdb();

    /* If the filename begins with 'http://' or 'https://', it's a URL.  In
       this case, don't search the WFDB path, but add its parent directory
       to the path if the file can be read. */
    if (strncmp(r, "http://", 7) == 0 || strncmp(r, "https://", 8) == 0) {
	spr1(&wfdb_filename, r, s);
	if ((ifile = wfdb_fopen(wfdb_filename, RB)) != NULL) {
	    /* Found it! Add its path info to the WFDB path. */
	    wfdb_addtopath(wfdb_filename);
	    SFREE(r);
	    return (ifile);
	}
    }

    for (c0 = wfdb_path_list; c0; c0 = c0->next) {
	char *long_filename = NULL;

	ireclen = strlen(irec);
	bufsize = 64;
	SALLOC(buf, 1, bufsize);
	len = 0;
	wfdb = c0->prefix;
	while (*wfdb) {
	    while (len + ireclen >= bufsize) {
		bufsize *= 2;
		SREALLOC(buf, bufsize, 1);
	    }
	    if (!buf)
		break;

	    if (*wfdb == '%') {
		/* Perform substitutions in the WFDB path where '%' is found */
		wfdb++;
		if (*wfdb == 'r') {
		    /* '%r' -> record name */
		    (void)strcpy(buf + len, irec);
		    len += ireclen;
		    wfdb++;
		}
		else if ('1' <= *wfdb && *wfdb <= '9' && *(wfdb+1) == 'r') {
		    /* '%Nr' -> first N characters of record name */
		    int n = *wfdb - '0';

		    if (ireclen < n) n = ireclen;
		    (void)strncpy(buf + len, irec, n);
		    len += n;
		    buf[len] = '\0';
		    wfdb += 2;
		}
		else    /* '%X' -> X, if X is neither 'r', nor a non-zero digit
			   followed by 'r' */
		    buf[len++] = *wfdb++;
	    }
	    else buf[len++] = *wfdb++;
	}
	/* Unless the WFDB component was empty, or it ended with a directory
	   separator, append a directory separator to wfdb_filename;  then
	   append the record and type components.  Note that names of remote
	   files (URLs) are always constructed using '/' separators, even if
	   the native directory separator is '\' (MS-DOS) or ':' (Macintosh).
	*/
	if (len + 2 >= bufsize) {
	    bufsize = len + 2;
	    SREALLOC(buf, bufsize, 1);
	}
	if (!buf)
	    continue;
	if (len > 0) {
	    if (c0->type == WFDB_NET) {
		if (buf[len-1] != '/') buf[len++] = '/';
	    }
#ifndef MSDOS
	    else if (buf[len-1] != DSEP)
#else
	    else if (buf[len-1] != DSEP && buf[len-1] != ':')
#endif
		buf[len++] = DSEP;
	}
	buf[len] = 0;
	wfdb_asprintf(&buf, "%s%s", buf, r);
	if (!buf)
	    continue;

	spr1(&wfdb_filename, buf, s);
	if ((ifile = wfdb_fopen(wfdb_filename, RB)) != NULL) {
	    /* Found it! Add its path info to the WFDB path. */
	    wfdb_addtopath(wfdb_filename);
	    SFREE(buf);
	    SFREE(r);
	    return (ifile);
	}
	/* Not found -- try again, using an alternate form of the name,
	   provided that that form is distinct. */
	SSTRCPY(long_filename, wfdb_filename);
	spr2(&wfdb_filename, buf, s);
	if (strcmp(wfdb_filename, long_filename) && 
	    (ifile = wfdb_fopen(wfdb_filename, RB)) != NULL) {
	    wfdb_addtopath(wfdb_filename);
	    SFREE(long_filename);
	    SFREE(buf);
	    SFREE(r);
	    return (ifile);
	}
	SFREE(long_filename);
	SFREE(buf);
    }
    /* If the file was not found in any of the directories listed in wfdb,
       return a null file pointer to indicate failure. */
    SFREE(r);
    return (NULL);
}

/* wfdb_checkname checks record and annotator names -- they must not be empty,
   and they must contain only letters, digits, hyphens, tildes, underscores, and
   directory separators. */

int wfdb_checkname(const char *p, const char *s)
{
    const char *end = p + strlen(p);

    /* A record name may end with ".wfdb", designating an archive. */
    if (strcmp(s, "record") == 0 && wfdb_isarchive(p))
	end -= 5;
    do {
	if (('0' <= *p && *p <= '9') || *p == '_' || *p == '~' || *p== '-' ||
	    *p == DSEP ||
#ifdef MSDOS
	    *p == ':' || *p == '/' ||
#endif
	    ('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z'))
	    p++;
	else {
	    wfdb_error("init: illegal character %d in %s name\n", *p, s);
	    return (-1);
	}
    } while (p < end);
    return (0);
}

/* wfdb_setirec saves the current record name (its argument) in irec (defined
above) to be substituted for '%r' in the WFDB path by wfdb_open as necessary.
wfdb_setirec is invoked by isigopen (except when isigopen is invoked
recursively to open a segment within a multi-segment record) and by annopen
(when it is about to open a file for input). */

void wfdb_setirec(const char *p)
{
    const char *r;
    int len;

    for (r = p; *r; r++)
	if (*r == DSEP) p = r+1;	/* strip off any path information */
#ifdef MSDOS
	else if (*r == ':') p = r+1;
#endif
    len = strlen(p);
    if (len > WFDB_MAXRNL)
	len = WFDB_MAXRNL;
    if (strcmp(p, "-")) {       /* don't record '-' (stdin) as record name */
	strncpy(irec, p, len);
	irec[len] = '\0';
    }
}

char *wfdb_getirec(void)
{
    return (*irec ? irec: NULL);
}

/* Remove trailing '.hea' from a record name, if present. */
void wfdb_striphea(char *p)
{
    if (p) {
	int len = strlen(p);

	if (len > 4 && strcmp(p + len-4, ".hea") == 0)
	    p[len-4] = '\0';
    }
}


/* WFDB file I/O functions

The WFDB library normally reads and writes local files.  If libcurl
(http://curl.haxx.se/) is available, the WFDB library can also read files from
any accessible World Wide Web (HTTP) or FTP server.  (Writing files to a remote
WWW or FTP server may be supported in the future.)

If you do not wish to allow access to remote files, or if libcurl is not
available, simply define the symbol WFDB_NETFILES as 0 when compiling the WFDB
library.  If the symbol WFDB_NETFILES is zero, wfdblib.h defines wfdb_fread as
fread, wfdb_fwrite as fwrite, etc.;  thus in this case, the I/O is performed
using the standard C I/O functions, and the function definitions in the next
section are not compiled.  This behavior exactly mimics that of versions of the
WFDB library earlier than version 10.0.1 (which did not support remote file
access), with no additional run-time overhead.

If WFDB_NETFILES is non-zero, however, these functions are compiled.  The
WFDB_FILE pointers that are among the arguments to these functions point to
objects that may contain either (local) FILE handles or (remote) NETFILE
handles, depending on the value of the 'type' member of the WFDB_FILE object.
All access to local files is handled by passing the 'fp' member of the
WFDB_FILE object to the appropriate standard C I/O function.  Access to remote
files via http or ftp is handled by passing the 'netfp' member of the WFDB_FILE
object to the appropriate libcurl function(s).

In order to read remote files, the WFDB environment variable should include
one or more components that specify http:// or ftp:// URL prefixes.  These
components are concatenated with WFDB file names to obtain complete URLs.  For
example, if the value of WFDB is
  /usr/local/database http://dilbert.bigu.edu/wfdb /cdrom/database
then an attempt to read the header file for record xyz would look first for
/usr/local/database/xyz.hea, then http://dilbert.bigu.edu/wfdb/xyz.hea, and
finally /cdrom/database/xyz.hea.  The second and later possibilities would
be checked only if the file had not been found already.  As a practical matter,
it would be best in almost all cases to search all available local file systems
before looking on remote http or ftp servers, but the WFDB library allows you
to set the search order in any way you wish, as in this example.
*/

#if WFDB_NETFILES

/* cache redirections for 5 minutes */
#define REDIRECT_CACHE_TIME (5 * 60)

struct netfile {
  char *url;
  char *data;
  int mode;
  long base_addr;
  long cont_len;
  long pos;
  long err;
  int fd;
  char *redirect_url;
  unsigned int redirect_time;
};

static int nf_open_files = 0;		/* number of open netfiles */
static long page_size = NF_PAGE_SIZE;	/* bytes per range request (0: disable
					   range requests) */
static int www_done_init = FALSE;	/* TRUE once libcurl is initialized */

static CURL *curl_ua = NULL;

/* Construct the User-Agent string to be sent with HTTP requests. */
static char *curl_get_ua_string(void)
{
    char *libcurl_ver;
    static char *s = NULL;

    libcurl_ver = curl_version();

    /* The +3XX flag informs the server that this client understands
       and supports HTTP redirection (CURLOPT_FOLLOWLOCATION
       enabled.) */
    wfdb_asprintf(&s, "libwfdb/%d.%d.%d (%s +3XX)", WFDB_MAJOR, WFDB_MINOR,
		  WFDB_RELEASE, libcurl_ver);
    return (s);
}

static char curl_error_buf[CURL_ERROR_SIZE];

/* This function will print out the curl error message if there was an
   error.  Zero means there was no error. */
static int curl_try(CURLcode err)
{
    if (err) {
      wfdb_error("curl error: %s\n", curl_error_buf);
    }
    return err;
}

/* Get the current time, as an unsigned number of seconds since some
   arbitrary starting point. */
static unsigned int www_time(void)
{
#ifdef HAVE_CLOCK_GETTIME
    struct timespec ts;
    if (!clock_gettime(CLOCK_MONOTONIC, &ts))
	return ((unsigned int) ts.tv_sec);
#endif
    return ((unsigned int) time(NULL));
}

struct chunk {
    long size, buffer_size;
    unsigned long start_pos, end_pos, total_size;
    char *data;
    char *url;
};

/* This is a dummy write callback, for when we don't care about the
   data curl is receiving. */

static size_t curl_null_write(void *ptr, size_t size, size_t nmemb,
			      void *stream)
{
    return (size*nmemb);
}

typedef struct chunk CHUNK;
#define chunk_size(x) ((x)->size)
#define chunk_data(x) ((x)->data)
#define chunk_new curl_chunk_new
#define chunk_delete curl_chunk_delete
#define chunk_putb curl_chunk_putb

static char **passwords;

/* www_parse_passwords parses the WFDBPASSWORD environment variable.
This environment variable contains a list of URL prefixes and
corresponding usernames/passwords.  Alternatively, the environment
variable may contain '@' followed by the name of a file containing
password information.

Each item in the list consists of a URL prefix, followed by a space,
then the username and password separated by a colon.  For example,
setting WFDBPASSWORD to "https://example.org john:letmein" would use
the username "john" and the password "letmein" for all HTTPS requests
to example.org.

If there are multiple items in the list, they must be separated by
end-of-line or tab characters. */
static void www_parse_passwords(const char *str)
{
    static char sep[] = "\t\n\r";
    char *xstr = NULL, *p, *q;
    int n;

    SSTRCPY(xstr, str);
    wfdb_getiwfdb(&xstr);
    if (!xstr)
	return;

    SALLOC(passwords, 1, sizeof(char *));
    n = 0;
    for (p = strtok(xstr, sep); p; p = strtok(NULL, sep)) {
	if (!(q = strchr(p, ' ')) || !strchr(q, ':'))
	    continue;
	SREALLOC(passwords, n + 2, sizeof(char *));
	if (!passwords)
	    return;
	SSTRCPY(passwords[n], p);
	n++;
    }
    passwords[n] = NULL;

    SFREE(xstr);
}

/* www_userpwd determines which username/password should be used for a
given URL.  It returns a string of the form "username:password" if one
is defined, or returns NULL if no login information is required for
that URL. */
static const char *www_userpwd(const char *url)
{
    int i, n;
    const char *p;

    for (i = 0; passwords && passwords[i]; i++) {
	p = strchr(passwords[i], ' ');
	if (!p || p == passwords[i])
	    continue;

	n = p - passwords[i];
	if (strncmp(passwords[i], url, n) == 0 &&
	    (url[n] == 0 || url[n] == '/' || url[n - 1] == '/')) {
	    return &passwords[i][n + 1];
	}
    }

    return NULL;
}

static void wfdb_wwwquit(void)
{
    int i;
    if (www_done_init) {
#ifndef _WINDOWS
	curl_easy_cleanup(curl_ua);
	curl_ua = NULL;
	curl_global_cleanup();
#endif
	www_done_init = FALSE;
	for (i = 0; passwords && passwords[i]; i++)
	    SFREE(passwords[i]);
	SFREE(passwords);
    }
}

static void www_init(void)
{
    if (!www_done_init) {
	char *p;

	if ((p = getenv("WFDB_PAGESIZE")) && *p)
	    page_size = strtol(p, NULL, 10);

	/* Initialize the curl "easy" handle. */
	curl_global_init(CURL_GLOBAL_ALL);
	curl_ua = curl_easy_init();
	/* Buffer for error messages */
	curl_easy_setopt(curl_ua, CURLOPT_ERRORBUFFER, curl_error_buf);
	/* String to send as a User-Agent header */
	curl_easy_setopt(curl_ua, CURLOPT_USERAGENT, curl_get_ua_string());
#ifdef USE_NETRC
	/* Search $HOME/.netrc for passwords */
	curl_easy_setopt(curl_ua, CURLOPT_NETRC, CURL_NETRC_OPTIONAL);
#endif
	/* Get password information from the environment if available */
	if ((p = getenv("WFDBPASSWORD")) && *p)
            www_parse_passwords(p);

	/* Get the name of the CA bundle file */
	if ((p = getenv("CURL_CA_BUNDLE")) && *p)
	    curl_easy_setopt(curl_ua, CURLOPT_CAINFO, p);

	/* Use any available authentication method */
	curl_easy_setopt(curl_ua, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

	/* Follow up to 5 redirections */
	curl_easy_setopt(curl_ua, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_ua, CURLOPT_MAXREDIRS, 5L);

	/* Show details of URL requests if WFDB_NET_DEBUG is set */
	if ((p = getenv("WFDB_NET_DEBUG")) && *p)
	    curl_easy_setopt(curl_ua, CURLOPT_VERBOSE, 1L);

	atexit(wfdb_wwwquit);
	www_done_init = TRUE;
    }
}

/* Send a request and wait for the response.  If using HTTP, check the
   response code to see whether the request was successful. */
static int www_perform_request(CURL *c)
{
    long code;
    if (curl_easy_perform(c))
	return (-1);
    if (curl_easy_getinfo(c, CURLINFO_HTTP_CODE, &code))
	return (0);
    return (code < 400 ? 0 : -1);
}

static long www_get_cont_len(const char *url)
{
    double length;

    length = 0;
    if (/* We just want the content length; NOBODY means we want to
	   send a HEAD request rather than GET */
	curl_try(curl_easy_setopt(curl_ua, CURLOPT_NOBODY, 1L))
	/* Set the URL to retrieve */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_URL, url))
	/* Set username/password */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_USERPWD,
	                             www_userpwd(url)))
	/* Don't send a range request */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_RANGE, NULL))
	/* Ignore both headers and body */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_WRITEFUNCTION,
				     curl_null_write))
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_HEADERFUNCTION,
				     curl_null_write))
	/* Actually perform the request and wait for a response */
	|| www_perform_request(curl_ua))
	return (0);

    if (curl_easy_getinfo(curl_ua, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length))
	return (0);

    return ((long) length);
}

/* Create a new, empty chunk. */
static CHUNK *curl_chunk_new(long len)
{
    struct chunk *c;

    SUALLOC(c, 1, sizeof(struct chunk));
    SALLOC(c->data, 1, len);
    c->size = 0L;
    c->buffer_size = len;
    c->start_pos = 0;
    c->end_pos = 0;
    c->total_size = 0;
    c->url = NULL;
    return c;
}

/* Delete a chunk */
static void curl_chunk_delete(struct chunk *c)
{
    if (c) {
	SFREE(c->data);
	SFREE(c->url);
	SFREE(c);
    }
}

/* Write metadata (e.g., HTTP headers) into a chunk.  This function is
   called by curl and must take the same arguments as fwrite().  ptr
   points to the data received, size*nmemb is the number of bytes, and
   stream is the user data specified by CURLOPT_WRITEHEADER. */
static size_t curl_chunk_header_write(void *ptr, size_t size, size_t nmemb,
				      void *stream)
{
    char *s = (char *) ptr;
    struct chunk *c = (struct chunk *) stream;

    if (0 == strncasecmp(s, "Content-Range:", 14)) {
	s += 14;
	while (*s == ' ')
	    s++;
	if (0 == strncasecmp(s, "bytes ", 6))
	    sscanf(s + 6, "%lu-%lu/%lu",
		   &c->start_pos, &c->end_pos, &c->total_size);
    }
    return (size * nmemb);
}

/* Write data into a chunk.  This function is called by curl and must
   take the same arguments as fwrite().  ptr points to the data
   received, size*nmemb is the number of bytes, and stream is the user
   data specified by CURLOPT_WRITEDATA. */
static size_t curl_chunk_write(void *ptr, size_t size, size_t nmemb,
			       void *stream)
{
    size_t count=0;
    char *p;
    struct chunk *c = (struct chunk *) stream;

    while (nmemb > 0) {
	while ((c->size + size) > c->buffer_size) {
	    c->buffer_size += 1024;
	    SREALLOC(c->data, c->buffer_size, 1);
	}
	if (c->data)
	    memcpy(c->data + c->size, ptr, size);
	c->size += size;
	count += size;
	p = (char *)ptr + size;	/* avoid arithmetic on void pointer */
	ptr = (void *)p;
	nmemb--;
    }
    return (count);
}

/* This function emulates the libwww function HTChunk_putb. */
static void curl_chunk_putb(struct chunk *chunk, char *data, size_t len)
{
    curl_chunk_write(data, 1, len, chunk);
}

static CHUNK *www_get_url_range_chunk(const char *url, long startb, long len)
{
    CHUNK *chunk = NULL, *extra_chunk = NULL;
    char range_req_str[6*sizeof(long) + 2];
    const char *url2 = NULL;

    if (url && *url) {
	sprintf(range_req_str, "%ld-%ld", startb, startb+len-1);
	chunk = chunk_new(len);
	if (!chunk)
	    return (NULL);

	if (/* In this case we want to send a GET request rather than
	       a HEAD */
	    curl_try(curl_easy_setopt(curl_ua, CURLOPT_NOBODY, 0L))
	    || curl_try(curl_easy_setopt(curl_ua, CURLOPT_HTTPGET, 1L))
	    /* URL to retrieve */
	    || curl_try(curl_easy_setopt(curl_ua, CURLOPT_URL, url))
	    /* Set username/password */
	    || curl_try(curl_easy_setopt(curl_ua, CURLOPT_USERPWD,
	                                 www_userpwd(url)))
	    /* Range request */
	    || curl_try(curl_easy_setopt(curl_ua, CURLOPT_RANGE,
					 range_req_str))
	    /* This function will be used to "write" data as it is received */
	    || curl_try(curl_easy_setopt(curl_ua, CURLOPT_WRITEFUNCTION,
					 curl_chunk_write))
	    /* The pointer to pass to the write function */
	    || curl_try(curl_easy_setopt(curl_ua, CURLOPT_WRITEDATA, chunk))
	    /* This function will be used to parse HTTP headers */
	    || curl_try(curl_easy_setopt(curl_ua, CURLOPT_HEADERFUNCTION,
					 curl_chunk_header_write))
	    /* The pointer to pass to the header function */
	    || curl_try(curl_easy_setopt(curl_ua, CURLOPT_WRITEHEADER, chunk))
	    /* Perform the request */
	    || www_perform_request(curl_ua)) {

	    chunk_delete(chunk);
	    return (NULL);
	}
	if (!chunk->data) {
	    chunk_delete(chunk);
	    chunk = NULL;
	}
	else if (!curl_easy_getinfo(curl_ua, CURLINFO_EFFECTIVE_URL, &url2) &&
		 url2 && *url2 && strcmp(url, url2)) {
	    SSTRCPY(chunk->url, url2);
	}
    }
    return (chunk);
}

static CHUNK *www_get_url_chunk(const char *url)
{
    CHUNK *chunk = NULL;

    chunk = chunk_new(1024);
    if (!chunk)
	return (NULL);

    if (/* Send a GET request */
	curl_try(curl_easy_setopt(curl_ua, CURLOPT_NOBODY, 0L))
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_HTTPGET, 1L))
	/* URL to retrieve */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_URL, url))
	/* Set username/password */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_USERPWD,
				     www_userpwd(url)))
	/* No range request */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_RANGE, NULL))
	/* Write to the chunk specified ... */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_WRITEFUNCTION,
				     curl_chunk_write))
	/* ... by this pointer */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_HEADERFUNCTION,
				     curl_null_write))
	/* and ignore the header data */
	|| curl_try(curl_easy_setopt(curl_ua, CURLOPT_WRITEDATA, chunk))
	/* perform the request */
	|| www_perform_request(curl_ua)) {

	chunk_delete(chunk);
	return (NULL);
    }
    if (!chunk->data) {
	chunk_delete(chunk);
	chunk = NULL;
    }

    return (chunk);
}

static void nf_delete(netfile *nf)
{
    if (nf) {
	SFREE(nf->url);
	SFREE(nf->data);
	SFREE(nf->redirect_url);
	SFREE(nf);
    }
}

static CHUNK *nf_get_url_range_chunk(netfile *nf, long startb, long len)
{
    char *url;
    CHUNK *chunk;
    unsigned int request_time;

    /* If a previous request for this file was recently redirected,
       use the previous (redirected) URL; otherwise, use the original
       URL.  (If the system clock moves backwards, the cache is
       assumed to be out-of-date.) */
    request_time = www_time();
    if (request_time - nf->redirect_time > REDIRECT_CACHE_TIME) {
	SFREE(nf->redirect_url);
    }
    url = (nf->redirect_url ? nf->redirect_url : nf->url);

    chunk = www_get_url_range_chunk(url, startb, len);

    if (chunk && chunk->url) {
	/* don't update redirect_time if we didn't hit nf->url */
	if (!nf->redirect_url)
	    nf->redirect_time = request_time;
	SSTRCPY(nf->redirect_url, chunk->url);
    }
    return (chunk);
}

/* nf_new attempts to read (at least part of) the file named by its
   argument (normally an http:// or ftp:// url).  If page_size is nonzero and
   the file can be read in segments (this will be true for files served by http
   servers that support range requests, and possibly for other types of files
   if NETFILES support is available), nf_new reads the first page_size bytes
   (or fewer, if the file is shorter than page_size).  Otherwise, nf_new
   attempts to read the entire file into memory.  If there is insufficient
   memory, if the file contains no data, or if the file does not exist (the
   most common of these three cases), nf_new returns a NULL pointer; otherwise,
   it allocates, fills in, and returns a pointer to a netfile structure that
   can be used by nf_fread, etc., to obtain the contents of the file.

   It would be useful to be able to copy a file that cannot be read in segments
   to a local file, but this operation is not currently implemented.  The way
   to do this would be to invoke
      HTLoadToFile(url, request, filename);
*/
static netfile *nf_new(const char* url)
{
    netfile *nf;
    CHUNK *chunk = NULL;

    SUALLOC(nf, 1, sizeof(netfile));
    if (nf && url && *url) {
	SSTRCPY(nf->url, url);
	nf->base_addr = 0;
	nf->pos = 0;
	nf->data = NULL;
	nf->err = NF_NO_ERR;
	nf->fd = -1;
	nf->redirect_url = NULL;

	if (page_size > 0L)
	    /* Try to read the first part of the file. */
	    chunk = nf_get_url_range_chunk(nf, 0L, page_size);
	else
	    /* Try to read the entire file. */
	    chunk = www_get_url_chunk(nf->url);

	if (!chunk) {
	    nf_delete(nf);
	    return (NULL);
	}

	if (chunk->start_pos == 0 &&
	    chunk->end_pos == chunk->size - 1 &&
	    chunk->total_size >= chunk->size &&
	    (chunk->size == page_size || chunk->size == chunk->total_size)) {
	    /* Range request works and the total file size is known. */
	    nf->cont_len = chunk->total_size;
	    nf->mode = NF_CHUNK_MODE;
	}
	else if (chunk->total_size == 0) {
	    if (page_size > 0 && chunk->size == page_size)
		/* This might be a range response from a protocol that
		   doesn't report the file size, or might be a file
		   that happens to be exactly the size we requested.
		   Check the full size of the file. */
		nf->cont_len = www_get_cont_len(nf->url);
	    else
		nf->cont_len = chunk->size;

	    if (nf->cont_len > chunk->size)
		nf->mode = NF_CHUNK_MODE;
	    else
		nf->mode = NF_FULL_MODE;
	}
	else {
	    wfdb_error("nf_new: unexpected range response (%lu-%lu/%lu)\n",
		       chunk->start_pos, chunk->end_pos, chunk->total_size);
	    chunk_delete(chunk);
	    nf_delete(nf);
	    return (NULL);
	}
	if (chunk->size > 0L) {
	    nf->data = chunk->data;
	    chunk->data = NULL;
	}
	if (nf->data == NULL) {
	    if (chunk->size > 0L)
		wfdb_error("nf_new: insufficient memory (needed %ld bytes)\n",
			   chunk->size);
	    /* If no bytes were received, the remote file probably doesn't
	       exist.  This happens routinely while searching the WFDB path, so
	       it's not flagged as an error.  Note, however, that we can't tell
	       the difference between a nonexistent file and an empty one.
	       Another possibility is that range requests are unsupported
	       (e.g., if we're trying to read an ftp url), and there is
	       insufficient memory for the entire file. */
	    nf_delete(nf);
	    nf = NULL;
	}
	if (chunk) chunk_delete(chunk);
    }
    return(nf);
}

static long nf_get_range(netfile* nf, long startb, long len, char *rbuf)
{
    CHUNK *chunk = NULL;
    char *rp = NULL;
    long avail = nf->cont_len - startb;

    if (len > avail) len = avail;	/* limit request to available bytes */
    if (nf == NULL || nf->url == NULL || *nf->url == '\0' ||
	startb < 0L || startb >= nf->cont_len || len <= 0L || rbuf == NULL)
	return (0L);	/* invalid inputs -- fail silently */

    if (nf->mode == NF_CHUNK_MODE) {	/* range requests acceptable */
	long rlen = (avail >= page_size) ? page_size : avail;

	if (len <= page_size) {	/* short request -- check if cached */
	    if ((startb < nf->base_addr) ||
		((startb + len) > (nf->base_addr + page_size))) {
		/* requested data not in cache -- update the cache */
		if (chunk = nf_get_url_range_chunk(nf, startb, rlen)) {
		    if (chunk_size(chunk) != rlen) {
			wfdb_error(
		     "nf_get_range: requested %ld bytes, received %ld bytes\n",
		                   rlen, (long)chunk_size(chunk));
			len = 0L;
		    }
		    else {
			nf->base_addr = startb;
			memcpy(nf->data, chunk_data(chunk), rlen);
		    }
		}
		else {	/* attempt to update cache failed */
		    wfdb_error(
	     "nf_get_range: couldn't read %ld bytes of %s starting at %ld\n", 
	                       len, nf->url, startb);
		    len = 0L;
		}
	    } 
      
	    /* move cached data to the return buffer */
	    rp = nf->data + startb - nf->base_addr;
	}

	else if (chunk = nf_get_url_range_chunk(nf, startb, len)) {
	    /* long request (> page_size) */
	    if (chunk_size(chunk) != len) {
		wfdb_error(
		     "nf_get_range: requested %ld bytes, received %ld bytes\n",
		           len, (long)chunk_size(chunk));
		len = 0L;
	    }
	    rp = chunk_data(chunk);
	}
	else {
	    wfdb_error(
	       "nf_get_range: couldn't read %ld bytes of %s starting at %ld\n",
		       len, nf->url, startb);
	    len = 0L;
	}
    }

    else  /* cannot use range requests -- cache contains full file */
	rp = nf->data + startb;		

    if (rp != NULL && len > 0)
	memcpy(rbuf, rp, len);
    if (chunk) chunk_delete(chunk);
    return (len);
}

/* nf_feof returns true after reading past the end of a file but before
   repositioning the pos in the file. */
static int nf_feof(netfile *nf)
{
    return ((nf->err == NF_EOF_ERR) ? TRUE : FALSE);
}

/* nf_eof returns true if the file pointer is at the EOF. */
static int nf_eof(netfile *nf)
{
    return (nf->pos >= nf->cont_len) ? TRUE : FALSE;
}

static netfile* nf_fopen(const char *url, const char *mode)
{
    netfile* nf = NULL;

    if (!www_done_init)
	www_init();
    if (*mode == 'w' || *mode == 'a')
	errno = EROFS;	/* no support for output */
    else if (*mode != 'r')
	errno = EINVAL;	/* invalid mode string */
    else if (nf = nf_new(url))
	nf_open_files++;
    return (nf);
}

static int nf_fclose(netfile* nf)
{
    nf_delete(nf);
    nf_open_files--;
    return (0);
}

static int nf_fgetc(netfile *nf)
{
    char c;

    if (nf_get_range(nf, nf->pos++, 1, &c))
	return (c & 0xff);
    nf->err = (nf->pos >= nf->cont_len) ? NF_EOF_ERR : NF_REAL_ERR;
    return (EOF);
}

static char* nf_fgets(char *s, int size, netfile *nf)
{
    int c = 0, i = 0; 

    if (s == NULL) return (NULL);
    while (c != '\n' && i < (size-1) && (c = nf_fgetc(nf)) != EOF)
	s[i++] = c;
    if ((c == EOF) && (i == 0))	return (NULL);
    s[i] = 0;
    return (s);
}

static size_t nf_fread(void *ptr, size_t size, size_t nmemb, netfile *nf)
{
    long bytes_available, bytes_read = 0L, bytes_requested = size * nmemb;

    if (nf == NULL || ptr == NULL || bytes_requested == 0) return ((size_t)0);
    bytes_available = nf->cont_len - nf->pos;
    if (bytes_requested > bytes_available) bytes_requested = bytes_available;
    if (bytes_requested > page_size && page_size) bytes_requested = page_size;
    nf->pos += bytes_read = nf_get_range(nf, nf->pos, bytes_requested, ptr);
    return ((size_t)(bytes_read / size));
}

static int nf_fseek(netfile* nf, long offset, int whence)
{
  int ret = -1;

  if (nf)
    switch (whence) {
      case SEEK_SET:
	if (offset >= 0 && offset <= nf->cont_len) {
	  nf->pos = offset;
	  nf->err = NF_NO_ERR;
	  ret = 0;
	}
	break;
      case SEEK_CUR:
	if (((unsigned long) nf->pos + offset) <= nf->cont_len) {
	  nf->pos += offset;
	  nf->err = NF_NO_ERR;
	  ret = 0;
	}
	break;
      case SEEK_END:
	if (offset <= 0 && offset >= -nf->cont_len) {
	  nf->pos = nf->cont_len + offset;
	  nf->err = NF_NO_ERR;
	  ret = 0; 
	}
	break;
      default:
	break;
    }
  return (ret);
}

static long nf_ftell(netfile *nf)
{
    return (nf->pos);
}

static int nf_ferror(netfile *nf)
{
    return ((nf->err == NF_REAL_ERR) ? TRUE : FALSE);
}

static void nf_clearerr(netfile *nf)
{
    nf->err = NF_NO_ERR;
}

static int nf_fflush(netfile *nf)
{
    /* no support yet for writing to remote files */
    errno = EROFS;
    return (EOF);
}

static size_t nf_fwrite(const void *ptr, size_t size, size_t nmemb,netfile *nf)
{
    /* no support yet for writing to remote files */
    errno = EROFS;
    return (0);
}

static int nf_putc(int c, netfile *nf)
{
    /* no support yet for writing to remote files */
    errno = EROFS;
    return (EOF);
}

#else	/* !WFDB_NETFILES */
# define nf_feof(nf)                      (0)
# define nf_fgetc(nf)                     (EOF)
# define nf_fgets(s, size, nf)            (NULL)
# define nf_fread(ptr, size, nmemb, nf)   (0)
# define nf_fseek(nf, offset, whence)     (-1)
# define nf_ftell(nf)                     (-1)
# define nf_ferror(nf)                    (0)
# define nf_clearerr(nf)                  ((void) 0)
# define nf_fflush(nf)                    (EOF)
# define nf_fwrite(ptr, size, nmemb, nf)  (0)
# define nf_putc(c, nf)                   (EOF)
#endif

/* WFDB archive functions

A WFDB archive is a ZIP file, named RECORD.wfdb, that holds the files of a
single record: its header file (RECORD.hea), the signal files listed in the
header, and optionally annotation and other files.  All files are at the top
level of the archive.  Files are normally stored without compression, so that
any range of samples can be read without reading the rest of the signal file.

A record in an archive is opened by giving its name with the '.wfdb'
extension (for example, isigopen("100.wfdb", ...) or annopen("100.wfdb",
...)).  wfdb_open then constructs a file name such as "100.wfdb.hea", which
wfdb_fopen recognizes (using wfdb_archive_split) as the file "100.hea" within
the archive "100.wfdb".  The archive itself is found by searching the WFDB
path in the usual way.  The signal files of a record in an archive are read
from the same archive (see isigopen, in signal.c).

Files written to an archive (for example, by setheader("100.wfdb", ...),
annopen("100.wfdb", ...), or osigfopen with a signal file name such as
"100.wfdb.dat") are collected in temporary files until wfdbquit is invoked
(or until the program exits).  At that point, wfdb_archive_close_all writes a
new archive that contains these files and any other files from the existing
archive, if there is one, and moves it into place.  If anything goes wrong,
the existing archive is left unchanged.

Archives on remote (http or ftp) servers, multi-segment records in archives,
and compressed files within archives are not supported.

These functions are intended only for use by other functions in the WFDB
library:
 wfdb_isarchive		(checks whether a record name refers to an archive)
 wfdb_archive_split	(splits a file name into archive and member names)
 wfdb_archive_open	(opens a file within an archive for reading)
 wfdb_archive_name	(returns the archive containing an open WFDB_FILE)
 wfdb_archive_close_all	(writes all archives opened for output)
*/

#define WFDB_ARCHIVE_EXT	".wfdb"
#define WFDB_ARCHIVE_EXTLEN	5
#define ARCHIVE_MAX_HEADER	(1024L*1024L) /* largest header to be read */
#define ARCHIVE_MAX_CDIR	(64L*1024L*1024L) /* largest central directory */
#define ZIP_LIMIT16		0xffffUL
#define ZIP_LIMIT32		0xffffffffUL

typedef unsigned long long zip_u64;

/* A file within an archive, opened via a WFDB_FILE */
struct wfdb_member {
    char *archive;		/* path of the archive */
    char *name;			/* name of the file within the archive */
    long start;			/* (input) offset of file data in the archive */
    long size;			/* (input) length of the file */
    long pos;			/* (input) current position within the file */
    int eof, err;		/* (input) end-of-file and error indicators */
    struct archive_out_file *out; /* (output) file being written, or NULL */
};

/* An entry in the central directory of a ZIP file */
struct zip_entry {
    char *name;
    unsigned int method, flags, time, date;
    unsigned long crc;
    zip_u64 csize, usize, offset;
    struct archive_out_file *replacement; /* (output) new version, or NULL */
};

struct zip_dir {
    struct zip_entry *entry;
    unsigned long n;
};

/* A file to be written to an output archive */
struct archive_out_file {
    char *name;
    FILE *tmp;			/* temporary file holding the contents */
    int open;			/* true if a WFDB_FILE refers to this file */
    struct archive_out_file *next;
};

/* An archive opened for output */
struct archive_out {
    char *path;
    struct archive_out_file *files;
    struct archive_out *next;
};

static struct archive_out *archive_out_list;

#if !defined(MSDOS) && !defined(_WIN32)
#include <sys/stat.h>	/* for stat and chmod */
#endif

static int lower(int c)
{
    return ((c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c);
}

static int is_dsep(int c)
{
    return (c == '/' || c == DSEP
#ifdef MSDOS
	    || c == ':' || c == '\\'
#endif
	);
}

/* Returns true if s ends with ".wfdb" (in either case). */
static int has_archive_ext(const char *s, size_t len)
{
    size_t i;

    if (len <= WFDB_ARCHIVE_EXTLEN || is_dsep(s[len-WFDB_ARCHIVE_EXTLEN-1]))
	return (0);
    for (i = 0; i < WFDB_ARCHIVE_EXTLEN; i++)
	if (lower(s[len-WFDB_ARCHIVE_EXTLEN+i]) != WFDB_ARCHIVE_EXT[i])
	    return (0);
    return (1);
}

/* wfdb_isarchive returns true if a record name refers to an archive (if
   it ends with ".wfdb"). */
int wfdb_isarchive(const char *record)
{
    return (record && has_archive_ext(record, strlen(record)));
}

/* Returns true if s is a valid name for a file within an archive:  one or
   more non-empty components separated by '.', each made up of ASCII
   letters, digits, hyphens, and underscores. */
static int valid_member_name(const char *s)
{
    int n = 0;

    for ( ; *s; s++) {
	if (*s == '.') {
	    if (n == 0) return (0);
	    n = 0;
	}
	else if (('0' <= *s && *s <= '9') || ('a' <= *s && *s <= 'z') ||
		 ('A' <= *s && *s <= 'Z') || *s == '-' || *s == '_')
	    n++;
	else
	    return (0);
    }
    return (n > 0);
}

/* Returns true if s is a valid name for a header file within an archive. */
static int is_header_name(const char *s)
{
    size_t len = strlen(s);

    return (len > 4 && strcmp(s + len - 4, ".hea") == 0 &&
	    strchr(s, '.') == s + len - 4 && valid_member_name(s));
}

/* Returns a pointer to the final component of a path. */
static const char *base_name(const char *path, const char *end)
{
    const char *p = end;

    while (p > path && !is_dsep(*(p-1)))
	p--;
    return (p);
}

/* wfdb_archive_split checks whether fname is the name of a file within an
archive, of the form "PATH/RECORD.wfdb.EXT".  If so, it sets *archive to
"PATH/RECORD.wfdb" and *member to "RECORD.EXT" (both allocated using
malloc), and returns 1;  otherwise, it returns 0. */
int wfdb_archive_split(const char *fname, char **archive, char **member)
{
    const char *p, *q, *b;
    size_t len;

    *archive = *member = NULL;
    if (fname == NULL)
	return (0);
    for (p = fname + strlen(fname) - 1; p > fname; p--) {
	if (is_dsep(*p))
	    return (0);
	if (*p == '.' && has_archive_ext(fname, p - fname))
	    break;
    }
    if (p <= fname || *(p+1) == '\0')
	return (0);
    len = p - fname;
    q = p - WFDB_ARCHIVE_EXTLEN;	/* end of the record name */
    b = base_name(fname, q);
    SUALLOC(*archive, len + 1, 1);
    SUALLOC(*member, (q - b) + strlen(p) + 1, 1);
    if (*archive == NULL || *member == NULL) {
	SFREE(*archive);
	SFREE(*member);
	return (0);
    }
    memcpy(*archive, fname, len);
    memcpy(*member, b, q - b);
    strcpy(*member + (q - b), p);
    return (1);
}

/* Little-endian integer encoding and decoding */

static unsigned int zget16(const unsigned char *p)
{
    return (p[0] | (p[1] << 8));
}

static unsigned long zget32(const unsigned char *p)
{
    return ((unsigned long)zget16(p) | ((unsigned long)zget16(p+2) << 16));
}

static zip_u64 zget64(const unsigned char *p)
{
    return ((zip_u64)zget32(p) | ((zip_u64)zget32(p+4) << 32));
}

static unsigned char *zput16(unsigned char *p, unsigned int x)
{
    p[0] = x & 0xff; p[1] = (x >> 8) & 0xff;
    return (p + 2);
}

static unsigned char *zput32(unsigned char *p, unsigned long x)
{
    zput16(p, x & 0xffff); zput16(p+2, (x >> 16) & 0xffff);
    return (p + 4);
}

static unsigned char *zput64(unsigned char *p, zip_u64 x)
{
    zput32(p, (unsigned long)(x & ZIP_LIMIT32));
    zput32(p+4, (unsigned long)((x >> 32) & ZIP_LIMIT32));
    return (p + 8);
}

/* CRC-32, as used in ZIP files */
static unsigned long crc32_update(unsigned long crc, const unsigned char *buf,
				  size_t len)
{
    static unsigned long table[256];
    static int table_ready = 0;
    unsigned long c;
    int i, k;

    if (!table_ready) {
	for (i = 0; i < 256; i++) {
	    c = (unsigned long)i;
	    for (k = 0; k < 8; k++)
		c = (c & 1) ? 0xedb88320UL ^ (c >> 1) : c >> 1;
	    table[i] = c;
	}
	table_ready = 1;
    }
    crc ^= ZIP_LIMIT32;
    while (len-- > 0)
	crc = table[(crc ^ *buf++) & 0xff] ^ (crc >> 8);
    return (crc ^ ZIP_LIMIT32);
}

static int read_at(FILE *f, zip_u64 offset, void *buf, size_t len)
{
    if (offset > (zip_u64)LONG_MAX || fseek(f, (long)offset, SEEK_SET) != 0 ||
	fread(buf, 1, len, f) != len)
	return (-1);
    return (0);
}

static void free_zip_dir(struct zip_dir *d)
{
    unsigned long i;

    for (i = 0; i < d->n; i++)
	SFREE(d->entry[i].name);
    SFREE(d->entry);
    d->n = 0;
}

/* read_zip_dir reads the central directory of the ZIP file f into d.  It
   returns 0 if successful, or -1 (after reporting an error) otherwise. */
static int read_zip_dir(FILE *f, const char *path, struct zip_dir *d)
{
    unsigned char *buf = NULL, *p, *end, z[56];
    long flen, tail, eocd;
    zip_u64 n, cdsize, cdoff, z64off;
    unsigned long i;

    d->entry = NULL;
    d->n = 0;
    if (fseek(f, 0L, SEEK_END) != 0 || (flen = ftell(f)) < 22)
	goto bad;

    /* Find the end of central directory record, which is followed by a
       comment of up to 65535 bytes. */
    tail = (flen < 65535L + 22) ? flen : 65535L + 22;
    SUALLOC(buf, tail, 1);
    if (buf == NULL || read_at(f, flen - tail, buf, tail))
	goto bad;
    for (eocd = tail - 22; eocd >= 0; eocd--)
	if (zget32(buf + eocd) == 0x06054b50UL)
	    break;
    if (eocd < 0)
	goto bad;
    p = buf + eocd;
    if (zget16(p+4) != 0 && zget16(p+4) != ZIP_LIMIT16) {
	wfdb_error("WFDB: archive %s spans multiple files\n", path);
	goto err;
    }
    n = zget16(p+10);
    cdsize = zget32(p+12);
    cdoff = zget32(p+16);
    eocd += flen - tail;	/* offset of the record within the file */

    /* Read the ZIP64 end of central directory record if there is one. */
    if (n == ZIP_LIMIT16 || cdsize == ZIP_LIMIT32 || cdoff == ZIP_LIMIT32) {
	if (eocd < 20 || read_at(f, eocd - 20, z, 20) ||
	    zget32(z) != 0x07064b50UL)
	    goto bad;
	z64off = zget64(z+8);
	if (read_at(f, z64off, z, 56) || zget32(z) != 0x06064b50UL)
	    goto bad;
	n = zget64(z+32);
	cdsize = zget64(z+40);
	cdoff = zget64(z+48);
    }
    if (cdoff > (zip_u64)flen || cdsize > (zip_u64)flen - cdoff)
	goto bad;
    if (cdsize > ARCHIVE_MAX_CDIR) {
	wfdb_error("WFDB: archive %s has too many files\n", path);
	goto err;
    }
    if (n > cdsize / 46)
	goto bad;

    /* Read the central directory. */
    SFREE(buf);
    SUALLOC(buf, (size_t)cdsize, 1);
    if (buf == NULL || read_at(f, cdoff, buf, (size_t)cdsize))
	goto bad;
    SUALLOC(d->entry, (size_t)n, sizeof(struct zip_entry));
    if (d->entry == NULL)
	goto err;
    for (p = buf, end = buf + cdsize, i = 0; i < n; i++) {
	struct zip_entry *e = &d->entry[i];
	unsigned int nlen, xlen, clen;
	unsigned char *x, *xend;

	if (end - p < 46 || zget32(p) != 0x02014b50UL)
	    goto bad;
	e->flags = zget16(p+8);
	e->method = zget16(p+10);
	e->time = zget16(p+12);
	e->date = zget16(p+14);
	e->crc = zget32(p+16);
	e->csize = zget32(p+20);
	e->usize = zget32(p+24);
	nlen = zget16(p+28);
	xlen = zget16(p+30);
	clen = zget16(p+32);
	e->offset = zget32(p+42);
	if ((size_t)(end - p) < 46 + (size_t)nlen + xlen + clen)
	    goto bad;
	SUALLOC(e->name, nlen + 1, 1);
	if (e->name == NULL)
	    goto err;
	memcpy(e->name, p+46, nlen);
	d->n = i + 1;

	/* Look for 64-bit sizes and offsets in the ZIP64 extra field. */
	for (x = p + 46 + nlen, xend = x + xlen; xend - x >= 4;
	     x += 4 + zget16(x+2)) {
	    unsigned char *y = x + 4, *yend = y + zget16(x+2);

	    if (yend > xend)
		goto bad;
	    if (zget16(x) != 0x0001)
		continue;
	    if (e->usize == ZIP_LIMIT32) {
		if (yend - y < 8) goto bad;
		e->usize = zget64(y); y += 8;
	    }
	    if (e->csize == ZIP_LIMIT32) {
		if (yend - y < 8) goto bad;
		e->csize = zget64(y); y += 8;
	    }
	    if (e->offset == ZIP_LIMIT32) {
		if (yend - y < 8) goto bad;
		e->offset = zget64(y); y += 8;
	    }
	}
	p += 46 + nlen + xlen + clen;
    }
    SFREE(buf);
    return (0);

 bad:
    wfdb_error("WFDB: %s is not a valid WFDB archive\n", path);
 err:
    SFREE(buf);
    free_zip_dir(d);
    return (-1);
}

static struct zip_entry *find_zip_entry(struct zip_dir *d, const char *name)
{
    unsigned long i;

    for (i = 0; i < d->n; i++)
	if (strcmp(d->entry[i].name, name) == 0)
	    return (&d->entry[i]);
    return (NULL);
}

/* zip_data_offset finds the offset of the data for entry e in f, which is
   after the entry's local file header.  It returns 0 if successful, or
   -1 (after reporting an error) otherwise. */
static int zip_data_offset(FILE *f, const char *path, struct zip_entry *e,
			   zip_u64 *offset)
{
    unsigned char h[30];

    if (read_at(f, e->offset, h, 30) || zget32(h) != 0x04034b50UL) {
	wfdb_error("WFDB: %s is not a valid WFDB archive\n", path);
	return (-1);
    }
    *offset = e->offset + 30 + zget16(h+26) + zget16(h+28);
    return (0);
}

/* open_member opens the file named name within the archive at path for
reading.  If there is no such file, and name is the name of a header file,
the archive's only header file (if it has exactly one) is opened instead.
open_member returns NULL if the archive can't be opened or doesn't contain
the file (and reports an error if the archive is invalid). */
static WFDB_FILE *open_member(const char *path, const char *name)
{
    FILE *f;
    struct zip_dir d;
    struct zip_entry *e;
    zip_u64 start;
    unsigned long i, nheaders = 0;
    WFDB_FILE *wp = NULL;
    struct wfdb_member *m = NULL;
    size_t len = strlen(name);

    if ((f = fopen(path, RB)) == NULL)
	return (NULL);
    if (read_zip_dir(f, path, &d) < 0) {
	fclose(f);
	return (NULL);
    }
    if ((e = find_zip_entry(&d, name)) == NULL &&
	len > 4 && strcmp(name + len - 4, ".hea") == 0) {
	for (i = 0; i < d.n; i++)
	    if (is_header_name(d.entry[i].name)) {
		e = &d.entry[i];
		nheaders++;
	    }
	if (nheaders != 1)
	    e = NULL;
    }
    if (e == NULL)
	goto fail;
    if (e->flags & 1) {
	wfdb_error("WFDB: %s in archive %s is encrypted\n", e->name, path);
	goto fail;
    }
    if (e->method != 0 || e->csize != e->usize) {
	wfdb_error("WFDB: %s in archive %s is compressed (only uncompressed"
		   " files can be read from archives)\n", e->name, path);
	goto fail;
    }
    if (is_header_name(e->name) && e->usize > ARCHIVE_MAX_HEADER) {
	wfdb_error("WFDB: header %s in archive %s is larger than %ld bytes\n",
		   e->name, path, ARCHIVE_MAX_HEADER);
	goto fail;
    }
    if (zip_data_offset(f, path, e, &start) < 0)
	goto fail;
    if (start > (zip_u64)LONG_MAX || e->usize > (zip_u64)LONG_MAX - start) {
	wfdb_error("WFDB: %s in archive %s is too large\n", e->name, path);
	goto fail;
    }
    SUALLOC(wp, 1, sizeof(WFDB_FILE));
    SUALLOC(m, 1, sizeof(struct wfdb_member));
    if (wp == NULL || m == NULL)
	goto fail;
    SSTRCPY(m->archive, path);
    SSTRCPY(m->name, e->name);
    if (m->archive == NULL || m->name == NULL)
	goto fail;
    m->start = (long)start;
    m->size = (long)e->usize;
    if (fseek(f, m->start, SEEK_SET) != 0)
	goto fail;
    wp->type = WFDB_ARCHIVE;
    wp->fp = f;
    wp->member = m;
    free_zip_dir(&d);
    return (wp);

 fail:
    if (m) {
	SFREE(m->archive);
	SFREE(m->name);
	SFREE(m);
    }
    SFREE(wp);
    free_zip_dir(&d);
    fclose(f);
    return (NULL);
}

/* wfdb_archive_open opens the file named member within an archive for
   reading.  Unlike wfdb_open, it does not search the WFDB path. */
WFDB_FILE *wfdb_archive_open(const char *archive, const char *member)
{
    if (archive == NULL || member == NULL || strstr(archive, "://"))
	return (NULL);
    return (open_member(archive, member));
}

/* wfdb_archive_name returns the path of the archive that contains an open
   file, or NULL if the file is not within an archive. */
const char *wfdb_archive_name(WFDB_FILE *wp)
{
    if (wp && wp->type == WFDB_ARCHIVE)
	return (wp->member->archive);
    return (NULL);
}

static void free_archive_out(struct archive_out *a)
{
    struct archive_out_file *o, *next;

    for (o = a->files; o; o = next) {
	next = o->next;
	if (o->tmp) fclose(o->tmp);
	SFREE(o->name);
	SFREE(o);
    }
    SFREE(a->path);
    SFREE(a);
}

/* remove_out_file removes a file from an output archive. */
static void remove_out_file(struct archive_out *a, struct archive_out_file *o)
{
    struct archive_out_file **op;

    for (op = &a->files; *op; op = &(*op)->next)
	if (*op == o) {
	    *op = o->next;
	    break;
	}
    if (o->tmp) fclose(o->tmp);
    SFREE(o->name);
    SFREE(o);
}

static void archive_atexit(void)
{
    (void)wfdb_archive_close_all();
}

/* copy_member copies the contents of an uncompressed file named name
   within the archive at path, if there is one, to f. */
static int copy_member(const char *path, const char *name, FILE *f)
{
    WFDB_FILE *wp;
    char buf[8192];
    size_t n;
    int stat = 0;

    if ((wp = open_member(path, name)) == NULL)
	return (0);
    while ((n = wfdb_fread(buf, 1, sizeof(buf), wp)) > 0)
	if (fwrite(buf, 1, n, f) != n) {
	    stat = -1;
	    break;
	}
    if (wfdb_ferror(wp))
	stat = -1;
    wfdb_fclose(wp);
    return (stat);
}

/* open_output_member opens the file named name within the archive at path
for writing (mode WB) or appending (mode AB).  The file's contents are kept
in a temporary file until wfdb_archive_close_all is invoked. */
static WFDB_FILE *open_output_member(const char *path, const char *name,
				     const char *mode)
{
    static int first_call = 1;
    struct archive_out *a;
    struct archive_out_file *o, **op;
    WFDB_FILE *wp = NULL;
    struct wfdb_member *m = NULL;
    int append = (*mode == 'a');
    size_t len = strlen(path);
    const char *b = base_name(path, path + len);

    if (!valid_member_name(name)) {
	wfdb_error("WFDB: can't write %s to archive %s (names of files in"
		   " archives must contain only letters, digits, hyphens,"
		   " underscores, and dots)\n", name, path);
	return (NULL);
    }
    if (is_header_name(name) &&
	(strncmp(name, b, (path + len - WFDB_ARCHIVE_EXTLEN) - b) != 0 ||
	 strlen(name) != (size_t)(path + len - WFDB_ARCHIVE_EXTLEN - b) + 4)) {
	wfdb_error("WFDB: can't write %s to archive %s (an archive can"
		   " contain only the header file named after it)\n",
		   name, path);
	return (NULL);
    }

    /* Find the output archive, or start a new one. */
    for (a = archive_out_list; a; a = a->next)
	if (strcmp(a->path, path) == 0)
	    break;
    if (a == NULL) {
	SUALLOC(a, 1, sizeof(struct archive_out));
	if (a == NULL)
	    return (NULL);
	SSTRCPY(a->path, path);
	if (a->path == NULL) {
	    SFREE(a);
	    return (NULL);
	}
	a->next = archive_out_list;
	archive_out_list = a;
	if (first_call) {
	    atexit(archive_atexit);
	    first_call = 0;
	}
    }

    /* Find the file, or add a new one. */
    for (o = a->files; o; o = o->next)
	if (strcmp(o->name, name) == 0)
	    break;
    if (o && o->open) {
	wfdb_error("WFDB: %s in archive %s is already open\n", name, path);
	return (NULL);
    }
    if (o == NULL) {
	SUALLOC(o, 1, sizeof(struct archive_out_file));
	if (o == NULL)
	    return (NULL);
	SSTRCPY(o->name, name);
	if (o->name == NULL || (o->tmp = tmpfile()) == NULL) {
	    if (o->name)
		wfdb_error("WFDB: can't create a temporary file for %s in"
			   " archive %s\n", name, path);
	    SFREE(o->name);
	    SFREE(o);
	    return (NULL);
	}
	for (op = &a->files; *op; op = &(*op)->next)
	    ;
	*op = o;	/* keep the files in the order they were created */
	/* When appending to a file that is already in the archive, start
	   with its current contents. */
	if (append && copy_member(path, name, o->tmp) < 0) {
	    wfdb_error("WFDB: can't read %s from archive %s\n", name, path);
	    remove_out_file(a, o);
	    return (NULL);
	}
    }
    else if (!append) {
	/* Start again with an empty file. */
	fclose(o->tmp);
	if ((o->tmp = tmpfile()) == NULL) {
	    wfdb_error("WFDB: can't create a temporary file for %s in"
		       " archive %s\n", name, path);
	    remove_out_file(a, o);
	    return (NULL);
	}
    }

    SUALLOC(wp, 1, sizeof(WFDB_FILE));
    SUALLOC(m, 1, sizeof(struct wfdb_member));
    if (m) {
	SSTRCPY(m->archive, path);
	SSTRCPY(m->name, name);
    }
    if (wp == NULL || m == NULL || m->archive == NULL || m->name == NULL ||
	fseek(o->tmp, 0L, SEEK_END) != 0) {
	if (m) {
	    SFREE(m->archive);
	    SFREE(m->name);
	}
	SFREE(m);
	SFREE(wp);
	return (NULL);
    }
    m->out = o;
    o->open = 1;
    wp->type = WFDB_ARCHIVE;
    wp->fp = o->tmp;
    wp->member = m;
    return (wp);
}

/* Functions for reading and writing files within archives, used by the
   wfdb_f* functions below */

/* am_readonly returns true if wp is a file within an archive that was
   opened for reading. */
static int am_readonly(WFDB_FILE *wp)
{
    return (wp->type == WFDB_ARCHIVE && wp->member->out == NULL);
}

static int am_getc(WFDB_FILE *wp)
{
    struct wfdb_member *m = wp->member;
    int c;

    if (m->out)
	return (getc(wp->fp));
    if (m->pos >= m->size) {
	m->eof = 1;
	return (EOF);
    }
    if ((c = getc(wp->fp)) == EOF) {
	m->err = ferror(wp->fp) ? 1 : 0;
	m->eof = 1;
	return (EOF);
    }
    m->pos++;
    return (c);
}

static char *am_fgets(char *s, int size, WFDB_FILE *wp)
{
    int c = 0, i = 0;

    if (wp->member->out)
	return (fgets(s, size, wp->fp));
    if (size <= 0)
	return (NULL);
    while (i < size - 1 && (c = am_getc(wp)) != EOF) {
	s[i++] = c;
	if (c == '\n')
	    break;
    }
    if (i == 0)
	return (NULL);
    s[i] = '\0';
    return (s);
}

static size_t am_fread(void *ptr, size_t size, size_t nmemb, WFDB_FILE *wp)
{
    struct wfdb_member *m = wp->member;
    size_t n, avail;

    if (m->out)
	return (fread(ptr, size, nmemb, wp->fp));
    if (size == 0 || nmemb == 0)
	return (0);
    avail = (m->pos < m->size) ? (size_t)(m->size - m->pos) : 0;
    if (nmemb > avail / size) {
	nmemb = avail / size;
	m->eof = 1;
    }
    n = fread(ptr, size, nmemb, wp->fp);
    m->pos += (long)(n * size);
    if (n < nmemb) {
	m->err = ferror(wp->fp) ? 1 : 0;
	m->eof = 1;
    }
    return (n);
}

static int am_fseek(WFDB_FILE *wp, long offset, int whence)
{
    struct wfdb_member *m = wp->member;
    long pos;

    if (m->out)
	return (fseek(wp->fp, offset, whence));
    switch (whence) {
      case SEEK_SET: pos = 0; break;
      case SEEK_CUR: pos = m->pos; break;
      case SEEK_END: pos = m->size; break;
      default: return (-1);
    }
    if ((offset < 0 && pos + offset < 0) ||
	(offset > 0 && offset > LONG_MAX - m->start - pos))
	return (-1);
    pos += offset;
    if (fseek(wp->fp, m->start + pos, SEEK_SET) != 0)
	return (-1);
    m->pos = pos;
    m->eof = 0;
    return (0);
}

static int archive_mkdirs(char *fname);

/* write_archive writes a new archive containing the files in a, and any
other files from the existing archive (if any), to a temporary file, then
moves it into place.  It returns 0 if successful, or -1 (after reporting an
error) otherwise;  in that case, any existing archive is left unchanged. */
static int write_archive(struct archive_out *a)
{
    FILE *old = NULL, *f = NULL;
    struct zip_dir d;
    struct zip_entry *cd = NULL, *e;
    struct archive_out_file *o;
    char *tmpname = NULL, buf[8192];
    unsigned char h[64], *p;
    unsigned long nfiles = 0, i, nentries;
    zip_u64 off = 0, cdoff, cdsize = 0;
    size_t n, len = strlen(a->path);
    const char *b = base_name(a->path, a->path + len);
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    unsigned int dostime = 0, dosdate = (1 << 5) | 1;
    int status = -1, replacing = 0, attempt, cstat;
#if !defined(MSDOS) && !defined(_WIN32)
    struct stat st;
#endif

    d.entry = NULL;
    d.n = 0;
    if (tm && tm->tm_year >= 80) {
	dostime = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
	dosdate = ((tm->tm_year - 80) << 9) | ((tm->tm_mon + 1) << 5) |
	    tm->tm_mday;
    }

    /* Read the directory of the existing archive, if there is one. */
    if ((old = fopen(a->path, RB)) != NULL) {
	replacing = 1;
	if (read_zip_dir(old, a->path, &d) < 0)
	    goto done;
    }

    /* Create a temporary file next to the archive. */
    for (attempt = 0; attempt < 100; attempt++) {
	FILE *tf;

	wfdb_asprintf(&tmpname, "%.*s.%s.%04x%04x.tmp", (int)(b - a->path),
		      a->path, b, (unsigned int)(t & 0xffff),
		      (unsigned int)((attempt * 40503U + (unsigned long)clock())
				     & 0xffff));
	if (tmpname == NULL)
	    goto done;
	if ((tf = fopen(tmpname, RB)) != NULL) {
	    fclose(tf);
	    continue;
	}
	if ((f = fopen(tmpname, WB)) == NULL) {
	    archive_mkdirs(tmpname);
	    f = fopen(tmpname, WB);
	}
	break;
    }
    if (f == NULL) {
	wfdb_error("WFDB: can't create a temporary file for archive %s\n",
		   a->path);
	goto done;
    }

    /* Make a list of the files to be written:  files from the existing
       archive (unless they are being replaced), followed by new files. */
    for (o = a->files; o; o = o->next)
	nfiles++;
    SUALLOC(cd, d.n + nfiles, sizeof(struct zip_entry));
    if (cd == NULL)
	goto done;
    nentries = 0;
    for (i = 0; i < d.n; i++) {
	for (o = a->files; o; o = o->next)
	    if (strcmp(o->name, d.entry[i].name) == 0)
		break;
	if (o == NULL)
	    cd[nentries++] = d.entry[i];
    }
    for (o = a->files; o; o = o->next) {
	e = &cd[nentries++];
	e->name = o->name;
	e->replacement = o;
	e->method = e->flags = 0;
	e->time = dostime;
	e->date = dosdate;
    }

    /* Write each file, preceded by its local file header. */
    for (i = 0; i < nentries; i++) {
	zip_u64 start, remaining;
	unsigned int nlen = strlen(cd[i].name);
	FILE *src;
	int zip64;

	e = &cd[i];
	if (e->replacement) {
	    /* Find the CRC and size of the new file. */
	    src = e->replacement->tmp;
	    if (fflush(src) != 0 || fseek(src, 0L, SEEK_SET) != 0)
		goto ioerr;
	    e->crc = 0;
	    e->usize = 0;
	    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
		e->crc = crc32_update(e->crc, (unsigned char *)buf, n);
		e->usize += n;
	    }
	    if (ferror(src) || fseek(src, 0L, SEEK_SET) != 0)
		goto ioerr;
	    e->csize = e->usize;
	    start = 0;
	}
	else {
	    /* Copy a file from the existing archive as it is. */
	    src = old;
	    if (zip_data_offset(old, a->path, e, &start) < 0)
		goto done;
	    if (start > (zip_u64)LONG_MAX ||
		fseek(old, (long)start, SEEK_SET) != 0)
		goto ioerr;
	    e->flags &= ~8U;	/* sizes are in the local header */
	}
	zip64 = (e->usize >= ZIP_LIMIT32 || e->csize >= ZIP_LIMIT32);
	p = zput32(h, 0x04034b50UL);
	p = zput16(p, zip64 ? 45 : 20);
	p = zput16(p, e->flags);
	p = zput16(p, e->method);
	p = zput16(p, e->time);
	p = zput16(p, e->date);
	p = zput32(p, e->crc);
	p = zput32(p, zip64 ? ZIP_LIMIT32 : (unsigned long)e->csize);
	p = zput32(p, zip64 ? ZIP_LIMIT32 : (unsigned long)e->usize);
	p = zput16(p, nlen);
	p = zput16(p, zip64 ? 20 : 0);
	if (zip64) {
	    p = zput16(p, 0x0001);
	    p = zput16(p, 16);
	    p = zput64(p, e->usize);
	    p = zput64(p, e->csize);
	}
	if (fwrite(h, 1, 30, f) != 30 ||
	    fwrite(e->name, 1, nlen, f) != nlen ||
	    (zip64 && fwrite(h + 30, 1, 20, f) != 20))
	    goto ioerr;
	e->offset = off;
	off += 30 + nlen + (zip64 ? 20 : 0);
	for (remaining = e->csize; remaining > 0; remaining -= n) {
	    n = remaining < sizeof(buf) ? (size_t)remaining : sizeof(buf);
	    if (fread(buf, 1, n, src) != n || fwrite(buf, 1, n, f) != n)
		goto ioerr;
	}
	off += e->csize;
    }

    /* Write the central directory. */
    cdoff = off;
    for (i = 0; i < nentries; i++) {
	unsigned int nlen = strlen(cd[i].name), xlen = 0;
	unsigned char x[28], *q = x + 4;

	e = &cd[i];
	if (e->usize >= ZIP_LIMIT32) q = zput64(q, e->usize);
	if (e->csize >= ZIP_LIMIT32) q = zput64(q, e->csize);
	if (e->offset >= ZIP_LIMIT32) q = zput64(q, e->offset);
	if (q > x + 4) {
	    zput16(x, 0x0001);
	    zput16(x + 2, (unsigned int)(q - x - 4));
	    xlen = q - x;
	}
	p = zput32(h, 0x02014b50UL);
	p = zput16(p, (3 << 8) | (xlen ? 45 : 20));	/* made by: Unix */
	p = zput16(p, xlen ? 45 : 20);
	p = zput16(p, e->flags);
	p = zput16(p, e->method);
	p = zput16(p, e->time);
	p = zput16(p, e->date);
	p = zput32(p, e->crc);
	p = zput32(p, e->csize >= ZIP_LIMIT32 ? ZIP_LIMIT32 :
		   (unsigned long)e->csize);
	p = zput32(p, e->usize >= ZIP_LIMIT32 ? ZIP_LIMIT32 :
		   (unsigned long)e->usize);
	p = zput16(p, nlen);
	p = zput16(p, xlen);
	p = zput16(p, 0);			/* comment length */
	p = zput16(p, 0);			/* disk number */
	p = zput16(p, 0);			/* internal attributes */
	p = zput32(p, 0100644UL << 16);		/* external attributes */
	p = zput32(p, e->offset >= ZIP_LIMIT32 ? ZIP_LIMIT32 :
		   (unsigned long)e->offset);
	if (fwrite(h, 1, 46, f) != 46 || fwrite(e->name, 1, nlen, f) != nlen ||
	    fwrite(x, 1, xlen, f) != xlen)
	    goto ioerr;
	cdsize += 46 + nlen + xlen;
    }
    off += cdsize;

    /* Write the end of central directory record, preceded by ZIP64
       records if needed. */
    if (nentries >= ZIP_LIMIT16 || cdsize >= ZIP_LIMIT32 ||
	cdoff >= ZIP_LIMIT32) {
	p = zput32(h, 0x06064b50UL);
	p = zput64(p, 44);
	p = zput16(p, (3 << 8) | 45);
	p = zput16(p, 45);
	p = zput32(p, 0);
	p = zput32(p, 0);
	p = zput64(p, nentries);
	p = zput64(p, nentries);
	p = zput64(p, cdsize);
	p = zput64(p, cdoff);
	p = zput32(p, 0x07064b50UL);
	p = zput32(p, 0);
	p = zput64(p, off);
	p = zput32(p, 1);
	if (fwrite(h, 1, 76, f) != 76)
	    goto ioerr;
    }
    p = zput32(h, 0x06054b50UL);
    p = zput16(p, 0);
    p = zput16(p, 0);
    p = zput16(p, nentries >= ZIP_LIMIT16 ? ZIP_LIMIT16 : nentries);
    p = zput16(p, nentries >= ZIP_LIMIT16 ? ZIP_LIMIT16 : nentries);
    p = zput32(p, cdsize >= ZIP_LIMIT32 ? ZIP_LIMIT32 : (unsigned long)cdsize);
    p = zput32(p, cdoff >= ZIP_LIMIT32 ? ZIP_LIMIT32 : (unsigned long)cdoff);
    p = zput16(p, 0);
    if (fwrite(h, 1, 22, f) != 22 || fflush(f) != 0 || ferror(f))
	goto ioerr;
    cstat = fclose(f);
    f = NULL;
    if (cstat != 0)
	goto ioerr;
    if (old) {
	fclose(old);
	old = NULL;
    }

    /* Give a replaced archive's permissions to the new one, then move the
       new archive into place. */
#if !defined(MSDOS) && !defined(_WIN32)
    if (replacing && stat(a->path, &st) == 0)
	(void)chmod(tmpname, st.st_mode & 07777);
#endif
    if (rename(tmpname, a->path) != 0) {
#if defined(MSDOS) || defined(_WIN32)
	/* rename doesn't replace an existing file on MS-Windows */
	if (!replacing || remove(a->path) != 0 ||
	    rename(tmpname, a->path) != 0)
#endif
	{
	    wfdb_error("WFDB: can't create archive %s\n", a->path);
	    goto done;
	}
    }
    status = 0;
    goto done;

 ioerr:
    wfdb_error("WFDB: error writing archive %s\n", a->path);
 done:
    if (f) fclose(f);
    if (old) fclose(old);
    if (status != 0 && tmpname) remove(tmpname);
    SFREE(tmpname);
    SFREE(cd);
    free_zip_dir(&d);
    return (status);
}

/* wfdb_archive_close_all writes all archives opened for output, and
   releases the resources allocated for them.  It is invoked by wfdbquit,
   and on exit.  It returns 0 if successful, or -1 if any archive could not
   be written. */
int wfdb_archive_close_all(void)
{
    struct archive_out *a;
    int stat = 0;

    while ((a = archive_out_list) != NULL) {
	archive_out_list = a->next;
	if (write_archive(a) < 0)
	    stat = -1;
	free_archive_out(a);
    }
    return (stat);
}

/* The definition of nf_vfprintf (which is a stub) has been moved;  it is
   now just before wfdb_fprintf, which refers to it.  There is no completely
   portable way to make a forward reference to a static (local) function. */

void wfdb_clearerr(WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	nf_clearerr(wp->netfp);
    else {
	if (wp->type == WFDB_ARCHIVE)
	    wp->member->eof = wp->member->err = 0;
	clearerr(wp->fp);
    }
}

int wfdb_feof(WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	return (nf_feof(wp->netfp));
    if (wp->type == WFDB_ARCHIVE && !wp->member->out)
	return (wp->member->eof);
    return (feof(wp->fp));
}

int wfdb_ferror(WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	return (nf_ferror(wp->netfp));
    if (wp->type == WFDB_ARCHIVE && !wp->member->out)
	return (wp->member->err || ferror(wp->fp));
    return (ferror(wp->fp));
}

int wfdb_fflush(WFDB_FILE *wp)
{
    if (wp == NULL) {	/* flush all WFDB_FILEs */
	nf_fflush(NULL);
	return (fflush(NULL));
    }
    else if (wp->type == WFDB_NET)
	return (nf_fflush(wp->netfp));
    else if (wp->type == WFDB_ARCHIVE && !wp->member->out)
	return (0);
    else
	return (fflush(wp->fp));
}

char* wfdb_fgets(char *s, int size, WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	return (nf_fgets(s, size, wp->netfp));
    if (wp->type == WFDB_ARCHIVE)
	return (am_fgets(s, size, wp));
    return (fgets(s, size, wp->fp));
}

size_t wfdb_fread(void *ptr, size_t size, size_t nmemb, WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	return (nf_fread(ptr, size, nmemb, wp->netfp));
    if (wp->type == WFDB_ARCHIVE)
	return (am_fread(ptr, size, nmemb, wp));
    return (fread(ptr, size, nmemb, wp->fp));
}

int wfdb_fseek(WFDB_FILE *wp, long int offset, int whence)
{
    if (wp->type == WFDB_NET)
	return (nf_fseek(wp->netfp, offset, whence));
    if (wp->type == WFDB_ARCHIVE)
	return (am_fseek(wp, offset, whence));
    return(fseek(wp->fp, offset, whence));
}

long wfdb_ftell(WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	return (nf_ftell(wp->netfp));
    if (wp->type == WFDB_ARCHIVE && !wp->member->out)
	return (wp->member->pos);
    return (ftell(wp->fp));
}

size_t wfdb_fwrite(const void *ptr, size_t size, size_t nmemb, WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	return (nf_fwrite(ptr, size, nmemb, wp->netfp));
    if (wp->type == WFDB_ARCHIVE && !wp->member->out)
	return (0);		/* opened for reading only */
    return (fwrite(ptr, size, nmemb, wp->fp));
}

int wfdb_getc(WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	return (nf_fgetc(wp->netfp));
    if (wp->type == WFDB_ARCHIVE)
	return (am_getc(wp));
    return (getc(wp->fp));
}

int wfdb_putc(int c, WFDB_FILE *wp)
{
    if (wp->type == WFDB_NET)
	return (nf_putc(c, wp->netfp));
    if (wp->type == WFDB_ARCHIVE && !wp->member->out)
	return (EOF);		/* opened for reading only */
    return (putc(c, wp->fp));
}

int wfdb_fclose(WFDB_FILE *wp)
{
    int status;

    if (wp->type == WFDB_ARCHIVE) {
	struct wfdb_member *m = wp->member;

	if (m->out) {	/* the file is written when the archive is closed */
	    status = fflush(wp->fp);
	    if (ferror(wp->fp)) status = EOF;
	    m->out->open = 0;
	}
	else
	    status = fclose(wp->fp);
	SFREE(m->archive);
	SFREE(m->name);
	SFREE(m);
	SFREE(wp);
	return (status);
    }
#if WFDB_NETFILES
    status = (wp->type == WFDB_NET) ? nf_fclose(wp->netfp) : fclose(wp->fp);
#else
    status = fclose(wp->fp);
#endif
    if (wp->fp != stdin)
	SFREE(wp);
    return (status);
}

WFDB_FILE *wfdb_fopen(char *fname, const char *mode)
{
    char *p = fname, *archive, *member;
    WFDB_FILE *wp;

    if (p == NULL || strstr(p, ".."))
	return (NULL);

    /* Check for a file within an archive (see wfdb_archive_split). */
    if (wfdb_archive_split(fname, &archive, &member)) {
	if (strstr(archive, "://"))
	    wp = NULL;	/* remote archives are not supported */
	else if (*mode == 'r')
	    wp = open_member(archive, member);
	else
	    wp = open_output_member(archive, member, mode);
	SFREE(archive);
	SFREE(member);
	return (wp);
    }

    SUALLOC(wp, 1, sizeof(WFDB_FILE));
    if (strstr(p, "://")) {
#if WFDB_NETFILES
	if (wp->netfp = nf_fopen(fname, mode)) {
	    wp->type = WFDB_NET;
	    return (wp);
	}
#endif
	SFREE(wp);
	return (NULL);
    }
    if (wp->fp = fopen(fname, mode)) {
	wp->type = WFDB_LOCAL;
	return (wp);
    }
    if (strcmp(mode, WB) == 0 || strcmp(mode, AB) == 0) {
	/* An attempt to create an output file failed.  Check to see if all
	   of the directories in the path exist, create them if necessary
	   and possible, then try again. */
	if (archive_mkdirs(fname) == 0 && (wp->fp = fopen(fname, mode))) {
		wp->type = WFDB_LOCAL;
		return (wp);
	}
    }
    SFREE(wp);
    return (NULL);
}

/* archive_mkdirs creates any missing directories in the path fname.  Only
   the last attempt to create a directory matters:  it returns 0 if and only
   if that attempt was successful, in which case the caller should try again
   to create the output file. */
static int archive_mkdirs(char *fname)
{
    char *p;
    int stat = 1;

    for (p = fname; *p; p++)
	if (*p == '/') {	/* only Unix-style directory separators */
	    *p = '\0';
	    stat = MKDIR(fname, 0755);
	    /* MKDIR is defined as mkdir in wfdblib.h;  depending on
	       the platform, mkdir may take two arguments (POSIX), or
	       only the first argument (MS-Windows without Cygwin).
	       The '0755' means that (under Unix), the directory will
	       be world-readable, but writable only by the owner. */
	    *p = '/';
	}
    return (stat);
}

/* Miscellaneous OS-specific functions. */

#ifdef NOSTRTOK
char *strtok(char *p, char *sep)
{
    char *psep;
    static char *s;

    if (p) s = p;
    if (!s || !(*s) || !sep || !(*sep)) return ((char *)NULL);
    for (psep = sep; *psep; psep++)
	if (*s == *psep) {
	    if (!(*(++s))) return ((char *)NULL);
	    psep = sep;
	}
    p = s;
    while (*s)
	for (psep = sep; *psep; psep++)
	    if (*s == *psep) {
		*s++ == '\0';
		return (p);
	    }
    return (p);
}
#endif

#ifdef _WINDLL
#ifndef _WIN32
/* Functions named LibMain and WEP are required in all 16-bit MS Windows
dynamic link libraries. The private library function wgetenv is a replacement
for the standard C getenv function, useful since the DOS environment is not
available to Windows DLLs except by use of the Windows GetDOSEnvironment
function.
*/

FINT LibMain(HINSTANCE hinst, WORD wDataSeg, WORD cbHeapSize,
	               LPSTR lpszCmdLine)
{
    if (cbHeapSize != 0)	/* if DLL data segment is moveable */
	UnlockData(0);
    return (1);
}

FINT WEP(int nParameter)
{
    if (nParameter == WEP_SYSTEM_EXIT || WEP_FREE_DLL)
	wfdbquit();	/* Always close WFDB files before shutdown or
			   when the last WFDB application exits. */
    return (1);
}


/* This is a quick-and-dirty reimplementation of getenv for the Windows 16-bit
   DLL environment.  It searches the MSDOS environment for a line beginning
   with the specified variable name, followed by '='.  This function can be
   fooled by pathologic variable names (e.g., with embedded '=' characters),
   but should be adequate for typical use. */

char FAR *wgetenv(char far *var)
{
    char FAR *ep = GetDOSEnvironment();
    int l = _fstrlen(var);
    
    if (var == (char FAR *)NULL || *var = '\0') return ((char FAR *)NULL);
    while (*ep) {
	if (_fstrncmp(ep, var, l) == 0 && *(ep+l) == '=')
	    /* Got it!  Skip '=', return a pointer to the value string. */
	    return (ep+l+1);
	/* Go on to the next environment string. */
	ep += _fstrlen(ep) + 1;
    }

    /* If here, the specified variable was not found in the environment. */
    return ((char FAR *)NULL);
}

#else	/* 32-bit MS Windows DLLs require DllMain instead of LibMain and WEP */

int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
    switch (fdwReason) {
	/* nothing to do when process (or thread) begins */
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
	break;
	/* when process terminates, always close WFDB files before shutdown or
	   when the last WFDB application exits */
      case DLL_PROCESS_DETACH:
	wfdbquit();
	break;
    }
    return (TRUE);
}
#endif
#endif
