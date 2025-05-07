#if !defined( ENDIAN_H_ )
#define ENDIAN_H_

#include <linux/stddef.h>
#include <linux/types.h>

void put_bigendian( void *target, unsigned long long value, size_t bytes );
unsigned long long get_bigendian( const void *target, size_t bytes );

#endif /* ENDIAN_H_ */
