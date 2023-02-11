// Let's pick some defines for platforms and start being consistent.
//
// For Operating Systems or Metal Platforms.
//   FOR_LINUX  -- run (as much as possible) on modern Linux platforms.
//   FOR_LEVEL2 -- OS9/NitrOS9, probably Level II.  (No level I project yet.)
//   FOR_DECB   -- Assumes DECB basic ROMs.
//   FOR_COCO   -- Assumes coco keyboard, PIAs, probably 32x16 screen at $0400.
//
// By Compilers
//   BY_CMOC  -- cmoc 0.1.81 or later
//   BY_GCC   -- gcc-4.6.* for m6809 as built here: https://raw.githubusercontent.com/beretta42/fip/master/docs/build_fuzix.txt
//                 or modern gcc for Linux
//
// With Devices -- NOT USED YET --
//   WITH_WIZNET  -- like the CocoIO ethernet board, probably at $FF68
//   WITH_COCO_KBD  -- Coco 1/2/3 keyboard and PIAs
//   WITH_VDG_0400  -- VDG text screen at $0400.

#ifndef _FROB2_FROBTYPE_H_
#define _FROB2_FROBTYPE_H_

/////////////////////////////////////


/////////////////////////////////////

// All compilers define stdarg for va_list.
#include <stdarg.h>

#ifndef __cplusplus
#define true 1
#define false 0
typedef unsigned char bool;  // use a byte for a bool.
#endif

typedef char* mstring;  // a Malloc'ed string
typedef unsigned char errnum;     // for OS error numbers.

// `prob`: High-Level Error Strings.
//   Use literal const char* for errors.
//   Use NotYet to mean try again later, because an asynchronous event hasn't happened yet.
//   Use GOOD (i.e. NULL) for good status.
// To communicate better error messages, application can LogError, or LogFatal.
typedef const char* prob;
#define GOOD ((const char*)NULL)
extern const char NotYet[]; // defined as "NotYet"


#ifdef unix

typedef unsigned char small;  // byte is 1 byte, unsigned.
typedef unsigned char byte;  // byte is 1 byte, unsigned.
typedef unsigned long word;  // word is 8 bytes, unsigned.
typedef unsigned long quad;  // quad is 8 bytes, unsigned, 4 bytes used.

#else // unix

// Iteration variables over small ranges (under 255 loops) can use small.
// gcc6809 can run out of byte-sized registers if it uses bytes for
// iterations variables, so we offer small which is 2 bytes on gcc6809.
#ifdef __GNUC__
typedef unsigned int small;  // gcc: small is 2 bytes, unsigned.
#else
typedef unsigned char small;  // CMOC: small is 1 byte, unsigned.
#endif

typedef unsigned char byte;  // byte is 1 byte, unsigned.
typedef unsigned int word;   // word is 2 bytes, unsigned.
typedef unsigned long quad;  // quad is 4 bytes, unsigned.
typedef unsigned int size_t;
#define NULL ((void*)0)

#endif // unix

#endif // _FROB2_FROBTYPE_H_
