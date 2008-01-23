/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SmallChange with software that can not be combined with the
 *  GNU GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org> for  more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no>.
 *
\**************************************************************************/

/*!
  \class SmTextureText2 SmallChange/nodes/SmTextureText2.h
  \brief The SmTextureText2 node is used for fast text rendering
  
  This node can render ~200k strings / second. It pregenerates a font
  texture which is used for rendering the text instead of using the
  glBitmap()/glPixmap() method.

  It can be used in two modes. If multiple 'position' values are specified,
  one string is rendered at each position. If a single 'position' value is supplied,
  the strings will be rendered vertically like the SoText2 node.

  This node also supports material binding and will pick materials from the
  state whenever the material binding != OVERALL.

*/

#include "SmTextureText2.h"
#include <Inventor/misc/SoState.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/system/gl.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/elements/SoGLTextureImageElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoShapeStyleElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoMaterialBindingElement.h>
#include <Inventor/misc/SoGLImage.h>
#include <Inventor/SbPlane.h>

#if COIN_DEBUG
#include <Inventor/errors/SoDebugError.h>
#endif // COIN_DEBUG

#include <limits.h>
#include <string.h>

static unsigned char texture_fontdata[][12] = {
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
  //  {  0,  0,  0,  0,  0, 34, 34, 20, 20,  8,  8,  0 }, // ^ (rotate)
  {  0,  0,  0,127,  1,  1, 65,225, 65, 65,127,  0 },  
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
  {  0,  0, 16, 40, 72,  4,  4,  2,  1,  0,  0,  0 },  // ~ v
  // iso-latin-1 norwegian letters
  {  0,  0, 59, 76, 72, 62,  9, 73, 54,  0,  0,  0 }, // ae
  {  0,  0, 92, 34, 50, 42, 42, 38, 29,  0,  0,  0 }, // oe
  {  0,  0, 29, 34, 34, 30,  2, 34, 28,  8, 20,  8 }, // aa
  {  0,  0, 79, 72, 72, 72,127, 72, 72, 72, 63,  0 }, // AE
  {  0,  0, 44, 18, 41, 41, 41, 37, 37, 18, 13,  0 }, // OE
  {  0,  0, 33, 33, 33, 63, 18, 18, 12, 12, 18, 12 }  // AA
};

// map from iso-latin1 to font data array index
static int texturetext_isolatin1_mapping[] = {
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

// FIXME: support more fonts
static unsigned char * texturetext_texture = NULL;
static SoGLImage * texturetext_glimage = NULL;
static int texturetext_numglyphs = 0; 
static SbVec2f * texturetext_glyphcoords = NULL;

/*!
*/

/*!
*/

/*!
*/

SO_NODE_SOURCE(SmTextureText2);

/*!
  Constructor.
*/
SmTextureText2::SmTextureText2()
{
  SO_NODE_CONSTRUCTOR(SmTextureText2);

  SO_NODE_ADD_FIELD(string, (""));
  SO_NODE_ADD_FIELD(justification, (CENTER));
  SO_NODE_ADD_FIELD(verticalJustification, (BOTTOM));
  SO_NODE_ADD_FIELD(maxRange, (-1.0f));
  SO_NODE_ADD_FIELD(position, (0.0f, 0.0f, 0.0f));

  SO_NODE_DEFINE_ENUM_VALUE(Justification, CENTER);
  SO_NODE_DEFINE_ENUM_VALUE(Justification, LEFT);
  SO_NODE_DEFINE_ENUM_VALUE(Justification, RIGHT);

  SO_NODE_DEFINE_ENUM_VALUE(VerticalJustification, TOP);
  SO_NODE_DEFINE_ENUM_VALUE(VerticalJustification, BOTTOM);
  SO_NODE_DEFINE_ENUM_VALUE(VerticalJustification, VCENTER);

  SO_NODE_SET_SF_ENUM_TYPE(justification, Justification);
  SO_NODE_SET_SF_ENUM_TYPE(verticalJustification, VerticalJustification);
}

/*!
  Destructor.
*/
SmTextureText2::~SmTextureText2()
{
}

// doc from parent
void
SmTextureText2::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(SmTextureText2, SoShape, "Shape");

    texturetext_texture = create_texture();
    texturetext_glimage = new SoGLImage;
    texturetext_glimage->setData(texturetext_texture,
                                 SbVec2s(256,128),
                                 2,
                                 SoGLImage::CLAMP,
                                 SoGLImage::CLAMP,
                                 0.0f);
  }
}

