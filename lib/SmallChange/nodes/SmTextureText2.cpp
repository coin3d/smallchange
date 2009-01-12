/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2009 by Systems in Motion.  All rights reserved.
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
#include "SmTextureFont.h"
#include "SmTextureText2Collector.h"
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
#include <Inventor/elements/SoGLTextureCoordinateElement.h>
#include <Inventor/misc/SoGLImage.h>
#include <Inventor/SbPlane.h>
#include <Inventor/C/tidbits.h>

#if COIN_DEBUG
#include <Inventor/errors/SoDebugError.h>
#endif // COIN_DEBUG

#include <climits>
#include <cstring>


SO_NODE_SOURCE(SmTextureText2);

/*!
  Constructor.
*/
SmTextureText2::SmTextureText2()
{
  SO_NODE_CONSTRUCTOR(SmTextureText2);

  SO_NODE_ADD_FIELD(string, (""));
  SO_NODE_ADD_FIELD(justification, (LEFT));
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
  if (getClassTypeId() == SoType::badType()) {
    SO_NODE_INIT_CLASS(SmTextureText2, SoShape, "Shape");
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
  if ((this->string.getNum() == 0) ||
      (this->string.getNum() == 1 && this->string[0] == "")) return;

  SoState * state = action->getState();
  SbBool perpart =
    SoMaterialBindingElement::get(state) !=
    SoMaterialBindingElement::OVERALL;

  if ((this->string.getNum() == this->position.getNum()) &&
      SmTextureText2CollectorElement::isCollecting(state)) {
    SbMatrix modelmatrix = SoModelMatrixElement::get(state);
    SbVec3f pos;

    SbColor4f col(SoLazyElement::getDiffuse(state, 0),
                     1.0f - SoLazyElement::getTransparency(state, 0));

    for (int i = 0; i < this->string.getNum(); i++) {
      if (perpart && i > 0) {
        col = SbColor4f(SoLazyElement::getDiffuse(state, i),
                        1.0f - SoLazyElement::getTransparency(state, i));
      }
      modelmatrix.multVecMatrix(this->position[i], pos);
      SmTextureText2CollectorElement::add(state,
                                          this->string[i],
                                          SmTextureFontElement::get(state),
                                          pos,
                                          this->maxRange.getValue(),
                                          col,
                                          static_cast<Justification>(
                                     this->justification.getValue()
                                                             ),
                                          static_cast<VerticalJustification>(
                                          this->verticalJustification.getValue()
                                                                    )
                                                                    );
    }

    // invalidate caches to make sure this node is traversed every frame.
    SoCacheElement::invalidate(state);
    return;
  }

  SmTextureFontBundle bundle(action);
  SoCacheElement::invalidate(state);

  if (!this->shouldGLRender(action)) {
    return;
  }

  // set up my font texture
  SoLightModelElement::set(state, SoLightModelElement::BASE_COLOR);
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
  glPushAttrib(GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LEQUAL);

  if (num > 1) {
    for (int i = 0; i < num; i++) {
      if (perpart) {
        mb.send(i, FALSE);
      }
      this->renderString(bundle,
                         &s[SbMin(i, numstring-1)], 1,
                         pos[i],
                         vv,
                         vp,
                         projmatrix,
                         modelmatrix,
                         inv);
    }
  }
  else {
    this->renderString(bundle,
                       &s[0],
                       numstring,
                       num > 0 ? pos[0] : SbVec3f(0.0f, 0.0f, 0.0f),
                       vv,
                       vp,
                       projmatrix,
                       modelmatrix,
                       inv);
  }
  glPopAttrib();
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
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

// convert normalized screen space coordinates into pixel screen
// space. Values that are too far from the viewport are culled.
static SbBool get_screenpoint_pixels(const SbVec3f & screenpoint,
                                     const SbVec2s & vpsize,
                                     SbVec2s & sp)
{
  float sx = screenpoint[0] * vpsize[0];
  float sy = screenpoint[1] * vpsize[1];

  // FIXME: just assume we won't have strings larger than 3000 pixels
  const float limit = 3000.0f;

  if ((sx > -limit) &&
      (sy > -limit) &&
      (sx < limit) &&
      (sy < limit)) {
    sp = SbVec2s(static_cast<short>(sx), static_cast<short>(sy));
    return TRUE;
  }
  return FALSE;
}

void
SmTextureText2::renderString(const SmTextureFontBundle & bundle,
                             const SbString * s,
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
  if (dist <= vv.getNearDist()) return;
  if (dist > (vv.getNearDist() + vv.getDepth())) return;

  float maxr = this->maxRange.getValue();
  if (maxr > 0.0f && dist > maxr) return;

  int i;
  SbVec2s vpsize = vp.getViewportSizePixels();

  SbVec3f screenpoint;
  projmatrix.multVecMatrix(pos, screenpoint);

  int xmin = 0;
  int ymax = bundle.getAscent();
  int ymin = ymax - (numstring * bundle.height() * bundle.getLeading());
  ymin += bundle.getLeading();

  short h = ymax - ymin;
  short halfh = h / 2;

  switch (this->verticalJustification.getValue()) {
  case SmTextureText2::BOTTOM:
    break;
  case SmTextureText2::TOP:
    ymin -= bundle.getAscent();
    ymax -= bundle.getAscent();
    break;
  case SmTextureText2::VCENTER:
    ymin -= halfh;
    ymax -= halfh;
    break;
  default:
    assert(0 && "unknown alignment");
    break;
  }
  SbList <int> widthlist;

  for (i = 0; i < numstring; i++) {
    widthlist.append(bundle.stringWidth(s[i]));
  }

  bundle.begin();
  for (i = 0; i < numstring; i++) {

    int len = s[i].getLength();
    if (len == 0) continue;

    SbVec2s sp;
    if (!get_screenpoint_pixels(screenpoint, vpsize, sp)) continue;

    SbVec2s n0 = SbVec2s(sp[0] + xmin,
                         sp[1] + ymax - (i+1)*bundle.height());

    short w = static_cast<short>(widthlist[i]);
    short halfw = w / 2;

    switch (this->justification.getValue()) {
    case SmTextureText2::LEFT:
      break;
    case SmTextureText2::RIGHT:
      n0[0] -= w;
      break;
    case SmTextureText2::CENTER:
      n0[0] -= halfw;
      break;
    default:
      assert(0 && "unknown alignment");
      break;
    }
    bundle.renderString(s[i], SbVec3f(n0[0], n0[1], screenpoint[2]));
  }
  bundle.end();
}
