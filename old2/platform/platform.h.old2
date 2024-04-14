#ifndef _FROB2_PLATFORM_PLATFORM_H_
#define _FROB2_PLATFORM_PLATFORM_H_

// Indicate abnormal or normal exit.
void PlatformAbort(void);
void PlatformExit(int status);

// Write a char or a string to the screen.
// This is more like stderr than like stdout.
void PlatformPutChar(int c);
void PlatformPrint(const char* s);

// Read one line from the keyboard.
// Assume it returns a static buffer, so copy it.
// This is more like /dev/tty than like stdin.
char* PlatformReadLine();

#ifndef // _FROB2_PLATFORM_PLATFORM_H_
