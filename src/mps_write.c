#include "mps_write.h"

static int cell_is_empty(pattern_cell_t *c)
{
	return (c->note == NOTE_EMPTY &&
	        c->effect_type == EFFECT_EMPTY);
}

static size_t write_nops(FILE *f, uint8_t nops)
{
	if (nops == 0)
	{
		return 0;
	}
	printf("NOP %d\n", nops);
	mps_cell_t wcell;
	wcell.note = MPS_NOTE_NOP;
	wcell.octave = nops;
	return fwrite((void *)&wcell, 1, sizeof(mps_cell_t), f);
}

static size_t write_cell(FILE *f, pattern_cell_t *c)
{
	printf("CELL\n");
	mps_cell_t wcell;
	// Handle special note cases in case values change in the future
	if (c->note == NOTE_EMPTY)
	{
		wcell.note = MPS_NOTE_EMPTY;
	}
	else if (c->note == NOTE_OFF)
	{
		wcell.note = MPS_NOTE_OFF;
	}
	else
	{
		wcell.note = c->note;
	}

	wcell.octave = c->octave;

	if (c->volume == VOL_EMPTY)
	{
		wcell.volume = MPS_VOL_EMPTY;
	}
	else
	{
		wcell.volume = c->volume;
	}

	wcell.instrument = c->instrument;

	if (c->effect_type == EFFECT_EMPTY)
	{
		wcell.effect_type = MPS_EFFECT_EMPTY;
	}
	else
	{
		wcell.effect_type = c->effect_type;
	}

	wcell.effect_value = c->effect_value;

	return fwrite((void *)&wcell, 1, sizeof(mps_cell_t), f);
}

size_t mps_write_pattern(FILE *f, pattern_t *p)
{
	size_t written = 0;
	unsigned int i = 0;
	unsigned int nops = 0;

	// Empty patterns are not written at all
	if (!p->touched)
	{
		return 0;
	}

	while (i < p->length)
	{
		pattern_cell_t *cell = &(p->cells[i]);
		// Count NOPs between notes
		if (cell_is_empty(cell))
		{
			nops++;
		}
		// If there's a note hit, write out nops and the
		else
		{
			written += write_nops(f, nops);
			nops = 0;
			written += write_cell(f, cell);
		}
		i++;
	}

	if (nops > 0)
	{
		written += write_nops(f, nops);
	}

	return written;
}

size_t mps_write_instrument(FILE *f, instrument_t *i)
{
	size_t written = 0;
	mps_instrument_t inst;

	inst.type = i->type;
	inst.pad32 = 0x55;
	// PSG
	if (inst.type == 0)
	{
		for (unsigned int j = 0; j < i->std.env_len; j++)
		{
			uint8_t setval = i->std.envelope[j];
			if (j == i->std.loop_point)
			{
				setval |= ENV_PROP_LOOP;
			}
			if (j == i->std.env_len - 2 || j >= 29)
			{
				setval |= ENV_PROP_END;
			}
			inst.psg.envelope[j] = setval;
		}
	}
	// FM
	else
	{
		// Patches are stereo; panning is done via effects
		inst.fm.rb4_stereo_lfo = 0xC | (i->fm.ams << 4) | i->fm.fms;
		inst.fm.rb0_fb_alg = (i->fm.feedback << 3) | i->fm.algorithm;

		for (unsigned int j = 0; j < 4; j++)
		{
			patch_opdata_t *op = &(i->fm.opdata[j]);

			inst.fm.r30_dt_mul[j] = (op->dt << 4) | (op->mult);
			inst.fm.r40_tl[j] = op->tl;
			inst.fm.r50_rs_ar[j] = (op->rs << 6) | (op->ar);
			inst.fm.r60_am_d1r[j] = (op->am << 7) | (op->dr);
			inst.fm.r70_d2r[j] = (op->d2r);
			inst.fm.r80_d1l_rr[j] = (op->sl << 4) | (op->rr);
			inst.fm.r90_ssg_eg[j] = op->ssgmode;
		}
	}

	written += fwrite((void *)&inst, 1, sizeof(mps_instrument_t), f);

	return written;
}
