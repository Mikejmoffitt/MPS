#ifndef MPS_TYPES_H
#define MPS_TYPES_H

#ifndef BYTE
#define BYTE unsigned char
#endif

/*
 == FM INSTRUMENTS ==
FM type instruments (type 0) are stored as register dumps. They are referred to
by the base register address they will be applied to; channel 1's dt_mul data
is written to Part 1 0x30, channel 2 is Part 1 0x31, Channel 6 is Part 2 0x32,
so on and so forth.

Frequency data is a little different, split across A0 and A4. This will be
calculated based on note value and is not part of a register write.

== PSG INSTRUMENTS ==
PSG instruments are stored as an attenuation envelope (inverse of volume) to
match the SN76489 registers. The envelope is max length 30.

Properties of the envelope, like marking the end and loop point are done in
the upper nybble, while the attenuation value is in the lower nybble. Masks
are provided below.

*/

// Attenuation is in the lower nybble; AND with ENV_ATT_MASK to get it.
#define ENV_ATT_MASK 0x0F
// To retrieve flags AND with this mask.
#define ENV_PROP_MASK 0xF0
// Loop point is denoted as ENV_PROP_LOOP in bit 6
#define ENV_PROP_LOOP 0x40
// End point is denoted as ENV_PROP_END in bit 7
#define ENV_PROP_END 0x80

#define MPS_NOTE_EMPTY 0
#define MPS_NOTE_OFF 100
#define MPS_NOTE_NOP 0xEA
#define MPS_EFFECT_EMPTY 0xFF
#define MPS_VOL_EMPTY 0xFF

// A single cell encountered in a stream.
typedef struct mps_cell_t
{
	BYTE note;
	BYTE octave;
	BYTE volume;
	BYTE instrument;
	BYTE effect_type;
	BYTE effect_value;
} mps_cell_t;

typedef struct mps_instrument_t
{
	BYTE type; // 0 is FM, 1 is PSG
	union
	{
		struct
		{
			BYTE r30_dt_mul[4]; // dt(6:4), mul(3:0)
			BYTE r40_tl[4]; // tl(6:0)
			BYTE r50_rs_ar[4]; // rs(7:6), ar(4:0)
			BYTE r60_am_d1r[4]; // am(7), d1r(4:0)
			BYTE r70_d2r[4]; // d2r(4:0)
			BYTE r80_d1l_rr[4]; // d1l(7:4), rr(3:0)
			BYTE r90_ssg_eg[4]; // SSG-EG(3:0)
			BYTE rb0_fb_alg; // feedback(5:3), algorithm(2:0)
			BYTE rb4_stereo_lfo; // L(7), R(6), ams(5:4), fms(2:0)
			// Frequency stuff is something else
		} fm;
		struct
		{
			BYTE envelope[30];
		} psg;
	};
	BYTE pad32;
} mps_instrument_t;


#endif // MPS_TYPES_H
