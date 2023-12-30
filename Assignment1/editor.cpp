// editor.cpp


#include "editor.h"



// Take the source image, apply the provided transform, T, and store
// the transformed image in the destination image.
//
// Where a pixel in the destination image has no corresponding (valid)
// pixel in the source image, use the 'transparentPixel'


void Editor::project( Texture *srcImage, Texture *destImage, mat4 &T )

{
  // Check that dimensions match
  
  if (srcImage->width != destImage->width || srcImage->height != destImage->height) {
    cerr << "in Editor::project() the source and destination images have different dimensions" << endl;
    exit(1);
  }

  // Project
  
  Pixel transparentPixel = { 0,0,0,0 }; // fully transparent pixel (alpha = 0, so r,g,b doesn't matter)
    
  if (projectionMode == FORWARD) { // Forward projection
    
    // Set all of the image to transparent pixels in case there are
    // destination locations that do not get written to with forward
    // projection.

    // UNCOMMENT THE THREE LINES BELOW *AFTER* YOU GET FORWARD
    // PROJECTION WORKING.  THEY JUST SERVE AS AN EXAMPLE OF (A) HOW
    // TO ITERATE OVER THE IMAGE PIXELS AND (B) HOW TO SET THE PIXEL
    // AT A PARTICULAR LOCATION.
    
    for (unsigned int x=0; x<destImage->width; x++)
    for (unsigned int y=0; y<destImage->height; y++)
    destImage->pixel( x, y ) = transparentPixel;
    
    // Do the forward projection

    // YOUR CODE HERE
    int i = 0;
      //for every pixel in the source image
      for (unsigned int x=0; x<srcImage->width; x++)
        for (unsigned int y=0; y<srcImage->height; y++){
            
            vec4 srcPixCord(x, y, 0, 1);
            vec4 dstPixCord = T * srcPixCord; 

            int dstX = (int) dstPixCord.x ;
            int dstY = (int) dstPixCord.y;

            destImage->pixel(dstX, dstY) = srcImage->pixel(x, y);

        }

  } else { // Backward projection

    // YOUR CODE HERE
    for (unsigned int x=0; x<destImage->width; x++)
        for (unsigned int y=0; y<destImage->height; y++){
            vec4 dstPixCord(x, y, 0, 1); 
            vec4 targetSrcPixCord = T.inverse() * dstPixCord; 

            int targetSrcX = (int) targetSrcPixCord.x ;
            int targetSrcY = (int) targetSrcPixCord.y;

            if (targetSrcX < 0 || targetSrcX > destImage->width || targetSrcY < 0 || targetSrcY > destImage->height) 
              destImage->pixel(x, y) = transparentPixel;
            else 
              destImage->pixel(x, y) = srcImage->pixel(targetSrcX, targetSrcY);

        }
  }

  destImage->updated = true; // necessary to get new image shipped to GPU
}



void Editor::startMouseMotion( float x, float y )

{
  initMousePosition = vec2(x,y);
  mouseDragging = true;
}



void Editor::mouseMotion( float x, float y )

{
  vec2 mousePosition( x, y );
  vec2 imageCentre( displayedImage->height/2, displayedImage->width/2 );

  if (editMode == TRANSLATE) {

    // Use translate() from linalg.h to build a 4x4 translation matrix
    
    recentMovementTransform = translate( mousePosition.x - initMousePosition.x,
					 initMousePosition.y - mousePosition.y,
					 0 );

    // Incorporate that translation into the already-accumulated transforms
    
    mat4 T = recentMovementTransform * accumulatedTransform;

    // Apply the new transform using the project() function
    
    project( editedImage, displayedImage, T );

  } else if (editMode == ROTATE) {


    //calculate the angles 
    float initAngle = atan2(initMousePosition.x-imageCentre.x,  initMousePosition.y -imageCentre.y);
    float currAngle = atan2(mousePosition.x-imageCentre.x,   mousePosition.y -imageCentre.y);
    float angle = currAngle-initAngle;

    //rotation transform
    mat4 rotateTransform = rotate(angle, vec3(0,0,1));

    // rotate about image centre
    mat4 translatetoOrigin = translate(-imageCentre.x, -imageCentre.y, 0); // translate to origin
    mat4 translateBack = translate(imageCentre.x, imageCentre.y, 0);    // translate back

    //combine transforms
    recentMovementTransform = translateBack * (rotateTransform * translatetoOrigin) ;
    
    //combine with accumulated transform 
    mat4 T = recentMovementTransform * accumulatedTransform;


    project(editedImage, displayedImage, T );


  } else if (editMode == SCALE) {

    // scale about the imageCentre

    // YOUR CODE HERE

    //calculating distances from initiual and current mous positions
    float distInit = sqrt(pow(mousePosition.x-imageCentre.x,2) + pow(mousePosition.y-imageCentre.y,2));
    float distCurr = sqrt(pow(initMousePosition.x-imageCentre.x,2) + pow(initMousePosition.y-imageCentre.y,2));

    //translating from orgin and back 
    mat4 translateToOrigin = translate(-imageCentre.x, -imageCentre.y, 0); 
    mat4 translateBack = translate(imageCentre.x, imageCentre.y, 0);

    //calculate scale factor
    float scaleFactor = 1 - (distCurr - distInit)/200;

    mat4 scaleTransform = scale(scaleFactor, scaleFactor, 1); //scaling transform
    recentMovementTransform = translateBack * (scaleTransform * translateToOrigin) ; // combine translating and scaling
    mat4 T = recentMovementTransform * accumulatedTransform; //combine recentMovementTransform with accumulatedtransform 

    project( editedImage, displayedImage, T );

    
  } else if (editMode == INTENSITY) {

    // YOUR CODE HERE (calculate scale and bias)

    for (unsigned int x=0; x<editedImage->width; x++)
      for (unsigned int y=0; y<editedImage->height; y++) {

	        Pixel &src  = editedImage->pixel( x, y );
	        Pixel &dest = displayedImage->pixel( x, y );

	// Convert the source pixel to HSL, modify the luminance
	// component, the convert back and store in the destination
	// pixel.
	
	// YOUR CODE HERE

    vec3 temp = rgb_to_hsl(src); //RGB to HSL

    float brt = (mousePosition.y - initMousePosition.y)/200; //modify luminance/intensity with y movement
    float con = 1+ (mousePosition.x - initMousePosition.x)/200; //modifying luminance/intensity with x movement

    //Contrast w/ birghtness adjustment 
    temp.z = (con * temp.z) + brt; 

    //clamping
    if (temp.z >= 0.99)
        temp.z = 0.99;
    if (temp.z <= 0.01)
        temp.z = 0.01;
   
    dest = hsl_to_rgb(temp); //Convert modified HSL back to RGB

	// Also copy alpha, as that is lost in the RGB -> HSL -> RGB conversion

	dest.a = src.a;
      }

    displayedImage->updated = true; // necessary to get new image shipped to GPU
  }
}



