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
#include "protocol.h"
#include "servlet.h"

GSList *sockets = NULL;
#define BUFFER 1024

void set_destination_ip(struct sockaddr_in *dest, in_addr_t local,
						in_addr_t current)
{
	if (local == 0)
	{
		printf("Setting as source\n");
		dest->sin_addr.s_addr = current;
	}
	else
	{
		printf("Setting as it wished\n");
		dest->sin_addr.s_addr = local;
	}
}

void fillin_packet(Packet * p, int operation, uint16_t datalen,
				   uint16_t request)
{
	// Set the various fields of the Packet
	p->magic = htons(PROTOCOL_MAGIC);
	p->version = htons(PROTOCOL_VERSION);
	p->operation = operation;
	p->ttl = STANDARD_TTL;
	p->request_handle = request;
	p->packet_length = datalen;
}

void send_to_all_except(struct sockaddr_in *addr, Packet * p, void *d)
{
	// This sends a message to all machines except the one with addr    
	GSList *l;
	int     sockfd;
	sockfd = get_outgoing_socket();
	l = sockets;
	while (l)
	{
		Servlet *serv;
		serv = l->data;
		printf("%i\n", serv->client.sin_port);
		printf("%i\n", addr->sin_port);
		if (cmp_sockaddr(serv, addr) != 0)
		{
			send_packet(sockfd, &serv->client, p, d);
		}
		l = l->next;
	}
}

void send_to_all(Packet * p, void *d)
{
	// This sends a packet to all machines this machine knows about
	GSList *l;
	int     sockfd;
	sockfd = get_outgoing_socket();
	l = sockets;
	while (l)
	{
		Servlet *serv;
		serv = l->data;
		send_packet(sockfd, &serv->client, p, d);
		l = l->next;
	}
}

int send_packet(int sockfd, struct sockaddr_in *addr, Packet * p, void *d)
{
	// Sends a packet to an ip address
	void   *data;
	Packet  temp;
	ssize_t packet;
	int     i;

	// Convert to network byte order
	memcpy((void *) &temp, p, sizeof(Packet));
	temp.packet_length = htons(temp.packet_length);

	packet = sizeof(Packet) + (p->packet_length);
	data = malloc(packet);
	memcpy(data, &temp, sizeof(Packet));
	memcpy(data + sizeof(Packet), d, p->packet_length);

	i = sendto(sockfd, data, packet, 0, (struct sockaddr *) addr,
			   (socklen_t) sizeof(struct sockaddr_in));

	free(data);
	return i;
}

void read_packet(void *read, FullPacket * m, struct sockaddr_in *ad)
{
	// Converts a packet from a chunk of bytes to a data structure
	void   *data;
	Packet *p;
	p = malloc(sizeof(Packet));
	memcpy(p, read, sizeof(Packet));
	p->packet_length = ntohs(p->packet_length);

	if (p->packet_length != 0)
	{
		data = malloc(p->packet_length);
		memcpy(data, read + sizeof(Packet), p->packet_length);
	}
	else
	{
		data = NULL;
	}
	m->packet = p;
	memcpy(&(m->addr), ad, sizeof(struct sockaddr_in));
	m->data = data;
}

void free_packet(FullPacket * m)
{
	free(m->packet);
	free(m->data);
	free(m);
}

GSList *possible_servers = NULL;	// Other servers that are
					// possibly available but
					// not used.
G_LOCK_DEFINE(add_address);

// This really shouldn't be here
int     generate_reply_servers_found;
G_LOCK_DEFINE(generate_reply_servers);

gboolean generate_reply_servers_single(Servlet * j, FullPacket * k)
{
	char    src[INET_ADDRSTRLEN];
	int     sockfd;
	uint16_t bar;
	Packet  pr;
	unsigned char foo[sizeof(in_addr_t) + sizeof(uint16_t)];
	// Is this the address we are asking from?
	if (cmp_sockaddr(j, &(k->addr)) != 0)
	{
		memcpy(foo, &(j->client.sin_addr.s_addr), 4);
		generate_reply_servers_found = 1;
		bar = j->client.sin_port;
		memcpy(foo + 4, &bar, sizeof(uint16_t));

		fillin_packet(&pr, REPLY_SERVERS,
					  sizeof(in_addr_t) + sizeof(uint16_t),
					  k->packet->request_handle);
		sockfd = get_outgoing_socket();
		send_packet(sockfd, &k->addr, &pr, foo);
	}
	return FALSE;
}

