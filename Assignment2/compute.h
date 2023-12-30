// compute.h


#ifndef COMPUTE_H
#define COMPUTE_H

#include "headers.h"
#include "texture.h"

#include <complex>
#include <fftw3.h>


typedef enum { TRANSLATE, ROTATE, SCALE, INTENSITY } EditMode;
typedef enum { FORWARD, BACKWARD } ProjectionMode;



// A 2D array of complex numbers

class ComplexArray2D {

 public:

  complex<double> *a;  // stored in row-major order

  int dimX, dimY;

  ComplexArray2D() {
    a = NULL;
  }

  ComplexArray2D( int _dimX, int _dimY ) {

    dimX = _dimX;
    dimY = _dimY;

    a = new complex<double>[ dimX * dimY ];
  }

  // Use the 'r' channel from a texture to initialize the array
  
  ComplexArray2D( Texture *t ) {

    dimX = t->width;
    dimY = t->height;

    a = new complex<double>[ dimX * dimY ];

    for (int x=0; x<dimX; x++)
      for (int y=0; y<dimY; y++)
	(*this)(x,y) = complex<double>( t->pixel(x,y).r, 0 );
  }

  ~ComplexArray2D() {
    if (a != NULL)
      delete [] a;
  }

  // Reference elements as a(x,y)
  
  complex<double> & operator()( int x, int y ) {
    return a[ x + dimX * y];
  }

  // Scale array elements by a factor

  void scale( double factor ) {

    complex<double>* p = a;

    for (int i=0; i<dimX*dimY; i++)
      *p++ *= factor;
  }
};



// Represent the positions of peaks in the Fourier spectrum in polar
// form as (angle, dist).

class PolarPeak {

public:
  
  float angle;
  float dist;

  PolarPeak() {}

  PolarPeak( float _angle, float _dist ) {
    angle = _angle;
    dist = _dist;
  }
};



// Represent (x,y) positions in an array

class ArrayPos {

public:

  int x,y;

  ArrayPos() {}

  ArrayPos( int _x, int _y ) {
    x = _x;
    y = _y;
  }
};



class Compute {

 public:

  int dimX, dimY;

  ComplexArray2D *image;      // original image
  ComplexArray2D *imageFT;    // FT of original image
  ComplexArray2D *grid;       // grid image
  ComplexArray2D *gridFT;     // FT of grid image
  ComplexArray2D *result;     // original with grid subtracted

  float gridLineMagnitudeThreshold = 16;  // min magnitude in grid image for a pixel to be on a grid line

  bool interpolateAroundGridLines = true; // when removing grid lines, fill them with surrounding pixels

  float thresholdPercentage = 0.40;       // percentage of max magnitude above which peaks are detected

  Compute( Texture *t ) {

    image = new ComplexArray2D( t ); // input image

    dimX = t->width;
    dimY = t->height;

    imageFT = new ComplexArray2D( dimX, dimY );
    grid    = new ComplexArray2D( dimX, dimY );
    gridFT  = new ComplexArray2D( dimX, dimY );
    result  = new ComplexArray2D( dimX, dimY );
  }

  void computeSolution();

  void forwardFT( ComplexArray2D *src, ComplexArray2D *dest );
  void inverseFT( ComplexArray2D *src, ComplexArray2D *dest );

  float averageOfNeighbours( int x, int y, ComplexArray2D *image, ComplexArray2D *grid, PolarPeak (&lines)[2] );
};

#endif
