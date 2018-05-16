#ifndef LIB_ENDIAN_H
#define LIB_ENDIAN_H

#include <cpu.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 */

/**
 * \addtogroup endian
 *@{
 */
#if (CPU_BYTE_ORDER == CPU_LITTLE_ENDIAN)
#define lib_htole16(host_16bits)          (host_16bits)
#define lib_htole32(host_32bits)          (host_32bits)
#define lib_htole64(host_64bits)          (host_64bits)

#define lib_le16toh(little_endian_16bits) (little_endian_16bits)
#define lib_le32toh(little_endian_32bits) (little_endian_32bits)
#define lib_le64toh(little_endian_64bits) (little_endian_64bits)

#elif (CPU_BYTE_ORDER == CPU_BIG_ENDIAN)
#define lib_htobe16(host_16bits)          (host_16bits)
#define lib_htobe32(host_32bits)          (host_32bits)
#define lib_htobe64(host_64bits)          (host_64bits)

#define lib_be16toh(big_endian_16bits)    (big_endian_16bits)
#define lib_be32toh(big_endian_32bits)    (big_endian_32bits)
#define lib_be64toh(big_endian_64bits)    (big_endian_64bits)

#else
#error "this cpu byte order is not supported by the current lib implementation"
#endif

#if (CPU_BYTE_ORDER != CPU_LITTLE_ENDIAN)
uint16_t  lib_htole16(uint16_t host_16bits);
uint32_t  lib_htole32(uint32_t host_32bits);
uint64_t  lib_htole64(uint64_t host_64bits);

uint16_t  lib_le16toh(uint16_t little_endian_16bits);
uint32_t  lib_le32toh(uint32_t little_endian_32bits);
uint64_t  lib_le64toh(uint64_t little_endian_64bits);
#endif

#if (CPU_BYTE_ORDER != CPU_BIG_ENDIAN)
uint16_t  lib_htobe16(uint16_t host_16bits);
uint32_t  lib_htobe32(uint32_t host_32bits);
uint64_t  lib_htobe64(uint64_t host_64bits);

uint16_t  lib_be16toh(uint16_t big_endian_16bits);
uint32_t  lib_be32toh(uint32_t big_endian_32bits);
uint64_t  lib_be64toh(uint64_t big_endian_64bits);
#endif

/**
 * store into a buffer an unsigned integer in a buffer using a big endian or little endian format
 */
uint8_t * lib_stbe64(uint64_t value, uint8_t *buffer);
uint8_t * lib_stmbe64(const uint64_t* value, uint8_t *buffer, int nb_double_words);
uint8_t * lib_stle64(uint64_t value, uint8_t *buffer);
uint8_t * lib_stmle64(const uint64_t* value, uint8_t *buffer, int nb_double_words);

uint8_t * lib_stbe32(uint32_t value, uint8_t *buffer);
uint8_t * lib_stmbe32(const uint32_t* value, uint8_t *buffer, int nb_words);
uint8_t * lib_stle32(uint32_t value, uint8_t *buffer);
uint8_t * lib_stmle32(const uint32_t* value, uint8_t *buffer, int nb_words);

uint8_t * lib_stle16(uint16_t value, uint8_t *buffer);
uint8_t * lib_stmle16(const uint16_t* value, uint8_t *buffer, int nb_half_words);
uint8_t * lib_stbe16(uint16_t value, uint8_t *buffer);
uint8_t * lib_stmbe16(const uint16_t* value, uint8_t *buffer, int nb_half_words);

uint8_t * lib_stmle8(const uint8_t* value, uint8_t *buffer, int nb_bytes);

/**
 * load from a buffer an unsigned integer formatted in the buffer using a big endian or little endian format
 */
const uint8_t * lib_ldbe64(const uint8_t *buffer, uint64_t * value);
const uint8_t * lib_ldmbe64(const uint8_t *buffer, uint64_t * value, int nb_double_words);
const uint8_t * lib_ldle64(const uint8_t *buffer, uint64_t * value);
const uint8_t * lib_ldmle64(const uint8_t *buffer, uint64_t * value, int nb_double_words);

const uint8_t * lib_ldle32(const uint8_t *buffer, uint32_t * value);
const uint8_t * lib_ldmle32(const uint8_t *buffer, uint32_t * value, int nb_words);
const uint8_t * lib_ldbe32(const uint8_t *buffer, uint32_t * value);
const uint8_t * lib_ldmbe32(const uint8_t *buffer, uint32_t * value, int nb_words);

const uint8_t * lib_ldle16(const uint8_t *buffer, uint16_t * value);
const uint8_t * lib_ldmle16(const uint8_t *buffer, uint16_t * value, int nb_half_words);
const uint8_t * lib_ldbe16(const uint8_t *buffer, uint16_t * value);
const uint8_t * lib_ldmbe16(const uint8_t *buffer, uint16_t * value, int nb_half_words);

const uint8_t * lib_ldmle8(const uint8_t *buffer, uint8_t * value, int nb_bytes);

/**@}*/
#ifdef __cplusplus
}
#endif

/**@} PAL */
/**@} LIB */

#endif