// doc from parent
void
SmTextureText2::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  // never cull this node. We do quick culling in the render method
  SoCacheElement::invalidate(action->getState());

  // this boundingbox will _not_ be 100% correct. We just supply an
  // estimate to avoid using lots of processing for calculating the
  // boundingbox.  FIXME: make it configurable if the bbox should be
  // accurate or not
  const int num = this->position.getNum();
  const SbVec3f * pos = this->position.getValues(0);

  for (int i = 0; i < num; i++) {
    box.extendBy(pos[i]);
  }
  center = box.getCenter();
}

// doc from parent
void
SmTextureText2::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  state->push();
  SoCacheElement::invalidate(state);
  
  SoShapeStyleElement::setTransparentMaterial(state, TRUE);
  
  if (!this->shouldGLRender(action)) {
    state->pop();
    return;
  }
  
  SoTextureQualityElement::set(state, 0.0f);
  SoLightModelElement::set(state, SoLightModelElement::BASE_COLOR);
  SoGLTextureImageElement::set(state, this,
                               texturetext_glimage, 
                               SoTextureImageElement::MODULATE,
                               SbColor(1.0f, 1.0f, 1.0f));
  SoGLTextureEnabledElement::set(state, this, TRUE);
  
  SoMaterialBundle mb(action);
  mb.sendFirst(); // make sure we have the correct material

  SbMatrix modelmatrix = SoModelMatrixElement::get(state); 
  SbMatrix inv = modelmatrix.inverse();

  SbMatrix normalize(0.5f, 0.0f, 0.0f, 0.0f,
                     0.0f, 0.5f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     0.5f, 0.5f, 0.0f, 1.0f);
  SbMatrix projmatrix = 
    modelmatrix *
    SoViewingMatrixElement::get(state) *
    SoProjectionMatrixElement::get(state) *
    normalize;

  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  const SbVec2s vpsize = vp.getViewportSizePixels();
  const int num = this->position.getNum();
  const SbVec3f * pos = this->position.getValues(0);
  const int numstring = this->string.getNum();
  const SbString * s = this->string.getValues(0);

  // Set up new view volume
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, vpsize[0], 0, vpsize[1], -1.0f, 1.0f);

  SbBool perpart = 
    SoMaterialBindingElement::get(state) != 
    SoMaterialBindingElement::OVERALL;

  if (num > 1) {
    SbBool render_border = FALSE; // just for testing
    if (render_border) {
      glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_POLYGON_BIT);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDisable(GL_TEXTURE_2D);
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      for (int i = 0; i < num; i++) {
        this->renderBorder(&s[SbMin(i, numstring-1)], 1, 
                           pos[i],
                           vv,
                           vp,
                           projmatrix,
                           modelmatrix,
                           inv);
      }
      glPopAttrib();
    }

    for (int i = 0; i < num; i++) {
      if (perpart) {
        mb.send(i, FALSE);
      }
      this->renderString(&s[SbMin(i, numstring-1)], 1, 
                         pos[i],
                         vv,
                         vp,
                         projmatrix,
                         modelmatrix,
                         inv);
    }
  }
  else {
    this->renderString(&s[0], 
                       numstring, 
                       num > 0 ? pos[0] : SbVec3f(0.0f, 0.0f, 0.0f),
                       vv,
                       vp,
                       projmatrix,
                       modelmatrix,
                       inv);
  }

  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  state->pop();
}

void 
SmTextureText2::rayPick(SoRayPickAction * action)
{
  // we don't want to pick on this text node
}

// doc from parent
void
SmTextureText2::getPrimitiveCount(SoGetPrimitiveCountAction *action)
{
  if (!this->shouldPrimitiveCount(action)) return;
  action->addNumText(this->position.getNum());
}

// doc from parent
void
SmTextureText2::generatePrimitives(SoAction *action)
{
  // no primitives to generate
}

