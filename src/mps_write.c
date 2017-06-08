#include "mps_write.h"

size_t mps_write_pattern(FILE *f, pattern_t *p)
{
	size_t written = 0;

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
