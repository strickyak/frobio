// nytypes.h -- systems programming types for using cmoc.
//
// This is your toplevel switch for fundamental types and includes.
// For making a new environment for a library with its own ideas
// about these things, perhaps #define a new special macro first,
// and intercept it first in the following list of environments.
//
// currently supported:
//    * unix
//    * os9cmoc 

#ifndef _FROBIO_NYTYPES_H_
#define _FROBIO_NYTYPES_H_

#ifdef _CMOC_VERSION_
#ifdef OS9
#define os9cmoc 1
#endif
#endif

#if unix
// ********************************************************************
// ***** FOR MODERN LINUX gcc, DEBIAN OR UBUNTU -style.
#include <assert.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fundamental type definitions for nylib & frobio.
typedef unsigned char byte;  // byte is 1 byte, unsigned.
typedef unsigned int word;   // word is 2 bytes, unsigned.
typedef unsigned long quad;  // quad is 4 bytes, unsigned.

#ifndef  __cplusplus
typedef unsigned char bool;  // use a byte for a bool.
#endif
typedef unsigned char error; // use a byte for an error (0 == OKAY).
#define OKAY (error)0

#ifndef true
#ifndef __cplusplus
#define true 1
#define false 0
#endif
#endif

#elif os9cmoc
// ********************************************************************
// ***** FOR CMOC on 6809 on NitrOS-9, see http://perso.b2b2c.ca/~sarrazip/dev/cmoc-manual.html
#include <cmoc.h>
#include <assert.h>

// Fundamental type definitions for nylib & frobio.
typedef unsigned char byte;  // byte is 1 byte, unsigned.
typedef unsigned int word;   // word is 2 bytes, unsigned.
typedef unsigned long quad;  // quad is 4 bytes, unsigned.

typedef unsigned char bool;  // use a byte for a bool.
typedef unsigned char error; // use a byte for an error (0 == OKAY).

#define true 1
#define false 0
#define OKAY (error)0

#else
// ********************************************************************
#error "what platform? unix? os9cmoc?"
#endif

/*
 *  Notes
 *  See "Programming for CMOC" http://perso.b2b2c.ca/~sarrazip/dev/cmoc-manual.html#t30
 *      _CMOC_VERSION_
 *      OS9
 */

#endif // _FROBIO_NYTYPES_H_