void 
SmTextureText2::renderBorder(const SbString * s, 
                             const int numstring,
                             const SbVec3f & pos,
                             const SbViewVolume & vv,
                             const SbViewportRegion & vp,
                             const SbMatrix & projmatrix,
                             const SbMatrix & modelmatrix,
                             const SbMatrix & invmodelmatrix)
{
  // get distance from pos to camera plane
  SbVec3f tmp;
  modelmatrix.multVecMatrix(pos, tmp);
  float dist = -vv.getPlane(0.0f).getDistance(tmp);
  if (dist < vv.getNearDist()) return;
  
  int i;
  SbVec2s vpsize = vp.getViewportSizePixels();
  
  SbVec3f screenpoint;
  projmatrix.multVecMatrix(pos, screenpoint);

  glBegin(GL_QUADS);
  for (i = 0; i < numstring; i++) {
    int len = s[i].getLength();
    if (len == 0) continue;
    
    const unsigned char * sptr = (const unsigned char *) s[i].getString();
    
    int xmin = -4;
    int xmax = len * 8 + 4;
    int ymin = -i*14;
    int ymax = -i*14 + 16;
    
    if (xmin >= vpsize[0] ||
        xmax < 0 ||
        ymin >= vpsize[0] ||
        ymax < 0) continue;
    
    SbVec2s n0,n1,n2,n3;
    SbVec2s sp((short) (screenpoint[0] * vpsize[0]), (short)(screenpoint[1] * vpsize[1]));
    
    n0 = SbVec2s(sp[0] + xmin,
                 sp[1] + ymax);
    n1 = SbVec2s(sp[0] + xmax,
                 sp[1] + ymax);
    n2 = SbVec2s(sp[0] + xmax,
                 sp[1] + ymin);
    n3 = SbVec2s(sp[0] + xmin,
                 sp[1] + ymin);
    
    short w = n1[0]-n0[0];
    short halfw = w / 2;

    switch (this->justification.getValue()) {
    case SmTextureText2::LEFT:
      break;
    case SmTextureText2::RIGHT:
      n0[0] -= w;
      n1[0] -= w;
      n2[0] -= w;
      n3[0] -= w;
      break;
    case SmTextureText2::CENTER:
      n0[0] -= halfw;
      n1[0] -= halfw;
      n2[0] -= halfw;
      n3[0] -= halfw;
      break;
    default:
      assert(0 && "unknown alignment");
      break;
    }

    short h = n0[1] - n3[1];
    short halfh = h / 2;

    switch (this->verticalJustification.getValue()) {
    case SmTextureText2::BOTTOM:
      break;
    case SmTextureText2::TOP:
      n0[1] -= h;
      n1[1] -= h;
      n2[1] -= h;
      n3[1] -= h;
      break;
    case SmTextureText2::VCENTER:
      n0[1] -= halfh;
      n1[1] -= halfh;
      n2[1] -= halfh;
      n3[1] -= halfh;
      break;
    default:
      assert(0 && "unknown alignment");
      break;
    }

    SbVec3f c0(n0[0], n0[1], -screenpoint[2]);
    SbVec3f c1(n1[0], n1[1], -screenpoint[2]);
    SbVec3f c2(n2[0], n2[1], -screenpoint[2]);
    SbVec3f c3(n3[0], n3[1], -screenpoint[2]);

    glVertex3fv(c0.getValue());
    glVertex3fv(c1.getValue());
    glVertex3fv(c2.getValue());
    glVertex3fv(c3.getValue());    
  }
  glEnd();
}

