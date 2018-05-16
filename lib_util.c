#include <lib.h>

/*
 * Euclidian division in base 2 of two integers using k precision return 0 if no overflow occured
 */
int util_fix32_udiv(uint32_t a, uint32_t b, uint32_t k, uint32_t * x)
{
   uint32_t K, f, r, i;
   K = k;
   i = a / b;
   a = a - i * b;
   a = a << 1;
   f = 0;

   while(K)
   {
      r = a / b;
      a = a - r * b;
      a = a << 1;
      f = (f << 1) + r;
      K--;
   }
   *x = (i << k) + f;

   return ( (i > 0) && (cpu_bls32(i) >= ((int)(32 - k))) ) ? -1 : 0;
}


int util_fix32_from_double(double value, int sign, int intp, int fracp, uint32_t * fix32)
{
   uint32_t * ptr = (uint32_t *)&value;
#if CPU_BYTE_ORDER == CPU_LITTLE_ENDIAN
   uint32_t   msb = *(ptr+1);
   uint32_t   lsb = *ptr;
#else
   uint32_t   lsb = *(ptr+1);
   uint32_t   msb = *ptr;
#endif
   int        exp = (msb >> 20) & 0x7ff;
   int        neg = msb >> 31;
   int        ret;
   uint32_t   result;

   // zero (0+ and 0-) ainsi que les notations 'dénormalisées' (petites valeurs)
   if(exp == 0)
   {
      result = 0;
      if(lsb == 0)
         ret = IEEE754_ZERO;  // absolute zero
      else
         ret = IEEE754_EPSILON; // infinitesimal small value
   }
   // +/- infinite, Nan
   else if(exp == 2047)
   {
      if(lsb == 0)
      {
         result = (0xffffffff >> (32 - intp - fracp));
         if(neg)
         {
            if(sign)
               result = ~result;
            else
               result = 0;
         }

         ret = IEEE754_INFINITE;
      }
      else
      {
         result = 0xdeadbeef;
         ret = IEEE754_NAN;
      }
   }
   // notation 'normalisée'
   else
   {
      int e = (int)exp - 1023;

      // take 11 bits from lsb, 20 bits from msb, and add implicit bit.
      uint32_t mantisse = 0x80000000 | (msb << 11) | (lsb >> 21);

      if(fracp < -e)
      {
         // no significatif bits can be found in mantisse
         result = 0;
         ret = IEEE754_FRACP;
      }
      // special case where number is signed, mantisse is 0x80000000, and intp == e
      else if(sign && neg && (intp == e) && (mantisse == 0x80000000))
      {
         result = 1 << (intp + fracp);
         ret = IEEE754_SIGNED_MIN;
      }
      else if(intp <= e)
      {
         // the '1' implicit cannot be placed in the result, this is an overflow
         result = (0xffffffff >> (32 - intp - fracp));
         if(neg)
         {
            if(sign)
               result = ~result;
            else
               result = 0;
         }

         ret = IEEE754_INTP;
      }
      else
      {
         // at least one significatif bit
         result = mantisse >> (31 - e - fracp);

         // complement à 2 pour obtenir la value signée
         if(neg)
            result = ~result + 1;

         ret = IEEE754_NORMAL;
      }
   }

   *fix32 = result;
   return ret;
}

/*
 * this function return the (signed) integral part of a (signed) fixed point number.
 * intp or fracp can be negative, but we assume that 0 <= intp + fracp <= 32
 */
int util_fix32_to_int(uint32_t raw, int sign, int intp, int fracp)
{
   int val;

   // if negative, there is a sign extension to do
   if(sign && (raw & (1 << (intp + fracp))))
   {
      int sign_ext = ((int)-1) << (fracp + intp);
      val = (int)raw | sign_ext;
   }
   else
      val = (int)raw;

   if(fracp < 0)
      return val << -fracp;
   else
      return val >> fracp;
}

/*
 * This function give the opposite of the previous one
 */
uint32_t util_fix32_from_int(int val, int sign, int intp, int fracp)
{
   uint32_t raw;

   if(intp <= 0)
      return 0;

   if(fracp < 0)
   {
      // shift right the value. the sign extension is done automatically
      raw =  (uint32_t)(val >> (-fracp));

      // if the number if unsigned, ensure that inserted number are all zeros
      if(sign == 0)
         raw &= ((1 << (32 + fracp)) - 1);
   }
   else
   {
      raw =  (uint32_t)(val << fracp);
   }

   return raw;
}

/*!
 * Convert an octopus double value to a signed integer
 * \param[in]  value          input double value
 * \param[in]  fracp          number of bits of fractional part
 * \param[in]  fromOctopus    convert from octopus little endian format
 * \return the converted value
 */
int util_int_from_double(const void * value, int fracp, int fromOctopus)
{
#if CPU_BYTE_ORDER == CPU_BIG_ENDIAN
   const uint16_t * b = (const uint16_t *)value;

   union 
   {
      uint16_t byte[4];
      double val;
   } u;

   if(fromOctopus)
   {
      /**
       * octopus values are in double little endian format
       */
      u.byte[0] = b[3];
      u.byte[1] = b[2];
      u.byte[2] = b[1];
      u.byte[3] = b[0];
   }
   else
      u.val = *((double*)value);

   int32_t val;
   int intp = 31 - fracp;

   util_fix32_from_double(u.val, 1, intp, fracp, (uint32_t *)&val);
   return (int)val;
#elif CPU_BYTE_ORDER == CPU_LITTLE_ENDIAN
   double val = *(double *)value;
   return (int)((1<<fracp)*val);
#endif
}

/*!
 * Print double value
 * \param[out] str           formated string double value
 * \param[in]  value         input double value
 * \param[in]  max           max size of formated string
 * \param[in]  fromOctopus   convert from octopus little endian format
 * \return string_printf return value
 */
int util_print_from_double(char * str, const void * value, int max, int fromOctopus)
{
#if CPU_BYTE_ORDER == CPU_BIG_ENDIAN
   /**
    *  we will convert it using fixed notation, with 5 bits of fractional part, 
    *  and displayed with 3 decimal digits
    */
#define FRACTIONAL_PART_NB_BITS  5
   int32_t val = util_int_from_double(value, FRACTIONAL_PART_NB_BITS, fromOctopus);
   uint32_t ceil = val & ((1 << FRACTIONAL_PART_NB_BITS) - 1);
   return string_nprintf(str, max, "%d.%03d", val >> FRACTIONAL_PART_NB_BITS, (ceil * 1000) / (1 << FRACTIONAL_PART_NB_BITS));
#undef FRACTIONAL_PART_NB_BITS
  
#elif CPU_BYTE_ORDER == CPU_LITTLE_ENDIAN
   return string_nprintf(str, max, "%lf", *(double *)value);
#endif
}

/*!
 * Print octopus double value
 * \param[out] str      formated string double value
 * \param[in]  value    input double value
 * \return string_printf return value
 */
int util_snr_from_double(char * str, const void * value, int max)
{
   return util_print_from_double(str, value, max, 1);
}