void generate_reply_servers(FullPacket * p)
{
	G_LOCK(generate_reply_servers);
	generate_reply_servers_found = 0;
	g_slist_foreach(sockets, (GFunc) generate_reply_servers_single, p);
	// We haven't sent any packets so
	if (generate_reply_servers_found == 0)
	{
		Packet  pr;
		int     sockfd;
		printf("I know no servers\n");
		fillin_packet(&pr, REPLY_NOSERVERS, sizeof(int),
					  p->packet->request_handle);
		sockfd = get_outgoing_socket();
		send_packet(sockfd, &p->addr, &pr, &sockfd);
	}
	G_UNLOCK(generate_reply_servers);
}


void fillin_host_string(char *src, size_t srclen, Servlet * j)
{
	inet_ntop(AF_INET, &(j->client.sin_addr), src, srclen);
}

void check_add_server(struct sockaddr_in *source)
{
	// Check if a server is already in our list, otherwise add it
	// Also reset the ping time
	gpointer k;
	GSList *moo;
	char    src[INET_ADDRSTRLEN];
	Servlet *serv;

	G_LOCK(add_address);

	printf("Resetting pong time\n");
	serv = find_servlet(source);
	if (serv == NULL)
	{
		// Not in the cache so add
		inet_ntop(AF_INET, &source->sin_addr, src, INET_ADDRSTRLEN);
		printf("Adding server %s\n", src);
		serv = new_servlet(source);
	}

	if (serv)
	{
		serv->lastponged = time(NULL);
	}

	G_UNLOCK(add_address);
}

void add_upstream_server(FullPacket * p)
{
	// This gets called when we get a REPLY_SERVERS
	uint16_t port, *k;
	char    src[INET_ADDRSTRLEN];
	GSList *moo;
	in_addr_t *f;
	struct sockaddr_in i;
	// This needs to
	// 1. Check if the incoming server is already in the list
	k = p->data + 4;
	f = p->data;
	i.sin_addr.s_addr = *f;
	i.sin_port = *k;
	i.sin_family = PF_INET;

	moo = g_slist_find_custom(possible_servers, &i,
							  (GCompareFunc) cmp_sockaddr);
	printf("received pong\n");
	if (!moo)
	{
		// 2. Add it
		Servlet *l;
		l = new_servlet_real(&i);
		possible_servers = g_slist_append(possible_servers, l);
		inet_ntop(AF_INET, p->data, src, INET_ADDRSTRLEN);
		printf("Reply_Servers %s:%i\n", src, htons(port));
	}
}

void    do_connection(char *host);
void look_for_new_servers()
{
	// This gets called when the servlet runs out of machines
	int     i = 0, j = 0;
	GSList *moo;
	// It creates a servlet 'object'/thread for a random selection (say 10)
	// in the list.
	printf("look_for_new_servers()\n");
	moo = possible_servers;
	while (moo)
	{
		Servlet *j;
		j = moo->data;
		new_servlet_startup(j);
		// If we are on our 5th time round the loop exit
		if (i == 5)
			break;
		i++;
		moo = moo->next;
	}
	// Try and avoid problems with deleting things your currently on.
	for (j = 0; j <= i; j++)
	{
		possible_servers = g_slist_remove(possible_servers, possible_servers);
	}
	if (i == 0)
	{
		gchar  *ask;
		// If the list is empty ask the user what to do.
		ask_new_server();
	}
}