void 
SmTextureText2::renderString(const SbString * s, 
                            const int numstring,
                             const SbVec3f & pos,
                             const SbViewVolume & vv,
                             const SbViewportRegion & vp,
                             const SbMatrix & projmatrix,
                             const SbMatrix & modelmatrix,
                             const SbMatrix & invmodelmatrix)
{
  // get distance from pos to camera plane
  SbVec3f tmp;
  modelmatrix.multVecMatrix(pos, tmp);
  float dist = -vv.getPlane(0.0f).getDistance(tmp);
  if (dist < vv.getNearDist()) return;
  
  float maxr = this->maxRange.getValue();
  if (maxr > 0.0f && dist > maxr) return; 

  int i;
  SbVec2s vpsize = vp.getViewportSizePixels();
  
  SbVec3f screenpoint;
  projmatrix.multVecMatrix(pos, screenpoint);

  glBegin(GL_QUADS);
  for (i = 0; i < numstring; i++) {
    int len = s[i].getLength();
    if (len == 0) continue;
    
    const unsigned char * sptr = (const unsigned char *) s[i].getString();
    
    int xmin = -4;
    int xmax = len * 8 + 4;
    int ymin = -i*14;
    int ymax = -i*14 + 16;
    
    if (xmin >= vpsize[0] ||
        xmax < 0 ||
        ymin >= vpsize[0] ||
        ymax < 0) continue;
    
    SbVec2s n0,n1;
    SbVec2s sp((short) (screenpoint[0] * vpsize[0]), (short)(screenpoint[1] * vpsize[1]));
    
    n0 = SbVec2s(sp[0] + xmin,
                 sp[1] + ymin);
    n1 = SbVec2s(sp[0] + xmax,
                 sp[1] + ymax);
    
    short w = n1[0]-n0[0];
    short halfw = w / 2;

    switch (this->justification.getValue()) {
    case SmTextureText2::LEFT:
      break;
    case SmTextureText2::RIGHT:
      n0[0] -= (w - 8);
      n1[0] -= (w - 8);
      break;
    case SmTextureText2::CENTER:
      n0[0] -= (halfw - 4);
      n1[0] -= (halfw - 4);
      break;
    default:
      assert(0 && "unknown alignment");
      break;
    }
    short h = n0[1] - n1[1];
    short halfh = h / 2;

    switch (this->verticalJustification.getValue()) {
    case SmTextureText2::BOTTOM:
      break;
    case SmTextureText2::TOP:
      n0[1] -= h;
      n1[1] -= h;
      break;
    case SmTextureText2::VCENTER:
      n0[1] -= halfh;
      n1[1] -= halfh;
      break;
    default:
      assert(0 && "unknown alignment");
      break;
    }

    
    for (int j = 0; j < len; j++) {
      const int gidx = texturetext_isolatin1_mapping[sptr[j]] * 4;
      float n00 = n0[0]; // compile fix for gcc 3.2.3 (20070518 frodo)
      SbVec3f c0(float(n00 + j*8),     float(n1[1]), -screenpoint[2]);
      SbVec3f c1(float(n00 + (j+2)*8), float(n1[1]), -screenpoint[2]);
      SbVec3f c2(float(n00 + (j+2)*8), float(n0[1]), -screenpoint[2]);
      SbVec3f c3(float(n00 + j*8),     float(n0[1]), -screenpoint[2]);

      glTexCoord2fv(texturetext_glyphcoords[gidx].getValue());
      glVertex3fv(c0.getValue());
      glTexCoord2fv(texturetext_glyphcoords[gidx+1].getValue());
      glVertex3fv(c1.getValue());
      glTexCoord2fv(texturetext_glyphcoords[gidx+2].getValue());
      glVertex3fv(c2.getValue());
      glTexCoord2fv(texturetext_glyphcoords[gidx+3].getValue());
      glVertex3fv(c3.getValue());

    }
  }
  glEnd();
}

