/*
 * xsel -- manipulate the X selection
 * Copyright (C) 2001 Conrad Parker <conrad@vergenet.net>
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */

/*
 * This file was modified by Karsten Heinze <karsten.heinze@sidenotes.de>.
 * See xsel.c.orig for the original source. Xsel is used as library to
 * allow selection of passwords to clipboard in sesame.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <setjmp.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "xsel.h"
#include "include/xselection.h"


/* The name of the lib.*/
static char progname[] = "xsel";

/* Verbosity level for debugging */
static int debug_level = DEBUG_LEVEL;
//static int debug_level = D_TRACE;

/* Our X Display and Window */
static Display * display;
static Window window;

/* Maxmimum request size supported by this X server */
static long max_req;

/* Our timestamp for all operations */
static Time timestamp;

static Atom timestamp_atom; /* The TIMESTAMP atom */
static Atom multiple_atom; /* The MULTIPLE atom */
static Atom targets_atom; /* The TARGETS atom */
static Atom delete_atom; /* The DELETE atom */
static Atom incr_atom; /* The INCR atom */
static Atom null_atom; /* The NULL atom */
static Atom text_atom; /* The TEXT atom */
static Atom utf8_atom; /* The UTF8 atom */
static Atom compound_text_atom; /* The COMPOUND_TEXT atom */

/* Number of selection targets served by this.
 * (MULTIPLE, INCR, TARGETS, TIMESTAMP, DELETE, TEXT, UTF8_STRING and STRING)
 * NB. We do not currently serve COMPOUND_TEXT; we can retrieve it but do not
 * perform charset conversion.
 */
#define MAX_NUM_TARGETS 9
static int NUM_TARGETS;
static Atom supported_targets[MAX_NUM_TARGETS];

static int total_input = 0;
static int current_alloc = 0;


/*
 * exit_err (fmt)
 *
 * Print a formatted error message and errno information to stderr,
 * then exit with return code 1.
 */
static void
exit_err (const char * fmt, ...)
{
  va_list ap;
  int errno_save;
  char buf[MAXLINE];
  int n;

  errno_save = errno;

  va_start (ap, fmt);

  snprintf (buf, MAXLINE, "%s: ", progname);
  n = strlen (buf);

  vsnprintf (buf+n, MAXLINE-n, fmt, ap);
  n = strlen (buf);

  snprintf (buf+n, MAXLINE-n, ": %s\n", strerror (errno_save));

  fflush (stdout); /* in case stdout and stderr are the same */
  fputs (buf, stderr);
  fflush (NULL);

  va_end (ap);
  exit (1);
}

/*
 * print_err (fmt)
 *
 * Print a formatted error message to stderr.
 */
static void
print_err (const char * fmt, ...)
{
  va_list ap;
  int errno_save;
  char buf[MAXLINE];
  int n;

  errno_save = errno;

  va_start (ap, fmt);

  snprintf (buf, MAXLINE, "%s: ", progname);
  n = strlen (buf);

  vsnprintf (buf+n, MAXLINE-n, fmt, ap);
  n = strlen (buf);

  fflush (stdout); /* in case stdout and stderr are the same */
  fputs (buf, stderr);
  fputc ('\n', stderr);
  fflush (NULL);

  va_end (ap);
}

/*
 * print_debug (level, fmt)
 *
 * Print a formatted debugging message of level 'level' to stderr
 */
#define print_debug(x,y...) {if (x <= debug_level) print_err (y);}

/*
 * get_atom_name (atom)
 *
 * Returns a string with a printable name for the Atom 'atom'.
 */
static char *
get_atom_name (Atom atom)
{
  char * ret;
  static char atom_name[MAXLINE+1];

  if (atom == None) return "None";
  if (atom == XA_STRING) return "STRING";
  if (atom == XA_PRIMARY) return "PRIMARY";
  if (atom == XA_SECONDARY) return "SECONDARY";
  if (atom == timestamp_atom) return "TIMESTAMP";
  if (atom == multiple_atom) return "MULTIPLE";
  if (atom == targets_atom) return "TARGETS";
  if (atom == delete_atom) return "DELETE";
  if (atom == incr_atom) return "INCR";
  if (atom == null_atom) return "NULL";
  if (atom == text_atom) return "TEXT";
  if (atom == utf8_atom) return "UTF8_STRING";

  ret = XGetAtomName (display, atom);
  strncpy (atom_name, ret, sizeof (atom_name));
  if (atom_name[MAXLINE] != '\0')
    {
      atom_name[MAXLINE-3] = '.';
      atom_name[MAXLINE-2] = '.';
      atom_name[MAXLINE-1] = '.';
      atom_name[MAXLINE] = '\0';
    }
  XFree (ret);

  return atom_name;
}

