#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <glib.h>
#include <stdint.h>										
#define DEFAULT_TIMEOUT 300 // 5 minutes
typedef void (*OnTimeout) (uint16_t req_no,void *k);
typedef struct _request
{
        time_t starttime; // Time this was started
	OnTimeout timeout; // Function to call on timeout
	void *data; // User data
	void *widget; // User data 2
	gboolean active; // Is the program actually using this or is it just waiting for timeout?
	gboolean timedout; // Has this timedout and waiting for the program to release it?
	int reqno;
} request;

void request_init();
void request_end(uint16_t req);
uint16_t request_start(request *k);
request *request_get(uint16_t k);
