/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2002 by Systems in TCB. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  version 2.1 as published by the Free Software Foundation. See the
 *  file LICENSE.LGPL at the root directory of the distribution for
 *  more details.
 *
 *  If you want to use Coin for applications not compatible with the
 *  LGPL, please contact SIM to acquire a Professional Edition license.
 *
 *  Systems in TCB, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/
 
/*!
  \class SoText2Set SoText2Set.h Inventor/nodes/SoText2Set.h
  \brief The SoText2Set class is a node type for visualizing a set of 2D texttags aligned with the camera plane.
  \ingroup nodes

  SoText2Set text is not scaled according to the distance from the
  camera, and is not influenced by rotation or scaling as 3D
  primitives are. If these are properties that you want the text to
  have, you should instead use an SoText3 or SoAsciiText node.

  Note that even though the size of the 2D text is not influenced by
  the distance from the camera, the text is still subject to the usual
  rules with regard to the depthbuffer, so it \e will be obscured by
  graphics laying in front of it.

  The text will be \e positioned according to the current transformation.
  The x origin of the text is the first pixel of the leftmost character
  of the text. The y origin of the text is the baseline of the first line
  of text (the baseline being the imaginary line on which all upper case
  characters are standing).

  \sa SoFont, SoFontStyle, SoText3, SoText2, SoAsciiText
*/

// FIXME -- FIXME -- FIXME
//
//  * computeBoundingBox() is not implemented properly, rayPick() and
//    generatePrimitives() are just stubs
//  * allocations aren't cleaned out on exit (Display *, XFontStructs,
//    SbDict, OpenGL display lists, ...)
//  * integrate with libfreetype to remove dependency on X11.
//         -- 19990418 mortene.

//  * This code is more or less copied directly from SoText2. I couldn't
//    figure out this strange SoText2P-class, so I removed the 
//    function called getFontSize and inserted a constant instead. 
//     
//         -- 06182002 torbjorv



#include <SmallChange/nodes/SoText2Set.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/details/SoLineDetail.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoGLLightModelElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/SbLine.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/details/SoTextDetail.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H


#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>

#include <string.h>

/*!
  \enum SoText2Set::Justification

  Enum contains the various options for how the horizontal text layout
  text should be done.
*/


/*!
  \var SoMFString SoText2Set::strings

  The set of strings to render.  Each string in the multiple value
  field will be according to the coordinates on the state. 

  The default value of the field is a single empty string.
*/

/*!
  \var SoSFEnum SoText2Set::justification

  Decides how the horizontal layout of the text strings is done.
*/
 
/*!
  \var SoSFVec2f SoText2Set::displacement

  Pixelwise displacement of the text after projection of 3D-position.
*/


// *************************************************************************

SO_NODE_SOURCE(SoText2Set);

// *************************************************************************

class SoText2SetP{
public:
  SoText2SetP(SoText2Set * master) {
    this->master = master;
  }

  static const SbVec2f fontsize;
private:
  SoText2Set * master;
};


#define PRIVATE(p) p->pimpl
#define PUBLIC(p) p->master

// *************************************************************************


const SbVec2f SoText2SetP::fontsize = SbVec2f(8.0f, 12.0f);

/*!
  Constructor.
*/
SoText2Set::SoText2Set(void)
{
  SO_NODE_CONSTRUCTOR(SoText2Set);
  PRIVATE(this) = new SoText2SetP(this);


  SO_NODE_ADD_FIELD(strings, (""));
  SO_NODE_ADD_FIELD(justification, (SoText2Set::LEFT));
  SO_NODE_ADD_FIELD(displacement, (0.0f, 0.0f));
    
  SO_NODE_DEFINE_ENUM_VALUE(Justification, LEFT);
  SO_NODE_DEFINE_ENUM_VALUE(Justification, RIGHT);
  SO_NODE_DEFINE_ENUM_VALUE(Justification, CENTER);
  SO_NODE_SET_SF_ENUM_TYPE(justification, Justification);
}//Constructor



/*!
  Destructor.
*/
SoText2Set::~SoText2Set()
{
  delete PRIVATE(this);
}




// Doc from parent class.
void
SoText2Set::initClass(void)
{
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_NODE_INIT_CLASS(SoText2Set, SoShape, "Shape");
}