/*
 * debug_property (level, requestor, property, target, length)
 *
 * Print debugging information (at level 'level') about a property received.
 */
static void
debug_property (int level, Window requestor, Atom property, Atom target,
                unsigned long length)
{
  print_debug (level, "Got window property: requestor 0x%x, property 0x%x, target 0x%x %s, length %ld bytes", requestor, property, target, get_atom_name (target), length);
}

/*
 * xs_malloc (size)
 *
 * Malloc wrapper. Always returns a successful allocation. Exits if the
 * allocation didn't succeed.
 */
static void *
xs_malloc (size_t size)
{
  void * ret;

  if (size == 0) size = 1;
  if ((ret = malloc (size)) == NULL) {
    exit_err ("malloc error");
  }

  return ret;
}

/*
 * xs_strlen (s)
 *
 * strlen wrapper for unsigned char *
 */
#define xs_strlen(s) (strlen ((const char *) s))

/*
 * xs_strncpy (s)
 *
 * strncpy wrapper for unsigned char *
 */
#define xs_strncpy(dest,s,n) (_xs_strncpy ((char *)dest, (const char *)s, n))
static char *
_xs_strncpy (char * dest, const char * src, size_t n)
{
  if (n > 0) {
    strncpy (dest, src, n);
    dest[n-1] = '\0';
  }
  return dest;
}

/*
 * The set of terminal signals we block while handling SelectionRequests.
 *
 * If we exit in the middle of handling a SelectionRequest, we might leave the
 * requesting client hanging, so we try to be nice and finish handling
 * requests before terminating.  Hence we block SIG{ALRM,INT,TERM} while
 * handling requests and unblock them only while waiting in XNextEvent().
 */
static sigset_t exit_sigs;

static void block_exit_sigs(void)
{
  sigprocmask (SIG_BLOCK, &exit_sigs, NULL);
}

static void unblock_exit_sigs(void)
{
  sigprocmask (SIG_UNBLOCK, &exit_sigs, NULL);
}

/*
 * get_timestamp ()
 *
 * Get the current X server time.
 *
 * This is done by doing a zero-length append to a random property of the
 * window, and checking the time on the subsequent PropertyNotify event.
 *
 * PRECONDITION: the window must have PropertyChangeMask set.
 */
static Time
get_timestamp (void)
{
  XEvent event;

  XChangeProperty (display, window, XA_WM_NAME, XA_STRING, 8,
                   PropModeAppend, NULL, 0);

  while (1) {
    XNextEvent (display, &event);

    if (event.type == PropertyNotify)
      return event.xproperty.time;
  }
}

/*
 * SELECTION RETRIEVAL
 * ===================
 *
 * The following functions implement retrieval of an X selection,
 * optionally within a user-specified timeout.
 *
 *
 * Selection timeout handling.
 * ---------------------------
 *
 * The selection retrieval can time out if no response is received within
 * a user-specified time limit. In order to ensure we time the entire
 * selection retrieval, we use an interval timer and catch SIGALRM.
 * [Calling select() on the XConnectionNumber would only provide a timeout
 * to the first XEvent.]
 */

/*
 * get_append_property ()
 *
 * Get a window property and append its data to a buffer at a given offset
 * pointed to by *offset. 'offset' is modified by this routine to point to
 * the end of the data.
 *
 * Returns True if more data is available for receipt.
 *
 * If an error is encountered, the buffer is free'd.
 */
static Bool
get_append_property (XSelectionEvent * xsl, unsigned char ** buffer,
                     unsigned long * offset, unsigned long * alloc)
{
  unsigned char * ptr;
  Atom target;
  int format;
  unsigned long bytesafter, length;
  unsigned char * value;

  XGetWindowProperty (xsl->display, xsl->requestor, xsl->property,
                      0L, 1000000, True, (Atom)AnyPropertyType,
                      &target, &format, &length, &bytesafter, &value);

  debug_property (D_TRACE, xsl->requestor, xsl->property, target, length);

  if (target != XA_STRING && target != utf8_atom) {
    print_debug (D_OBSC, "target %s not XA_STRING nor UTF8_STRING in get_append_property()",
                 get_atom_name (target));
    free (*buffer);
    *buffer = NULL;
    return False;
  } else if (length == 0) {
    /* A length of 0 indicates the end of the transfer */
    print_debug (D_TRACE, "Got zero length property; end of INCR transfer");
    return False;
  } else if (format == 8) {
    if (*offset + length > *alloc) {
      *alloc = *offset + length;
      if ((*buffer = realloc (*buffer, *alloc)) == NULL) {
        exit_err ("realloc error");
      }
    }
    ptr = *buffer + *offset;
    xs_strncpy (ptr, value, length);
    *offset += length;
    print_debug (D_TRACE, "Appended %d bytes to buffer\n", length);
  } else {
    print_debug (D_WARN, "Retrieved non-8-bit data\n");
  }

  return True;
}

