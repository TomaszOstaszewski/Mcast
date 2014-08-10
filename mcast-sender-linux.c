/**
 * @file mcast-sender-linux.c
 * @brief 1
 * @details 2
 * @author Tomasz Ostaszewski (ato013)
 * @date 2014-Aug-07
 * @par History
 * <pre>
 * </pre>
 */

#include "pcc.h"
#include <unistd.h>
#include <assert.h>
#include <limits.h>

#include "mcast-settings.h"
#include "mcast_utils.h"
#include "wave_utils.h"

#define DEFAULT_TARGET_UNICAST_ADDR "192.168.0.2"
#define DEFAULT_TARGET_MULTICAST_ADDR "239.0.0.1"
#define DEFAULT_TARGET_BROADCAST_ADDR "192.168.0.0"
#define DEFAULT_TARGET_PORT 8888
#define DEFAULT_CHUNK_SIZE 160

#define INTEFACE_BIND_ADDRESS "0.0.0.0"
#define MCAST_GROUP_ADDRESS "239.0.0.1"
#define MCAST_PORT_NUMBER "25000"
#define FILE_TO_SEND_NAME "play.wav"
#define DEFAULT_TTL (2)

typedef enum AddressingType
{
    USE_MULTICAST,
    USE_BROADCAST,
    USE_UNICAST
} AddressingType_t;

typedef struct cmdLineOptions
{
    const char* pFileName_;
    AddressingType_t addrType_;
    const char* pTargetAddress_;
    uint16_t nPort_;
    uint16_t nChunkSize_;
} cmdLineOptions_t;

#define DEFAULT_CMD_LINE_OPTIONS                                                                   \
    {                                                                                              \
        .pFileName_ = FILE_TO_SEND_NAME, .addrType_ = USE_BROADCAST,                               \
        .pTargetAddress_ = DEFAULT_TARGET_BROADCAST_ADDR, .nPort_ = DEFAULT_TARGET_PORT,           \
        .nChunkSize_ = DEFAULT_CHUNK_SIZE                                                          \
    };

static volatile sig_atomic_t g_stop_processing;

static void dumpCmdLineOptions(FILE* fp, const cmdLineOptions_t* pParsedOpt)
{
    fprintf(fp, "%s %s:%5.5hu %d\n", pParsedOpt->pFileName_, pParsedOpt->pTargetAddress_,
            pParsedOpt->nPort_, pParsedOpt->addrType_);
}

static int parseCmdLine(int argc, char** argv, cmdLineOptions_t* pParsedOpt)
{
    do
    {
        int result;
        result = getopt(argc, argv, "bmut:p:f:s:");
        if (-1 == result)
        {
            break;
        }
        switch (result)
        {
            case 's':
            {
                int nParsed;
                unsigned int nChunkSize = (unsigned int)-1;
                nParsed = sscanf(optarg, "%u", &nChunkSize);
                if (1 != nParsed || nChunkSize > USHRT_MAX || nChunkSize < 8)
                {
                    fprintf(stderr, "%4.4u %s : %d %u\n", __LINE__, __func__, nParsed, nChunkSize);
                    return -1;
                }
                else
                {
                    pParsedOpt->nChunkSize_ = nChunkSize;
                }
            }
            break;
            case 'f':
                pParsedOpt->pFileName_ = optarg;
                break;
            case 'b':
                pParsedOpt->addrType_ = USE_BROADCAST;
                break;
            case 'm':
                pParsedOpt->addrType_ = USE_MULTICAST;
                break;
            case 'u':
                pParsedOpt->addrType_ = USE_UNICAST;
                break;
            case 't':
                pParsedOpt->pTargetAddress_ = optarg;
                {
                    struct in_addr afInetAddr = { 0 };
                    int result;
                    result = inet_pton(AF_INET, optarg, &afInetAddr);
                    fprintf(stdout, "%4.4u %s : %d %8.8x\n", __LINE__, __func__, result,
                            afInetAddr.s_addr);
                }
                break;
            case 'p':
            {
                int nParsed;
                unsigned int nPort = (unsigned int)-1;
                nParsed = sscanf(optarg, "%u", &nPort);
                if (1 != nParsed || nPort > USHRT_MAX)
                {
                    fprintf(stderr, "%4.4u %s : %d %u\n", __LINE__, __func__, nParsed, nPort);
                    return -1;
                }
                else
                {
                    pParsedOpt->nPort_ = nPort;
                }
            }
            break;
            default:
                fprintf(stderr, "Unknown option: %c\n", result);
                return -1;
        }
    } while (1);
    return 0;
}

static useconds_t calculateSleepTime(size_t chunkSize, size_t samplingRate, size_t bytesPerSample)
{
    return (1000000 * chunkSize) / (bytesPerSample * samplingRate);
}

