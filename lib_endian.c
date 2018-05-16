#include "lib_endian.h"

/**
 * conditional swap functions
 */
#if (CPU_BYTE_ORDER == CPU_BIG_ENDIAN)
uint16_t  lib_htole16(uint16_t host_16bits)          { return cpu_bswap16(host_16bits); }
uint32_t  lib_htole32(uint32_t host_32bits)          { return cpu_bswap32(host_32bits); }
uint64_t  lib_htole64(uint64_t host_64bits)          { return cpu_bswap64(host_64bits); }

uint16_t  lib_le16toh(uint16_t little_endian_16bits) { return cpu_bswap16(little_endian_16bits); }
uint32_t  lib_le32toh(uint32_t little_endian_32bits) { return cpu_bswap32(little_endian_32bits); }
uint64_t  lib_le64toh(uint64_t little_endian_64bits) { return cpu_bswap64(little_endian_64bits); }
#endif

#if (CPU_BYTE_ORDER == CPU_LITTLE_ENDIAN)
uint16_t  lib_htobe16(uint16_t host_16bits)          { return cpu_bswap16(host_16bits); }
uint32_t  lib_htobe32(uint32_t host_32bits)          { return cpu_bswap32(host_32bits); }
uint64_t  lib_htobe64(uint64_t host_64bits)          { return cpu_bswap64(host_64bits); }

uint16_t  lib_be16toh(uint16_t big_endian_16bits)    { return cpu_bswap16(big_endian_16bits); }
uint32_t  lib_be32toh(uint32_t big_endian_32bits)    { return cpu_bswap32(big_endian_32bits); }
uint64_t  lib_be64toh(uint64_t big_endian_64bits)    { return cpu_bswap64(big_endian_64bits); }
#endif

/**
 * store multiple/single unsigned 64/32/16 bit integers in a buffer using a big endian or little endian format
 * return the next offset
 */
uint8_t * lib_stmbe64(const uint64_t* val, uint8_t *buffer, int nb_double_words)
{
   uint8_t * ret = buffer + nb_double_words * sizeof(uint64_t);
   uint64_t value;
   int nb_bytes;

   // start from the end
   buffer = ret; 
   val += nb_double_words;

   while(nb_double_words-- > 0)
   {
      nb_bytes = 8;
      value = *(--val);
      while(nb_bytes-- > 0) {
         *(--buffer) = value & 0xFF;
         value >>= 8;
      }
   }
   return ret;
}

uint8_t * lib_stbe64(uint64_t val, uint8_t *buffer)
{
   return lib_stmbe64(&val, buffer, 1);
}

uint8_t * lib_stmle64(const uint64_t* val, uint8_t *buffer, int nb_double_words)
{
   uint64_t value;
   int nb_bytes;

   while(nb_double_words-- > 0)
   {
      nb_bytes = 8;
      value = *(val++);
      while(nb_bytes-- > 0) {
         *(buffer++) = value & 0xFF; value >>= 8;
      }
   }
   return buffer;
}

uint8_t * lib_stle64(uint64_t val, uint8_t *buffer)
{
   return lib_stmle64(&val, buffer, 1);
}


uint8_t * lib_stmbe32(const uint32_t* value, uint8_t *buffer, int nb_words)
{
   uint8_t * ret;
   uint32_t val;
   ret = buffer + nb_words * sizeof(uint32_t);

   // start from the end
   buffer = ret;
   value += nb_words;

   while(nb_words-- > 0)
   {
      val = *(--value);
      *(--buffer) = val & 0xFF; val >>= 8;
      *(--buffer) = val & 0xFF; val >>= 8;
      *(--buffer) = val & 0xFF; val >>= 8;
      *(--buffer) = val & 0xFF;
   }
   return ret;
}

uint8_t * lib_stbe32(uint32_t val, uint8_t *buffer)
{
   return lib_stmbe32(&val, buffer, 1);
}

uint8_t * lib_stmle32(const uint32_t* value, uint8_t *buffer, int nb_words)
{
   uint32_t val;

   while(nb_words-- > 0)
   {
      val = *(value++);
      *(buffer++) = val & 0xFF; val >>= 8;
      *(buffer++) = val & 0xFF; val >>= 8;
      *(buffer++) = val & 0xFF; val >>= 8;
      *(buffer++) = val & 0xFF;
   }
   return buffer;
}

uint8_t * lib_stle32(uint32_t val, uint8_t *buffer)
{
   return lib_stmle32(&val, buffer, 1);
}


uint8_t * lib_stmbe16(const uint16_t* value, uint8_t *buffer, int nb_half_words)
{
   uint8_t * ret;
   uint16_t val;
   ret = buffer + nb_half_words * sizeof(uint16_t);

   // start from the end
   buffer = ret;
   value += nb_half_words;

   while(nb_half_words-- > 0)
   {
      val = *(--value);
      *(--buffer) = val & 0xFF; val >>= 8;
      *(--buffer) = val & 0xFF;
   }
   return ret;
}