/*
 * SELECTION SETTING
 * =================
 *
 * The following functions allow a given selection to be set, appended to
 * or cleared, or to exchange the primary and secondary selections.
 */

/* Forward declaration of refuse_all_incr () */
static void
refuse_all_incr (void);

/*
 * handle_x_errors ()
 *
 * XError handler.
 */
static int
handle_x_errors (Display * display, XErrorEvent * eev)
{
  char err_buf[MAXLINE];

  /* Make sure to send a refusal to all waiting INCR requests
   * and delete the corresponding properties. */
  if (eev->error_code == BadAlloc) refuse_all_incr ();

  XGetErrorText (display, eev->error_code, err_buf, MAXLINE);
  exit_err (err_buf);

  return 0;
}

/*
 * clear_selection (selection)
 *
 * Clears the specified X selection 'selection'. This requests that no
 * process should own 'selection'; thus the X server will respond to
 * SelectionRequests with an empty property and we don't need to leave
 * a daemon hanging around to service this selection.
 */
static void
clear_selection (Atom selection)
{
  XSetSelectionOwner (display, selection, None, timestamp);
  /* Call XSync to ensure this operation completes before program
   * termination, especially if this is all we are doing. */
  XSync (display, False);
}

/*
 * own_selection (selection)
 *
 * Requests ownership of the X selection. Returns True if ownership was
 * granted, and False otherwise.
 */
static Bool
own_selection (Atom selection)
{
  Window owner;

  XSetSelectionOwner (display, selection, window, timestamp);
  /* XGetSelectionOwner does a round trip to the X server, so there is
   * no need to call XSync here. */
  owner = XGetSelectionOwner (display, selection);
  if (owner != window) {
    return False;
  } else {
    XSetErrorHandler (handle_x_errors);
    return True;
  }
}


static IncrTrack * incrtrack_list = NULL;

/*
 * add_incrtrack (it)
 *
 * Add 'it' to the head of incrtrack_list.
 */
static void
add_incrtrack (IncrTrack * it)
{
  if (incrtrack_list) {
    incrtrack_list->prev = it;
  }
  it->prev = NULL;
  it->next = incrtrack_list;
  incrtrack_list = it;
}

/*
 * remove_incrtrack (it)
 *
 * Remove 'it' from incrtrack_list.
 */
static void
remove_incrtrack (IncrTrack * it)
{
  if (it->prev) {
    it->prev->next = it->next;
  }
  if (it->next) {
    it->next->prev = it->prev;
  }

  if (incrtrack_list == it) {
    incrtrack_list = it->next;
  }
}

/*
 * fresh_incrtrack ()
 *
 * Create a new incrtrack, and add it to incrtrack_list.
 */
static IncrTrack *
fresh_incrtrack (void)
{
  IncrTrack * it;

  it = xs_malloc (sizeof (IncrTrack));
  add_incrtrack (it);

  return it;
}

/*
 * trash_incrtrack (it)
 *
 * Remove 'it' from incrtrack_list, and free it.
 */
static void
trash_incrtrack (IncrTrack * it)
{
  remove_incrtrack (it);
  free (it);
}

/*
 * find_incrtrack (atom)
 *
 * Find the IncrTrack structure within incrtrack_list pertaining to 'atom',
 * if it exists.
 */
static IncrTrack *
find_incrtrack (Atom atom)
{
  IncrTrack * iti;

  for (iti = incrtrack_list; iti; iti = iti->next) {
    if (atom == iti->property) return iti;
  }

  return NULL;
}

/* Forward declaration of handle_multiple() */
static HandleResult
handle_multiple (Display * display, Window requestor, Atom property,
                 unsigned char * sel, Atom selection, Time time,
                 MultTrack * mparent);

/* Forward declaration of process_multiple() */
static HandleResult
process_multiple (MultTrack * mt, Bool do_parent);

/*
 * confirm_incr (it)
 *
 * Confirm the selection request of ITER tracked by 'it'.
 */
static void
notify_incr (IncrTrack * it, HandleResult hr)
{
  XSelectionEvent ev;

  /* Call XSync here to make sure any BadAlloc errors are caught before
   * confirming the conversion. */
  XSync (it->display, False);

  print_debug (D_TRACE, "Confirming conversion");

  /* Prepare a SelectionNotify event to send, placing the selection in the
   * requested property. */
  ev.type = SelectionNotify;
  ev.display = it->display;
  ev.requestor = it->requestor;
  ev.selection = it->selection;
  ev.time = it->time;
  ev.target = it->target;

  if (hr & HANDLE_ERR) ev.property = None;
  else ev.property = it->property;

  XSendEvent (display, ev.requestor, False,
              (unsigned long)NULL, (XEvent *)&ev);
}

