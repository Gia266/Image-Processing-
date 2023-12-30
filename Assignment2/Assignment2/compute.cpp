// compute.cpp


#include "compute.h"

#include <vector>
#include <algorithm>

//run with:   ./ecg ../images/small.png

/*
F Compute forward FTs
I Compute inverse FTs
C Compute the solution

G Toggle grid line filling
M toggle phase/magnitude display of FT image
*/

// Comparison function used to sort peaks in Fourier spectrum by
// increasing angle or increasing distance.

bool increasingAngle( PolarPeak &a, PolarPeak &b ) {
  return a.angle < b.angle;
}

bool increasingDistance( PolarPeak &a, PolarPeak &b ) {
  return a.dist < b.dist;
}

//sorting function for sorting distances between peaks
int distSort(const void *a, const void *b) {
    float difference = (*(float*)a - *(float*)b);

    if (difference > 0) {
        return 1;
    } else if (difference < 0) {
        return -1;
    } else {
        return 0;
    }
}


// Remove the grid from the global 'image'.
//
// Determine the direction and spacing of the grid lines.
//
// The direction is the angle, in degrees, of the normal to the grid line
// counterclockwise from the horizontal.
//
// Debug using the 'small.png' file.  Once debugged, test your code on
// all files in the 'images' directory.



void Compute::computeSolution()