void apply_packet(FullPacket * p)
{
	// This checks the packet is a proper packet and then does a switch
	// for the correct operation.
	struct sockaddr_in *source;
	char    src[INET_ADDRSTRLEN];
	source = &(p->addr);
	// this really shouldn't be done
	if (!inet_ntop(AF_INET, &source->sin_addr, src, INET_ADDRSTRLEN))
	{
		printf("problem getting host\n");
	}
	else
	{
		printf("packet came from %s:%i type %i\n", src,
			   ntohs(source->sin_port), p->packet->operation);
	}
	if (p->packet->magic != htons(PROTOCOL_MAGIC))
	{
		printf("Not the right protocol\n");
	}
	else
	{
		if (p->packet->version != htons(PROTOCOL_VERSION))
		{
			printf("Not the same protocol version\n");
		}
		else
		{
			switch (p->packet->operation)
			{
			case REQUEST_PING:
				printf("Ping\n");
				check_add_server(source);
				generate_reply_servers(p);
				break;
			case REQUEST_FILESEARCH:
				get_request_search(p);
				break;
			case REQUEST_WANTFILE:
				get_request_wantfile(p);
				break;
			case REPLY_TRANSFERWAITING:
				download_reply_transferwaiting(p);
				break;
			case REPLY_FILESEARCH:
				get_reply_search(p);
				break;
			case REPLY_SERVERS:
				{
					check_add_server(source);
					add_upstream_server(p);
					break;

				}
			case REPLY_NOSERVERS:
				{
					printf("Other end knows no servers\n");
					check_add_server(source);
					break;
				}
			default:
				printf("Unknown operation\n");
				break;
			}
		}
	}
	free_packet(p);
}

G_LOCK_DEFINE(socket_list);
G_LOCK_DEFINE(closedown);

int     listening = 0;
GThreadPool *request_processors;
GThreadPool *pingers;

void close_me(Servlet * s)
{
	free(s);
}

void closedown(int signal)
{
	//This segfaults far far far too much
	GSList *moo;
	// Don't run the socket shutdown twice at the same time
	G_LOCK(socket_list);
	if (sockets != NULL)
	{
		// g_thread_pool_free(pingers,TRUE,TRUE);
		// g_thread_pool_free(request_processors,TRUE,TRUE);
		printf("closedown\n");
		// close each socket
		moo = sockets;
		g_slist_foreach(sockets, (GFunc) close_me, NULL);
		printf("%p\n", sockets);
		g_slist_free(sockets);
		sockets = NULL;
	}
	shutdown(listening, SHUT_RDWR);
	close(listening);
	G_UNLOCK(socket_list);
	// g_thread_exit(0);
	exit(0);
}

#define MAXBUF 256
#define MAX_PACKET 65536
void incoming(Servlet * servlet)
{
	// Listens to the socket
	void   *buffer[MAX_PACKET];
	int     bytes;
	FullPacket *fullpacket;
	struct sockaddr_in client;
	ssize_t j;
	j = sizeof(client);
	printf("i'm listening\n");
	while (1)
	{
		fullpacket = malloc(sizeof(FullPacket));
		bytes = recvfrom(listening, buffer, MAX_PACKET, 0,
						 (struct sockaddr *) &client, &j);
		if (bytes < 0)
		{
			perror("incoming recvfrom");
			return;
		}
		read_packet(buffer, fullpacket, &client);
		g_thread_pool_push(request_processors, fullpacket, NULL);
	}
}

G_LOCK_DEFINE(hostlookup);

void do_connection(char *host)
{
	// Adds an outgoing connection to the servlets
	struct hostent *serverDNS;
	struct sockaddr_in server;
	Servlet *j;

	G_LOCK(hostlookup);
	serverDNS = gethostbyname(host);
	memcpy(&server.sin_addr.s_addr, serverDNS->h_addr, serverDNS->h_length);
	G_UNLOCK(hostlookup);

	server.sin_family = AF_INET;
	server.sin_port = htons(3773);

	j = new_servlet(&server);
}

void idle_pinger(Servlet * servlet)
{
	// This sends a packet to the specified server once every few seconds
	// it also checks to see if it has no recieved a pong for over a few
	// seconds.
	time_t  f, i;
	Packet  fr;
	f = time(NULL);
	i = (servlet->lastpinged) + PING_TIME;
	if (servlet->lastpinged == 0 || i < f)
	{
		fillin_packet(&fr, REQUEST_PING, 0, 0);
		printf("sending ping, %i seconds since last pong %x:%i\n",
			   f - servlet->lastponged, servlet->client.sin_addr.s_addr,
			   ntohs(servlet->client.sin_port));
		send_packet(listening, &(servlet->client), &fr, NULL);
		// :sleep(1);
		servlet->lastpinged = time(NULL);
	}
	i = (servlet->lastponged) + PONG_TIME;
	if (i < f)
	{
		// This removes a machine when it ping timeouts
		printf("Ping timeout\n");
		sockets = model_gslist_delete_record(servermodel, servlet);
		free_servlet(servlet);
		if (g_slist_length(sockets) == 0)
		{
			look_for_new_servers();
		}
		return;
	}
	sleep(4);
	// Push myself back into the queue to be processed
	g_thread_pool_push(pingers, servlet, NULL);
}

