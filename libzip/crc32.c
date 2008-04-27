unsigned long crc32_table[256];

static unsigned long crc32_reflect(unsigned long ref, int ch) {
  unsigned long value = 0;
  int i;

  for (i = 1; i <= ch; i++) {
    if (ref & 1) {
      value |= (1 << (ch - i));
    }

    ref = ((ref >> 1) & 0x7fffffffL);
  }

  return value;
}

void crc32_init_table() {
  unsigned long polynomial = 0x04c11db7L;
  int i, j;

  for (i = 0; i < 256; i++) {
    crc32_table[i] = (crc32_reflect(i, 8) << 24);

    for (j = 0; j < 8; j++) {
      crc32_table[i] = ((crc32_table[i] << 1) ^ ((crc32_table[i] & (1 << 31)) ? polynomial : 0));
    }

    crc32_table[i] = crc32_reflect(crc32_table[i], 32);
  }
}
