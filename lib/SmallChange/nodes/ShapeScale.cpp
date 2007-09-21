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
  \brief The ShapeScale class is used for scaling a shape based on projected size.

  This nodekit can be inserted in your scene graph to add for instance
  3D markers that will be of a constant projected size.

  The marker shape is stored in the "shape" part. Any kind of node
  can be used, even group nodes with several shapes, but the marker
  shape should be approximately of unit size, and with a center 
  position in (0, 0, 0).
*/

/*!
  \var SoSFFloat ShapeScale::active
  
  Turns the scaling on/off. Default value is TRUE.
*/

/*!
  \var SoSFFloat ShapeScale::projectedSize

  The requested projected size of the shape. Default value is 5.0.
*/

#include "ShapeScale.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/SbRotation.h>
#include <Inventor/caches/SoCache.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

SO_KIT_SOURCE(ShapeScale);

ShapeScale::ShapeScale(void) 
{
  SO_KIT_CONSTRUCTOR(ShapeScale);

  SO_KIT_ADD_FIELD(active, (TRUE));
  SO_KIT_ADD_FIELD(projectedSize, (5.0f));  

#ifndef __COIN__
#error catalog setup probably not compatible with non-Coin Inventor implementation
#endif // !__COIN__

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(scale, SoScale, FALSE, topSeparator, shape, FALSE);
  SO_KIT_ADD_CATALOG_ABSTRACT_ENTRY(shape, SoNode, SoCube, TRUE, topSeparator, "", TRUE);

  SO_KIT_INIT_INSTANCE();
  this->didrender = FALSE;
  this->cache = NULL;
}

ShapeScale::~ShapeScale()
{
  if (this->cache) this->cache->unref();
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
  // only write to field when scaling has changed.
  if (!scale->scaleFactor.getValue().equals(v, 0.000001f)) {
    scale->scaleFactor = v;
  }
}

void
ShapeScale::preRender(SoAction * action)
{
  SoState * state = action->getState();
  if (this->cache && this->cache->isValid(state)) return;

  if (this->cache) this->cache->unref();
  
  SbBool storedinvalid = SoCacheElement::setInvalid(FALSE);

  state->push();
  this->cache = new SoCache(state);
  this->cache->ref();
  SoCacheElement::set(state, this->cache);
  
  SoNode * shape = (SoNode*) this->getAnyPart(SbName("shape"), FALSE);
  assert(shape);

  SoScale * scale = (SoScale*) this->getAnyPart(SbName("scale"), TRUE);
  if (!this->active.getValue()) {
    update_scale(scale, SbVec3f(1.0f, 1.0f, 1.0f));
  }
  else {
    SbBox3f bbox(-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f);
    const SbViewportRegion & vp = SoViewportRegionElement::get(state);
    const SbViewVolume & vv = SoViewVolumeElement::get(state);
    SbVec3f center(0.0f, 0.0f, 0.0f);
    const float nsize = this->projectedSize.getValue() / float(vp.getViewportSizePixels()[0]);
    const SbMatrix & mm = SoModelMatrixElement::get(state);
    mm.multVecMatrix(center, center);
    const float scalefactor = vv.getWorldToScreenScale(center, nsize);
    
#if 1 // new version that considers the current model-matrix scale
    SbVec3f t;
    SbRotation r;
    SbVec3f s;
    SbRotation so;
    mm.getTransform(t, r, s, so);
    
    update_scale(scale, SbVec3f(scalefactor/s[0], scalefactor/s[1], scalefactor/s[2]));
#else
    update_scale(scale, SbVec3f(scalefactor, scalefactor, scalefactor));
    
#endif
  }
  state->pop();
  SoCacheElement::setInvalid(storedinvalid);
}

// Overridden to (re)initialize image and other data before rendering.
void 
ShapeScale::GLRender(SoGLRenderAction * action)
{
  this->preRender(action);
  inherited::GLRender(action);
  SoGLCacheContextElement::shouldAutoCache(action->getState(),
                                           SoGLCacheContextElement::DONT_AUTO_CACHE);
}

void 
ShapeScale::getBoundingBox(SoGetBoundingBoxAction * action)
{
  if (this->cache) {
    SoCacheElement::addCacheDependency(action->getState(), this->cache);
  }
  else {
    SoCacheElement::invalidate(action->getState());
  }
  inherited::getBoundingBox(action);
}

void
ShapeScale::notify(SoNotList * l)
{
  SoField * f = l->getLastField();
  if (f == &this->active) {
    if (this->cache) this->cache->invalidate();
  }
  inherited::notify(l);
}