static unsigned char fontdata[][12] = {
  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, //
  {  0,  0, 12, 12,  0,  8, 12, 12, 12, 12, 12,  0 }, // !
  {  0,  0,  0,  0,  0,  0,  0,  0,  0, 20, 20, 20 }, // "
  {  0,  0, 18, 18, 18, 63, 18, 18, 63, 18, 18,  0 }, // #
  {  0,  8, 28, 42, 10, 10, 12, 24, 40, 42, 28,  8 }, // $
  {  0,  0,  6, 73, 41, 22,  8, 52, 74, 73, 48,  0 }, // %
  {  0, 12, 18, 18, 12, 25, 37, 34, 34, 29,  0,  0 }, // &
  {  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 12, 12 }, // '
  {  0,  6,  8,  8, 16, 16, 16, 16, 16,  8,  8,  6 }, // (
  {  0, 48,  8,  8,  4,  4,  4,  4,  4,  8,  8, 48 }, // )
  {  0,  0,  0,  0,  0,  0,  8, 42, 20, 42,  8,  0 }, // *
  {  0,  0,  0,  8,  8,  8,127,  8,  8,  8,  0,  0 }, // +
  {  0, 24, 12, 12,  0,  0,  0,  0,  0,  0,  0,  0 }, // ,
  {  0,  0,  0,  0,  0,  0,127,  0,  0,  0,  0,  0 }, // -
  {  0,  0, 24, 24,  0,  0,  0,  0,  0,  0,  0,  0 }, // .
  {  0, 32, 32, 16, 16,  8,  8,  8,  4,  4,  2,  2 }, // /
  {  0,  0, 28, 34, 34, 34, 34, 34, 34, 34, 28,  0 }, // 0
  {  0,  0,  8,  8,  8,  8,  8,  8, 40, 24,  8,  0 }, // 1
  {  0,  0, 62, 32, 16,  8,  4,  2,  2, 34, 28,  0 }, // 2
  {  0,  0, 28, 34,  2,  2, 12,  2,  2, 34, 28,  0 }, // 3
  {  0,  0,  4,  4,  4,126, 68, 36, 20, 12,  4,  0 }, // 4
  {  0,  0, 28, 34,  2,  2,  2, 60, 32, 32, 62,  0 }, // 5
  {  0,  0, 28, 34, 34, 34, 60, 32, 32, 34, 28,  0 }, // 6
  {  0,  0, 16, 16, 16,  8,  8,  4,  2,  2, 62,  0 }, // 7
  {  0,  0, 28, 34, 34, 34, 28, 34, 34, 34, 28,  0 }, // 8
  {  0,  0, 28, 34,  2,  2, 30, 34, 34, 34, 28,  0 }, // 9
  {  0,  0, 24, 24,  0,  0,  0, 24, 24,  0,  0,  0 }, // :
  {  0, 48, 24, 24,  0,  0,  0, 24, 24,  0,  0,  0 }, // ;
  {  0,  0,  0,  2,  4,  8, 16,  8,  4,  2,  0,  0 }, // <
  {  0,  0,  0,  0,  0,127,  0,127,  0,  0,  0,  0 }, // =
  {  0,  0,  0, 16,  8,  4,  2,  4,  8, 16,  0,  0 }, // >
  {  0,  0, 16, 16,  0, 16, 28,  2,  2,  2, 60,  0 }, // ?
  {  0,  0, 28, 32, 73, 86, 82, 82, 78, 34, 28,  0 }, // @
  {  0,  0, 33, 33, 33, 63, 18, 18, 18, 12, 12,  0 }, // A
  {  0,  0, 60, 34, 34, 34, 60, 34, 34, 34, 60,  0 }, // B
  {  0,  0, 14, 16, 32, 32, 32, 32, 32, 18, 14,  0 }, // C
  {  0,  0, 56, 36, 34, 34, 34, 34, 34, 36, 56,  0 }, // D
  {  0,  0, 62, 32, 32, 32, 60, 32, 32, 32, 62,  0 }, // E
  {  0,  0, 16, 16, 16, 16, 30, 16, 16, 16, 30,  0 }, // F
  {  0,  0, 14, 18, 34, 34, 32, 32, 32, 18, 14,  0 }, // G
  {  0,  0, 34, 34, 34, 34, 62, 34, 34, 34, 34,  0 }, // H
  {  0,  0, 62,  8,  8,  8,  8,  8,  8,  8, 62,  0 }, // I
  {  0,  0,112,  8,  8,  8,  8,  8,  8,  8, 62,  0 }, // J
  {  0,  0, 33, 33, 34, 36, 56, 40, 36, 34, 33,  0 }, // K
  {  0,  0, 30, 16, 16, 16, 16, 16, 16, 16, 16,  0 }, // L
  {  0,  0, 33, 33, 33, 45, 45, 45, 51, 51, 33,  0 }, // M
  {  0,  0, 34, 34, 38, 38, 42, 42, 50, 50, 34,  0 }, // N
  {  0,  0, 12, 18, 33, 33, 33, 33, 33, 18, 12,  0 }, // O
  {  0,  0, 32, 32, 32, 60, 34, 34, 34, 34, 60,  0 }, // P
  {  3,  6, 12, 18, 33, 33, 33, 33, 33, 18, 12,  0 }, // Q
  {  0,  0, 34, 34, 34, 36, 60, 34, 34, 34, 60,  0 }, // R
  {  0,  0, 60,  2,  2,  6, 28, 48, 32, 32, 30,  0 }, // S
  {  0,  0,  8,  8,  8,  8,  8,  8,  8,  8,127,  0 }, // T
  {  0,  0, 28, 34, 34, 34, 34, 34, 34, 34, 34,  0 }, // U
  {  0,  0, 12, 12, 18, 18, 18, 33, 33, 33, 33,  0 }, // V
  {  0,  0, 34, 34, 34, 54, 85, 73, 73, 73, 65,  0 }, // W
  {  0,  0, 34, 34, 20, 20,  8, 20, 20, 34, 34,  0 }, // X
  {  0,  0,  8,  8,  8,  8, 20, 20, 34, 34, 34,  0 }, // Y
  {  0,  0, 62, 32, 16, 16,  8,  4,  4,  2, 62,  0 }, // Z
  {  0, 14,  8,  8,  8,  8,  8,  8,  8,  8,  8, 14 }, // [
  {  0,  2,  2,  4,  4,  8,  8,  8, 16, 16, 32, 32 }, // [backslash]
  {  0, 56,  8,  8,  8,  8,  8,  8,  8,  8,  8, 56 }, // ]
  {  0,  0,  0,  0,  0, 34, 34, 20, 20,  8,  8,  0 }, // ^
  {  0,127,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // _
  {  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 24, 12 }, // `
  {  0,  0, 29, 34, 34, 30,  2, 34, 28,  0,  0,  0 }, // a
  {  0,  0, 60, 34, 34, 34, 34, 50, 44, 32, 32, 32 }, // b
  {  0,  0, 14, 16, 32, 32, 32, 16, 14,  0,  0,  0 }, // c
  {  0,  0, 26, 38, 34, 34, 34, 34, 30,  2,  2,  2 }, // d
  {  0,  0, 28, 34, 32, 62, 34, 34, 28,  0,  0,  0 }, // e
  {  0,  0, 16, 16, 16, 16, 16, 16, 62, 16, 16, 14 }, // f
  { 28,  2,  2, 26, 38, 34, 34, 34, 30,  0,  0,  0 }, // g
  {  0,  0, 34, 34, 34, 34, 34, 50, 44, 32, 32, 32 }, // h
  {  0,  0,  8,  8,  8,  8,  8,  8, 56,  0,  8,  8 }, // i
  { 56,  4,  4,  4,  4,  4,  4,  4, 60,  0,  4,  4 }, // j
  {  0,  0, 33, 34, 36, 56, 40, 36, 34, 32, 32, 32 }, // k
  {  0,  0,  8,  8,  8,  8,  8,  8,  8,  8,  8, 56 }, // l
  {  0,  0, 73, 73, 73, 73, 73,109, 82,  0,  0,  0 }, // m
  {  0,  0, 34, 34, 34, 34, 34, 50, 44,  0,  0,  0 }, // n
  {  0,  0, 28, 34, 34, 34, 34, 34, 28,  0,  0,  0 }, // o
  { 32, 32, 60, 34, 34, 34, 34, 50, 44,  0,  0,  0 }, // p
  {  2,  2, 26, 38, 34, 34, 34, 34, 30,  0,  0,  0 }, // q
  {  0,  0, 16, 16, 16, 16, 16, 24, 22,  0,  0,  0 }, // r
  {  0,  0, 60,  2,  2, 28, 32, 32, 30,  0,  0,  0 }, // s
  {  0,  0, 14, 16, 16, 16, 16, 16, 62, 16, 16,  0 }, // t
  {  0,  0, 26, 38, 34, 34, 34, 34, 34,  0,  0,  0 }, // u
  {  0,  0,  8,  8, 20, 20, 34, 34, 34,  0,  0,  0 }, // v
  {  0,  0, 34, 34, 34, 85, 73, 73, 65,  0,  0,  0 }, // w
  {  0,  0, 34, 34, 20,  8, 20, 34, 34,  0,  0,  0 }, // x
  { 48, 16,  8,  8, 20, 20, 34, 34, 34,  0,  0,  0 }, // y
  {  0,  0, 62, 32, 16,  8,  4,  2, 62,  0,  0,  0 }, // z
  {  0,  6,  8,  8,  8,  4, 24,  4,  8,  8,  8,  6 }, // {
  {  0,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 }, // |
  {  0, 48,  8,  8,  8, 16, 12, 16,  8,  8,  8, 48 }, // }
  {  0,  0,  0,  0,  0,  0, 78, 57,  0,  0,  0,  0 },  // ~
  // iso-latin-1 norwegian letters
  {  0,  0, 59, 76, 72, 62,  9, 73, 54,  0,  0,  0 }, // ae
  {  0,  0, 92, 34, 50, 42, 42, 38, 29,  0,  0,  0 }, // oe
  {  0,  0, 29, 34, 34, 30,  2, 34, 28,  8, 20,  8 }, // aa
  {  0,  0, 79, 72, 72, 72,127, 72, 72, 72, 63,  0 }, // AE
  {  0,  0, 44, 18, 41, 41, 41, 37, 37, 18, 13,  0 }, // OE
  {  0,  0, 33, 33, 33, 63, 18, 18, 12, 12, 18, 12 }  // AA
};

// map from iso-latin1 to font data array index
static int fontdata_isolatin1_mapping[] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   1,   2,   3,   4,   5,   6,   7,
    8,   9,  10,  11,  12,  13,  14,  15,
   16,  17,  18,  19,  20,  21,  22,  23,
   24,  25,  26,  27,  28,  29,  30,  31,
   32,  33,  34,  35,  36,  37,  38,  39,
   40,  41,  42,  43,  44,  45,  46,  47,
   48,  49,  50,  51,  52,  53,  54,  55,
   56,  57,  58,  59,  60,  61,  62,  63,
   64,  65,  66,  67,  68,  69,  70,  71,
   72,  73,  74,  75,  76,  77,  78,  79,
   80,  81,  82,  83,  84,  85,  86,  87,
   88,  89,  90,  91,  92,  93,  94,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0, 100,  98,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
   99,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,  97,  95,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
   96,   0,   0,   0,   0,   0,   0,   0
};

