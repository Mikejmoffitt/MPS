#include <stdio.h>
#include "dmf.h"
#include "mps_write.h"

int main(int argc, char **argv)
{
	dmf_info_t *dmf_info;
	if (argc < 2)
	{
		printf("Usage: %s [unzipped DMF]\n", argv[0]);
		return 0;
	}

	dmf_info = dmf_create(argv[1]);
	dmf_print(dmf_info);
	printf("Writing MPS instruments\n");
	FILE *f = fopen("instruments.bin", "wb");

	size_t wp = 0;

	for (unsigned int i = 0; i < dmf_info->total_instruments; i++)
	{
		printf("Instrument \"%s\"\n", dmf_info->instruments[i].name);
		wp += mps_write_instrument(f, &dmf_info->instruments[i]);
		printf("Wrote %d; wp = %X\n", i, wp);
	}

	fclose(f);
}
