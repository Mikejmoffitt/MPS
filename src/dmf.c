#include "dmf.h"
#include <zlib.h>
#include <stdlib.h>
#include <string.h>

// Lookup for number of channels per system
static const unsigned int system_total_channels[] = 
{
	0, // Invalid System
	0, //
	10, // FM6 + PSG4
	4, // PSG4
	4, // PU1, PU2, WAV, NOI
	6, // WAV6
	5, // PU1, PU2, TRI, NOI, DPCM
	3, // SID3
	13, // FM8 + PCM6
	0, // 0x09
	0, // 0x0A
	0, // 0x0B
	0, // 0x0C
	0, // 0x0D
	0, // 0x0E
	0, // 0x0F
	0, // 0x10
	0, // 0x11
	13, // FM~9 + PSG4
	0, // 0x13
	0, // 0x14
	0, // 0x15
	0, // 0x16
	3 // SID3
};

uint8_t *dmf_decompress(FILE *f)
{
	uint8_t *dst = NULL;
	uint8_t a = 0;
	uint8_t *src = NULL;
	size_t dstSize = 0;
	size_t srcSize = 0;
	size_t i = 0;

	fseek(f, 0, SEEK_SET);
	while (!feof(f))
	{
		fread(&a, 1, 1, f);
		srcSize++;
	}
	dstSize = srcSize * 256;

	src = malloc(srcSize);
	dst = malloc(dstSize);

	if (src && dst)
	{
		fseek(f, 0, SEEK_SET);
		while (!feof(f))
		{
			fread(&src[i], 1, 1, f);
			i++;
		}
		uncompress(dst, &dstSize, src, srcSize);
		free(src);
		fclose(f);
	}
	return dst;
}

dmf_info_t *dmf_create(const char *path)
{
	uint8_t *dmf_raw;
	dmf_info_t *ret = NULL;
	unsigned int i = 0;
	FILE *f = fopen(path, "rb");
	if (!f)
	{
		fprintf(stderr, "Could not open %s for reading.\n", path);
		return NULL;
	}

	printf("Trying to decompress:\n");
	dmf_raw = dmf_decompress(f);
	if (strncmp((const char *)&dmf_raw[0], ".DelekDefleMask.", 16) != 0)
	{
		printf("Error: This is not a Deflemask file. Aborting.\n");
		return NULL;
	}
	else
	{
		printf("Detected deflemask file.\n");
	}

	// get started bringing in data
	ret = (dmf_info_t *)malloc(sizeof(dmf_info_t));
	if (!ret)
	{
		fprintf(stderr, "Couldn't allocate memory for DMF info.\n");
		return NULL;
	}

	i = 16;

	// Format, system set
	ret->file_version = dmf_raw[i++];
	ret->system_type = dmf_raw[i++];
	printf("File version: %X\n", ret->file_version);
	printf("System type: %X\n", ret->system_type);

	// Visual information
	{
		unsigned int song_name_len = dmf_raw[i++];
		ret->song_name = (char *)malloc(sizeof(char) * song_name_len);
		if (!ret->song_name)
		{
			fprintf(stderr, "Malloc failed for song name\n");
			free(ret);
			return NULL;
		}
		strncpy(ret->song_name, (const char *)&dmf_raw[i], song_name_len);
		printf("Song name: \"%s\"\n", ret->song_name);
		i += song_name_len;

		unsigned int author_name_len = dmf_raw[i++];
		ret->author_name = (char *)malloc(sizeof(char) * author_name_len);
		if (!ret->author_name)
		{
			fprintf(stderr, "Malloc failed for author name\n");
			free(ret->song_name);
			free(ret);
			return NULL;
		}
		strncpy(ret->author_name, (const char *)&dmf_raw[i], author_name_len);
		printf("Author name: \"%s\"\n", ret->author_name);
		i += author_name_len;

		ret->highlight[0] = dmf_raw[i++];
		ret->highlight[1] = dmf_raw[i++];
		printf("Highlight A: %d\n", ret->highlight[0]);
		printf("Highlight B: %d\n", ret->highlight[1]);
	}
	printf("\n");
	// Module information
	{
		ret->time_base = dmf_raw[i++];
		printf("Time base: %d\n", ret->time_base);
		ret->tick[0] = dmf_raw[i++];
		ret->tick[1] = dmf_raw[i++];
		printf(" ( %d : %d )\n", ret->tick[0], ret->tick[1]);
		ret->frames_mode = dmf_raw[i++];
		printf("Frames mode: %d\n", ret->frames_mode);
		ret->custom_rate = dmf_raw[i++];
		printf("Custom rate: %d\n", ret->custom_rate);
		ret->custom_rate_val[0] = dmf_raw[i++];
		ret->custom_rate_val[1] = dmf_raw[i++];
		ret->custom_rate_val[2] = dmf_raw[i++];
		ret->total_rows_per_pattern = *(uint32_t *)(dmf_raw + i);
		i += 4;
		ret->total_rows_in_pattern_matrix = dmf_raw[i++];
		printf("Rows per pattern: %d\n", ret->total_rows_per_pattern);
	}
	// Pattern matrix
	{
	}

	free(dmf_raw);

	return ret;
}

void dmf_print(dmf_info_t *data)
{

}
