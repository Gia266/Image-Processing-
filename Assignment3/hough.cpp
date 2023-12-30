// hough.cpp


#include "hough.h"

#include <algorithm>


/* 
Run the program:
./hough ../images/road.png

? - help
C - compute solution
S - toggle smoothing
L - toggle log display of accumulator array
F - toggle finding of calibration marker
H - toggle highlighting (for debugging)
+ - more peaks
- - fewer peaks


export CPATH=/opt/homebrew/include
export LIBRARY_PATH=/opt/homebrew/lib
*/

// Compute the Hough transform of 'image' and store in 'counts'.  Then
// find a rectangle.


void Hough::computeSolution( bool smoothCounts, int numPeaks, bool findMarker )

{
  // ----------------------------------------------------------------
  //
  // 1. Fill the 'counts' array
  //
  // Each image pixel of value greater than 128 is considered an edge
  // pixel.  Use the 'r' component of an RGBA pixel as the pixel
  // value.  At (i,j), that component is
  //
  //     image->pixel(i,j).r
  //
  // See hough.h for how the 'counts' array is set up.
  //
  // The image origin is to be the CENTRE of the image, as defined in
  // hough.h, so the pixel at position (i,j) is considered to have
  // coordinates (i-centreX,j-centreY).

  for (int i=0; i<countsDimX; i++) //clear counts to 0
    for (int j=0; j<countsDimY; j++)
      counts[i][j] = 0;

  // YOUR CODE HERE (2 marks)
  for (int i=0; i<image->width; i++){    //for every pixel in the image
    for (int j=0; j<image->height; j++){

      int x_ = i-centreX; //modified x and y value
      int y_ = j-centreY;
      //for all x y values in the image (every pixel)

      float brightness = image->pixel(i,j).r; //if above edge pixel threshold

      if (brightness > 128){
        for (int k = 0; k < countsDimX; k++){ //for every value (0.5 degree)
          //r = x cos(theta) + y sin(theta) equation
          float theta = thetaResolution * k; //half a degree in radians
          int r = int(x_ * cos(theta) + y_ * sin(theta)); //find r on that line
          r = int(r - countsDimY / 2) - 44; //adjust
          counts[k][r] += 1; //add a count at that coordinate
      }
      }  

    }} //counts array is now filled with edge pixel hough transfrom totals

  // ----------------------------------------------------------------
  //
  // 2. Smooth the 'counts' array
  //
  // Use a square kernal with the 'halfWidth' below.  Each weight in
  // the kernal is the same.
  //
  // When finding the calibration marker, do not smooth.

  if (smoothCounts && !findMarker) {
    
    const int halfWidth = 1; // 3x3 kernel with weights 1/9 each

    // Temporary array to hold convolution result
    int **temp = new int*[countsDimX];
    for (int i=0; i<countsDimX; i++)
      temp[i] = new int[countsDimY];

    // YOUR CODE HERE (2 marks)
    // Perform convolution
    for (int i = 0; i < countsDimX; i++) { //for every 'pixel' in counts
      for (int j = 0; j < countsDimY; j++) {
            int sum = 0;
            int count = 0;

            for (int x = -halfWidth; x <= halfWidth; x++) {   //sum n average all the pixels around it
                for (int y = -halfWidth; y <= halfWidth; y++) {
                    int currx = i + x; 
                    int curry = j + y;
                    //IF that neighborhood pixel is within image bounds
                    if (currx >= 0 && currx < countsDimX && curry >= 0 && curry < countsDimY) {
                        sum = sum + counts[currx][curry]; //add to sum
                        count++; //count (for average)
                    }
                }
            }
              temp[i][j] = int(sum / count); //store smoothed average
        } 
    } 
    
    for (int i = 0; i < countsDimX; i++){  //for every 'pixel' in counts
    for (int j = 0; j < countsDimY; j++){
     counts[i][j] = temp[i][j]; //update counts with new smoothed values
    }}



    // Delete temporary array
  
    for (int i=0; i<countsDimX; i++)
      delete [] temp[i];
    delete [] temp;

  }

  // ----------------------------------------------------------------
  //
  // 3. Find the coordinates of the 'numPeaks' largest counts.
  //
  // Only consider counts that do not have an adjacent entry in the
  // 'counts' array having a *larger* count.

  // Keep a 'maxCounts' array, order by increasing count
  
  int *maxCounts = new int[numPeaks];
  for (int i=0; i<numPeaks; i++)
    maxCounts[i] = 0; //set to 0 (so values can compare higher)

  // Keep the corresponding coordinates of the max-count peaks
  
  Coords *maxCoords = new Coords[numPeaks];
  //fill with empty coords
  for (int i=0; i<numPeaks; i++){
    maxCoords[i].x = 0;
    maxCoords[i].y = 0;
  }

  // YOUR CODE HERE (4 marks ... clean and efficient code)

    for (int i=0; i<countsDimX; i++){ //for each pixel on the hough image (counts array)
      for (int j=0; j<countsDimY; j++){
        int current = counts[i][j];    //get current value
        int notMax = 0; //flag
        //first check IF it is a local maximum
         for (int x = -1; x <= 1; x++) {    //check 3x3 neighborhood around it
            for (int y = -1; y <= 1; y++) {
                int currx = i + x; 
                int curry = j + y;
                //If that neighborhood pixel is within image bounds
                if (currx >= 0 && currx < countsDimX && curry >= 0 && curry < countsDimY) {
                  if (counts[currx][curry] > counts[i][j]) //if ANY pixel around is higher, it is skipped
                   notMax = 1; //set not local maximum true
            }}}
        if (notMax == 1)
          continue; //skip non-max pixels
        //we now check pixels that ARE local maximums

        int shift = 0; //right shift flag
        int temp; 
        int tempx; //temp coords value
        int tempy;
        //this will check values against the maxCounts array
        //if the value higher, it will stop comparing and start 'shifting' the values right instead
        //this is needed as using qsort will sort count values, but wont be able to keep the coords lined up.
        for (int idx = 0; idx < numPeaks; idx++){ //for each index in maxCounts

        if (shift == 1){ //if in rightshift mode
            int temp2 = maxCounts[idx]; //shift values right
            maxCounts[idx] = temp; 
            temp = temp2;

            int temp2x = maxCoords[idx].x; //shift values right
            maxCoords[idx].x = tempx;
            tempx = temp2x;

            int temp2y = maxCoords[idx].y; //shift values right
            maxCoords[idx].y = tempy;
            tempy = temp2y;
        }
        else if(current > maxCounts[idx]){//if current value is higher than maxCounts in index
              shift = 1; //shift flag
              temp = maxCounts[idx]; //store new value
              maxCounts[idx] = current;

              tempx = maxCoords[idx].x; //store and bank coords
              maxCoords[idx].x = i;

              tempy = maxCoords[idx].y;
              maxCoords[idx].y = j;
          }
        //else value is too low to be considered
        }
        shift = 0; //set flag to 0
      }}
  // Debugging: output the maxCounts array

#if 1
  for (int i=0; i<numPeaks; i++)
    cout << maxCounts[i] << ": " << maxCoords[i].x << "," << maxCoords[i].y << endl;
#endif

  // Debugging: draw the points and lines found

#if 1
  accumulatorPoints.clear();
  for (int i=0; i<numPeaks; i++)
    accumulatorPoints.push_back( maxCoords[i] );

  imageLines.clear();
  for (int i=0; i<numPeaks; i++)
    imageLines.push_back( maxCoords[i] );
#endif

  // ----------------------------------------------------------------
  //
  // 4. For the cxr.png image, find the four lines that bound the
  // calibration marker, which is in the upper-left of the image.
  //
  // Report the dimensions, in pixels, of the calibration marker.

  // 4a. Find the top and bottom lines.  These should be peaks in the
  // unsmoothed accumulator array with approximately the same theta.
  //
  // Recall that counts[i][j] corresponds to (rho,theta) as
  //
  //     i in [0,countsDimX) corresponds to theta in [0,pi)
  //     j in [0,countsDimY) corresponds to rho in [-countsDimY/2, +countsDimY/2)

  if (!findMarker)
    return;
  
  Coords lines[4];
  
  // Fill lines[0] and lines[1] for top and bottom lines of
  // calibration marker.  Order does not matter.
  //
  // Note that lines[k] is a Coords object with its x and y components
  // being the i and j indices, respectively, of the line in 'counts'
  // array at 'counts[i][j]'.  As above, i is for theta and j is for
  // rho.


  // YOUR CODE HERE (1 mark)
  for (int i = 0; i < numPeaks; i ++){ //compare every line in numPeaks
      for (int j = 0; j < numPeaks; j++){
        if(i != j){ //if not the same line

          if (maxCoords[j].x == maxCoords[i].x){ //if two lines are the exact same angle
          //put those lines and line 0 and 1
          lines[0].x = maxCoords[i].x;
          lines[0].y = maxCoords[i].y;
          lines[1].x = maxCoords[j].x;
          lines[1].y = maxCoords[j].y;
          }
        }
      }
  }


  // Debugging: draw the top and bottom lines that were found
  
#if 1
  accumulatorPoints.clear();
  accumulatorPoints.push_back( lines[0] );
  accumulatorPoints.push_back( lines[1] );

  imageLines.clear();
  imageLines.push_back( lines[0] );
  imageLines.push_back( lines[1] );
#endif

  // 4b. Search in the accumulator array for peaks that are offset
  // about pi/2 from the peaks found above.  These are the vertical
  // dividing lines on the calibration marker.
  //
  // Only collect peaks that have counts at least 'offsetPeakThreshold'.
  // Keep the peak of minimum row and that of maximum row in the
  // 'counts' array.  Those should be the left and right edges of the
  // calibration.
  //
  // If your debugging display shows that you are missing peaks or
  // finding the wrong peaks, you might have to adjust 'offsetPeakThreshold'.

  int offsetPeakThreshold = 35;  // ignore entries in 'counts' below this

  // Find the column that is offset by pi/2
  
  int peakCol;

  // YOUR CODE HERE (2 marks ... set peakCol)
    peakCol = lines[0].x + M_PI/2 - 1; //column is at an angle 90 degrees from the first lines
    peakCol = peakCol % 180; //handle wraparound

  // Find the highest-row and lowest-row peaks above the threshold
  
  int minRow;
  int maxRow;


  // YOUR CODE HERE (2 marks)
  for (int j = 0; j < countsDimY; j++){ //for every rho value at the same theta
    if (counts[peakCol][j] > offsetPeakThreshold){ //search from 0 to first peak
      minRow = j;
      break;
    }
  }
  for (int j = countsDimY-1; j > 0; j--){ //for every rho value at the same theta
    if (counts[peakCol][j] > offsetPeakThreshold){ //search from max to first peak the other way
      maxRow = j;
      break;
    }
  }

  // Fill lines[2] and lines[3] for left and right lines of
  // calibration marker.  Order does not matter.


  // YOUR CODE HERE (0 marks)
  lines[2].x = peakCol;
  lines[3].x = peakCol;
  lines[2].y = minRow;
  lines[3].y = maxRow;

  // Debugging: draw the points and lines found

#if 1
  accumulatorPoints.push_back( lines[2] );
  accumulatorPoints.push_back( lines[3] );

  imageLines.push_back( lines[2] );
  imageLines.push_back( lines[3] );
#endif

  // Report the lines

  for (int i=0; i<4; i++) 
    cout << "line " << i << ": distance " << lines[i].y * rhoResolution - countsDimY/2
	 << " at " << lines[i].x  / countsDimX * 180 << " degrees" << endl;

  // YOUR CODE HERE (1 mark ... replace the 0s below with correct values)
  int dimx = abs(lines[0].y - lines[1].y);
  int dimy = abs(lines[2].y - lines[3].y);
  cout << "marker dimensions: " << dimx << " x " << dimy << endl;
}


// Output operation for Coords class

std::ostream& operator << ( std::ostream& stream, Coords const& p )

{
  stream << p.x << " " << p.y;
  return stream;
}

