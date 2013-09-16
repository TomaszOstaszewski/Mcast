
#include "pcc.h"

#include <assert.h>
#include "mcast-settings.h"
#include "mcast_utils.h"
#include "wave_utils.h"

#define INTEFACE_BIND_ADDRESS "0.0.0.0"
#define MCAST_GROUP_ADDRESS "239.0.0.1"
#define MCAST_PORT_NUMBER "25000"
#define CHUNK_SIZE (1024)
#define DEFAULT_TTL (5)
#define DEFAULT_SLEEP_TIME ((1000000*1024)/8000)

static uint8_t g_input_buffer[CHUNK_SIZE];

static void dump_addrinfo(FILE * fp, struct addrinfo const * p_addr)
{
    struct addrinfo const * p_idx = p_addr;
    for (; NULL != p_idx; p_idx = p_idx->ai_next)
    {
        struct sockaddr_in const * p_sock_addr_in = (struct sockaddr_in const *)p_idx->ai_addr;
        uint8_t const * in_addr = (uint8_t const *)&p_sock_addr_in->sin_addr;
        printf("%hhu.%hhu.%hhu.%hhu\n", in_addr[0], in_addr[1], in_addr[2], in_addr[3]);
    }
}

void dump_buffer(const uint8_t * p_buffer, size_t length, struct sockaddr * p_from, socklen_t from_length)
{
	fprintf(stderr, "%4.4u %s : %u \n", __LINE__, __func__, from_length);
}

int main(int argc, char ** argv)
{
    int result;
    struct addrinfo * p_group_address;
    struct addrinfo * p_iface_address;
    struct addrinfo a_hints;
    memset(&a_hints, 0, sizeof(a_hints));
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    assert(s>=0); 
    a_hints.ai_family = AF_INET;
    a_hints.ai_protocol = 0;
    a_hints.ai_socktype = AF_INET;
    result = getaddrinfo(MCAST_GROUP_ADDRESS, MCAST_PORT_NUMBER, &a_hints, &p_group_address);
    assert(0 == result);
    dump_addrinfo(stderr, p_group_address);
    a_hints.ai_flags = AI_PASSIVE;
    result = getaddrinfo("192.168.0.190", NULL, &a_hints, &p_iface_address);
    assert(0 == result);
    dump_addrinfo(stderr, p_iface_address);
    result = bind(s, p_iface_address->ai_addr, p_iface_address->ai_addrlen);
    result = join_mcast_group_set_ttl(s, p_group_address, p_iface_address, DEFAULT_TTL);
    while (1)
    {
	ssize_t bytes_read;
	struct sockaddr recv_from_data;
	socklen_t recv_from_length;
	bytes_read = recvfrom(s, &g_input_buffer[0], sizeof(g_input_buffer), 0, &recv_from_data, &recv_from_length);  
	if (bytes_read > 0)
	{
	}
	else
	{
		fprintf(stderr, "%4.4u %s : %d %s\n", __LINE__, __func__, errno, strerror(errno));
	}
    }
    return 0;
}

