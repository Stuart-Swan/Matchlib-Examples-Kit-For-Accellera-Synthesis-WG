// INSERT_EULA_COPYRIGHT: 2020

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

class bmpheader
{
  char signature[2];
  unsigned filesize;
  unsigned reserved;
  unsigned dataoffset;

public:

  void read(FILE *fp);
  bmpheader();
  void write(unsigned int realfilesize, FILE *fp);
};

class bmpinfoheader
{
  unsigned size;

  unsigned short planes;
  unsigned xpixelsperm;
  unsigned ypixelsperm;
  unsigned colorsused;
  unsigned colorsimportant;

public:

  unsigned width;
  unsigned height;
  unsigned imagesize;
  unsigned short bitcount;
  unsigned compression;

  void write(FILE *fp);
  void read(FILE *fp);

  bmpinfoheader();
};

class bmpcolortable
{
  int numcolors;
  unsigned char array[1024];

public:

  void dumparray();

  void read(int bitsperpixel, FILE *fp);
  unsigned char *bgr(unsigned int index);
  bmpcolortable();
  ~bmpcolortable();
};

// LIMITED tO no compression, 1-bit, 4-bit, 24-bit support only
class bmp
{
private:

  const char *filename;
  bmpheader header;
  bmpinfoheader infoheader;
  bmpcolortable colortable;
  unsigned char *array;
  unsigned short bytesleft;
  unsigned int fourbytebuffer;
  unsigned short bitsleft;
  unsigned char pixelbuffer;

  //
  // Always reads in 4-byte blocks so handles end-line padding
  //
  unsigned char getbyte(FILE *fp);

  //
  // returns 4-byte value representing a pixel
  // without any colortable expansion
  //
  unsigned int getpixel(FILE *fp);

  //
  // writes just the array data in 24-bit bmp format
  //
  void writearray(FILE *ofp);

  //
  // reads array data with colortable expansion if necessary
  //
  void readarray(bmpcolortable *colortable, FILE *fp);

public:

  inline unsigned rows() { return infoheader.height; }
  inline unsigned columns() { return infoheader.width; }

  // returns pixel colors with 0-index in top left (rather than bottom left as in bmp file)
  inline char blue(unsigned row, unsigned column) { return array[3 * ((rows() - row - 1) * columns() + column)]; }
  inline char green(unsigned row, unsigned column) { return array[3 * ((rows() - row - 1) * columns() + column) + 1]; }
  inline char red(unsigned row, unsigned column) { return array[3 * ((rows() - row - 1) * columns() + column) + 2]; }

  // reads in a bit-map file on construction
  bmp(const char *f);

  // debug the array data
  void dumparray();

  // write out a bmp file called f (in 24-bit irrespective of the resolution read in)
  void write(char *f);

  ~bmp();
};

