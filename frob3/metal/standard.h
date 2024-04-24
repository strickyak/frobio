////////////////////////////////////////////////////////
///
///  GCC Standard Library Routines.

void* memcpy(void* dest, const void* src, size_t n) {
  char* d = (char*)dest;
  char* s = (char*)src;
  for (size_t i = 0; i < n; i++) *d++ = *s++;
  return dest;
}

void* memset(void* s, int c, size_t n) {
  char* dest = (char*)s;
  for (size_t i = 0; i < n; i++) *dest++ = c;
  return s;
}

char* strcpy(char* restrict dest, const char* restrict src) {
  void* z = dest;
  while (*src) *dest++ = *src++;
  return z;
}

char* strncpy(char* restrict dest, const char* restrict src, size_t n) {
  void* z = dest;
  int i = 0;
  while (*src) {
    *dest++ = *src++;
    i++;
    if (i >= n) break;
  }
  return z;
}

size_t strlen(const char* s) {
  const char* p = s;
  while (*p) p++;
  return p - s;
}

size_t strnlen(const char* s, size_t max) {
  const char* p = s;
  while (*p && (p - s < max)) p++;
  return p - s;
}

////////////////////////////////////////////////////////
