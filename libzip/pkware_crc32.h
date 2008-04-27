/*
 pkware_crc32.c -- create pkware crc32 table
 Copyright (c) 2008 SUGAWARA Genki <sgwr_dts@yahoo.co.jp>
*/
#ifndef __CRC32_H__
#define __CRC32_H__

void pkware_crc32_init_table();
unsigned long pkware_crc32(unsigned long crc, int c);

#endif
