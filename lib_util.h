#ifndef LIB_UTILS
#define LIB_UTILS

#include <cpu.h>

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 * \addtogroup util
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

#define IEEE754_ZERO        0       // zero, zero+, zero-
#define IEEE754_EPSILON     1       // +/- epsilon
#define IEEE754_INFINITE    2       // +/- infinite
#define IEEE754_NAN        -1       // not an number
#define IEEE754_FRACP      -2       // fractional part is not large enought to store at least one sign bit
#define IEEE754_INTP       -3       // interger part is not enough large to store the msb
#define IEEE754_NORMAL      3       // number can be expressed with intp and fracp
#define IEEE754_SIGNED_MIN  4       // number can be expressed if we have an additional sign bit


int       util_fix32_udiv(uint32_t a, uint32_t b, uint32_t k, uint32_t * x);

int       util_fix32_from_double(double value, int sign, int intp, int fracp, uint32_t * fix32);

int       util_fix32_to_int(uint32_t raw, int sign, int intp, int fracp);

uint32_t  util_fix32_from_int(int val, int sign, int intp, int fracp);



/* octopus double processig */

int       util_snr_from_double(char * str, const void * value, int max);
 
int       util_int_from_double(const void * value, int fracp, int fromOctopus);

int       util_print_from_double(char * str, const void * value, int max, int fromOctopus);

/**@} util */
/**@} LIB  */
/**@} PAL  */

#ifdef __cplusplus
}
#endif
#endif