{
  // 1. Compute the FT of the image.  Store it in 'imageFT'.
  // 
  // [0 marks]

  forwardFT (image, imageFT); //Fourier Transform (FT) src --> dst

  // 2. Find the maximum magnitude, excluding the DC component in [0,0].
  //
  // [1 mark]

  float maxMag = 0; 
  for (int i = 1; i < dimX; i++){ //for each pixel in imageFT (except 0,0)
    for (int j = 1; j < dimY; j++){

      float currMag = abs((*imageFT)(i, j)); //find magnitude of pixel

      if (currMag > maxMag) //if pixel is new maximum magnitude, update it
          maxMag = currMag;
    }
  }

  // 3. Set to zero the components of 'imageFT' that have magnitude
  //    less than 40% the maximum magnitude.  Store this new FT in
  //    'gridFT'.  Record in a list the (x,y) locations of the
  //    non-zero magnitudes of 'gridFT'.  These are the "peaks".
  //
  // [1 mark]

  float threshold = thresholdPercentage * maxMag;

  vector<ArrayPos> peakPositions; // (x,y) positions of peaks

  for (int i = 1; i < dimX; i++){ //for each pixel in imageFT (except 0,0)
    for (int j = 1; j < dimY; j++){

        float currMag = abs((*imageFT)(i, j)); //find magnitude of pixel

        if (currMag >= threshold){    //if above threshold, copy pixel to gridFT & record location
          (*gridFT)(i,j) = (*imageFT)(i,j);
          peakPositions.push_back({i,j}); //record array position
        }
        else    
          (*gridFT)(i,j) = 0;   //else clear pixel to 0 on gridFT

    } 
  }   //end of nested i j loop

  (*gridFT)(0,0) = (*imageFT)(0,0); // just in case the DC component is too small

  // 4. From the peaks, find the angles in the FT of the two principal
  //    grid line directions and, for each such line direction, find
  //    the spacing in the FT between the peaks corresponding to that
  //    line direction.
  //
  //    THIS IS DIFFICULT and can be left until you have zeroed the grid
  //    line pixels as described in Step 6.
  //
  //    The end result of this code is to fill the 'lines' array with
  //    two (angle,distance) values corresponding to the two grid line
  //    directions.
  
  PolarPeak lines[2] = { PolarPeak( 0,0 ), PolarPeak( 0,0 ) };

  // 4a. Gather in 'peaks' the angle and distance of each peak in
  //     'peakPositions'.  Take into account the quadrant that the
  //     peak comes from: Correctly convert array locations to the
  //     corresponding locations in the frequency domain.  Don't
  //     include peaks that are closer than 'minPeakDist' to the
  //     origin.
  //
  //     [2 marks]
  
  vector<PolarPeak> peaks;

  const float minPeakDist = 20;
      
  for (int i = 0; i < peakPositions.size(); i++){//for every peak position

      float _x = peakPositions[i].x; //get x and y coords for each peak
      float _y = peakPositions[i].y;

      if (_x >= dimX/2) //offset quadrants for X
          _x = _x - dimX;
      if (_y >= dimY/2) //offset quadrants for Y
          _y = _y - dimX;

      float distance = sqrt(pow(_x, 2)+pow(_y, 2)); //calc distance and angle

      float angle = atan2(_y, _x); //angles in radian

      // Ensure angle is in the range [0, 2PI)
      if (angle < 0) {
         angle += 2 * M_PI; //else wrap around to fix
      }

      if (distance > minPeakDist){ //if greater than min req
        peaks.push_back({angle, distance});
      }
  }

  
  if (peaks.size() < 2) {
    cerr << "Not enough peaks detected" << endl;
    return;
   }
    
  // 4b. Split the peaks found in 4a into two groups such that the
  //     peaks in each group are approximately collinear.  These
  //     correspond to the harmonics of the grid lines.  Include in
  //     the same group peaks that are 180 degrees apart.
  //
  //     [2 marks]

  vector<PolarPeak> collinearPeaks[2];  // [0] and [1] store the two groups of peaks
  
  // YOUR CODE HERE
  float tolerance = 0.35; //20 degrees of tolerance

  float group0Angle = peaks[0].angle; //inital angle to compare points to

  if (group0Angle > M_PI) //wrap angles around so they can be grouped easier
      group0Angle = group0Angle - M_PI;
  collinearPeaks[0].push_back(peaks[0]); //place into group 0
   
  //the first peak will go into group 0. arbitary
  //Any peaks close enough (tolerance) to the first peak are group 0
  //else they are group 1.
  //this may not be the best solution, but will be good enough for only 2 sets of grid lines

  float highLim = group0Angle + tolerance; 
  float lowLim = group0Angle - tolerance;

  //doesn't matter to clamp,

  for (int i = 1; i < peaks.size(); i++){ //for every peak (other than the first one again)
    float angle = peaks[i].angle;

      if (angle > M_PI)             //wraps peaks over 180 deg around to line up and group easier
          angle = angle - M_PI;

    if (lowLim < angle && angle < highLim) //if within the tolerance to group 0
      collinearPeaks[0].push_back(peaks[i]); //filled with adjusted angles RAD

    else //if not in tolerance -> goes into group 1
      collinearPeaks[1].push_back(peaks[i]);
  }

  // 4c. Find the median angle in each of the two groups of
  //     collinearPeaks.  Store in 'peakAngles'.  This is the best
  //     estimate of the angle of the grid line direction.
  //
  //     [1 mark]

  float peakAngles[2];

  // YOUR CODE HERE

    //sort
    for (int i = 0; i < 2; i++){
      //sort peaks by angle
      sort(collinearPeaks[i].begin(), collinearPeaks[i].end(), increasingAngle);

    //find median
    int middle = collinearPeaks[i].size() / 2; //rounded or truncated value, close enough
    peakAngles[i] = collinearPeaks[i][middle+1].angle; 
    }
  // 4d. Find the distance between peaks in each group of collinearPeaks.
  //
  //     Here we sort all the distances (which are fxrom the origin),
  //     then gather the distances *between* adjacent pairs of sorted
  //     distances.  Many of these inter-pair distances will be small
  //     and due to multiple peaks in one place in the FT, so we
  //     discard any inter-pair distances <= 2.  The median of the
  //     remaining inter-pair distances gives the best estimate of the
  //     inter-peak distance in the Fourier domain.  Results for the
  //     two groups of peaks go into 'interPeakDistances'.
  //
  //     [2 marks]

  float interPeakDistances[2];

  // YOUR CODE HERE

    //sort
    for (int i = 0; i < 2; i++){
      //sort distances
      sort(collinearPeaks[i].begin(), collinearPeaks[i].end(), increasingDistance);
      //we now have a sorted array of all distances

      float disArr[collinearPeaks[i].size()]; //make a new array to hold distances between
      int  disArrSize = 0; //size
      int index = 0;
      for (int j = 1; j < collinearPeaks[i].size(); j++){//for all distances in the group
          //get distance between adjacent peaks
        float disBetween = collinearPeaks[i][j].dist - collinearPeaks[i][j-1].dist;

      if (disBetween > 2){ //if distance is significant
        disArr[index] = disBetween; //add it to array
        disArrSize++;
        index++;
      }
    }
      //sort array by size
      qsort(disArr, disArrSize, sizeof(float), distSort);

    //find and store median
    int middle = disArrSize/2;
    interPeakDistances[i] = disArr[middle]; //don't really care if its exactly middle or off by 1
  }


  // Record the grid lines from the angles (step 4c) and distances (step 4d)
 
  lines[0] = PolarPeak( peakAngles[0] , interPeakDistances[0] );
  lines[1] = PolarPeak( peakAngles[1] , interPeakDistances[1] );

  // 5. Apply the inverse FT to 'gridFT' to get 'grid'.
  //
  //    [0 marks]

  inverseFT(gridFT, grid);  //Invserse Fourier Transform  src --> dst

  // 6. For each (x,y) location in 'grid' that has a bright pixel of
  //    value > gridLineMagnitudeThreshold (i.e. is one of the grid
  //    lines), set the corresponding pixel in the 'result' to the
  //    average of the pixels on either side of the grid line that are
  //    not also grid line pixels.  Do not modify 'image'; instead,
  //    store your result in 'result'.
  //
  //    You should call the provided 'averageOfNeighbours' function,
  //    which uses the 'lines' that you should compute above.
  //
  //    However, if 'interpolateAroundGridLines is false, you should
  //    instead set the corresponding pixel in the 'result' to zero.
  //    Do not call 'averageOfNeighbours' in this case.
  //
  //    In your initial implementation (before you get 'lines'
  //    computed correctly), set 'interpolateAroundGridLines' to false
  //    so that 'averageOfNeighbours' is not called.  Once 'lines' is
  //    computed correctly, remove the assignment of false to
  //    'interpolateAroundGridLines'.
  //
  //    [1 mark]

  for (int x=0; x<image->dimX; x++){    //for every pixel
    for (int y=0; y<image->dimY; y++){ 
      (*result)(x,y) = (*image)(x,y); //fill in result before its modified

      if ( abs((*grid)(x,y)) > (gridLineMagnitudeThreshold)){ //if pixel is on the grid, replace it
        
      if (interpolateAroundGridLines){ //interpolate pixel
        (*result)(x,y) = averageOfNeighbours(x, y, image, grid, lines); //lines is in radians
     }  
      else{ //do not interpolate, just set to 0
        (*result)(x,y) = 0;
      }
    }

    } //end of loop
    }
  // 7. For the two grid lines recorded in 'lines', output the angle and
  //    inter-line distances.
  //
  //    The angles and distances in 'lines' are measures in the Fourier
  //    domain, so must be mapped to the spatial domain using the
  //    *frequencies* of the grid line directions and the image
  //    dimensions.
  //
  //    [2 marks]
  for (unsigned int g=0; g<2; g++) {

    float angle;
    float wavelength;
    angle = lines[g].angle;
    if (angle > M_PI) //move angle around
      angle = angle - M_PI;
    //angles should be good then.

    float dist = lines[g].dist; //distance in fourier domain
    float u = dist * cos(angle); //decompose to u and v
    float v = dist * sin(angle); 
    //u and v in cycles per image dimension

    float freqx = u / image->dimX; 
    float freqy = v / image->dimY; 
    //in cycles per pixel

    float freq = sqrt(freqx * freqx + freqy * freqy); //return to 2D 
    //wavelength
    wavelength = 1 / freq;


    cout << "line " << g
	 << ": angle " << angle * 180/M_PI
	 << ", wavelength " << wavelength << " pixels" << endl;
  }
}
  



