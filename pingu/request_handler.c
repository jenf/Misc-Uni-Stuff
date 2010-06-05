#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include "request_handler.h"
static GTree *tree = NULL;		// Nobody outside should play with the tree.

static int compare_func(uint16_t left, uint16_t right)
{
	if ((left) < (right))
		return -1;
	if ((left) > (right))
		return 1;
	return 0;
}

static request *request_get_local(uint16_t reqno)
{
	// This version doesn't do a mutex lock
	request *r;
	r = g_tree_lookup(tree, GINT_TO_POINTER((gint) reqno));
	return r;
}

void request_init()
{
	tree = g_tree_new((GCompareFunc) compare_func);
}

static gboolean find_next_number(uint16_t requestno, request * k,
								 uint16_t * curmax)
{
	if (*curmax != requestno)
	{
		// We can use this number
		return TRUE;
	}
	else
	{
		*curmax = (requestno) + 1;	// Try next
		return FALSE;
	}
}

G_LOCK_DEFINE(request_start);

void request_end(uint16_t req)
{
	// This sets the request to inactive
	request *r;
	G_LOCK(request_start);
	r = request_get_local(req);
	if (r)
	{
		r->active = FALSE;
	}
	G_UNLOCK(request_start);
}

static time_t curt;
static GSList *list;
static gboolean request_do_timeout_single(uint16_t req, request * k)
{
	// Does a single request timeout.
	if (curt > (k->starttime))
	{
		list = g_slist_append(list, k);
	}
	return FALSE;
}

void request_do_timeout_single2(request * k)
{
	// This is a single request (the linked list one) timeout
	printf("Found timeout\n");
	if (k->active && (!k->timedout))
		k->timeout(k->reqno, k);
	if (!k->active)
	{
		g_tree_remove(tree, GINT_TO_POINTER((gint) k->reqno));
		free(k);
	}
	k->timedout = TRUE;
}

void request_do_timeout()
{
	// This goes through the list of requests trying to find those that have
	// and builds up a list, it then goes through this list (you can't make
	// modifications to a tree while travesing it) and processes them..
	G_LOCK(request_start);
	curt = time(NULL);
	g_tree_foreach(tree, (GTraverseFunc) request_do_timeout_single, NULL);
	g_slist_foreach(list, (GFunc) request_do_timeout_single2, NULL);
	g_slist_free(list);
	list = NULL;
	G_UNLOCK(request_start);
}

uint16_t request_start(request * k)
{
	uint16_t found;
	// add a request to the list.
	G_LOCK(request_start);
	found = 1;
	g_tree_foreach(tree, (GTraverseFunc) find_next_number, &found);
	g_tree_insert(tree, GINT_TO_POINTER((gint) found), k);
	k->active = TRUE;
	k->starttime = time(NULL) + 5;
	k->reqno = found;
	k->timedout = FALSE;
	G_UNLOCK(request_start);
	return found;
}


request *request_get(uint16_t reqno)
{
	// Find the request with this number
	request *r;
	G_LOCK(request_start);
	r = g_tree_lookup(tree, GINT_TO_POINTER((gint) reqno));
	G_UNLOCK(request_start);
	return r;
}

#if 0
int main(void)
{
	int     n, i;
	request_init();
	for (i = 0; i < 20; i++)
	{
		n = request_start(NULL);
		printf("%i %i\n", i, n);
	}
	request_end(1);
	n = request_start(NULL);
	printf("%i\n", n);

//      g_tree_foreach(tree,dump,NULL);
}
#endif