uint8_t * lib_stbe16(uint16_t val, uint8_t *buffer)
{
   return lib_stmbe16(&val, buffer, 1);
}

uint8_t * lib_stmle16(const uint16_t* value, uint8_t *buffer, int nb_half_words)
{
   uint16_t val;

   while(nb_half_words-- > 0)
   {
      val = *(value++);
      *(buffer++) = val & 0xFF; val >>= 8;
      *(buffer++) = val & 0xFF;
   }
   return buffer;
}

uint8_t * lib_stle16(uint16_t val, uint8_t *buffer)
{
   return lib_stmle16(&val, buffer, 1);
}

uint8_t * lib_stmle8(const uint8_t* value, uint8_t *buffer, int nb_bytes)
{
   uint8_t val;

   while(nb_bytes-- > 0)
   {
      val = *(value++);
      *(buffer++) = val & 0xFF;
   }
   return buffer;
}


/**
 * load from a buffer several/a single 64/32/16 bit unsigned integer formatted in big endian or little endian
 */

const uint8_t * lib_ldmbe64(const uint8_t *buffer, uint64_t * value, int nb_double_words)
{
   uint64_t val;
   int nb_bytes;

   while(nb_double_words-- > 0)
   {
      nb_bytes = 8;
      val = 0;
      while(nb_bytes-- > 0) {
         val <<= 8;
         val |= *(buffer++);
      }
      *(value++) = val;
   }
   return buffer;
}

const uint8_t * lib_ldbe64(const uint8_t *buffer, uint64_t * value)
{
   return lib_ldmbe64(buffer, value, 1);
}


const uint8_t * lib_ldmle64(const uint8_t *buffer, uint64_t * value, int nb_double_words)
{
   const uint8_t * ret = buffer + nb_double_words * sizeof(uint64_t);
   uint64_t val;
   int nb_bytes;

   // start from the end
   buffer = ret; 
   value += nb_double_words;

   while(nb_double_words-- > 0)
   {
      nb_bytes = 8;
      val = 0;
      while(nb_bytes-- > 0) {
         val <<= 8;
         val |= *(--buffer);
      }
      *(--value) = val;
   }
   return ret;
}

const uint8_t * lib_ldle64(const uint8_t *buffer, uint64_t * value)
{
   return lib_ldmle64(buffer, value, 1);
}



const uint8_t * lib_ldmbe32(const uint8_t *buffer, uint32_t * value, int nb_words)
{
   while(nb_words-- > 0)
   {
      *value = ((uint32_t)buffer[0] << 24)
              |((uint32_t)buffer[1] << 16)
              |((uint32_t)buffer[2] <<  8)
              |((uint32_t)buffer[3]);
      buffer+=4;
      value++;
   }
   return buffer;
}

const uint8_t * lib_ldbe32(const uint8_t *buffer, uint32_t * value)
{
   return lib_ldmbe32(buffer, value, 1);
}

const uint8_t * lib_ldmle32(const uint8_t *buffer, uint32_t * value, int nb_words)
{
   while(nb_words-- > 0)
   {
      *value = ((uint32_t)buffer[0])
              |((uint32_t)buffer[1] << 8)
              |((uint32_t)buffer[2] << 16)
              |((uint32_t)buffer[3] << 24);
      value++;
      buffer+=4;
   }
   return buffer;
}

const uint8_t * lib_ldle32(const uint8_t *buffer, uint32_t * value)
{
   return lib_ldmle32(buffer, value, 1);
}



const uint8_t * lib_ldmbe16(const uint8_t *buffer, uint16_t * value, int nb_half_words)
{
   while(nb_half_words-- > 0)
   {
      *value = ((uint16_t)buffer[0] <<  8)
              |((uint16_t)buffer[1]);
      buffer+=sizeof(uint16_t);
      value++;
   }
   return buffer;
}

const uint8_t * lib_ldbe16(const uint8_t *buffer, uint16_t * value)
{
   return lib_ldmbe16(buffer, value, 1);
}

const uint8_t * lib_ldmle16(const uint8_t *buffer, uint16_t * value, int nb_half_words)
{
   while(nb_half_words-- > 0)
   {
      *value = ((uint16_t)buffer[0])
              |((uint16_t)buffer[1] << 8);
      buffer+=sizeof(uint16_t);
      value++;
   }
   return buffer;
}

const uint8_t * lib_ldle16(const uint8_t *buffer, uint16_t * value)
{
   return lib_ldmle16(buffer, value, 1);
}

const uint8_t * lib_ldmle8(const uint8_t *buffer, uint8_t * value, int nb_bytes)
{
   while(nb_bytes-- > 0)
   {
      *value = buffer[0];
      buffer+=sizeof(uint8_t);
      value++;
   }
   return buffer;
}