// doc in super
void 
SoText2Set::GLRender(SoGLRenderAction *action)
{
  SoState * state = action->getState();

  if (!shouldGLRender(action)) return;

  const SoCoordinateElement *coords;
  coords = SoCoordinateElement::getInstance(state);
  assert(coords);

  SoMaterialBundle mb(action);
  mb.sendFirst();

#if COIN_MAJOR_VERSION >= 2
  SoGLLightModelElement::forceSend(state, SoLightModelElement::BASE_COLOR);
#else // COIN_MAJOR_VERSION >= 2
  SoGLLightModelElement::getInstance(state)->forceSend(SoLightModelElement::BASE_COLOR);
#endif // COIN_MAJOR_VERSION < 2
  const SbMatrix & mat = SoModelMatrixElement::get(state);

  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();

  // Set new state.
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, vpsize[0], 0, vpsize[1], -1.0f, 1.0f);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  int xpos,ypos,width;

  // Draw all strings
  for (int i = 0; i < this->strings.getNum(); i++) {
    SbVec3f nilpoint = coords->get3(i);

    mat.multVecMatrix(nilpoint, nilpoint);
    vv.projectToScreen(nilpoint, nilpoint);
    nilpoint[2] = nilpoint[2]*2 - 1;
    nilpoint[0] = nilpoint[0] * float(vpsize[0]);
    nilpoint[1] = nilpoint[1] * float(vpsize[1]);

    xpos = (int)nilpoint[0];      
    ypos = (int)nilpoint[1];

    const char *s = this->strings[i].getString();
    width = strlen(s);

    switch (this->justification.getValue()) {

    case SoText2Set::LEFT:
      xpos = (int)nilpoint[0];
      break;


    case SoText2Set::RIGHT:
      xpos = (int)nilpoint[0] - (width*8);
      break;


    case SoText2Set::CENTER:
      xpos = (int)nilpoint[0] - (width*8)/2;
      break;
    }//switch

    xpos += (int) (displacement.getValue()[0]);
    ypos -= (int) (displacement.getValue()[1]);

    for (int i2 = 0; i2 < strings[i].getLength(); i2++) {
      if (s[i2] >= 32) {
        glRasterPos3f(float(xpos), float(ypos), -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)fontdata 
                 + 12 * fontdata_isolatin1_mapping[s[i2]]);
      }// if
      xpos += 8;
    }// for
  }// for


  glPixelStorei(GL_UNPACK_ALIGNMENT,4);


  // Pop old GL state.
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}// GLRender






