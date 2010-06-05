#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "model_gslist.h"
#include "protocol.h"
#include "servlet.h"
#include "request_handler.h"
#include <dirent.h>
#include "file_db.h"
#define BUFFER 1024

#define OSET_FILESIZE 0
#define OSET_MD5SUM 4
#define OSET_FILENAME MAX_DIGEST_BIN_BYTES+4
// request file search is as follows
// Byte 0-3 IP Address (if 0.0.0.0 modify to the address the request came from)
// Byte 3-5 Port Number
// Byte 5-7 Reference Number
// Byte 7-n Filename

// reply file search is as follows
// Byte 0-3 filesize
// Byte 4-20 md5sum

void request_searchfile(char *search);
void get_request_search_passon(FullPacket * p);

struct filedata_response
{
	char   *filename;
	unsigned char *md5sum;		// Binary
	int     size;
	struct sockaddr_in addr;
	int     port;
	char   *hostname;
};

void send_file_details(FullPacket * p, struct filedata *f)
{
	// When a file has been found, this constructs the "we've found it" msg
	unsigned char *data;
	Packet  pa;
	size_t  m;
	struct sockaddr_in j;
	void   *da;
	m = 20 + strlen(f->fullfilename);
	data = malloc(m);
#ifdef DEBUG
	md5 = get_md5sum_text(f->md5sum);
	printf("Found %s:%s:%i\n", f->filename, md5, f->filesize);
	g_free(md5);
#endif
	memcpy(data, &f->filesize, 4);	// ASSUMPTION: size_t=32 bit
	memcpy(data + 4, f->md5sum, 16);
	memcpy(data + 20, f->fullfilename, strlen(f->fullfilename));

	da = p->data;
	// Get it to go back to where it came from
	set_destination_ip(&j, *(in_addr_t *) p->data, p->addr.sin_addr.s_addr);
	j.sin_port = *((uint16_t *) (da + 4));
	j.sin_family = AF_INET;

	fillin_packet(&pa, REPLY_FILESEARCH, m, p->packet->request_handle);
	send_packet(get_outgoing_socket(), &j, &pa, data);
}

void filesearch_unallocate_request(uint16_t req)
{
	request *r;
	request_end(req);
	r = request_get(req);
	if (!r)
		return;
	filename_search_close_freewidgets(r->widget);
	printf("unallocate request\n");
}

void get_request_search(FullPacket * p)
{
	// Called when it gets a request search packet
	gchar  *k;
	GSList *m;
	unsigned char *data;
	GPatternSpec *patt;
	char   *search;
	Packet  pa;
	search = malloc(p->packet->packet_length - 7);
	memcpy(search, p->data + 8, p->packet->packet_length - 8);
	search[p->packet->packet_length - 8] = 0;
	patt = g_pattern_spec_new(search);
	printf("request search %s\n", search);
	m = get_file_database();
	while (m)
	{
		struct filedata *f;
		f = m->data;
		if (g_pattern_match_string(patt, f->filename))
		{
			send_file_details(p, f);
		}
		m = m->next;
	}
	g_pattern_spec_free(patt);
	printf("Finished search\n");
	free(search);
	get_request_search_passon(p);
}

void get_request_search_passon(FullPacket * p)
{
	// Passes the packet on to those we know, Probably should be in socket.c
	in_addr_t bar;
	Servlet *j;
	if (p->packet->ttl > 2)
		p->packet->ttl = 2;
	if (p->packet->ttl > 0)
	{
		(p->packet->ttl)--;
		if ((*((in_addr_t *) p->data)) == 0)
		{
			*(in_addr_t *) p->data = p->addr.sin_addr.s_addr;
		}
		send_to_all_except(&p->addr, p->packet, p->data);
	}
}

gchar  *get_reply_column(int colno, struct filedata_response *k)
{
	// GUI stuff again
	gchar  *reply;
	switch (colno)
	{
	case 0:
		reply = g_strdup(k->filename);
		break;
	case 1:
		reply = get_md5sum_text(k->md5sum);
		break;
	case 2:
		reply = g_strdup_printf("%i bytes", k->size);
		break;
	case 3:
		reply = g_strdup_printf("%s:%i", k->hostname, k->addr.sin_port);
		break;
	default:
		reply = g_strdup_printf("Bar");
	}
	return reply;
}

int get_reply_no_columns(void *k)
{
	// More GUI stuff again
	return 4;
}

static GType get_reply_column_type(gint column)
{
	// Yet more GUI stuff
	return G_TYPE_STRING;		// Always a string.
}

void get_reply_download_file(struct filedata_response *k)
{
	// When a user clicks on the download button
	download_make_request(k->md5sum, &(k->addr));
	printf("%s at %s\n", k->filename, k->hostname);
}

void get_reply_search(FullPacket * p)
{
	gchar  *md5;
	unsigned char *data;
	uint32_t f;
	struct filedata_response *k;
	GSList *list;
	ModelGslist *model;
	request *l;
	ssize_t filen;

	l = request_get(p->packet->request_handle);
	if (l->active == TRUE)
	{
		k = malloc(sizeof(struct filedata_response));
		// marshal types
		data = p->data;
		f = *(uint32_t *) (p->data);
		md5 = get_md5sum_text(data + 4);

		printf("reply search %s %i\n", md5, p->packet->packet_length);

		// Allocate memory
		k->hostname = malloc(INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(&p->addr)->sin_addr, k->hostname,
				  INET_ADDRSTRLEN);
		k->addr = p->addr;
		k->md5sum = malloc(MAX_DIGEST_BIN_BYTES);
		k->size = f;
		filen = p->packet->packet_length - 20;
		k->filename = malloc(filen + 1);	// 20 (plus null) 
		memset(k->filename, 0, filen + 1);
		memcpy(k->filename, (p->data) + 20, filen);
		k->filename[filen] = 0;
		memcpy(k->md5sum, (p->data) + 4, MAX_DIGEST_BIN_BYTES);

		model = l->data;
		list = model_gslist_get_list(model);
		list = g_slist_append(list, k);
		model_gslist_new_record(model, list);
		g_free(md5);
	}
}

void request_ontimeout(int reqno, request * req)
{
	gui_set_searchtimeout(req->widget);
}

void searchfile(char *search)
{
	Packet  p;
	request *req;
	ModelGslist *model;
	char   *reque;
	uint16_t mo;
	size_t  size;
	void   *meek;

	// Set up the request handler
	req = malloc(sizeof(request));
	req->starttime = time(NULL) + DEFAULT_TIMEOUT;
	req->timeout = (OnTimeout) request_ontimeout;
	mo = request_start(req);

	// Set up list model
	model = model_gslist_new((GetColumn) get_reply_column,
				 (GetNumberColumns) get_reply_no_columns,
				 (GetColumnType) get_reply_column_type);
	meek = (void *) filename_search_begin(search, model, mo);
	//model_gslist_new_record(model,NULL);
	req->data = model;			// GSList
	req->widget = meek;
	printf("%p %p", req->widget, meek);


	size = 8 + strlen(search);
	// ip addr(4)+port(2)+refno(2)=8
	reque = malloc(size);
	*((in_addr_t *) reque) = 0;
	*((uint16_t *) (reque + 4)) = ntohs(3773);
	// FIXME we may not actually be on port 3773
	memcpy(reque + 8, search, strlen(search));
	fillin_packet(&p, REQUEST_FILESEARCH, size, mo);
	send_to_all(&p, reque);
	printf("search for %s\n", search);
}
