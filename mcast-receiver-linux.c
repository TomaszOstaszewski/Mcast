
#include "pcc.h"

#include <assert.h>
#include <math.h>
#include "mcast-settings.h"
#include "mcast_utils.h"
#include "wave_utils.h"

#define INTEFACE_BIND_ADDRESS "0.0.0.0"
#define MCAST_GROUP_ADDRESS "239.0.0.1"
#define MCAST_PORT_NUMBER "25000"
#define CHUNK_SIZE (1024)
#define DEFAULT_TTL (5)
#define DEFAULT_SLEEP_TIME ((1000000*(CHUNK_SIZE)/8000)
#define TEMPLATE_FILE_NAME "play.wav"

static uint8_t g_input_buffer[CHUNK_SIZE];

typedef  union S {
    struct sockaddr const * p_sockaddr;
    struct sockaddr_in const * p_sockaddr_in;
} sock_union_t;

typedef struct mcast_receiver_options {
    const char * p_save_file_name_;
} mcast_receiver_options_t;

static void dump_addrinfo(FILE * fp, struct addrinfo const * p_addr)
{
    sock_union_t sock;
    sock.p_sockaddr = p_addr->ai_addr;
    struct addrinfo const * p_idx = p_addr;
    for (; NULL != p_idx; p_idx = p_idx->ai_next)
    {
        struct sockaddr_in const * p_sock_addr_in = sock.p_sockaddr_in;
        uint8_t const * in_addr = (uint8_t const *)&p_sock_addr_in->sin_addr;
        printf("%hhu.%hhu.%hhu.%hhu\n", in_addr[0], in_addr[1], in_addr[2], in_addr[3]);
    }
}

void dump_buffer(const uint8_t * p_buffer, size_t length, struct sockaddr * p_from, socklen_t from_length)
{
    fprintf(stderr, "%4.4u %s : %u \n", __LINE__, __func__, from_length);
}

static int set_reuse_addr(SOCKET s)
{
    int optval, rc;
    optval = 1;
    rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
    if (rc == SOCKET_ERROR)
    {
        return -1;
    }
    return 0;
}

volatile sig_atomic_t g_stop_processing;

static void sigint_handle(int signal)
{
    g_stop_processing = 1;
}

double avg_squar_power(int16_t const * samples, size_t samples_size)
{
    double ret_val = 0;
    size_t idx = 0;
    switch (samples_size%4)
    {
        for (idx = 0; idx < samples_size/4; idx+=4)
        {
            case 0:
                ret_val += samples[idx]*samples[idx]/samples_size;
            case 1:
                ret_val += samples[idx+1]*samples[idx+1]/samples_size;
            case 2:
                ret_val += samples[idx+2]*samples[idx+2]/samples_size;
            case 3:
                ret_val += samples[idx+3]*samples[idx+3]/samples_size;
        }
    }
    return sqrt(ret_val);
}

