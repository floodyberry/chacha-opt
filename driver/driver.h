#ifndef DRIVER_H
#define DRIVER_H

#include <stddef.h>
#include "config.h"

/* use HAVE_INTxxx to determine if an integer with [8,16,32,64,128] bits is available */

#if defined(HAVE_STDINT)
	#include <stdint.h>
#else
	DEFINE_INT64
	DEFINE_INT32
	DEFINE_INT16
	DEFINE_INT8
#endif

/* stdint doesn't currently detect this, but may in the future */
DEFINE_INT128

#endif /* DRIVER_H */
