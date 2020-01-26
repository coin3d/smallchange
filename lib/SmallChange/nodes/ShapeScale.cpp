/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

/*!
  \class ShapeScale ShapeScale.h
  \brief The ShapeScale class is used for scaling a shape based on projected size.

  This nodekit can be inserted in your scene graph to add for instance
  3D markers that will be of a constant projected size.

  The marker shape is stored in the "shape" part. Any kind of node
  can be used, even group nodes with several shapes, but the marker
  shape should be approximately of unit size, and with a center
  position in (0, 0, 0).
*/

/*!
  \var SoSFBool ShapeScale::active

  Turns the scaling on/off. Default value is TRUE.
*/

/*!
  \var SoSFFloat ShapeScale::projectedSize

  The requested projected size of the shape. Default value is 5.0.
*/

/*!
  \var SoSFFloat ShapeScale::minScale

  The minimum scale factor applied to the shape. Default value is 0.0.
*/

/*!
  \var SoSFFloat ShapeScale::maxScale

  The maximum scale factor applied to the shape. Default value is FLT_MAX.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ShapeScale.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/SbRotation.h>
#include <Inventor/caches/SoCache.h>
#include <cfloat>

SO_KIT_SOURCE(ShapeScale);

ShapeScale::ShapeScale(void)
{
  SO_KIT_CONSTRUCTOR(ShapeScale);

  SO_KIT_ADD_FIELD(active, (TRUE));
  SO_KIT_ADD_FIELD(projectedSize, (5.0f));
  SO_KIT_ADD_FIELD(minScale, (0.0f));
  SO_KIT_ADD_FIELD(maxScale, (FLT_MAX));

#ifndef __COIN__
#error catalog setup probably not compatible with non-Coin Inventor implementation
#endif // !__COIN__

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(callback, SoCallback, FALSE, topSeparator, shape, FALSE);
  SO_KIT_ADD_CATALOG_ABSTRACT_ENTRY(shape, SoNode, SoCube, TRUE, topSeparator, "", TRUE);

  SO_KIT_INIT_INSTANCE();

  SoCallback * cb = static_cast<SoCallback*> (this->getAnyPart("callback", TRUE));
  cb->setCallback(scaleCB, this);
}

ShapeScale::~ShapeScale()
{
}

// doc in superclass
void
ShapeScale::initClass(void)
{
  SO_KIT_INIT_CLASS(ShapeScale, SoBaseKit, "BaseKit");
}

void
ShapeScale::scaleCB(void * closure, SoAction * action)
{
  ShapeScale * thisp = (ShapeScale*) closure;
  if (!thisp->active.getValue()) return;

  // if the current action doesn't support all elements we need we'll just return
  SoState * state = action->getState();
  if (!state->isElementEnabled(SoViewportRegionElement::getClassStackIndex()) ||
      !state->isElementEnabled(SoViewVolumeElement::getClassStackIndex()) ||
      !state->isElementEnabled(SoModelMatrixElement::getClassStackIndex())) return;

  SbBox3f bbox(-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f);
  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  SbVec3f center(0.0f, 0.0f, 0.0f);
  const float nsize = thisp->projectedSize.getValue() / float(vp.getViewportSizePixels()[0]);
  const SbMatrix & mm = SoModelMatrixElement::get(state);
  mm.multVecMatrix(center, center);
  float scalefactor = vv.getWorldToScreenScale(center, nsize);
  if (scalefactor < thisp->minScale.getValue()) {
      scalefactor = thisp->minScale.getValue();
  }
  else if (scalefactor > thisp->maxScale.getValue()) {
    scalefactor = thisp->maxScale.getValue();
  }

  SbVec3f t;
  SbRotation r;
  SbVec3f s;
  SbRotation so;
  mm.getTransform(t, r, s, so);

  SbVec3f scale(scalefactor / s[0], scalefactor / s[1], scalefactor / s[2]);
  SoModelMatrixElement::scaleBy(state, thisp, scale);
}
