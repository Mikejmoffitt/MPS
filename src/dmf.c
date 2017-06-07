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

static const char *note_names[] =
{
	"--",
	"C#",
	"D-",
	"D#",
	"E-",
	"F-",
	"F#",
	"G-",
	"G#",
	"A-",
	"A#",
	"B-",
	"C-"
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

// Big megafunction that returns the decoded music data
dmf_info_t *dmf_create(const char *path)
{
	uint8_t *dmf_raw;
	dmf_info_t *ret = NULL;
	unsigned int i = 0;
	unsigned int channel_count = 0;
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

	if (ret->system_type > sizeof(system_total_channels) / sizeof(unsigned int))
	{
		ret->system_type = SYSTEM_GENESIS_EXT3;
	}

	if (ret->system_type != SYSTEM_GENESIS)
	{
		printf("Non-Genesis DMF files are unsupported.\n");
		free(ret);
		return NULL;
	}

	// Visual information
	{
		unsigned int song_name_len = dmf_raw[i++];
		printf("Name len: %d\n", song_name_len);
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
		printf("Name len: %d\n", author_name_len);
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
		printf("Rows in matrix: %d\n", ret->total_rows_in_pattern_matrix);
	}
	// Pattern matrix
	{
		channel_count = system_total_channels[ret->system_type];
		ret->pattern_matrix = (uint8_t *)malloc(sizeof(char) * channel_count * ret->total_rows_in_pattern_matrix);
		if (!ret->pattern_matrix)
		{
			fprintf(stderr, "Malloc failed for pattern matrix.\n");
			free(dmf_raw);
			return NULL;
		}

		for (unsigned int x = 0; x < channel_count; x++)
		{
			for (unsigned int y = 0; y < ret->total_rows_in_pattern_matrix; y++)
			{
				uint8_t entry = dmf_raw[i++];
				ret->pattern_matrix[y*channel_count + x] = entry;
			}
		}

		for (unsigned int y = 0; y < ret->total_rows_in_pattern_matrix; y++)
		{
			for (unsigned int x = 0; x < channel_count; x++)
			{
				printf("%X ", ret->pattern_matrix[y*channel_count + x]);
			}
			printf("\n");
		}
	}
	// Instruments
	{
		uint8_t num_instruments = dmf_raw[i++];
		printf("Number of instruments: %d\n", num_instruments);
		ret->instruments = (instrument_t *)malloc(sizeof(instrument_t) * num_instruments);
		for (unsigned int k = 0; k < num_instruments; k++)
		{
			uint8_t namelen = dmf_raw[i++];
			for (unsigned int j = 0; j < namelen; j++)
			{
				char fetch = dmf_raw[i++];
				if ( j < 15)
				{
					ret->instruments[k].name[j] = fetch;
				}
			}
			ret->instruments[k].name[15] = '\0';
			printf("\"%s\"", ret->instruments[k].name);
			uint8_t ins_type;
			ins_type = dmf_raw[i++];
			printf(" #%d: Type %d\n", k, ins_type);
			// FM Type
			if (ins_type == 1)
			{
				ret->instruments[k].fm.algorithm = dmf_raw[i++];
				ret->instruments[k].fm.feedback = dmf_raw[i++];
				ret->instruments[k].fm.fms = dmf_raw[i++];
				ret->instruments[k].fm.ams = dmf_raw[i++];
				for (unsigned int j = 0; j < 4; j++)
				{
					ret->instruments[k].fm.opdata[j].am = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].ar = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].dr = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].mult = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].rr = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].sl = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].tl = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].dt2 = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].rs = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].dt = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].d2r = dmf_raw[i++];
					ret->instruments[k].fm.opdata[j].ssgmode = dmf_raw[i++];
				}
			}
			else

			{
				// Grab volume macro
				uint8_t env_len = dmf_raw[i++];
				for (unsigned int j = 0; j < env_len; j++)
				{
					uint32_t env_value = *(uint32_t *)(dmf_raw + i);
					i += 4;
					if (j < ENV_LEN_MAX)
					{
						ret->instruments[k].std.envelope[j] = env_value;
					}
				}
				ret->instruments[k].std.loop_point = dmf_raw[i++];
				// Skip arpeggio and noise macro
				env_len = dmf_raw[i++];
				i += env_len + 1;
				env_len = dmf_raw[i++];
				i += env_len + 1;
			}
		}
	}
	// Wavetables
	{
		uint8_t num_wavetables = dmf_raw[i++];
		for (uint8_t j = 0; j < num_wavetables; j++)
		{
			uint32_t len = *(uint32_t *)(dmf_raw + i);
			i += 4;
			i += 4 * len;
		}
	}
	// Patterns: the good shit
	{
		unsigned int chan_cnt = system_total_channels[ret->system_type];
		// Initialize pattern table
		for (unsigned int y = 0; y < chan_cnt; y++)
		{
			for (unsigned int x = 0; x < MAX_PTRN; x++)
			{
				ret->patterns[y][x].touched = 0;
			}
		}

		// Pull pattern data
		for (unsigned int x = 0; x < chan_cnt; x++)
		{
			uint8_t num_fx = dmf_raw[i++];
			for (unsigned int y = 0; y < ret->total_rows_in_pattern_matrix; y++)
			{
				unsigned int pattern_number = ret->pattern_matrix[y*chan_cnt + x];
				pattern_t *pattern = &(ret->patterns[x][pattern_number]);
				printf("Channel %d PATTERN #%d\n", x, pattern_number);

				if (pattern_number >= MAX_PTRN)
				{
					fprintf(stderr, "Error: Pattern number exceeds max %d.\n", MAX_PTRN);
					break;
				}
			
				if (pattern->touched)
				{
					printf("(already captured)\n");
				}

				pattern->touched = 1;
				// Capture pattern data
				for (unsigned int z = 0; z < ret->total_rows_per_pattern; z++)
				{
					int16_t note, octave, vol, instrument;
					int16_t fx_code, fx_val;
					note = *(uint16_t *)(dmf_raw + i);
					i += 2;
					octave = *(uint16_t *)(dmf_raw + i);
					i += 2;
					vol = *(uint16_t *)(dmf_raw + i);
					i += 2;
					fx_code = -1;
					fx_val = -1;

					for (unsigned int e = 0; e < num_fx; e++)
					{
						if (e == 0)
						{
							fx_code = *(uint16_t *)(dmf_raw + i);
							i += 2;
							fx_val = *(uint16_t *)(dmf_raw + i);
							i += 2;
						}
						// Skip additional effects, we aren't capturing them
						else
						{
							i += 4;
						}
					}
					instrument = *(uint16_t *)(dmf_raw + i);
					i += 2;

					// Write to pattern row
					pattern_cell_t *cell = &(pattern->cells[z]);

					// DEBUG PRINT
					// note & octave
					cell->octave = octave;
					if (note == NOTE_OFF)
					{
						printf(" === ");
						cell->note = NOTE_OFF;
					}
					else if (note >= 1 && note <= 12)
					{
						printf(" %s%01X ", note_names[note], octave);
						cell->note = note;
					}
					else
					{
						printf(" --- ");
						cell->note = NOTE_EMPTY;
					}
					// volume
					if (vol >= 0 && vol <= 0x7F)
					{
						printf("%02X ", vol);
						cell->volume = vol;
					}
					else
					{
						printf("-- ");
						cell->volume = VOL_EMPTY;
					}
					// instrument
					cell->instrument = instrument;
					if (instrument >= 0)
					{
						printf("%02X ", instrument);
					}
					else
					{
						printf("-- ");
					}
					// effect
					if (fx_code >= 0 && fx_val >= 0)
					{
						printf("%02X%02X\n", fx_code, fx_val);
						cell->effect_type = fx_code;
						cell->effect_value = fx_val;
					}
					else
					{
						printf("----\n");
						cell->effect_type = EFFECT_EMPTY;
						cell->effect_value = EFFECT_EMPTY;
					}
				}
			}
		}
	}


	free(dmf_raw);

	return ret;
}

void dmf_print(dmf_info_t *data)
{

}
