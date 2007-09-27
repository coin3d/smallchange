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

#include "SmAnnotationWall.h"
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/caches/SoCache.h>
#include <SmallChange/nodes/SmTextureText2.h>
#include <string.h>

// *************************************************************************

SO_KIT_SOURCE(SmAnnotationWall);

// *************************************************************************

class SmAnnotationWallP {

public:
  SmAnnotationWallP(SmAnnotationWall * master) {
    this->master = master;
  }

  SmAnnotationWall * master;
  SoCache * cache;
};

#define PRIVATE(p) ((p)->pimpl)
#define PUBLIC(p) ((p)->master)

// *************************************************************************

SmAnnotationWall::SmAnnotationWall()
{
  PRIVATE(this) = new SmAnnotationWallP(this);
  PRIVATE(this)->cache = NULL;

  SO_KIT_CONSTRUCTOR(SmAnnotationWall);
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(material, SoMaterial, TRUE, topSeparator, lineSet, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(lineSet, SoLineSet, FALSE, topSeparator, text, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(text, SmTextureText2, FALSE, topSeparator, "", TRUE);

  SO_KIT_ADD_FIELD(ccw, (TRUE));
  SO_KIT_ADD_FIELD(bottomLeft, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(bottomRight, (1.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(topRight, (1.0f, 1.0f, 0.0f));
  SO_KIT_ADD_FIELD(topLeft, (0.0f, 1.0f, 0.0f));
  
  SO_KIT_INIT_INSTANCE();
}

SmAnnotationWall::~SmAnnotationWall()
{
  if (PRIVATE(this)->cache) PRIVATE(this)->cache->unref();
  delete PRIVATE(this);
}

void
SmAnnotationWall::initClass(void)
{
  SO_KIT_INIT_CLASS(SmAnnotationWall, SoBaseKit, "BaseKit");
}

void 
SmAnnotationWall::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SoState * state = action->getState();

  if ((PRIVATE(this)->cache == NULL) || !PRIVATE(this)->cache->isValid(state)) {
   // supply an approximate bbox and always invalidate the bbox cache
    SoCacheElement::invalidate(state);
    
    SbBox3f bbox;
    bbox.makeEmpty();
    bbox.extendBy(this->bottomLeft.getValue());
    bbox.extendBy(this->bottomRight.getValue());
    bbox.extendBy(this->topLeft.getValue());
    bbox.extendBy(this->topRight.getValue());
    
    action->extendBy(bbox);
    action->setCenter(bbox.getCenter(), TRUE);
  }
  else {
    // FIXME: didn't work as intended. Look into why
    SoCacheElement::invalidate(state);
  }
}

void 
SmAnnotationWall::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  if ((PRIVATE(this)->cache == NULL) || !PRIVATE(this)->cache->isValid(state)) {

    SbBool storedinvalid = SoCacheElement::setInvalid(FALSE);
    state->push();
    PRIVATE(this)->cache = new SoCache(state);
    PRIVATE(this)->cache->ref();
    SoCacheElement::set(state, PRIVATE(this)->cache);

    int i;
    
    SbVec3f p[5];
    
    p[0] = this->bottomLeft.getValue();
    p[1] = this->bottomRight.getValue();
    p[2] = this->topRight.getValue();
    p[3] = this->topLeft.getValue();
    p[4] = p[0];
    
    SbMatrix projmatrix;
    projmatrix = (SoModelMatrixElement::get(state) *
                  SoViewingMatrixElement::get(state) *
                  SoProjectionMatrixElement::get(state));
    
    SbVec2s vpsize = SoViewportRegionElement::get(state).getViewportSizePixels();
    
    SbVec3f projp[4];
    for (i = 0; i < 4; i++) {
      projmatrix.multVecMatrix(p[i], projp[i]);
    }
    SbVec3f v0 = projp[1] - projp[0];
    SbVec3f v1 = projp[3] - projp[0];
    
    // do backface culling
    float crossz = v0[0]*v1[1] - v0[1]*v1[0];
    if (crossz < 0.0f && this->ccw.getValue()) return;
    if (crossz >= 0.0f && !this->ccw.getValue()) return;
    
    SoLineSet * ls = dynamic_cast<SoLineSet*> (this->getAnyPart("lineSet", TRUE));
    SoVertexProperty * vp = dynamic_cast<SoVertexProperty*> (ls->vertexProperty.getValue());
    if (vp == NULL) {
      vp = new SoVertexProperty;
      ls->vertexProperty = vp;
    }
    if (ls->numVertices.getNum() != 1 || ls->numVertices[0] != 5) {
      ls->numVertices = 5;
    }
    SbBool needupdate = TRUE;
    if (vp->vertex.getNum() == 5) {
      needupdate = FALSE;
      if (memcmp(p, vp->vertex.getValues(0), 5*sizeof(SbVec3f))) needupdate = TRUE;
    }
    if (needupdate) {
      vp->vertex.setNum(5);
      SbVec3f * dst = vp->vertex.startEditing();
      memcpy(dst, p, 5*sizeof(SbVec3f));
      vp->vertex.finishEditing();
    }
    state->pop();
    (void) SoCacheElement::setInvalid(storedinvalid);
  }
  inherited::GLRender(action);
}

void 
SmAnnotationWall::notify(SoNotList * list)
{
  if (PRIVATE(this)->cache) PRIVATE(this)->cache->invalidate();
  inherited::notify(list);
}

// *************************************************************************

