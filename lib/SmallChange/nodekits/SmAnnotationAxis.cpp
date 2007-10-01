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

#include "SmAnnotationAxis.h"
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/caches/SoCache.h>
#include <SmallChange/nodes/SmTextureText2.h>
#include <string.h>

// *************************************************************************

SO_KIT_SOURCE(SmAnnotationAxis);

// *************************************************************************

class SmAnnotationAxisP {

public:
  SmAnnotationAxisP(SmAnnotationAxis * master) {
    this->master = master;
  }

  SmAnnotationAxis * master;
  SoCache * cache;
  SbList <int> axisidx;

  void add_anno_text(SbList <int> & list,
                     const SbMatrix & projm, 
                     const float maxdist,
                     const SbVec3f * pos, int i0, int i1);
};

#define PRIVATE(p) ((p)->pimpl)
#define PUBLIC(p) ((p)->master)

// *************************************************************************

SmAnnotationAxis::SmAnnotationAxis()
{
  PRIVATE(this) = new SmAnnotationAxisP(this);
  PRIVATE(this)->cache = NULL;

  SO_KIT_CONSTRUCTOR(SmAnnotationAxis);
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(text, SmTextureText2, FALSE, topSeparator, "", FALSE);

  SO_KIT_ADD_FIELD(annotation, (""));
  SO_KIT_ADD_FIELD(annotationPos, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(annotationGap, (30.0f));

  this->annotation.setNum(0);
  this->annotationPos.setNum(0);
  this->annotation.setDefault(TRUE);
  this->annotationPos.setDefault(TRUE);

  SO_KIT_INIT_INSTANCE();
}

SmAnnotationAxis::~SmAnnotationAxis()
{
  if (PRIVATE(this)->cache) PRIVATE(this)->cache->unref();
  delete PRIVATE(this);
}

void
SmAnnotationAxis::initClass(void)
{
  SO_KIT_INIT_CLASS(SmAnnotationAxis, SoBaseKit, "BaseKit");
}

void 
SmAnnotationAxis::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SoState * state = action->getState();

  if ((PRIVATE(this)->cache == NULL) || !PRIVATE(this)->cache->isValid(state)) {
    // supply an approximate bbox and always invalidate the bbox cache
    SoCacheElement::invalidate(state);
    
    SbBox3f bbox;
    bbox.makeEmpty();
    for (int i = 0; i < this->annotationPos.getNum(); i++) {
      bbox.extendBy(this->annotationPos[i]);
    }

    if (!bbox.isEmpty()) {
      action->extendBy(bbox);
      action->setCenter(bbox.getCenter(), TRUE);
    }
  }
  else {
    inherited::getBoundingBox(action);
  }
}

void 
SmAnnotationAxis::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();

  SbBool createcache = FALSE;
  SbBool render = TRUE;

  SbBool storedinvalid = FALSE;
  if ((PRIVATE(this)->cache == NULL) || !PRIVATE(this)->cache->isValid(state)) {
    storedinvalid = SoCacheElement::setInvalid(FALSE);
    state->push();
    PRIVATE(this)->cache = new SoCache(state);
    PRIVATE(this)->cache->ref();
    SoCacheElement::set(state, PRIVATE(this)->cache);
    createcache = TRUE;
  }
  
  SbMatrix projmatrix;
  projmatrix = (SoModelMatrixElement::get(state) *
                SoViewingMatrixElement::get(state) *
                SoProjectionMatrixElement::get(state));
  
  SbVec2s vpsize = SoViewportRegionElement::get(state).getViewportSizePixels();
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  float maxsize = SbMax(vpsize[0], vpsize[1]);
  
  SbList <int> l1;
  if (this->annotationPos.getNum() >= 2) {
    l1.truncate(0);
    l1.append(0);
    l1.append(this->annotationPos.getNum()-1);
    PRIVATE(this)->add_anno_text(l1, projmatrix,
                                 (this->annotationGap.getValue() * 2.0f) / maxsize, 
                                 this->annotationPos.getValues(0), 
                                 0, this->annotationPos.getNum() - 1); 
    
  }
  
  if (l1 != PRIVATE(this)->axisidx) {
    SmTextureText2 * t = dynamic_cast<SmTextureText2*>(this->getAnyPart("text", TRUE));
    assert(t);
    t->justification = SmTextureText2::CENTER;
    t->position.setNum(l1.getLength());
    t->string.setNum(l1.getLength());
    SbVec3f * pos = t->position.startEditing();
    SbString * text = t->string.startEditing();
    for (int i = 0; i < l1.getLength(); i++) {
      pos[i] = this->annotationPos[l1[i]];
      text[i] = this->annotation.getValues(0)[l1[i]];
    }
    t->position.finishEditing();
    t->string.finishEditing();

    PRIVATE(this)->axisidx = l1;
  }
  if (createcache) {
    state->pop();
    (void) SoCacheElement::setInvalid(storedinvalid);
  }
  if (render) inherited::GLRender(action);
}

void 
SmAnnotationAxis::notify(SoNotList * list)
{
  if (PRIVATE(this)->cache) PRIVATE(this)->cache->invalidate();
  PRIVATE(this)->axisidx.truncate(0);
  inherited::notify(list);
}

// *************************************************************************

void 
SmAnnotationAxisP::add_anno_text(SbList <int> & list,
                                 const SbMatrix & projm, 
                                 const float maxdist,
                                 const SbVec3f * pos, int i0, int i1)
{
  if ((i1-i0) <= 1) return;
  int mid = (i0 + i1) / 2;
  assert(mid != i0 && mid != i1);

  SbVec3f p[3];
  p[0] = pos[i0];
  p[1] = pos[mid];
  p[2] = pos[i1];
  
  int i;

  for (i = 0; i < 3; i++) {
    projm.multVecMatrix(p[i], p[i]);
  }
  if (p[1][2] < 1.0f) {
    SbBool add = FALSE;
    float len = 0.0f;
    if (p[0][2] < 1.0f) {
      p[0][2] = 0.0f;
      p[1][2] = 0.0f;
      len = (p[1]-p[0]).length();
    }
    else if (p[2][2] < 1.0f) {
      p[2][2] = 0.0f;
      p[1][2] = 0.0f;
      len = (p[1]-p[2]).length();
    }
    if (len > maxdist) {
      list.append(mid);
    }
  }
  add_anno_text(list, projm, maxdist, pos, i0, mid);
  add_anno_text(list, projm, maxdist, pos, mid, i1);
}

// *************************************************************************
