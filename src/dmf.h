#ifndef DMF_H
#define DMF_H
#include <stdint.h>
#include <stdio.h>

#define FRAMES_MODE_PAL 0
#define FRAMES_MODE_NTSC 1

typedef enum system_t
{
	SYSTEM_GENESIS = 0x02,
	SYSTEM_GENESIS_EXT3 = 0x12,
	SYSTEM_SMS = 0x03,
	SYSTEM_GAMEBOY = 0x04,
	SYSTEM_PCENGINE = 0x05,
	SYSTEM_NES = 0x06,
	SYSTEM_C64_8580 = 0x07,
	SYSTEM_C64_6581 = 0x17,
	SYSTEM_YM2151 = 0x08
} system_t;

// Represents configuration of one operator's parameters.
typedef struct patch_opdata_t
{
	uint8_t am; // Amplitude modulation enable
	uint8_t ar; // Attack rate
	uint8_t dr; // Decay rate
	uint8_t mult; // Frequency multiplier
	uint8_t rr; // Release rate
	uint8_t sl; // Sustain level
	uint8_t tl; // Total level (coefficient)
	uint8_t dt2; // Detune 2 (YM2151 only, I think?)
	uint8_t rs; // Rate scaling
	uint8_t dt; // Detune
	uint8_t d2r; // Decay 2 rate
	uint8_t ssgmode; // Bit 4 = enable flag; bits 0-3 = mode
} patch_opdata_t;

// Represents an instrument patch.
typedef struct instrument_t
{
	char name[256];
	uint8_t algorithm;
	uint8_t feedback;
	uint8_t fms;
	uint8_t ams;
	patch_opdata_t opdata[4];
} instrument_t;

typedef struct dmf_info_t
{
	// Format flags
	uint8_t file_version;

	// System set
	system_t system_type; // This must be exported back out as one byte.

	// Visual information
	char *song_name;
	char *author_name;
	uint8_t highlight[2];

	// Module information
	uint8_t time_base;
	uint8_t tick[2];
	uint8_t frames_mode;
	uint8_t custom_rate;
	uint8_t custom_rate_val[3];

	// Pattern matrix values
	uint32_t total_rows_per_pattern;
	uint8_t total_rows_in_pattern_matrix;
	uint8_t *pattern_matrix;

	// Instrument data
	uint8_t total_instruments;
	instrument_t *instruments;

	// Wavetable data
	// TODO: Think about doing this. Not really interested, this is only
	// really supposed to be for YM2612 stuff

	// Patterns data
	// TODO: This

	// PCM sample data
	// TODO: This
	
} dmf_info_t;

// TODO: Integrate zlib compression and decompression here
dmf_info_t *dmf_create(const char *path);
void dmf_destroy(dmf_info_t *dmf);
void dmf_print(dmf_info_t *data);

#endif
