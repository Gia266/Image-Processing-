// hough.h


#ifndef HOUGH_H
#define HOUGH_H

#include "headers.h"
#include "texture.h"

#include <vector>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))


// An (x,y) coordinate pair

class Coords {

 public:

  float x, y;

  Coords() {}

  Coords( float _x, float _y ) {
    x = _x;
    y = _y;
  }
};


// Output operator for Coords

std::ostream& operator << ( std::ostream& stream, Coords const& p );


// Hough transform code

class Hough {

 public:

  Texture *image;             // image
  int    **counts;            // Hough counts
  int centreX, centreY;       // image centre coordinates
  int countsDimX, countsDimY; // dimensions of 'counts' array'

  vector<Coords> imagePoints;        // Points to highlight in the image as (x,y)
  vector<Coords> accumulatorPoints;  // Points to highlight in the accumulator array as (x,y)

  vector<Coords> imageLines;            // Lines to highlight in the image, as (rho,theta)
  vector<Coords> accumulatorSinusoids;  // Sinusoids to highlight in the accumulator array as (rho,theta)

  // Resolution of Hough Transform accumulation buffer

  const float thetaResolution = 0.5/180.0*M_PI;	// 0.5 degree resolution in angle
  const float rhoResolution = 1; // 1 pixel resolution in distance from origin


  Hough( Texture *t ) {

    // record input image and its centre coordinates
    
    image = t;

    centreX = t->width/2;
    centreY = t->height/2;

    // Set up 'counts' array
    //
    // We'll use an origin in the CENTRE of the image, so the max
    // distance to any line will be half the corner-to-corner distance
    // (i.e. 'countsDimY' below).
    
    countsDimX = (int) rint( M_PI / thetaResolution );
    countsDimY = (int) rint( sqrt( t->width*t->width + t->height*t->height ) / rhoResolution );
    
    counts = new int*[countsDimX];
    for (int i=0; i<countsDimX; i++)
      counts[i] = new int[countsDimY];
  }

  ~Hough() {

    // Delete 'counts' array
  
    for (int i=0; i<countsDimX; i++)
      delete [] counts[i];
    delete [] counts;
  }

  void computeSolution( bool smoothCounts, int numPeaks, bool findMarker );
  void smoothCounts();
};

#endif
