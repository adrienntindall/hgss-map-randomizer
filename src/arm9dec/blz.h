#include "common.h"

typedef struct footer
{
	u32 total_size : 24;
	u32 hdr_size : 8;
	u32 ext_size;
} footer;

u8 *decompress_blz_buffer(u8 *in, u32 length_in, u32 *length_out);