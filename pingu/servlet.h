#include <time.h>
#include <netinet/in.h>
typedef struct _Servlet
{
        struct sockaddr_in client;
        time_t lastpinged;
        time_t lastponged;
} Servlet;

void closedown(int signal);
void set_destination_ip(struct sockaddr_in *dest,in_addr_t local,in_addr_t current);

char *ask_new_server();
Servlet *new_servlet_real(struct sockaddr_in * in);
void new_servlet_startup(Servlet * serv);
void servlet_dump(Servlet *serv);
Servlet *new_servlet(struct sockaddr_in *in);
Servlet *find_servlet(struct sockaddr_in *in);
int cmp_sockaddr(Servlet *j,struct sockaddr_in *in);
void free_servlet(Servlet *serv);
extern void *servermodel;

int get_outgoing_socket();
