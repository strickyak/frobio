// nytypes.h -- systems programming types for using cmoc.

#ifndef _FROBIO_NYTYPES_H_
#define _FROBIO_NYTYPES_H_

// Fundamental type definitions for using cmoc.
typedef unsigned char byte;  // byte is 1 byte, unsigned.
typedef unsigned int word;   // word is 2 bytes, unsigned.
typedef unsigned long quad;  // quad is 4 bytes, unsigned.

typedef unsigned char bool;  // use a byte for a bool.
typedef unsigned char error; // use a byte for an error (0 == OKAY).

#define true 1
#define false 0
#define OKAY (error)0

#endif // _FROBIO_NYTYPES_H_
