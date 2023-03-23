#include "blz.h"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s arm9.bin output\n", argv[0]);
		return 1;
	}
	
	printf("Saving %s to %s...\n", argv[1], argv[2]);

	FILE *in  = fopen(argv[1], "rb");
	FILE *out = fopen(argv[2], "wb");

	fseek(in, 0, SEEK_END);
	int sz = ftell(in) - 12;
	rewind(in);

	u8 *buf = malloc(sz);

	puts("Reading input...");

	fread(buf, 1, sz, in);
	fclose(in);

	puts("Decompressing...");

	u32 len = 0;
	u8 *outbuf = decompress_blz_buffer(buf, sz, &len);

	printf("Successfully decompressed 0x%x bytes to 0x%x bytes...\n", sz, len);
	puts("Writing output...");

	fwrite(outbuf, 1, len, out);
	fclose(out);

	free(buf);
	free(outbuf);

	puts("Done!");

	return 0;
}