// doc in super
void
SoText2Set::generatePrimitives(SoAction * action)
{
  // This is supposed to be empty. There are no primitives.
}



// doc in super
void
SoText2Set::computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center)
{
  SoState * state = action->getState();

  const SoCoordinateElement *coords;
  coords = SoCoordinateElement::getInstance(state);
  assert(coords);

  const SbMatrix & mat = SoModelMatrixElement::get(state);
  SbMatrix inv = mat.inverse();
  SbVec3f screenpoint;
  const SbViewVolume &vv = SoViewVolumeElement::get(state);

  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();

  // FIXME: this only works for the default font
  SbVec2f fontsize = PRIVATE(this)->fontsize;
  fontsize[0] /= vpsize[0];
  fontsize[1] /= vpsize[1];

  // Check all texttags
  for (int i = 0; i < strings.getNum(); i++) {
    SbVec3f v0, v1, v2, v3;

    // Calculate a quad fencing strings[i]
    SbVec3f nilpoint = coords->get3(i);
    mat.multVecMatrix(nilpoint, nilpoint);
    vv.projectToScreen(nilpoint, screenpoint);

    float w = strings[i].getLength()*fontsize[0];

    float halfw = w * 0.5f;
    SbVec2f n0, n1, n2, n3;

    n0 = SbVec2f(screenpoint[0]-halfw, screenpoint[1]);
    n1 = SbVec2f(screenpoint[0]+halfw, screenpoint[1]);
    n2 = SbVec2f(screenpoint[0]+halfw, screenpoint[1] + fontsize[1]);
    n3 = SbVec2f(screenpoint[0]-halfw, screenpoint[1] + fontsize[1]);

    switch (justification.getValue()) {
    case SoText2Set::LEFT:
      n0[0] += halfw;
      n1[0] += halfw;
      n2[0] += halfw;
      n3[0] += halfw;
      break;
    case SoText2Set::RIGHT:
      n0[0] -= halfw;
      n1[0] -= halfw;
      n2[0] -= halfw;
      n3[0] -= halfw;
      break;
    case SoText2Set::CENTER:
      break;
    }//switch

    // get distance from nilpoint to camera plane
    float dist = -vv.getPlane(0.0f).getDistance(nilpoint);

    // find the four image points in the plane
    v0 = vv.getPlanePoint(dist, n0);
    v1 = vv.getPlanePoint(dist, n1);
    v2 = vv.getPlanePoint(dist, n2);
    v3 = vv.getPlanePoint(dist, n3);

    // transform back to object space
    inv.multVecMatrix(v0, v0);
    inv.multVecMatrix(v1, v1);
    inv.multVecMatrix(v2, v2);
    inv.multVecMatrix(v3, v3);

    box.extendBy(v0);
    box.extendBy(v1);
    box.extendBy(v2);
    box.extendBy(v3);
  }//for
}//computeBBox




