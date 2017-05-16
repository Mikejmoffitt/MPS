#include <stdio.h>
#include "dmf.h"

int main(int argc, char **argv)
{
	dmf_info_t dmf_info;
	if (argc < 2)
	{
		printf("Usage: %s [unzipped DMF]\n", argv[0]);
		return 0;
	}

	dmf_create(argv[1]);
	dmf_print(&dmf_info);
}
