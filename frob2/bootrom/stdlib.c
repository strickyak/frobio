#if 0
void abort() {
  // Assume text screen is still at 0x0400,
  // and splash the middle of it with Xs.
  for (int p=0x04C0; p < 0x0540; p++) {
    *(char*)p = 'X' & 63;
  }
  while (1) {}
}
#endif

void memcpy(char* a, const char* b, int c) {
  for (int i = 0; i < c; i++) *a++ = *b++;
}

void memset(char* a, int b, int c) {
  for (int i = 0; i < c; i++) *a++ = b;
}
void strcpy(char* a, const char*b) {
  while (*b) *a++ = *b++;
}
void strncpy(char* a, const char*b, int n) {
  int i = 0;
  while (*b) {
    *a++ = *b++;
    i++;
    if (i>=n) break;
  }
}
int strlen(const char* s) {
  const char* p = s;
  while (*p) p++;
  return p-s;
}