static void dump_addrinfo(FILE* fp, struct addrinfo const* p_addr)
{
    struct addrinfo const* p_idx = p_addr;
    for (; NULL != p_idx; p_idx = p_idx->ai_next)
    {
        struct sockaddr_in const* p_sock_addr_in = (struct sockaddr_in const*)p_idx->ai_addr;
        uint8_t const* in_addr = (uint8_t const*)&p_sock_addr_in->sin_addr;
        printf("%hhu.%hhu.%hhu.%hhu\n", in_addr[0], in_addr[1], in_addr[2], in_addr[3]);
    }
}

static void dump_wave(FILE* fp, struct master_riff_chunk const* p_header)
{
    wav_format_chunk_2_t const* p_wave_format_chunk;
    plainwaveformat_t const* p_plainwave;
    wav_subchunk_t const* p_subchunk;
    p_wave_format_chunk = &p_header->format_chunk_2_;
    p_plainwave = &p_wave_format_chunk->plain_wav_;
    p_subchunk = &p_plainwave->subchunk_;

    fprintf(fp, "%4.4u %s : %c%c%c%c %u\n", __LINE__, __FILE__, p_header->ckid_[0],
            p_header->ckid_[1], p_header->ckid_[2], p_header->ckid_[3], p_header->cksize_);

    fprintf(fp, "%4.4u %s : %c%c%c%c %c%c%c %u\n", __LINE__, __FILE__,
            p_wave_format_chunk->waveid_[0], p_wave_format_chunk->waveid_[1],
            p_wave_format_chunk->waveid_[2], p_wave_format_chunk->waveid_[3],
            p_wave_format_chunk->ckid_[0], p_wave_format_chunk->ckid_[1],
            p_wave_format_chunk->ckid_[2], p_wave_format_chunk->cksize_);

    fprintf(fp, "%4.4u %s : 0x%4.4hx %hu %u %u %hu\n", __LINE__, __FILE__,
            p_plainwave->wavFormat_.wFormatTag, p_plainwave->wavFormat_.nChannels,
            p_plainwave->wavFormat_.nSamplesPerSec, p_plainwave->wavFormat_.nAvgBytesPerSec,
            p_plainwave->wavFormat_.nBlockAlign);

    fprintf(fp, "%4.4u %s : 0x%4.4hx %hu %u %u %hu\n", __LINE__, __FILE__,
            p_plainwave->wavFormat_.wFormatTag, p_plainwave->wavFormat_.nChannels,
            p_plainwave->wavFormat_.nSamplesPerSec, p_plainwave->wavFormat_.nAvgBytesPerSec,
            p_plainwave->wavFormat_.nBlockAlign);

    fprintf(fp, "%4.4u %s : %c%c%c%c 0x%8.8x 0x%4.4hx 0x%4.4hx ..\n", __LINE__, __FILE__,
            p_subchunk->subchunkId_[0], p_subchunk->subchunkId_[1], p_subchunk->subchunkId_[2],
            p_subchunk->subchunkId_[3], p_subchunk->subchunk_size_, p_subchunk->samples16_[0],
            p_subchunk->samples16_[1]);
}

const int8_t* get_samples_buffer(struct master_riff_chunk const* p_header)
{
    wav_format_chunk_2_t const* p_wave_format_chunk;
    plainwaveformat_t const* p_plainwave;
    wav_subchunk_t const* p_subchunk;
    p_wave_format_chunk = &p_header->format_chunk_2_;
    p_plainwave = &p_wave_format_chunk->plain_wav_;
    p_subchunk = &p_plainwave->subchunk_;
    return &p_subchunk->samples8_[0];
}

size_t get_samples_buffer_size(struct master_riff_chunk const* p_header)
{
    wav_format_chunk_2_t const* p_wave_format_chunk;
    plainwaveformat_t const* p_plainwave;
    wav_subchunk_t const* p_subchunk;
    p_wave_format_chunk = &p_header->format_chunk_2_;
    p_plainwave = &p_wave_format_chunk->plain_wav_;
    p_subchunk = &p_plainwave->subchunk_;
    return p_subchunk->subchunk_size_;
}

static void sigint_handle(int signal) { g_stop_processing = 1; }

