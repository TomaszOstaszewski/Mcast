/**
 * @brief
 * @details
 * Add refernce to Milewski's book (C++ in Action) in which he claims that such a files
 * do more harm than good. I claim otherwise.
 * @file receiver-limits.h
 */
#if !defined RECEIVER_LIMITS_H_8EB576C7_5DFD_4274_9144_6DA21FC72039
#define RECEIVER_LIMITS_H_8EB576C7_5DFD_4274_9144_6DA21FC72039

/**
 * @brief Defines an upper limit for number of replay chunks that create the replay buffer for DirectSound subsystem.
 * @details The DirectSound subsytem replays the WAV data which is feed into it.
 * The data is not feed byte by byte, but rather in chunks. This preprocessor constant
 * defines the upper limit on number of such chunks. The collection of such chunks, 
 * no more than RECEIVER_SETTINGS_MAX_NUMBER_OF_CHUNKS, creates the replay buffer.
 */
#define RECEIVER_SETTINGS_MAX_NUMBER_OF_CHUNKS (16)

/**
 * @brief Defines a lower limit on of replay chunks that create the replay buffer for DirectSound subsystem.
 * @details The DirectSound subsytem replays the WAV data which is feed into it.
 * The data is not feed byte by byte, but rather in chunks. This preprocessor constant
 * defines the lower limit on number of such chunks. The collection of such chunks, 
 * no less than RECEIVER_SETTINGS_MIN_NUMBER_OF_CHUNKS, creates the replay buffer.
 */
#define RECEIVER_SETTINGS_MIN_NUMBER_OF_CHUNKS (2)

/**
 * @brief Imposes a lower limit on size of a single replay chunk. 
 * @details A single chunk from the replay buffer has size that is a power of 2. This preprocessor
 * constant imposes a lower limit on the exponent of this number. A single chunk size cannot be less
 * than \c 2**RECEIVER_SETTINGS_MIN_CHUNK_SIZE_EXPONENT.
 * @sa RECEIVER_SETTINGS_MAX_CHUNK_SIZE_EXPONENT
 */
#define RECEIVER_SETTINGS_MIN_CHUNK_SIZE_EXPONENT (9)

/**
 * @brief Imposes a lower limit on size of a single replay chunk. 
 * @details A single chunk from the replay buffer has size that is a power of 2. This preprocessor
 * constant imposes an upper limit on the exponent of this number. A single chunk size cannot be more
 * than \c 2**RECEIVER_SETTINGS_MIN_CHUNK_SIZE_EXPONENT.
 * @sa RECEIVER_SETTINGS_MAX_CHUNK_SIZE_EXPONENT
 */
#define RECEIVER_SETTINGS_MAX_CHUNK_SIZE_EXPONENT (14)

/**
 * @brief
 */
#define RECEIVER_SETTINGS_MIN_JITTER_BUFFER_SIZE_EXPONENT (12)

/**
 * @brief
 */
#define RECEIVER_SETTINGS_MIN_JITTER_BUFFER_SIZE_EXPONENT (16)

#endif /* !defined RECEIVER_LIMITS_H_8EB576C7_5DFD_4274_9144_6DA21FC72039 */