// doc in super
void
SoText2Set::getPrimitiveCount(SoGetPrimitiveCountAction *action)
{
  if (!this->shouldPrimitiveCount(action)) return;

  action->addNumText(this->strings.getNum());
}



// doc in super
void
SoText2Set::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;
  action->setObjectSpace();

  SoState * state = action->getState();

  const SoCoordinateElement *coords;
  coords = SoCoordinateElement::getInstance(state);
  assert(coords);

  const SbMatrix & mat = SoModelMatrixElement::get(state);
  SbMatrix inv = mat.inverse();
  SbVec3f screenpoint;
  const SbViewVolume &vv = SoViewVolumeElement::get(state);

  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();

  // FIXME: this only works for the default font
  SbVec2f fontsize = PRIVATE(this)->fontsize;
  fontsize[0] /= vpsize[0];
  fontsize[1] /= vpsize[1];

  // Check all texttags
  for (int i = 0; i < strings.getNum(); i++) {
    SbVec3f v0, v1, v2, v3;



    // Calculate a quad fencing strings[i]
    SbVec3f nilpoint = coords->get3(i);
    mat.multVecMatrix(nilpoint, nilpoint);
    vv.projectToScreen(nilpoint, screenpoint);

    float w = strings[i].getLength()*PRIVATE(this)->fontsize[0];

    float halfw = w * 0.5f;
    SbVec2f n0, n1, n2, n3;

    n0 = SbVec2f(screenpoint[0]-halfw, screenpoint[1]);
    n1 = SbVec2f(screenpoint[0]+halfw, screenpoint[1]);
    n2 = SbVec2f(screenpoint[0]+halfw, screenpoint[1] + fontsize[1]);
    n3 = SbVec2f(screenpoint[0]-halfw, screenpoint[1] + fontsize[1]);

    switch (justification.getValue()) {
    case SoText2Set::LEFT:
      n0[0] += halfw;
      n1[0] += halfw;
      n2[0] += halfw;
      n3[0] += halfw;
      break;
    case SoText2Set::RIGHT:
      n0[0] -= halfw;
      n1[0] -= halfw;
      n2[0] -= halfw;
      n3[0] -= halfw;
      break;
    case SoText2Set::CENTER:
      break;
    }//switch

    // get distance from nilpoint to camera plane
    float dist = -vv.getPlane(0.0f).getDistance(nilpoint);

    // find the four image points in the plane
    v0 = vv.getPlanePoint(dist, n0);
    v1 = vv.getPlanePoint(dist, n1);
    v2 = vv.getPlanePoint(dist, n2);
    v3 = vv.getPlanePoint(dist, n3);

    // transform back to object space
    inv.multVecMatrix(v0, v0);
    inv.multVecMatrix(v1, v1);
    inv.multVecMatrix(v2, v2);
    inv.multVecMatrix(v3, v3);



    if (v0 == v1 || v0 == v3) return; // empty

    SbVec3f isect;
    SbVec3f bary;
    SbBool front;
    SbBool hit = action->intersect(v0, v1, v2, isect, bary, front);
    if (!hit) hit = action->intersect(v0, v2, v3, isect, bary, front);

    if (hit && action->isBetweenPlanes(isect)) {
      // find normalized 2D hitpoint on quad
      float h = (v3 - v0).length();
      float w = (v1 - v0).length();
      SbLine horiz(v2, v3);
      SbVec3f ptonline = horiz.getClosestPoint(isect);
      float vdist = (ptonline-isect).length();
      vdist /= h;

      SbLine vert(v0,v3);
      ptonline = vert.getClosestPoint(isect);
      float hdist = (ptonline-isect).length();
      hdist /= w;

      // find which strings and character was hit
      float fonth =  1.0f;
      int stringidx = i;

      // assumes all characters are equal size...
      float fontw = 1.0f / strings[i].getLength();

      // find the character
      int charidx = -1;
      int strlength = this->strings[stringidx].getLength();
      switch (this->justification.getValue()) {
      case LEFT:
        charidx = int(hdist / fontw);
        break;
      case RIGHT:
        charidx = (strlength-1) - int((1.0f-hdist)/fontw);
        break;
      case CENTER:
        {
          float strstart = 0.5f - fontw*float(strlength)*0.5f;
          charidx = int((hdist-strstart) / fontw);
        }
        break;
      default:
        assert(0 && "unknown justification");
      }

      if (charidx >= 0 && charidx < strlength) { // we have a hit!
        SoPickedPoint * pp = action->addIntersection(isect);
        if (pp) {
          SoTextDetail * detail = new SoTextDetail;
          detail->setStringIndex(stringidx);
          detail->setCharacterIndex(charidx);
          pp->setDetail(detail, this);
          pp->setMaterialIndex(0);
          pp->setObjectNormal(SbVec3f(0.0f, 0.0f, 1.0f));

          return;
        }
      }
    }

  }//for


}//rayPick



