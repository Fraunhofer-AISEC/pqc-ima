#ifndef PQCLEAN_RANDOMBYTES_H
#define PQCLEAN_RANDOMBYTES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/random.h>
#include <linux/types.h>

#ifdef _WIN32
/* Load size_t on windows */
#include <crtdefs.h>
#endif /* _WIN32 */

/*
 * Write `n` bytes of high quality random bytes to `buf`
 */
#define randombytes     get_random_bytes

#ifdef __cplusplus
}
#endif

#endif /* PQCLEAN_RANDOMBYTES_H */
