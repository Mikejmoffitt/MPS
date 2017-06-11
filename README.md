Mega Playback System
====================

Overview
--------
MPS is an alternative to VGM, XGM, and other similar large stream formats used
to play back music on a Sega Megadrive / Genesis system. MPS stores phrases and
chains them together to form an arrangement, like many tracker authoring tools.

Track Creation
--------------
Presently, MPS's only track creator is a decoder that converts from the DMF
format, used by the Deflemask tracker. The data emitted is relatively small.

MPS File Format
===============
An MPS file is formatted as follows:

Header
------
* 4 byte string "MPS" (includes null terminator)
* 1 byte describing total track length (in patterns)
* 1 byte describing length of a pattern (in rows)

Arrangement
-----------
Phrases are indicated in Big-Endian to a location relative to the beginning
of the MPS file. The arrangment table is channel-major.
* 10 x (track length) table of pointers to phrases.

Instruments
-----------
Instruments are more or less YM2612 register dumps, with frequency exempt.
* 32 bytes per instrument:
*   1 byte for type (0 is FM, 1 is PSG)
*   if FM:
*     4 bytes for R30
*     4 bytes for R40
*     4 bytes for R50
*     4 bytes for R60
*     4 bytes for R70
*     4 bytes for R80
*     4 bytes for R90
*     1 byte for RB0
*     1 byte for RB4
*   else if PSG:
*     30 bytes for envelope (flags in upper nybble, value in lower nybble)
*   1 byte of padding (value 0x55)

Patterns
--------
Patterns are compressed in a way similar to run-length encoding. They are
composed of "cells", which contain:
* 1 byte for the Note
* 1 byte for the Octave
* 1 byte for the Volume
* 1 byte for the Instrument
* 1 byte for the Effect Type
* 1 byte for the Effect Value

If the Note has the invalid value 0xEA, it is counted as a no-op. This is
treated as a pass, and exists to delay playback and eliminate gaps of no
activity between notes. The second byte, usually used for Octave, indicates
how long to wait.

TODO
----
 * PCM sample playback
 * Effect in playback
 * Annotate instrument format