// Replace each removed pixel with the (possibly weighted) average of
// pixels on either side of the grid line, perpendicular to the grid
// line.
//
// Many implementations are possible.  The implementation below finds
// the (delta-x,delta-y) that is unit length and perpendicular to each
// grid line, then walks in the positive and negative (delta-x,delta-y)
// direction from (x,y) until a non-grid-line pixel is found.  At most
// three steps are taken in each direction.  If a non-grid-line pixel
// is found in both directions, their values are averaged.  If one is
// found in only one direction, its value is used.
//
// Since there are two different grid lines (lines[0] and lines[1] in
// the parameters), 90 degrees apart, we search in both directions.
//
// This isn't a great way to do this, but it works.

float Compute::averageOfNeighbours( int x, int y, ComplexArray2D *image, ComplexArray2D *grid, PolarPeak (&lines)[2] )

{
  float aspect = image->dimY / (float) image->dimX;

  for (int l=0; l<2; l++) {

    PolarPeak line = lines[l];
    
    // Find offset perpendicular to grid line

    float dx = 1;
    float dy = tanf( line.angle ) / (aspect*aspect); // (include aspect ratio)

    float len = sqrtf( dx*dx + dy*dy ); // make (dx,dy) unit length
    dx /= len;
    dy /= len;

    // Look ahead for non-grid pixel

    const int distToSearch = 3;

    // search in positive direction
    
    float valAhead = -1;
      
    for (int i=0; i<distToSearch; i++) {
      int px = (int) rint(x + i * dx);
      int py = (int) rint(y + i * dy);
      if (px >= 0 && px < image->dimX && py >= 0 && py < image->dimY) // stay within image
        if ( abs( (*grid)(px,py) ) <= gridLineMagnitudeThreshold ) {
          valAhead = abs( (*image)(px,py) ); // found a non-grid-line pixel
	  break;
	}
    }
      
    // search in negative direction
    
    float valBehind = -1;
      
    for (int i=0; i<distToSearch; i++) {
      int px = (int) rint(x - i * dx);
      int py = (int) rint(y - i * dy);
      if (px >= 0 && px < image->dimX && py >= 0 && py < image->dimY) // stay within image
        if ( abs( (*grid)(px,py) ) <= gridLineMagnitudeThreshold ) {
          valBehind = abs( (*image)(px,py) ); // found a non-grid-line pixel
	  break;
	}
    }
      
    if (valBehind == -1)
      if (valAhead == -1)
        continue; // nothing found
      else
        return valAhead; // only found in the positive direction
    else
      if (valAhead == -1)
        return valBehind; // only found in the negative direction
      else
        return 0.5 * (valBehind + valAhead); // found in both directions
  }

  // Nothing found in either direction for any grid line

  return 0;
}




void Compute::forwardFT( ComplexArray2D *src, ComplexArray2D *dest )

{
  fftw_plan p = fftw_plan_dft_2d( src->dimY, src->dimX, // dimY, then dimX is the correct order
				  (fftw_complex *) src->a,
				  (fftw_complex *) dest->a, 
				  FFTW_FORWARD, FFTW_ESTIMATE );

  fftw_execute( p );

  fftw_destroy_plan( p );
}


void Compute::inverseFT( ComplexArray2D *src, ComplexArray2D *dest )

{
  fftw_plan p = fftw_plan_dft_2d( src->dimY, src->dimX,
				  (fftw_complex *) src->a,
				  (fftw_complex *) dest->a, 
				  FFTW_BACKWARD, FFTW_ESTIMATE );

  fftw_execute( p );

  fftw_destroy_plan( p );

  // Scale inverse
  
  dest->scale( 1.0 / (float) (dest->dimX * dest->dimY) );
}