int cmp_sockaddr(Servlet * j, struct sockaddr_in *in)
{
	// Compares a single servlet against an ip address returns 0 if true
	if ((j->client.sin_addr.s_addr) > (in->sin_addr.s_addr))
		return 1;
	if ((j->client.sin_addr.s_addr) < (in->sin_addr.s_addr))
		return -1;
	if ((j->client.sin_port) > (in->sin_port))
		return 1;
	if ((j->client.sin_port) < (in->sin_port))
		return -1;
	return 0;
}

Servlet *find_servlet(struct sockaddr_in * in)
{
	// Given an ip address finds the first one which matches the ip address
	// Returns NULL on nothing, or the servlet
	GSList *moo;
	moo = g_slist_find_custom(sockets, in, (GCompareFunc) cmp_sockaddr);
	if (!moo)
		return NULL;
	return moo->data;
}

Servlet *new_servlet_real(struct sockaddr_in * in)
{
	// Set up storage
	Servlet *moo;
	moo = (Servlet *) malloc(sizeof(Servlet));
	memcpy(&moo->client, in, sizeof(struct sockaddr_in));
	moo->lastpinged = 0;
	moo->lastponged = time(NULL);
	return moo;
}

void free_servlet(Servlet * serv)
{
	// This really should inform the listening stuff that it's done.
	// free(serv);
}

Servlet *new_servlet(struct sockaddr_in *in)
{
	// Allocates the servlet and adds it to the list.
	Servlet *moo;
	moo = new_servlet_real(in);
	new_servlet_startup(moo);
	return moo;
}

void new_servlet_startup(Servlet * serv)
{
	// Start pinging
	G_LOCK(socket_list);
	sockets = g_slist_append(sockets, serv);
	// Urgh, I hate Glib data structures at times.
	model_gslist_new_record(servermodel, sockets);
	printf("Should be added\n");
	G_UNLOCK(socket_list);

	g_thread_pool_push(pingers, serv, NULL);
}

void servlet_dump(Servlet * serv)
{
	printf("Servlet is %x:%i\n", serv->client.sin_addr.s_addr,
		   ntohs(serv->client.sin_port));
}

void do_listen()
{
	struct sockaddr_in server;
	Servlet *j;
	char    one[256];
	// Create an internet stream socket
	if ((listening = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("failed creating socket");
		exit(1);
	}
	// We're using internet sockets here
	server.sin_family = AF_INET;
	// We want port 3773, but make sure it is in the right endian.
	server.sin_port = htons(3773);
	// We want it bound to all incoming IP addresses
	server.sin_addr.s_addr = INADDR_ANY;

	// Disable the wait time
	if (setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) <
		0)
	{
		perror("failure setting REUSEADDR");
		exit(1);
	}
	// Bind to the socket
	if (bind(listening, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		perror("failed binding socket");
		exit(1);
	}
	printf("listening socket %i\n", listening);
	j = new_servlet_real(&server);
	g_thread_create((GThreadFunc) incoming, j, TRUE, NULL);
}

int get_outgoing_socket(void)
{
	return listening;
}

void setup_sockets()
{
	// If we get a interrupt signal shut the sockets down properly
	printf("%i\n", SIGINT);
	// create a thread for the listening socket and wait for that thread to
	// finish.
	pingers = g_thread_pool_new((GFunc) idle_pinger, NULL, 30, FALSE, NULL);
	request_processors =
		g_thread_pool_new((GFunc) apply_packet, NULL, 30, FALSE, NULL);
	do_listen();
	look_for_new_servers();
}
