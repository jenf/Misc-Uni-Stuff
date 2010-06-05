#include "request_handler.h"
#include "servlet.h"
#include "protocol.h"
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <glib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "file_db.h"
// REQUEST_WANTFILE
// 0-3          IP Address of request
// 4-7          Offset wanted
// 8-MD5BINLEN  md5sum of file
#define DOWNLOAD_DIRECTORY "../downloads"

enum
{
    DOWNLOAD_STATE_ACCEPT,
    DOWNLOAD_STATE_CONNECTING,
    DOWNLOAD_STATE_UPLOADING,
    DOWNLOAD_STATE_DOWNLOADING,
    DOWNLOAD_STATE_FINISHED
} DOWNLOAD_STATE;

static GThreadPool *download_pool;
static GThreadPool *upload_pool;

static inline int md5cmp(unsigned char *text, unsigned char *bar)
{
    // This is the actually eq not cmp
    int i;
    for (i = 0; i < MAX_DIGEST_BIN_BYTES; i++)
    {
	if (text[i] != bar[i])
	    return FALSE;
    }
    return TRUE;
}

struct download_data *download_send_file(FullPacket * p,
					 gchar * filename, gchar * md5sum,
					 gchar * sfilename);
struct download_data
{
    uint16_t listenport;
    int listenid;
    int fileid;
};

void get_request_wantfile(FullPacket * p)
{
    // This gets called from the packet handler switch
    GSList *m;
    unsigned char *md5sum;
    m = get_file_database();
    md5sum = p->data;
    // This loop tries to find the md5sum out of the file database
    while (m)
    {
	struct filedata *f;
	f = (struct filedata *) m->data;
	// FIXME Should have file offset.
	if (md5cmp(f->md5sum, md5sum))
	{
	    struct download_data *d;
	    printf("Found md5sum for %s\n", f->fullfilename);
	    d = download_send_file(p, f->fullfilename, md5sum, f->filename);
	    // create reply
	    g_thread_pool_push(download_pool, d, NULL);
	    break;
	}
	m = m->next;
    }
}

void download_send_filenotfound(FullPacket * p, gchar * filename)
{
    // FIXME: Not implemented yet
}

int download_create_socket()
{
    // Create a socket for downloading from.
    int listening;
    int one = 0;
    if ((listening = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("failed creating socket");
	return -1;
    }
    // Allow it to be reused ASAP
    if (setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) <
	0)
    {
	perror("failure setting REUSEADDR");
	return -1;
    }
    // Without bind it gives us a random port
    if (listen(listening, 2) < 0)
    {
	perror("failed listening socket");
	return -1;
    }
    return listening;
}

void download_send_file_reply(FullPacket * p, struct download_data *d,
			      char *filename, gchar * md5sum)
{
    int j, sockfd;
    Packet pa;
    unsigned char *k;
    // 0-3  IP address or 0 if that's the origin
    // 4-5  Port number
    // 6-21 MD5 Sum of file on port
    // 22-n Filename
    j = 6 + MAX_DIGEST_BIN_BYTES + strlen(filename);
    printf("j size:%i\n", j);
    k = (unsigned char *) malloc(j);
    // FIXME: This should really find the real ip address of us, but it isn't too important as long as it's not routed (it can be filled in)

    // IP Address
    memset(k, 0, 4);
    // Port number
    memcpy(k + 4, &d->listenport, sizeof(2));
    // MD5 Sum of file
    memcpy(k + 6, md5sum, MAX_DIGEST_BIN_BYTES);
    // File name
    memcpy(k + 6 + MAX_DIGEST_BIN_BYTES, filename, strlen(filename));

    fillin_packet(&pa, REPLY_TRANSFERWAITING, j, p->packet->request_handle);
    sockfd = get_outgoing_socket();
    send_packet(sockfd, &p->addr, &pa, k);
    free(k);
}

#define BUFFER 1024
struct download_data *download_send_file(FullPacket * p,
					 gchar * filename, gchar * md5sum,
					 gchar * sfilename)
{
    int fileid, listenid;
    ssize_t j;
    struct sockaddr_in server;
    struct download_data *d;
    if ((fileid = open(filename, O_RDONLY, 0)) == -1)
    {
	perror("Filename");
	download_send_filenotfound(p, filename);
	return NULL;
    }
    // FIXME Do seeking
    printf("blah\n");
    // create socket
    if ((listenid = download_create_socket()) < 0)
	return NULL;


    // tell the other side the port number
    // wait for an accept
    j = sizeof(server);
    // Find out what port it is actually on
    if (getsockname(listenid, (struct sockaddr *) &server, &j) < 0)
    {
	perror("failed getsockname");
	return NULL;
    }
    printf("listening on %i\n", htons(server.sin_port));

    // allocate the memory for the filename
    d = g_malloc(sizeof(struct download_data));
    d->listenport = server.sin_port;
    d->listenid = listenid;
    download_send_file_reply(p, d, sfilename, md5sum);
    d->fileid = fileid;
    return d;
}