void 
SmTextureText2::oldRenderString(const SbString * s, 
                                const int numstring,
                                const SbVec3f & pos,
                                const SbViewVolume & vv,
                                const SbViewportRegion & vp,
                                const SbMatrix & projmatrix,
                                const SbMatrix & modelmatrix,
                                const SbMatrix & invmodelmatrix)
{
  // get distance from pos to camera plane
  SbVec3f tmp;
  modelmatrix.multVecMatrix(pos, tmp);
  float dist = -vv.getPlane(0.0f).getDistance(tmp);
  if (dist < vv.getNearDist()) return;
  
  int i;
  SbVec2s vpsize = vp.getViewportSizePixels();
  
  SbVec3f screenpoint;
  projmatrix.multVecMatrix(pos, screenpoint);

  // FIXME: set up an orthographic view volume and do the rendering
  // there instead of projecting back to the current object space.
  glBegin(GL_QUADS);
  for (i = 0; i < numstring; i++) {
    int len = s[i].getLength();
    if (len == 0) continue;
    
    const unsigned char * sptr = (const unsigned char *) s[i].getString();

    int xmin = 0;
    int xmax = len * 9;
    int ymin = i*14 + 14;
    int ymax = i*14;
    
    SbVec2f n0,n1,n2,n3;
    SbVec2s sp((short) (screenpoint[0] * vpsize[0]), (short)(screenpoint[1] * vpsize[1]));
    
    n0 = SbVec2f(float(sp[0] + xmin)/float(vpsize[0]), 
                 float(sp[1] + ymax)/float(vpsize[1]));
    n1 = SbVec2f(float(sp[0] + xmax)/float(vpsize[0]), 
                 float(sp[1] + ymax)/float(vpsize[1]));
    n2 = SbVec2f(float(sp[0] + xmax)/float(vpsize[0]), 
                 float(sp[1] + ymin)/float(vpsize[1]));
    n3 = SbVec2f(float(sp[0] + xmin)/float(vpsize[0]), 
                 float(sp[1] + ymin)/float(vpsize[1]));
    
    n1[0] += 0.1f / float(vpsize[0]);
    n2[0] += 0.1f / float(vpsize[0]);
    
    n2[1] += 0.1f / float(vpsize[1]);
    n3[1] += 0.1f / float(vpsize[1]);


    float w = n1[0]-n0[0];
    float halfw = w*0.5f;
    switch (this->justification.getValue()) {
    case SmTextureText2::LEFT:
      break;
    case SmTextureText2::RIGHT:
      n0[0] -= w;
      n1[0] -= w;
      n2[0] -= w;
      n3[0] -= w;
      break;
    case SmTextureText2::CENTER:
      n0[0] -= halfw;
      n1[0] -= halfw;
      n2[0] -= halfw;
      n3[0] -= halfw;
      break;
    default:
      assert(0 && "unknown alignment");
      break;
    }
    
    SbVec3f v0,v1,v2,v3;
    // find the four image points in the plane
    v0 = vv.getPlanePoint(dist, n0);
    v1 = vv.getPlanePoint(dist, n1);
    v2 = vv.getPlanePoint(dist, n2);
    v3 = vv.getPlanePoint(dist, n3);
    
    // transform back to object space
    invmodelmatrix.multVecMatrix(v0, v0);
    invmodelmatrix.multVecMatrix(v1, v1);
    invmodelmatrix.multVecMatrix(v2, v2);
    invmodelmatrix.multVecMatrix(v3, v3);
    
    float invflen = 1.0f / float(len);
    
    for (int j = 0; j < len; j++) {
      const int gidx = texturetext_isolatin1_mapping[sptr[j]] * 4;
      
      SbVec3f c0 = v0 + (v1-v0) * float(j) * invflen;
      SbVec3f c1 = v0 + (v1-v0) * float(j+1) * invflen;
      SbVec3f c2 = v3 + (v2-v3) * float(j+1) * invflen;
      SbVec3f c3 = v3 + (v2-v3) * float(j) * invflen;

      glTexCoord2fv(texturetext_glyphcoords[gidx].getValue());
      glVertex3fv(c0.getValue());
      glTexCoord2fv(texturetext_glyphcoords[gidx+1].getValue());
      glVertex3fv(c1.getValue());
      glTexCoord2fv(texturetext_glyphcoords[gidx+2].getValue());
      glVertex3fv(c2.getValue());
      glTexCoord2fv(texturetext_glyphcoords[gidx+3].getValue());
      glVertex3fv(c3.getValue());

    }
  }
  glEnd();
}

