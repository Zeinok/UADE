/* UADE
 *
 * Copyright 2005 Heikki Orsila <heikki.orsila@iki.fi>
 *
 * Loads contents of 'eagleplayer.conf' and 'song.conf'. The file formats are
 * specified in doc/eagleplayer.conf and doc/song.conf.
 *
 * This source code module is dual licensed under GPL and Public Domain.
 * Hence you may use _this_ module (not another code module) in any you
 * want in your projects.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <strlrep.h>

#include <eagleplayer.h>


#define eperror(fmt, args...) do { fprintf(stderr, "Eagleplayer.conf error on line %zd: " fmt "\n", lineno, ## args); exit(-1); } while (0)


static int ufcompare(const void *a, const void *b);


/* skips whitespace characters from string 's'. returns -1 if end of string is
   reached before a non-whitespace character */
static int skip_ws(const char *s, int pos)
{
  while (s[pos] != 0 && isspace(s[pos]))
    pos++;
  if (s[pos] == 0)
    return -1;
  return pos;
}


/* skips non-whitespace characters from string 's'. returns -1 if end of string
   is reached before a whitespace character */
static int skip_nws(const char *s, int pos)
{
  while (s[pos] != 0 && !isspace(s[pos]))
    pos++;
  if (s[pos] == 0)
    return -1;
  return pos;
}


struct eagleplayer *uade_get_eagleplayer(const char *extension, struct eagleplayerstore *ps)
{
  struct eagleplayermap *uf = ps->map;
  struct eagleplayermap *f;
  struct eagleplayermap key = {.extension = (char *) extension};

  f = bsearch(&key, uf, ps->nextensions, sizeof(uf[0]), ufcompare);
  if (f == NULL)
    return NULL;

  return f->player;
}


/* Split line with respect to white space. */
static char **split_line(int *nitems, size_t *lineno, FILE *f)
{
  char line[1024];
  int have_line = 0;
  int i, begin;
  char **items = NULL;
  int pos;

  *nitems = 0;

  while (fgets(line, sizeof line, f) != NULL) {

    (*lineno)++;

    /* skip, if a comment line */
    if (line[0] == '#')
      continue;

    /* skip, if a pure whitespace line */
    if ((i = skip_ws(line, 0)) < 0)
      continue;

    /* compute number of items */
    while (1) {
      (*nitems)++;
      begin = i;
      if ((i = skip_nws(line, begin)) < 0)
	break;
      begin = i;
      if ((i = skip_ws(line, begin)) < 0)
	break;
    }
    have_line = 1;
    break;
  }

  if (!have_line)
    return NULL;

  i = skip_ws(line, 0);
  assert(i >= 0);

  items = malloc(sizeof(items[0]) * (*nitems));
  if (items == NULL) {
    fprintf(stderr, "No memory for nws items.\n");
    exit(-1);
  }

  pos = 0;

  while (1) {
    begin = i;
    i = skip_nws(line, begin);
    if (i < 0) {
      items[pos] = strdup(&line[begin]);
      if (items[pos] == NULL) {
	fprintf(stderr, "No memory for an nws item.\n");
	exit(-1);
      }
    } else {
      size_t len = i - begin;
      items[pos] = malloc(len + 1);
      if (items[pos] == NULL) {
	fprintf(stderr, "No memory for an nws item.\n");
	exit(-1);
      }
      memcpy(items[pos], &line[begin], len);
      items[pos][len] = 0;
    }

    pos++;

    begin = i;
    i = skip_ws(line, begin);
    if (i < 0)
      break;
  }
  assert(pos == *nitems);

  return items;
}


/* Read eagleplayer.conf. */
struct eagleplayerstore *uade_read_eagleplayer_conf(const char *filename)
{
  FILE *f;
  struct eagleplayer *p;
  size_t allocated;
  char **items;
  int nitems;
  size_t lineno = 0;
  struct eagleplayerstore *ps = NULL;
  size_t exti;
  int i, j;

  f = fopen(filename, "r");
  if (f == NULL)
    goto error;

  ps = calloc(1, sizeof ps[0]);
  if (ps == NULL)
    eperror("No memory for ps.");

  allocated = 16;
  ps->players = malloc(allocated * sizeof(ps->players[0]));
  if (ps->players == NULL)
    eperror("No memory for eagleplayer.conf file.\n");