// Handle the button release at the end of mouse dragging
//
// This required that any movement changed be incorporated into the
// 'accumulatedTransform', and any intensity changes be incorporated
// into the 'editedImage'

void Editor::stopMouseMotion()

{
  if (editMode == TRANSLATE || editMode == ROTATE || editMode == SCALE) {

    // Incorporate the transform from the mouse drag into the 'accumulatedTransform'.

    // YOUR CODE HERE
    accumulatedTransform = recentMovementTransform  *  accumulatedTransform;

  } else if (editMode == INTENSITY) {

    // Incorporate the intensity changes from the mouse drag into the 'editedImage'.

    // YOUR CODE HERE
    editedImage = new Texture( *displayedImage );
  }
  
  mouseDragging = false;
}



// Convert between RGB in [0,255] and HSL in [0,1]
//
// From https://gist.github.com/ciembor/1494530


#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

vec3 Editor::rgb_to_hsl( Pixel rgb )

{
  vec3 result;
  
  float r = rgb.r / 255.0;
  float g = rgb.g / 255.0;
  float b = rgb.b / 255.0;
  
  float max = MAX(MAX(r,g),b);
  float min = MIN(MIN(r,g),b);
  
  result.x = result.y = result.z = (max + min) / 2;

  if (max == min) {
    result.x = result.y = 0; // achromatic
  }
  else {
    float d = max - min;
    result.y = (result.z > 0.5) ? d / (2 - max - min) : d / (max + min);
    
    if (max == r) {
      result.x = (g - b) / d + (g < b ? 6 : 0);
    }
    else if (max == g) {
      result.x = (b - r) / d + 2;
    }
    else if (max == b) {
      result.x = (r - g) / d + 4;
    }
    
    result.x /= 6;
  }

  return result;
}


float Editor::hue_to_rgb( float p, float q, float t ) // From https://gist.github.com/ciembor/1494530

{
  if (t < 0) 
    t += 1;
  if (t > 1) 
    t -= 1;
  if (t < 1./6.0) 
    return p + (q - p) * 6 * t;
  if (t < 1./2.0) 
    return q;
  if (t < 2./3.0)   
    return p + (q - p) * (2./3.0 - t) * 6;
    
  return p;
}


Pixel Editor::hsl_to_rgb( vec3 hsl ) // From https://gist.github.com/ciembor/1494530

{
  float h = hsl.x;
  float s = hsl.y;
  float l = hsl.z;

  Pixel result;
  
  if(0 == s) {
    result.r = result.g = result.b = l; // achromatic
  } else {
    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;
    result.r = hue_to_rgb(p, q, h + 1./3) * 255;
    result.g = hue_to_rgb(p, q, h) * 255;
    result.b = hue_to_rgb(p, q, h - 1./3) * 255;
  }

  return result;
}



void Editor::keyPress( int key )

{
  // ignore key presses while the mouse is being dragged
  
  if (mouseDragging)
    return;

  // handle key press
  
  switch (key) {

    // Transformation modes
    
  case 'T':
    editMode = TRANSLATE;
    break;
  case 'R':
    editMode = ROTATE;
    break;
  case 'S':
    editMode = SCALE;
    break;

    // Pixel editing modes

  case 'I':
    editMode = INTENSITY;
    break;

    // Projection modes
    
  case 'F':
    projectionMode = FORWARD;
    project( editedImage, displayedImage, accumulatedTransform );
    break;
  case 'B':
    projectionMode = BACKWARD;
    project( editedImage, displayedImage, accumulatedTransform );
    break;
  }
}