static int parse_cmd_line(char ** argv, int argc, struct mcast_receiver_options * p_opt)
{
    int opt;
    while ((opt = getopt(argc, argv, "s:")) != -1) 
    {
        switch (opt) 
        {
            case 's':
                p_opt->p_save_file_name_ = optarg;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-m value] [-f file name] \n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    return 0;
}

const int8_t * get_samples_buffer_int8_const(struct master_riff_chunk const * p_header)
{
    wav_format_chunk_2_t const * p_wave_format_chunk;
    plainwaveformat_t const * p_plainwave;
    wav_subchunk_t const * p_subchunk;
    p_wave_format_chunk = &p_header->format_chunk_2_;
    p_plainwave = &p_wave_format_chunk->plain_wav_;
    p_subchunk = &p_plainwave->subchunk_;
    return &p_subchunk->samples8_[0];
}

int16_t * get_samples_buffer_int16(struct master_riff_chunk * p_header)
{
    wav_format_chunk_2_t * p_wave_format_chunk;
    plainwaveformat_t * p_plainwave;
    wav_subchunk_t * p_subchunk;
    p_wave_format_chunk = &p_header->format_chunk_2_;
    p_plainwave = &p_wave_format_chunk->plain_wav_;
    p_subchunk = &p_plainwave->subchunk_;
    return &p_subchunk->samples16_[0];
}

size_t get_samples_buffer_size(struct master_riff_chunk const * p_header)
{
    wav_format_chunk_2_t const * p_wave_format_chunk;
    plainwaveformat_t const * p_plainwave;
    wav_subchunk_t const * p_subchunk;
    p_wave_format_chunk = &p_header->format_chunk_2_;
    p_plainwave = &p_wave_format_chunk->plain_wav_;
    p_subchunk = &p_plainwave->subchunk_;
    return p_subchunk->subchunk_size_;
}

struct master_riff_chunk const * get_template_file_data(const char * template_file_path)
{
    int fd;
    int result;
    void * p_file_data = NULL;
    struct stat st_file;
    fd = open(template_file_path, O_RDONLY);
    assert(fd >= 0);
    if (fd >= 0)
    {
        result = fstat(fd, &st_file);
        assert(0 == result);
        p_file_data = mmap(NULL, st_file.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
        close(fd);
    }
    return (struct master_riff_chunk const *)p_file_data;
}

struct master_riff_chunk * create_save_file(const char * save_file_name, struct master_riff_chunk const * p_template)
{
    int result;
    int fd;
    struct master_riff_chunk * p_retval = NULL;
    fd = open(save_file_name, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd >= 0)
    {
        uint32_t file_size = 8 + p_template->cksize_;
        result = ftruncate(fd, file_size);
        if (0 == result)
        {
            p_retval = (struct master_riff_chunk *) mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            close(fd);
            memcpy(p_retval, p_template, sizeof(struct master_riff_chunk)); 
            fprintf(stdout, "%4.4u %d %u %p", __LINE__, file_size, sizeof(struct master_riff_chunk), p_retval);
            msync(p_retval, file_size, MS_SYNC); 
        }
        else
        {
            fprintf(stderr, "%4.4u %d %d %s", __LINE__, result, errno, strerror(errno));
        }
    }
    return p_retval; 
}

void append_data_to_wav_file(struct master_riff_chunk * p_wav_file, int16_t const * p_data, size_t data_size)
{
    static size_t s_current_offset = 0;
    int16_t * samples_buffer = get_samples_buffer_int16(p_wav_file);
    size_t output_buffer_capacity_bytes = get_samples_buffer_size(p_wav_file);
    size_t in_idx, out_idx;
    out_idx = s_current_offset;
    for (in_idx = 0; in_idx < data_size; ++in_idx, ++out_idx)
    {
        if (out_idx >= output_buffer_capacity_bytes/2)
            out_idx = 0;
        samples_buffer[out_idx] = p_data[in_idx];
    } 
    s_current_offset = out_idx;
}

int main(int argc, char ** argv)
{
    int result;
    fd_set read_fd;
    struct addrinfo * p_group_address;
    struct addrinfo * p_iface_address;
    struct addrinfo a_hints;
    struct mcast_receiver_options prog_options;
    memset(&prog_options, 0, sizeof(prog_options));
    memset(&a_hints, 0, sizeof(a_hints));
    if (0 == parse_cmd_line(argv, argc, &prog_options))
    {
        struct master_riff_chunk const * p_template = NULL;
        struct master_riff_chunk * p_out_file = NULL;
        fprintf(stderr, "%4.4u : %s\n", __LINE__, prog_options.p_save_file_name_);
        SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
        assert(s>=0); 
        a_hints.ai_family = AF_INET;
        a_hints.ai_protocol = 0;
        a_hints.ai_socktype = SOCK_DGRAM;
        result = getaddrinfo(MCAST_GROUP_ADDRESS, MCAST_PORT_NUMBER, &a_hints, &p_group_address);
        assert(0 == result);
        dump_addrinfo(stderr, p_group_address);
        a_hints.ai_flags = AI_PASSIVE;
        result = getaddrinfo(INTEFACE_BIND_ADDRESS, MCAST_PORT_NUMBER, &a_hints, &p_iface_address);
        assert(0 == result);
        result = set_reuse_addr(s);
        assert(0 == result);
        dump_addrinfo(stderr, p_iface_address);
        result = join_mcast_group_set_ttl(s, p_group_address, p_iface_address, DEFAULT_TTL);
        assert(0 == result);
        {
            struct sigaction query_action;
            memset(&query_action, 0, sizeof(query_action));
            query_action.sa_handler = &sigint_handle;
            if (sigaction (SIGINT, NULL, &query_action) < 0)
                exit(EXIT_FAILURE);
            /* sigaction returns -1 in case of error. */
        }
        if (prog_options.p_save_file_name_)
        {
            p_template = get_template_file_data(TEMPLATE_FILE_NAME);
            assert(NULL != p_template);
            p_out_file = create_save_file(prog_options.p_save_file_name_, p_template);
            assert(NULL != p_out_file);
        }
        while (!g_stop_processing)
        {
            ssize_t bytes_read;
            struct sockaddr recv_from_data;
            socklen_t recv_from_length = sizeof(recv_from_data);
            struct timeval select_timeout = { 1, 0 };
            FD_ZERO(&read_fd);
            FD_SET(s, &read_fd);
            result = select(s+1, &read_fd, NULL, NULL, &select_timeout); 
            switch (result)
            {
                case -1:
                    break;
                case 0: 
                    fprintf(stdout, "%4.4u %s : Timeout\n", __LINE__, __func__);
                    break;
                default:
                    if (FD_ISSET(s, &read_fd))
                    {
                        bytes_read = recvfrom(s, 
                                &g_input_buffer[0], 
                                sizeof(g_input_buffer), 
                                0, 
                                &recv_from_data, 
                                &recv_from_length);  
                        if (bytes_read >= 0)
                        {
                            int16_t const * p_samples = (int16_t const *)&g_input_buffer[0];
#if 0
                            sock_union_t sock;
                            sock.p_sockaddr = &recv_from_data;
                            fprintf(stdout, "%4.4u %s : %u %s %u %-8.8f %-5.hd %-5.hd %-5.hd %-5.hd ..\n", __LINE__, __func__, bytes_read, 
                                    inet_ntoa(sock.p_sockaddr_in->sin_addr),
                                    ntohs(sock.p_sockaddr_in->sin_port),
                                    avg_squar_power(p_samples, bytes_read/sizeof(int16_t)),
                                    p_samples[0], 
                                    p_samples[1],
                                    p_samples[3],
                                    p_samples[4]
                                   );
#endif
                            if (p_out_file)
                                append_data_to_wav_file(p_out_file, p_samples, bytes_read/2);
                        }
                        else
                        {
                            fprintf(stderr, "%4.4u %s : %d %s\n", __LINE__, __func__, errno, strerror(errno));
                        }
                    }
                    break;
            }
        }
    }
    return 0;
}