  while (1) {

    items = split_line(&nitems, &lineno, f);
    if (items == NULL)
      break;

    assert(nitems > 0);

    if (ps->nplayers == allocated) {
      allocated *= 2;
      ps->players = realloc(ps->players, allocated * sizeof(ps->players[0]));
      if (ps->players == NULL)
	eperror("No memory for players.");
    }

    p = &ps->players[ps->nplayers];
    ps->nplayers++;

    memset(p, 0, sizeof p[0]);

    p->playername = strdup(items[0]);
    if (p->playername == NULL) {
      fprintf(stderr, "No memory for playername.\n");
      exit(-1);
    }

    for (i = 1; i < nitems; i++) {
      if (strncasecmp(items[i], "prefixes=", 9) == 0) {
	int begin;
	char *s = items[i] + 9;
	int n;

	assert(p->nextensions == 0 && p->extensions == NULL);

	j = 0;
	if (s[j] == 0)
	  continue;

	p->nextensions = 1;

	while (s[j] != 0) {
	  if (s[j] == ',') {
	    if (s[j + 1] == ',' || s[j + 1] == 0)
	      eperror("Premature line end or two consecutive , chars.");
	    p->nextensions++;
	  }
	  j++;
	}

	p->extensions = malloc(p->nextensions * sizeof p->extensions[0]);
	if (p->extensions == NULL)
	  eperror("No memory for extensions.");

	j = 0;
	n = 0;
	while (1) {
	  size_t len;
	  char *extname;

	  begin = j;

	  if (s[j] == ',' || s[j] == 0)
	    eperror("Crapo. Invalid character 0x%x", s[j]);

	  while (s[j] != 0 && s[j] != ',')
	    j++;

	  len = j - begin;

	  p->extensions[n] = malloc(len + 1);
	  extname = p->extensions[n];
	  if (extname == NULL)
	    eperror("No memory for prefix.");
	  memcpy(extname, &s[begin], len);
	  extname[len] = 0;
	  n++;

	  if (s[j] == 0)
	    break;

	  j++;
	}

      } else if (strcasecmp(items[i], "a500") == 0) {
	p->attributes |= EP_A500;
      } else if (strcasecmp(items[i], "a1200") == 0) {
	p->attributes |= EP_A1200;
      } else if (strcasecmp(items[i], "always_ends") == 0) {
	p->attributes |= EP_ALWAYS_ENDS;
      } else if (strcasecmp(items[i], "content_detection") == 0) {
	p->attributes |= EP_CONTENT_DETECTION;
      } else if (strcasecmp(items[i], "speed_hack") == 0) {
	p->attributes |= EP_SPEED_HACK;
      } else if (strncasecmp(items[i], "comment:", 8) == 0) {
	break;
      } else {
	fprintf(stderr, "Unrecognized option: %s\n", items[i]);
      }
    }

    free(items);
  }

  fclose(f);

  if (ps->nplayers == 0) {
    free(ps->players);
    free(ps);
    return NULL;
  }

  for (i = 0; i < ps->nplayers; i++)
    ps->nextensions += ps->players[i].nextensions;

  ps->map = malloc(sizeof(ps->map[0]) * ps->nextensions);
  if (ps->map == NULL)
    eperror("No memory for extension map.");

  exti = 0;
  for (i = 0; i < ps->nplayers; i++) {
    if (exti >= ps->nextensions) {
      fprintf(stderr, "pname %s\n", ps->players[i].playername);
      fflush(stderr);
    }
    assert(exti < ps->nextensions);
    p = &ps->players[i];
    for (j = 0; j < p->nextensions; j++) {
      ps->map[exti].player = p;
      ps->map[exti].extension = p->extensions[j];
      exti++;
    }
  }

  assert(exti == ps->nextensions);

  qsort(ps->map, ps->nextensions, sizeof(ps->map[0]), ufcompare);

  return ps;

 error:
  free(ps->players);
  free(ps);
  if (f != NULL)
    fclose(f);
  return NULL;
}


/* compare function for qsort() */
static int ufcompare(const void *a, const void *b)
{
  const struct eagleplayermap *ua = a;
  const struct eagleplayermap *ub = b;
  return strcasecmp(ua->extension, ub->extension);
}