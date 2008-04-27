/*
 pkware_crc32.c -- make pkware crc32 table
 Copyright (c) 2008 SUGAWARA Genki <sgwr_dts@yahoo.co.jp>
 see http://php.benscom.com/manual/ja/function.crc32.php#31832
*/
static unsigned long pkware_crc32_table[256];

static unsigned long pkware_crc32_reflect(unsigned long ref, int ch) {
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

void pkware_crc32_init_table() {
  unsigned long polynomial = 0x04c11db7L;
  int i, j;

  for (i = 0; i < 256; i++) {
    pkware_crc32_table[i] = (pkware_crc32_reflect(i, 8) << 24);

    for (j = 0; j < 8; j++) {
      pkware_crc32_table[i] = ((pkware_crc32_table[i] << 1) ^ ((pkware_crc32_table[i] & (1 << 31)) ? polynomial : 0));
    }

    pkware_crc32_table[i] = pkware_crc32_reflect(pkware_crc32_table[i], 32);
  }
}

unsigned long pkware_crc32(unsigned long crc, int c) {
  int i;

  i = (int) ((crc ^ c) & 0xff);
  return pkware_crc32_table[i] ^ (crc >> 8);
}
