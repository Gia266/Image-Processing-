// editor.h


#ifndef EDITOR_H
#define EDITOR_H

#include "headers.h"
#include "texture.h"


typedef enum { TRANSLATE, ROTATE, SCALE, INTENSITY } EditMode;
typedef enum { FORWARD, BACKWARD } ProjectionMode;


class Editor {

  Texture *editedImage;		// stores per-pixel changes
  Texture *displayedImage;      // is transformed version of 'editedImage'

  mat4 accumulatedTransform;    // all transforms so far, in one matrix
  mat4 recentMovementTransform; // most recent transform made during a movement edit

  vec2 initMousePosition;       // position on initial mouse click
  bool mouseDragging;		// true while mouse is being dragged to edit

 public:

  EditMode  editMode;
  ProjectionMode projectionMode;

  Editor( Texture *image ) {

    displayedImage = image;
    editedImage = new Texture( *image ); // a copy

    accumulatedTransform = identity4();

    editMode = TRANSLATE;
    projectionMode = FORWARD;
  }

  vec3 rgb_to_hsl( Pixel rgb );
  Pixel hsl_to_rgb( vec3 hsl );
  float hue_to_rgb( float p, float q, float t );
  
  void startMouseMotion( float x, float y );
  void mouseMotion( float x, float y );
  void stopMouseMotion();
  void keyPress( int key );
  void project( Texture *srcImage, Texture *destImage, mat4 &T );
};

#endif
