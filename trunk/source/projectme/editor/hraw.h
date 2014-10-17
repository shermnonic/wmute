#ifndef HRAW_H
#define HRAW_H

bool read_hraw( const char* filename, float*& data, unsigned* size );
void write_hraw( const char* filename, const float* data, unsigned sizeX, unsigned sizeY=1, unsigned sizeZ=1 );
void write_hraw( const char* filename, const float* data, const unsigned* size );

#endif // HRAW_H
