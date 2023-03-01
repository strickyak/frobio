void abort() {
  // Assume text screen is still at 0x0400,
  // and splash the middle of it with Xs.
  for (int p=0x04C0; p < 0x0540; p++) {
    *(char*)p = 'X';
  }
  while (1) {}
}
