#pragma once

unsigned int get_decompressed_size(void *compressHeader);
void decompress(void *inData, void *outData);
unsigned int compress(void *inData, unsigned int inSize, void *outData, bool bHasData);
bool compression_open();
void compression_close();
void enable_compression_extended_file_size();