unsigned char * 
SmTextureText2::create_texture(void)
{
  int num = sizeof(texture_fontdata) / 12;
  texturetext_numglyphs = num;
  texturetext_glyphcoords = new SbVec2f[num*4];
  
  SbVec2f * tc = texturetext_glyphcoords;

  assert(num < 128);
  
  unsigned char * ptr = new unsigned char[256*128*2];
  memset(ptr, 0, 256*128*2);

  enum Effect { NORMAL, BORDER, DROP_SHADOW, BOLD, BOLD_DROPSHADOW, ANTIALIAS };

  Effect effect = BORDER;
  //Effect effect = DROP_SHADOW;
  //Effect effect = BOLD_DROPSHADOW;
  //Effect effect = ANTIALIAS;

  const int DROP_SHADOW_DIST = 2;

  int x,y;
  for (int i = 0; i < num; i++) {
    get_text_pixmap_position(i, x, y);

    if (effect == NORMAL) {
      render_text(ptr, i, x+4, y+2, 255, 255);
    }
    else if (effect == BORDER) {
      // move text to the middle of the block
      x += 4;
      y += 2;

      render_text(ptr, i, x-1, y, 0, 255);
      render_text(ptr, i, x+1, y, 0, 255);
      render_text(ptr, i, x, y-1, 0, 255);
      render_text(ptr, i, x, y+1, 0, 255);
      
      render_text(ptr, i, x-1, y+1, 0, 255);
      render_text(ptr, i, x+1, y+1, 0, 255);
      render_text(ptr, i, x-1, y-1, 0, 255);
      render_text(ptr, i, x+1, y-1, 0, 255);
      
      render_text(ptr, i, x, y, 255, 255);

      // move it back for texture coords
      x -= 4;
      y -= 2;

    }
    else if (effect == ANTIALIAS) { // not very impressive effect :)
      // move text to the middle of the block
      x += 4;
      y += 2;

      render_text(ptr, i, x-1, y, 255, 64);
      render_text(ptr, i, x+1, y, 255, 64);
      render_text(ptr, i, x, y-1, 255, 64);
      render_text(ptr, i, x, y+1, 255, 64);
      
      render_text(ptr, i, x-1, y+1, 255, 64);
      render_text(ptr, i, x+1, y+1, 255, 64);
      render_text(ptr, i, x-1, y-1, 255, 64);
      render_text(ptr, i, x+1, y-1, 255, 64);
      
      render_text(ptr, i, x, y, 255, 255);

      // move it back for texture coords
      x -= 4;
      y -= 2;

    }
    else if (effect == DROP_SHADOW) {
      x += 2;

      render_text(ptr, i, x+DROP_SHADOW_DIST, y+DROP_SHADOW_DIST, 0, 255);
      render_text(ptr, i, x, y, 255, 255);

      x -= 2;
    }
    else if (effect == BOLD) {
      x += 4;
      y += 2;

      render_text(ptr, i, x, y, 255, 255);
      render_text(ptr, i, x+1, y, 255, 255);


      x -= 4;
      y -= 2;      
    }

    else if (effect == BOLD_DROPSHADOW) {
      x += 2;
      
      render_text(ptr, i, x+DROP_SHADOW_DIST, y+DROP_SHADOW_DIST, 0, 255);
      render_text(ptr, i, x+DROP_SHADOW_DIST+1, y+DROP_SHADOW_DIST, 0, 255);

      render_text(ptr, i, x, y, 255, 255);
      render_text(ptr, i, x+1, y, 255, 255);

      x -= 2;
    }

    float x0 = float(x) / float(256);
    float y0 = float(y) / float(128);
    
    float x1 = float(x+16) / float(256);
    float y1 = float(y+16) / float(128);
    
    *tc++ = SbVec2f(x0, y0);
    *tc++ = SbVec2f(x1, y0);
    *tc++ = SbVec2f(x1, y1);
    *tc++ = SbVec2f(x0, y1);
 
  }
  
  return ptr;
}

void 
SmTextureText2::get_text_pixmap_position(const int idx, int & x, int & y)
{
  y = idx / 16; // 16 chars per line
  x = idx % 16;

  // each char uses 16x16 pixels
  x *= 16;
  y *= 16;
}

void
SmTextureText2::render_text(unsigned char * dst,
                            const int idx, const int x, const int y,
                            const unsigned char value,
                            const unsigned char alpha)
{
  unsigned char * src = texture_fontdata[idx];
  
  for (int sy = 0; sy < 12; sy++) {
    for (int sx = 0; sx  < 8; sx++) {
      if (src[sy] & (0x80 >> sx)) {
        int dstidx = (y+(11-sy))*256*2 + (x+sx)*2;
        dst[dstidx] = value;
        dst[dstidx+1] = alpha;
      }
    } 
  }
}