/*
 * refuse_all_incr ()
 *
 * Refuse all INCR transfers in progress. ASSUMES that this is called in
 * response to an error, and that the program is about to bail out;
 * ie. incr_track is not cleaned out.
 */
static void
refuse_all_incr (void)
{
  IncrTrack * it;

  for (it = incrtrack_list; it; it = it->next) {
    XDeleteProperty (it->display, it->requestor, it->property);
    notify_incr (it, HANDLE_ERR);
    /* Don't bother trashing and list-removing these; we are about to
     * bail out anyway. */
  }
}

/*
 * complete_incr (it)
 *
 * Finish off an INCR retrieval. If it was part of a multiple, continue
 * that; otherwise, send confirmation that this completed.
 */
static void
complete_incr (IncrTrack * it, HandleResult hr)
{
  MultTrack * mparent = it->mparent;

  if (mparent) {
    trash_incrtrack (it);
    process_multiple (mparent, True);
  } else {
    notify_incr (it, hr);
    trash_incrtrack (it);
  }
}

/*
 * notify_multiple (mt, hr)
 *
 * Confirm the selection request initiated with MULTIPLE tracked by 'mt'.
 */
static void
notify_multiple (MultTrack * mt, HandleResult hr)
{
  XSelectionEvent ev;

  /* Call XSync here to make sure any BadAlloc errors are caught before
   * confirming the conversion. */
  XSync (mt->display, False);

  /* Prepare a SelectionNotify event to send, placing the selection in the
   * requested property. */
  ev.type = SelectionNotify;
  ev.display = mt->display;
  ev.requestor = mt->requestor;
  ev.selection = mt->selection;
  ev.time = mt->time;
  ev.target = multiple_atom;

  if (hr & HANDLE_ERR) ev.property = None;
  else ev.property = mt->property;

  XSendEvent (display, ev.requestor, False,
              (unsigned long)NULL, (XEvent *)&ev);
}

/*
 * complete_multiple (mt, do_parent, hr)
 *
 * Complete a MULTIPLE transfer. Iterate to its parent MULTIPLE if
 * 'do_parent' is true. If there is not parent MULTIPLE, send notification
 * of its completion with status 'hr'.
 */
static void
complete_multiple (MultTrack * mt, Bool do_parent, HandleResult hr)
{
  MultTrack * mparent = mt->mparent;

  if (mparent) {
    free (mt);
    if (do_parent) process_multiple (mparent, True);
  } else {
    notify_multiple (mt, hr);
    free (mt);
  }
}

/*
 * change_property (display, requestor, property, target, format, mode,
 *                  data, nelements)
 *
 * Wrapper to XChangeProperty that performs INCR transfer if required and
 * returns status of entire transfer.
 */
static HandleResult
change_property (Display * display, Window requestor, Atom property,
                 Atom target, int format, int mode,
                 unsigned char * data, int nelements,
                 Atom selection, Time time, MultTrack * mparent)
{
  XSelectionEvent ev;
  int nr_bytes;
  IncrTrack * it;

  print_debug (D_TRACE, "change_property ()");

  nr_bytes = nelements * format / 8;

  if (nr_bytes <= max_req) {
    print_debug (D_TRACE, "data within maximum request size");
    XChangeProperty (display, requestor, property, target, format, mode,
                     data, nelements);

    return HANDLE_OK;
  }

  /* else */
  print_debug (D_TRACE, "large data transfer");


  /* Send a SelectionNotify event of type INCR */
  ev.type = SelectionNotify;
  ev.display = display;
  ev.requestor = requestor;
  ev.selection = selection;
  ev.time = time;
  ev.target = incr_atom; /* INCR */
  ev.property = property;

  XSelectInput (ev.display, ev.requestor, PropertyChangeMask);

  XChangeProperty (ev.display, ev.requestor, ev.property, incr_atom, 32,
                   PropModeReplace, (unsigned char *)&nr_bytes, 1);

  XSendEvent (display, requestor, False,
              (unsigned long)NULL, (XEvent *)&ev);

  /* Set up the IncrTrack to track this */
  it = fresh_incrtrack ();

  it->mparent = mparent;
  it->state = S_INCR_1;
  it->display = display;
  it->requestor = requestor;
  it->property = property;
  it->selection = selection;
  it->time = time;
  it->target = target;
  it->format = format;
  it->data = data;
  it->nelements = nelements;
  it->offset = 0;

  /* Maximum nr. of elements that can be transferred in one go */
  it->max_elements = max_req * 8 / format;

  /* Nr. of elements to transfer in this instance */
  it->chunk = MIN (it->max_elements, it->nelements - it->offset);

  /* Wait for that property to get deleted */
  print_debug (D_TRACE, "Waiting on intial property deletion (%s)",
               get_atom_name (it->property));

  return HANDLE_INCOMPLETE;
}