int main(int argc, char** argv)
{
    const uint8_t* p_file;
    struct stat st_file;
    int result;
    struct addrinfo aTargetAddress = { 0 };
    struct addrinfo* p_iface_address;
    struct addrinfo a_hints;
    struct sockaddr_in anInetAddr = { .sin_family = AF_INET };
    cmdLineOptions_t aCmdLineOpt = DEFAULT_CMD_LINE_OPTIONS;
    SOCKET s;

    aTargetAddress.ai_addrlen = sizeof(struct sockaddr_in);

    result = parseCmdLine(argc, argv, &aCmdLineOpt);
    if (0 != result)
    {
        dumpCmdLineOptions(stdout, &aCmdLineOpt);
        exit(EXIT_FAILURE);
    }
    dumpCmdLineOptions(stdout, &aCmdLineOpt);

    memset(&a_hints, 0, sizeof(a_hints));
    s = socket(AF_INET, SOCK_DGRAM, 0);
    assert(s >= 0);

    a_hints.ai_flags = AI_PASSIVE;
    result = getaddrinfo("0.0.0.0", NULL, &a_hints, &p_iface_address);
    assert(0 == result);
    dump_addrinfo(stderr, p_iface_address);
    fprintf(stderr, "%4.4u %s : %d\n", __LINE__, __FILE__, result);
    aTargetAddress.ai_addr = (struct sockaddr*)&anInetAddr;
    aTargetAddress.ai_addrlen = sizeof(anInetAddr);
    result = inet_pton(AF_INET, aCmdLineOpt.pTargetAddress_, &anInetAddr.sin_addr);
    if (0 >= result)
    {
        fprintf(stderr, "%4.4u %s : %d %d %s\n", __LINE__, __func__, result, errno,
                strerror(errno));
        result = 1;
    }
    dump_addrinfo(stderr, &aTargetAddress);

    result = 0;
    anInetAddr.sin_port = htons(aCmdLineOpt.nPort_);
    fprintf(stdout, "%4.4u %s : %8.8x %4.4hx\n", __LINE__, __func__, anInetAddr.sin_addr.s_addr,
            anInetAddr.sin_port);

    switch (aCmdLineOpt.addrType_)
    {
        case USE_UNICAST:
            break;
        case USE_BROADCAST:
        {
            int broadcastEnable = 1;
            result =
                setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
        }
        break;
        case USE_MULTICAST:
            assert(0);
#if 0
            aTargetAddress.ai_family = AF_INET;
            aTargetAddress.ai_socktype = SOCK_DGRAM;
            result = join_mcast_group_set_ttl(s, &aTargetAddress,, p_iface_address, DEFAULT_TTL);e
            fprintf(stderr, "%4.4u %s : %d %8.8x %5.5hu\n", __LINE__, __func__, result,
                    ntohl(((struct sockaddr_in*)aTargetAddress.ai_addr)->sin_addr.s_addr),
                    ntohs(((struct sockaddr_in*)aTargetAddress.ai_addr)->sin_port));
#endif
            break;
        default:
            break;
    }
    assert(0 == result);
    {
        int fd;
        fd = open(aCmdLineOpt.pFileName_, O_RDONLY);
        assert(fd >= 0);
        result = fstat(fd, &st_file);
        assert(0 == result);
        p_file = mmap(NULL, st_file.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
        close(fd);
    }
    struct master_riff_chunk const* p_header = (struct master_riff_chunk const*)p_file;
    dump_wave(stdout, p_header);
    {
        struct sigaction query_action;
        memset(&query_action, 0, sizeof(query_action));
        query_action.sa_handler = &sigint_handle;
        if (sigaction(SIGINT, NULL, &query_action) < 0)
        {
            exit(EXIT_FAILURE);
        }
        /* sigaction returns -1 in case of error. */
    }
    useconds_t sleepTime = calculateSleepTime(aCmdLineOpt.nChunkSize_, 8000, 2);
    fprintf(stdout, "%4.4u %s : %u\n", __LINE__, __func__, sleepTime);
    size_t samples_buffer_size = get_samples_buffer_size(p_header);

    int8_t const* p_buffer;
    p_buffer = get_samples_buffer(p_header);

    while (!g_stop_processing)
    {
        ssize_t bytes_written;
        size_t idx;
        const int8_t* p_buffer_to_send = p_buffer + aCmdLineOpt.nChunkSize_ * idx;
        p_buffer_to_send = p_buffer;

        for (idx = 0; idx < samples_buffer_size / aCmdLineOpt.nChunkSize_;
             ++idx, p_buffer_to_send += aCmdLineOpt.nChunkSize_)
        {
            bytes_written = sendto(s, p_buffer_to_send, aCmdLineOpt.nChunkSize_, 0,
                                   aTargetAddress.ai_addr, aTargetAddress.ai_addrlen);
            usleep(sleepTime);
            if (bytes_written < 0)
            {
                fprintf(stderr, "%4.4u %s : %d %s\n", __LINE__, __FILE__, errno, strerror(errno));
                break;
            }
        }
    }
    munmap((void*)p_file, st_file.st_size);
    close(s);
    return 0;
}
