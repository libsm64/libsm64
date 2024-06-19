#ifndef _OS_LIBC_H_
#define _OS_LIBC_H_

#include "ultratypes.h"

// These are defined in macOS (BSD?), so we want to def them out.
#ifndef __APPLE__

// Old deprecated functions from strings.h, replaced by memcpy/memset.
extern void bcopy(const void *, void *, size_t);
extern void bzero(void *, size_t);

#endif

#endif /* !_OS_LIBC_H_ */
