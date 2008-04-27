#ifndef __CRC32_H__
#define __CRC32_H__

#define CRC32(c, b) (crc32_table[((int) (c) ^ (b)) & 0xff] ^ ((c) >> 8))

extern long crc32_table[256];
void crc32_init_table();

#endif
