#include <stdio.h>
#include "dmf.h"
#include "mps_write.h"

size_t fwrite16be(FILE *f, uint16_t value)
{
	size_t written = 0;
	written += fputc(((value & 0xFF00) >> 8), f);
	written += fputc((value & 0x00FF), f);
	return written;
}

void decode(const char *fname, const char *outname)
{
	size_t wp = 0;
	// TODO: Check this, not hard-code it
	const unsigned int channel_count = 10;
	FILE *f;
	dmf_info_t *dmf_info;
	dmf_info = dmf_create(fname);
	dmf_print(dmf_info);

	unsigned int pattern_loc[10][MAX_PTRN];

	f = fopen(outname, "wb");

	printf("Allocating space for arrangement and metainfo\n");
	wp += sizeof(mps_header_t) + (2 * channel_count * dmf_info->total_rows_in_pattern_matrix);
	fseek(f, wp, SEEK_SET);

	printf("Writing MPS instruments\n");
	for (unsigned int i = 0; i < dmf_info->total_instruments; i++)
	{
		printf("Instrument \"%s\"\n", dmf_info->instruments[i].name);
		wp += mps_write_instrument(f, &dmf_info->instruments[i]);
		printf("Wrote %d; wp = %zX\n", i, wp);
	}

	printf("Writing MPS patterns\n");

	for (unsigned int x = 0; x < channel_count; x++)
	{
		for (unsigned int y = 0; y < MAX_PTRN; y++)
		{
			pattern_t *p = &(dmf_info->patterns[x][y]);
			if (p->touched)
			{
				printf("Pattern %d, %d: %zX\n", x, y, wp);
			}
			pattern_loc[x][y] = wp;
			wp += mps_write_pattern(f, p);
		}
	}

	printf("Writing arrangement\n");

	fseek(f, sizeof(mps_header_t), SEEK_SET);
	wp = 0;

	for (unsigned int x = 0; x < channel_count; x++)
	{
		for (unsigned int y = 0; y < dmf_info->total_rows_in_pattern_matrix; y++)
		{
			unsigned int pattern_id = dmf_info->pattern_matrix[(y * channel_count) + x];
			printf("%d, %d: pattern %d at location %X\n", x, y, pattern_id, pattern_loc[x][pattern_id]);
			wp += fwrite16be(f, pattern_loc[x][pattern_id]);
		}
	}

	printf("Wriitng header\n");
	fseek(f, 0, SEEK_SET);
	mps_write_header(f, dmf_info);

	fclose(f);
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printf("Usage: %s <track.dmf> <outname.mps>\n", argv[0]);
		return 0;
	}

	decode(argv[1], argv[2]);
	return 0;
}
