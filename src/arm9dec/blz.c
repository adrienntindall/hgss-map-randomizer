#include "blz.h"

static int get_decompressed_length(u8 *end, u32 input_len)
{
	return input_len + *(u32 *)&end[-4];
}

static void decompress(u8 *end)
{
	footer f = *(footer *)&end[-8];

	u8 *data = end - f.total_size;

	u32 cmpSize = f.total_size - f.hdr_size,
		outSize = f.total_size + f.ext_size;

	while (outSize)
	{
		u32 flags = data[--cmpSize];

		for (u32 i = 0; i < 8; i++)
		{
			if ((flags << i) & 0x80)
			{
				cmpSize -= 2;

				u16 op = *(u16 *)&data[cmpSize];

				u32 len = (op >> 12) + 3,
					ofs = (op & 0xFFF) + 3;

				outSize -= len;

				while (len--)
					data[outSize + len] = data[outSize + len + ofs];
			}
			else
				data[--outSize] = data[--cmpSize];

			if (!outSize)
				break;
		}
	}
}

u8 *decompress_blz_buffer(u8 *in, u32 length_in, u32 *length_out)
{
	*length_out = get_decompressed_length(in + length_in, length_in);

	u8 *buf = malloc(*length_out);
	memcpy(buf, in, length_in);
	decompress(buf + length_in);

	return buf;
}