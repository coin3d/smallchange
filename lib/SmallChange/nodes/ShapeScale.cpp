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
  \class ShapeScale ShapeScale.h
  \brief The ShapeScale class ...

  FIXME: class doc
*/

#include "ShapeScale.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

SO_KIT_SOURCE(ShapeScale);

ShapeScale::ShapeScale(void) 
{
  SO_KIT_CONSTRUCTOR(ShapeScale);

  SO_KIT_ADD_FIELD(active, (TRUE));
  SO_KIT_ADD_FIELD(projectedSize, (5.0f));  
  SO_KIT_ADD_FIELD(threadSafe, (FALSE));
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(scale, SoScale, FALSE, topSeparator, shape, FALSE);
  SO_KIT_ADD_CATALOG_ABSTRACT_ENTRY(shape, SoNode, SoCube, TRUE, topSeparator, "", TRUE);

  SO_KIT_INIT_INSTANCE();
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

static void
update_scale(SoScale * scale, const SbVec3f & v)
{
  if (scale->scaleFactor.getValue() != v) {
    scale->scaleFactor = v;
  }
}

void
ShapeScale::preRender(SoAction * action)
{
  SoState * state = action->getState();

  SoNode * shape = (SoNode*) this->getAnyPart(SbName("shape"), FALSE);
  assert(shape);

  SoScale * scale = (SoScale*) this->getAnyPart(SbName("scale"), TRUE);
  if (!this->active.getValue()) {
    update_scale(scale, SbVec3f(1.0f, 1.0f, 1.0f));
  }
  else {
    const SbViewportRegion & vp = SoViewportRegionElement::get(state);
    const SbViewVolume & vv = SoViewVolumeElement::get(state);
#if 0 // obsoleted
    SbBox3f bbox;
    SbVec3f center;
    shape->computeBBox(action, bbox, center);
    SbVec3f size = bbox.getMax() - bbox.getMin();
    float maxsize = size[0];
    if (size[1] > maxsize) maxsize = size[1];
    if (size[2] > maxsize) maxsize = size[2];
#else // obsoleted
    SbVec3f center(0.0f, 0.0f, 0.0f);
#endif
    const float nsize = this->projectedSize.getValue() / float(vp.getViewportSizePixels()[1]);
    SoModelMatrixElement::get(state).multVecMatrix(center, center);
    const float scalefactor = vv.getWorldToScreenScale(center, nsize);
    update_scale(scale, SbVec3f(scalefactor, scalefactor, scalefactor));
  }
}

// Overridden to (re)initialize image and other data before rendering.
void 
ShapeScale::GLRender(SoGLRenderAction * action)
{
  if (!this->threadSafe.getValue()) this->preRender(action);
  inherited::GLRender(action);
}
