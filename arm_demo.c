/* Freestanding: Fedora's cross-compiler ships without a libc, so no #include. */
int fibonacci(int n) {
  if (n <= 0) return 0;
  if (n == 1) return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}

unsigned checksum(const unsigned char *buf, unsigned len) {
  unsigned sum = 0;
  for (unsigned i = 0; i < len; i++)
    sum = (sum << 1) ^ buf[i];
  return sum;
}
