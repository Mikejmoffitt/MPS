#ifndef MPS_WRITE_H
#define MPS_WRITE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "mps_types.h"

#include "dmf.h"

// Writes the pattern_t to the FILE handle, returns bytes written.
size_t mps_write_pattern(FILE *f, pattern_t *p);

// Writes the instrument_t to the FILE handle, returns bytes written.
size_t mps_write_instrument(FILE *f, instrument_t *i);

#endif // MPS_WRITE_H
