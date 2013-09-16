#include "pcc.h"

#include <assert.h>
#include "mcast-settings.h"
#include "mcast_utils.h"
#include "wave_utils.h"

#define INTEFACE_BIND_ADDRESS "0.0.0.0"
#define MCAST_GROUP_ADDRESS "239.0.0.1"
#define MCAST_PORT_NUMBER "25000"
#define FILE_TO_SEND_NAME "play.wav"
#define DEFAULT_TTL (2)
#define CHUNK_SIZE (1024)
#define DEFAULT_SLEEP_TIME ((useconds_t)((1000000*1024)/16000))

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

static void dump_wave(FILE * fp, struct master_riff_chunk const * p_header)
{
    wav_format_chunk_2_t const * p_wave_format_chunk;
    plainwaveformat_t const * p_plainwave;
    wav_subchunk_t const * p_subchunk;
    p_wave_format_chunk = &p_header->format_chunk_2_;
    p_plainwave = &p_wave_format_chunk->plain_wav_;
    p_subchunk = &p_plainwave->subchunk_;

    fprintf(fp, "%4.4u %s : %c%c%c%c %u\n", __LINE__, __FILE__,
            p_header->ckid_[0],
            p_header->ckid_[1],
            p_header->ckid_[2],
            p_header->ckid_[3],
            p_header->cksize_);

    fprintf(fp, "%4.4u %s : %c%c%c%c %c%c%c %u\n", __LINE__, __FILE__,
            p_wave_format_chunk->waveid_[0],
            p_wave_format_chunk->waveid_[1],
            p_wave_format_chunk->waveid_[2],
            p_wave_format_chunk->waveid_[3],
            p_wave_format_chunk->ckid_[0],
            p_wave_format_chunk->ckid_[1],
            p_wave_format_chunk->ckid_[2],
            p_wave_format_chunk->cksize_ 
           );

    fprintf(fp, "%4.4u %s : 0x%4.4hx %hu %u %u %hu\n", __LINE__, __FILE__, 
            p_plainwave->wavFormat_.wFormatTag,
            p_plainwave->wavFormat_.nChannels,
            p_plainwave->wavFormat_.nSamplesPerSec,
            p_plainwave->wavFormat_.nAvgBytesPerSec,
            p_plainwave->wavFormat_.nBlockAlign
           );

    fprintf(fp, "%4.4u %s : 0x%4.4hx %hu %u %u %hu\n", __LINE__, __FILE__, 
            p_plainwave->wavFormat_.wFormatTag,
            p_plainwave->wavFormat_.nChannels,
            p_plainwave->wavFormat_.nSamplesPerSec,
            p_plainwave->wavFormat_.nAvgBytesPerSec,
            p_plainwave->wavFormat_.nBlockAlign
           );

    fprintf(fp, "%4.4u %s : %c%c%c%c 0x%8.8x 0x%4.4hx 0x%4.4hx ..\n", __LINE__, __FILE__, 
            p_subchunk->subchunkId_[0],
            p_subchunk->subchunkId_[1],
            p_subchunk->subchunkId_[2],
            p_subchunk->subchunkId_[3],
            p_subchunk->subchunk_size_,
            p_subchunk->samples16_[0],
            p_subchunk->samples16_[1]
           );

    //    fprintf(fp, "%4.4u %s : %8.8hx\n", __LINE__, __FILE__, p_header->format_chunk_.wavefmt_.wFormatTag);
}

const int8_t * get_samples_buffer(struct master_riff_chunk const * p_header)
{
    wav_format_chunk_2_t const * p_wave_format_chunk;
    plainwaveformat_t const * p_plainwave;
    wav_subchunk_t const * p_subchunk;
    p_wave_format_chunk = &p_header->format_chunk_2_;
    p_plainwave = &p_wave_format_chunk->plain_wav_;
    p_subchunk = &p_plainwave->subchunk_;
    return &p_subchunk->samples8_[0];
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

int main(int argc, char ** argv)
{
    uint8_t const * p_file;
    struct stat st_file;
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
    result = getaddrinfo("0.0.0.0", NULL, &a_hints, &p_iface_address);
    assert(0 == result);
    dump_addrinfo(stderr, p_iface_address);
    fprintf(stderr, "%4.4u %s : %d\n", __LINE__, __FILE__, result);
    result = join_mcast_group_set_ttl(s, p_group_address, p_iface_address, DEFAULT_TTL); 
    assert(0 == result);
    {
        int fd;
        fd = open(FILE_TO_SEND_NAME, O_RDONLY);
        assert(fd >= 0);
        result = fstat(fd, &st_file);
        assert(0 == result);
        p_file = mmap(NULL, st_file.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
        close(fd);
    }
    struct master_riff_chunk const * p_header = (struct master_riff_chunk const *)p_file; 
    dump_wave(stdout, p_header);
    while (1)
    {
        int8_t const * p_buffer;
        useconds_t sleep_time_usec = DEFAULT_SLEEP_TIME;
        ssize_t bytes_written;
        size_t idx = 0;
        p_buffer = get_samples_buffer(p_header);
        size_t samples_buffer_size = get_samples_buffer_size(p_header);
        fprintf(stderr, "%4.4u %s : %u \n", __LINE__, __FILE__, samples_buffer_size);
        for (; idx < samples_buffer_size / CHUNK_SIZE; ++idx)
        {
            int8_t const * p_buffer_to_send = p_buffer + CHUNK_SIZE*idx;
            bytes_written = sendto(s, p_buffer_to_send, 
                    CHUNK_SIZE, 0, p_group_address->ai_addr, p_group_address->ai_addrlen); 
            if (bytes_written < 0)
            {
                fprintf(stderr, "%4.4u %s : %d %s\n", __LINE__, __FILE__, errno, strerror(errno));
                break;
            }
            else
            {
                //fprintf(stderr, "%4.4u %s : %d %p\n", __LINE__, __FILE__, bytes_written, p_buffer_to_send);
            }
            usleep(sleep_time_usec);
        }
    }
    return 0;
}
