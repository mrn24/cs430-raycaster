#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef struct RGBpixel {
  unsigned char r, g, b;
} RGBpixel;

typedef struct Image{
  int width, height, format, range;
  RGBpixel *buffer;
}Image;

typedef struct {
  int type;//1 - sphere, 2 - plane.
  double* color;
  double radius;
  double* position;
  double* normal;
}Shape;
