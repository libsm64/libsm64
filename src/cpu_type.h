#pragma once

#ifdef WIN32
	#define DL_INT_SIZE int32_t
#endif
#ifdef WIN64
	#define DL_INT_SIZE int64_t
#endif