static HandleResult
incr_stage_1 (IncrTrack * it)
{
  /* First pass: PropModeReplace, from data, size chunk */
  print_debug (D_TRACE, "Writing first chunk (%d bytes) (target 0x%x %s) to property 0x%x of requestor 0x%x", it->chunk, it->target, get_atom_name(it->target), it->property, it->requestor);
  XChangeProperty (it->display, it->requestor, it->property, it->target,
                   it->format, PropModeReplace, it->data, it->chunk);

  it->offset += it->chunk;

  /* wait for PropertyNotify events */
  print_debug (D_TRACE, "Waiting on subsequent deletions ...");

  it->state = S_INCR_2;

  return HANDLE_INCOMPLETE;
}

static HandleResult
incr_stage_2 (IncrTrack * it)
{
  it->chunk = MIN (it->max_elements, it->nelements - it->offset);

  if (it->chunk <= 0) {

    /* Now write zero-length data to the property */
    XChangeProperty (it->display, it->requestor, it->property, it->target,
                     it->format, PropModeAppend, NULL, 0);
    it->state = S_NULL;
    print_debug (D_TRACE, "Set si to state S_NULL");
    return HANDLE_OK;
  } else {
    print_debug (D_TRACE, "Writing chunk (%d bytes) to property",
                 it->chunk);
    XChangeProperty (it->display, it->requestor, it->property, it->target,
                     it->format, PropModeAppend, it->data+it->offset,
                     it->chunk);
    it->offset += it->chunk;
    print_debug (D_TRACE, "%d bytes remaining",
                 it->nelements - it->offset);
    return HANDLE_INCOMPLETE;
  }
}


/*
 * handle_timestamp (display, requestor, property)
 *
 * Handle a TIMESTAMP request.
 */
static HandleResult
handle_timestamp (Display * display, Window requestor, Atom property,
                  Atom selection, Time time, MultTrack * mparent)
{
  return
    change_property (display, requestor, property, XA_INTEGER, 32,
                     PropModeReplace, (unsigned char *)&timestamp, 1,
                     selection, time, mparent);
}

/*
 * handle_targets (display, requestor, property)
 *
 * Handle a TARGETS request.
 */
static HandleResult
handle_targets (Display * display, Window requestor, Atom property,
                Atom selection, Time time, MultTrack * mparent)
{
  Atom * targets_cpy;

  targets_cpy = malloc (sizeof (supported_targets));
  memcpy (targets_cpy, supported_targets, sizeof (supported_targets));

  return
    change_property (display, requestor, property, XA_ATOM, 32,
                     PropModeReplace, (unsigned char *)targets_cpy,
                     NUM_TARGETS, selection, time, mparent);
}

/*
 * handle_string (display, requestor, property, sel)
 *
 * Handle a STRING request; setting 'sel' as the data
 */
static HandleResult
handle_string (Display * display, Window requestor, Atom property,
               unsigned char * sel, Atom selection, Time time,
               MultTrack * mparent)
{
  return
    change_property (display, requestor, property, XA_STRING, 8,
                     PropModeReplace, sel, xs_strlen(sel),
                     selection, time, mparent);
}

/*
 * handle_utf8_string (display, requestor, property, sel)
 *
 * Handle a UTF8_STRING request; setting 'sel' as the data
 */
static HandleResult
handle_utf8_string (Display * display, Window requestor, Atom property,
                    unsigned char * sel, Atom selection, Time time,
                    MultTrack * mparent)
{
  return
    change_property (display, requestor, property, utf8_atom, 8,
                     PropModeReplace, sel, xs_strlen(sel),
                     selection, time, mparent);
}

/*
 * handle_delete (display, requestor, property)
 *
 * Handle a DELETE request.
 */
static HandleResult
handle_delete (Display * display, Window requestor, Atom property)
{
  XChangeProperty (display, requestor, property, null_atom, 0,
                   PropModeReplace, NULL, 0);

  return DID_DELETE;
}

/*
 * process_multiple (mt, do_parent)
 *
 * Iterate through a MultTrack until it completes, or until one of its
 * entries initiates an interated selection.
 *
 * If 'do_parent' is true, and the actions proscribed in 'mt' are
 * completed during the course of this call, then process_multiple
 * is iteratively called on mt->mparent.
 */
