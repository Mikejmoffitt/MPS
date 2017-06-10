#include <stdio.h>
#include "dmf.h"
#include "mps_write.h"

void decode(const char *fname, const char *outname)
{
	size_t wp = 0;
	FILE *f;
	dmf_info_t *dmf_info;
	dmf_info = dmf_create(fname);
	dmf_print(dmf_info);

	unsigned int pattern_loc[10][MAX_PTRN];

	f = fopen(outname, "wb");

	// TODO: System channel lookup, not 10

	// Big-endian WORD per entry, per channel, times total length
	printf("Allocating space for arrangement\n");
	wp = 2 * 10 * dmf_info->total_rows_in_pattern_matrix;
	fseek(f, wp, SEEK_SET);

	printf("Writing MPS instruments\n");
	for (unsigned int i = 0; i < dmf_info->total_instruments; i++)
	{
		printf("Instrument \"%s\"\n", dmf_info->instruments[i].name);
		wp += mps_write_instrument(f, &dmf_info->instruments[i]);
		printf("Wrote %d; wp = %zX\n", i, wp);
	}

	printf("Writing MPS patterns\n");

	// TODO: This should use a system channel lookup, just just be ten
	for (unsigned int x = 0; x < 10; x++)
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