void download_send_accept(struct download_data *k)
{
    // Listen for an incoming connection and then send the file down it.
    int listenid;
    int sockid;
    int fileid;
    ssize_t j, retcode;
    char buf[BUFFER];
    struct sockaddr_in server;
    listenid = k->listenid;
    fileid = k->fileid;
    if ((sockid = accept(listenid, (struct sockaddr *) &server, &j)) < 0)
    {
	perror("accept");
	return;
    }
    // close the listening socket
    close(listenid);
    retcode = 1;
    // send the file down the new socket
    while (retcode > 0)
    {
	// Ye old read write loop
	retcode = read(fileid, buf, BUFFER);
	if (retcode > 0)
	{
	    char *m;
	    ssize_t k;
	    k = retcode;
	    m = buf;
	    while (k > 0)
	    {
		ssize_t h;
		h = write(sockid, buf, k);
		if (h > 0)
		{
		    k -= h;
		}
	    }
	}
    }
    close(fileid);
    close(sockid);
    g_free(k);
}

void download_recv_fetch(struct download_data *d);
void download_init()
{
    // create a thread pool
    download_pool =
	g_thread_pool_new((GFunc) download_send_accept, NULL, 10, FALSE,
			  NULL);
    upload_pool =
	g_thread_pool_new((GFunc) download_recv_fetch, NULL, 10, FALSE, NULL);
}

void download_request_timeout()
{
    // In real life this would present something to the user
    printf("Download request timed out\n");
}

void download_make_request(char *md5sum, struct sockaddr_in *host)
{
    request *req;
    uint16_t req_no;
    Packet p;
    int sockfd;
    req = (request *) malloc(sizeof(request));
    // Call back
    req->timeout = (OnTimeout) download_request_timeout;
    req_no = request_start(req);

    sockfd = get_outgoing_socket();
    fillin_packet(&p, REQUEST_WANTFILE, MAX_DIGEST_BIN_BYTES, req_no);
    send_packet(sockfd, host, &p, md5sum);
    printf("download request sent\n");
}


void download_recv_fetch(struct download_data *d)
{
    // Write the file to disk from the socket
    ssize_t r = 1;
    ssize_t w = 1;
    char buf[4096];
    printf("Writing file to disk\n");
    while (r > 0)
    {
	// Ye old read-write loop
	r = read(d->listenid, buf, 4096);
	if (r > 0)
	{
	    w = write(d->fileid, buf, r);
	    if (w == -1)
	    {
		perror("problem writing file");
		return;
	    }
	}
    }
    close(d->fileid);
    close(d->listenid);
    g_free(d);
    printf("Finished writing file to disk\n");
}

void download_reply_transferwaiting(FullPacket * p)
{
    // This is the clients size of the connection initialisation
    int sockfd;
    int filefd;
    struct sockaddr_in ad;
    uint16_t *port;
    struct download_data *d;
    char *filename, *fullfile;
    ssize_t foo;
    // This will tell us what socket it is on.
    // FIXME: This should use the address in the data (if it's not 0)
    // TODO: Find out why this takes ages, i think it's a threading issue.
    // TODO: This should shove something in the GUI.
    foo = (p->packet->packet_length) - (6 + MAX_DIGEST_BIN_BYTES);
    filename = (char *) malloc(foo + 1);
    memcpy(filename, p->data + 6 + MAX_DIGEST_BIN_BYTES, foo);
    filename[foo] = 0;
    fullfile = g_strdup_printf("%s/%s", DOWNLOAD_DIRECTORY, filename);
    printf("%s to transfer\n", fullfile);
    free(filename);
    port = (p->data) + 4;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = p->addr.sin_addr.s_addr;
    ad.sin_port = *port;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("socket() Failed");
	return;
    }
    if (connect(sockfd, (struct sockaddr *) &ad, sizeof(struct sockaddr_in))
	!= 0)
    {
	perror("connect() Failed");
	return;
    }
    if ((filefd = creat(fullfile, 0666)) < 1)
    {
	perror("open() Failed");
	return;
    }
    // start thread for this
    g_free(fullfile);
    d = (struct download_data *) malloc(sizeof(struct download_data));
    d->listenid = sockfd;
    d->fileid = filefd;
    printf("creating thread\n");
    g_thread_pool_push(upload_pool, d, NULL);
}