static HandleResult
process_multiple (MultTrack * mt, Bool do_parent)
{
  HandleResult retval = HANDLE_OK;
  unsigned long i;

  if (!mt) return retval;

  for (; mt->index < mt->length; mt->index += 2) {
    i = mt->index;
    if (mt->atoms[i] == timestamp_atom) {
      retval |= handle_timestamp (mt->display, mt->requestor, mt->atoms[i+1],
                                  mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == targets_atom) {
      retval |= handle_targets (mt->display, mt->requestor, mt->atoms[i+1],
                                mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == multiple_atom) {
      retval |= handle_multiple (mt->display, mt->requestor, mt->atoms[i+1],
                                 mt->sel, mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == XA_STRING || mt->atoms[i] == text_atom) {
      retval |= handle_string (mt->display, mt->requestor, mt->atoms[i+1],
                               mt->sel, mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == utf8_atom) {
      retval |= handle_utf8_string (mt->display, mt->requestor, mt->atoms[i+1],
                                    mt->sel, mt->selection, mt->time, mt);
    } else if (mt->atoms[i] == delete_atom) {
      retval |= handle_delete (mt->display, mt->requestor, mt->atoms[i+1]);
    } else if (mt->atoms[i] == None) {
      /* the only other thing we know to handle is None, for which we
       * do nothing. This block is, like, __so__ redundant. Welcome to
       * Over-engineering 101 :) This comment is just here to keep the
       * logic documented and separate from the 'else' block. */
    } else {
      /* for anything we don't know how to handle, we fail the conversion
       * by setting this: */
      mt->atoms[i] = None;
    }

    /* If any of the conversions failed, signify this by setting that
     * atom to None ...*/
    if (retval & HANDLE_ERR) {
      mt->atoms[i] = None;
    }
    /* ... but don't propogate HANDLE_ERR */
    retval &= (~HANDLE_ERR);

    if (retval & HANDLE_INCOMPLETE) break;
  }

  if ((retval & HANDLE_INCOMPLETE) == 0) {
    complete_multiple (mt, do_parent, retval);
  }

  return retval;
}

/*
 * continue_incr (it)
 *
 * Continue an incremental transfer of IncrTrack * it.
 *
 * NB. If the incremental transfer was part of a multiple request, this
 * function calls process_multiple with do_parent=True because it is
 * assumed we are continuing an interrupted ITER, thus we must continue
 * the multiple as its original handler did not complete.
 */
static HandleResult
continue_incr (IncrTrack * it)
{
  HandleResult retval = HANDLE_OK;

  if (it->state == S_INCR_1) {
    retval = incr_stage_1 (it);
  } else if (it->state == S_INCR_2) {
    retval = incr_stage_2 (it);
  }

  /* If that completed the INCR, deal with completion */
  if ((retval & HANDLE_INCOMPLETE) == 0) {
    complete_incr (it, retval);
  }

  return retval;
}

/*
 * handle_multiple (display, requestor, property, sel, selection, time)
 *
 * Handle a MULTIPLE request; possibly setting 'sel' if any STRING
 * requests are processed within it. Return value has DID_DELETE bit set
 * if any delete requests are processed.
 *
 * NB. This calls process_multiple with do_parent=False because it is
 * assumed we are "handling" the multiple request on behalf of a
 * multiple already in progress, or (more likely) directly off a
 * SelectionRequest event.
 */
static HandleResult
handle_multiple (Display * display, Window requestor, Atom property,
                 unsigned char * sel, Atom selection, Time time,
                 MultTrack * mparent)
{
  MultTrack * mt;
  int format;
  unsigned long bytesafter;
  HandleResult retval = HANDLE_OK;

  mt = xs_malloc (sizeof (MultTrack));

  XGetWindowProperty (display, requestor, property, 0L, 1000000,
                      False, (Atom)AnyPropertyType, &mt->property,
                      &format, &mt->length, &bytesafter,
                      (unsigned char **)&mt->atoms);

  /* Make sure we got the Atom list we want */
  if (format != 32) return HANDLE_OK;


  mt->mparent = mparent;
  mt->display = display;
  mt->requestor = requestor;
  mt->sel = sel;
  mt->selection = selection;
  mt->time = time;
  mt->index = 0;

  retval = process_multiple (mt, False);

  return retval;
}

/*
 * handle_selection_request (event, sel)
 *
 * Processes a SelectionRequest event 'event' and replies to its
 * sender appropriately, eg. with the contents of the string 'sel'.
 * Returns False if a DELETE request is processed, indicating to
 * the calling function to delete the corresponding selection.
 * Returns True otherwise.
 */
static Bool
handle_selection_request (XEvent event, unsigned char * sel)
{
  XSelectionRequestEvent * xsr = &event.xselectionrequest;
  XSelectionEvent ev;
  HandleResult hr = HANDLE_OK;
  Bool retval = True;

  print_debug (D_TRACE, "handle_selection_request, property=0x%x (%s), target=0x%x (%s)",
               xsr->property, get_atom_name (xsr->property),
               xsr->target, get_atom_name (xsr->target));

  /* Prepare a SelectionNotify event to send, either as confirmation of
   * placing the selection in the requested property, or as notification
   * that this could not be performed. */
  ev.type = SelectionNotify;
  ev.display = xsr->display;
  ev.requestor = xsr->requestor;
  ev.selection = xsr->selection;
  ev.time = xsr->time;
  ev.target = xsr->target;

  if (xsr->property == None && ev.target != multiple_atom) {
      /* Obsolete requestor */
      xsr->property = xsr->target;
  }

  if (ev.time != CurrentTime && ev.time < timestamp) {
    /* If the time is outside the period we have owned the selection,
     * which is any time later than timestamp, or if the requested target
     * is not a string, then refuse the SelectionRequest. NB. Some broken
     * clients don't set a valid timestamp, so we have to check against
     * CurrentTime here. */
    ev.property = None;
  } else if (ev.target == timestamp_atom) {
    /* Return timestamp used to acquire ownership if target is TIMESTAMP */
    ev.property = xsr->property;
    hr = handle_timestamp (ev.display, ev.requestor, ev.property,
                           ev.selection, ev.time, NULL);
  } else if (ev.target == targets_atom) {
    /* Return a list of supported targets (TARGETS)*/
    ev.property = xsr->property;
    hr = handle_targets (ev.display, ev.requestor, ev.property,
                         ev.selection, ev.time, NULL);
  } else if (ev.target == multiple_atom) {
    if (xsr->property == None) { /* Invalid MULTIPLE request */
      ev.property = None;
    } else {
      /* Handle MULTIPLE request */
      hr = handle_multiple (ev.display, ev.requestor, ev.property, sel,
                            ev.selection, ev.time, NULL);
    }
  } else if (ev.target == XA_STRING || ev.target == text_atom) {
    /* Received STRING or TEXT request */
    ev.property = xsr->property;
    hr = handle_string (ev.display, ev.requestor, ev.property, sel,
                        ev.selection, ev.time, NULL);
  } else if (ev.target == utf8_atom) {
    /* Received UTF8_STRING request */
    ev.property = xsr->property;
    hr = handle_utf8_string (ev.display, ev.requestor, ev.property, sel,
                             ev.selection, ev.time, NULL);
  } else if (ev.target == delete_atom) {
    /* Received DELETE request */
    ev.property = xsr->property;
    hr = handle_delete (ev.display, ev.requestor, ev.property);
    retval = False;
  } else {
    /* Cannot convert to requested target. This includes most non-string
     * datatypes, and INSERT_SELECTION, INSERT_PROPERTY */
    ev.property = None;
  }

  /* Return False if a DELETE was processed */
  retval = (hr & DID_DELETE) ? False : True;

  /* If there was an error in the transfer, it should be refused */
  if (hr & HANDLE_ERR) {
    print_debug (D_TRACE, "Error in transfer");
    ev.property = None;
  }

  if ((hr & HANDLE_INCOMPLETE) == 0) {
    if (ev.property == None) {print_debug (D_TRACE, "Refusing conversion");}
    else { print_debug (D_TRACE, "Confirming conversion");}

    XSendEvent (display, ev.requestor, False,
                (unsigned long)NULL, (XEvent *)&ev);

    /* If we return False here, we may quit immediately, so sync out the
     * X queue. */
    if (!retval) XSync (display, False);
  }

  return retval;
}

/*
 * set_selection (selection, sel)
 *
 * Takes ownership of the selection 'selection', then loops waiting for
 * its SelectionClear or SelectionRequest events.
 *
 * Handles SelectionRequest events, first checking for additional
 * input if the user has specified 'follow' mode. Returns when a
 * SelectionClear event is received for the specified selection.
 */
static void
set_selection (Atom selection, pthread_mutex_t* mutex, unsigned char * sel)
{
  XEvent event;
  IncrTrack * it;

  if (own_selection (selection) == False) return;

  if ( strlen( (char*)sel ) == 0 )
  {
     return;
  }

  for (;;) {
    /* Flush before unblocking signals so we send replies before exiting */
    XFlush (display);
    unblock_exit_sigs ();
    XNextEvent (display, &event);
    block_exit_sigs ();
    pthread_mutex_lock( mutex );

    switch (event.type) {
    case SelectionClear:
      if (event.xselectionclear.selection == selection)
      {
         pthread_mutex_unlock( mutex );
         return;
      }
      break;
    case SelectionRequest:
      if (event.xselectionrequest.selection != selection) break;

      if (!handle_selection_request (event, sel))
      {
         pthread_mutex_unlock( mutex );
         return;
      }

      break;
    case PropertyNotify:
      if (event.xproperty.state != PropertyDelete) break;

      it = find_incrtrack (event.xproperty.atom);

      if (it != NULL) {
        continue_incr (it);
      }

      break;
    default:
      break;
    }

    pthread_mutex_unlock( mutex );
  }
}

int xopen_display()
{
  return (XOpenDisplay(NULL)!=NULL);
}

void xclip( pthread_mutex_t* mutex, const char* text )
{
  Window root;
  Atom selection = XA_PRIMARY, test_atom;
  int black;
  int i, s=0;
  unsigned char * old_sel = NULL, * new_sel = NULL;

  display = XOpenDisplay (NULL);
  if (display==NULL) {
    //exit_err ("Can't open display\n");
    return;
  }
  root = XDefaultRootWindow (display);
  
  /* Create an unmapped window for receiving events */
  black = BlackPixel (display, DefaultScreen (display));
  window = XCreateSimpleWindow (display, root, 0, 0, 1, 1, 0, black, black);

  print_debug (D_INFO, "Window id: 0x%x (unmapped)", window);

  /* Get a timestamp */
  XSelectInput (display, window, PropertyChangeMask);
  timestamp = get_timestamp ();

  print_debug (D_OBSC, "Timestamp: %lu", timestamp);

  /* Get the maximum incremental selection size in bytes */
  /*max_req = MAX_SELECTION_INCR (display);*/
  max_req = 4000;

  print_debug (D_OBSC, "Maximum request size: %ld bytes", max_req);

  /* Consistency check */
  test_atom = XInternAtom (display, "PRIMARY", False);
  if (test_atom != XA_PRIMARY)
    print_debug (D_WARN, "XA_PRIMARY not named \"PRIMARY\"\n");
  test_atom = XInternAtom (display, "SECONDARY", False);
  if (test_atom != XA_SECONDARY)
    print_debug (D_WARN, "XA_SECONDARY not named \"SECONDARY\"\n");

  NUM_TARGETS=0;

  /* Get the TIMESTAMP atom */
  timestamp_atom = XInternAtom (display, "TIMESTAMP", False);
  supported_targets[s++] = timestamp_atom;
  NUM_TARGETS++;

  /* Get the MULTIPLE atom */
  multiple_atom = XInternAtom (display, "MULTIPLE", False);
  supported_targets[s++] = multiple_atom;
  NUM_TARGETS++;

  /* Get the TARGETS atom */
  targets_atom = XInternAtom (display, "TARGETS", False);
  supported_targets[s++] = targets_atom;
  NUM_TARGETS++;

  /* Get the DELETE atom */
  delete_atom = XInternAtom (display, "DELETE", False);
  supported_targets[s++] = delete_atom;
  NUM_TARGETS++;

  /* Get the INCR atom */
  incr_atom = XInternAtom (display, "INCR", False);
  supported_targets[s++] = incr_atom;
  NUM_TARGETS++;

  /* Get the TEXT atom */
  text_atom = XInternAtom (display, "TEXT", False);
  supported_targets[s++] = text_atom;
  NUM_TARGETS++;

  /* Get the UTF8_STRING atom */
  utf8_atom = XInternAtom (display, "UTF8_STRING", True);
  if(utf8_atom != None) {
    supported_targets[s++] = utf8_atom;
    NUM_TARGETS++;
  } else {
    utf8_atom = XA_STRING;
  }

  supported_targets[s++] = XA_STRING;
  NUM_TARGETS++;

  if (NUM_TARGETS > MAX_NUM_TARGETS) {
    exit_err ("internal error num-targets (%d) > max-num-targets (%d)\n",
              NUM_TARGETS, MAX_NUM_TARGETS);
  }

  /* Get the NULL atom */
  null_atom = XInternAtom (display, "NULL", False);

  /* Get the COMPOUND_TEXT atom.
   * NB. We do not currently serve COMPOUND_TEXT; we can retrieve it but
   * do not perform charset conversion.
   */
  compound_text_atom = XInternAtom (display, "COMPOUND_TEXT", False);

  sigemptyset (&exit_sigs);
  sigaddset (&exit_sigs, SIGALRM);
  sigaddset (&exit_sigs, SIGINT);
  sigaddset (&exit_sigs, SIGTERM);

  /* Find the "CLIPBOARD" selection if required */
  selection = XInternAtom (display, "CLIPBOARD", False);
  set_selection (selection, mutex, (unsigned char*)text);
  print_debug (D_INFO, "exit xclip()");
}
