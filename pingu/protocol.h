#define STANDARD_TTL 3
#define REQUEST_PING 0x00
#define REQUEST_FILESEARCH 0x01
#define REQUEST_WANTFILE 0x02
#define REPLY_SERVERS 0xFF	// Reply to ping
#define REPLY_NOSERVERS 0xFB	// Reply to ping, no known servers
#define REPLY_FILESEARCH 0xFE   // Reply to filesearch
#define REPLY_TRANSFERWAITING 0xFD // You have a transfer waiting
#define REPLY_FILENOTFOUND 0xFC // The file is not found.
#define PROTOCOL_MAGIC 0x3773
#define PROTOCOL_VERSION 0x0010
#define PING_TIME 5 
#define PONG_TIME 30
typedef struct _Packet
{
        uint16_t magic;
        uint16_t version;
        uint16_t request_handle;
        uint8_t  ttl;
        uint8_t  operation;
        uint16_t packet_length;
} Packet;

typedef struct _FullPacket
{
        Packet *packet;
        void *data;
	int sockfd;
	struct sockaddr_in addr;
} FullPacket;
void fillin_packet(Packet *p,int operation,uint16_t datalen,uint16_t request);
int send_packet(int sockfd,struct sockaddr_in *addr,Packet *p,void *d);
