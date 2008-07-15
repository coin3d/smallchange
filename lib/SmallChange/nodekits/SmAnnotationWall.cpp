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
#include <cstring>
#include <Inventor/SbClip.h>

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
  SbClip clipper;
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
  SO_KIT_ADD_CATALOG_ENTRY(geometry, SoSeparator, TRUE, topSeparator, "", TRUE);

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
SmAnnotationWall::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();

  SbBool createcache = FALSE;
  SbBool render = TRUE;

  SbBool storedinvalid = FALSE;
  if ((PRIVATE(this)->cache == NULL) || !PRIVATE(this)->cache->isValid(state)) {
    if (PRIVATE(this)->cache) PRIVATE(this)->cache->unref();
    storedinvalid = SoCacheElement::setInvalid(FALSE);
    state->push();
    PRIVATE(this)->cache = new SoCache(state);
    PRIVATE(this)->cache->ref();
    SoCacheElement::set(state, PRIVATE(this)->cache);
    createcache = TRUE;
  }

  int i;
  
  SbVec3f p[5];
  
  p[0] = this->bottomLeft.getValue();
  p[1] = this->bottomRight.getValue();
  p[2] = this->topRight.getValue();
  p[3] = this->topLeft.getValue();
  p[4] = p[0];

  // FIXME: consider using the actual "geometry" bbox
  SbBox3f bbox;
  bbox.makeEmpty();
  for (i = 0; i < 4; i++) {
    bbox.extendBy(p[i]);
  }
  
  SbMatrix projmatrix;
  projmatrix = (SoModelMatrixElement::get(state) *
                SoViewingMatrixElement::get(state) *
                SoProjectionMatrixElement::get(state));
  
  SbVec2s vpsize = SoViewportRegionElement::get(state).getViewportSizePixels();
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  
  float maxsize = SbMax(vpsize[0], vpsize[1]);
 
  render = FALSE;
  if (!SoCullElement::cullBox(state, bbox, TRUE)) {
    SbClip & clipper = PRIVATE(this)->clipper;
    
    SbPlane vvplane[6];
    vv.getViewVolumePlanes(vvplane);
    SbMatrix toobj = SoModelMatrixElement::get(state).inverse();
    clipper.reset();

    for (i = 0; i < 4; i++) {
      clipper.addVertex(p[i]);
    }
    for (i =0; i < 6; i++) {
      vvplane[i].transform(toobj);
      clipper.clip(vvplane[i]);
    }
    if (clipper.getNumVertices() >= 3) {
      SbVec3f projp[3];
      for (i = 0; i < 3; i++) {
        SbVec3f v;
        clipper.getVertex(i, v);
        projmatrix.multVecMatrix(v, projp[i]);
      }
      SbVec3f v0 = projp[1] - projp[0];
      SbVec3f v1 = projp[2] - projp[0];
    
      // do backface culling
      float crossz = v0[0]*v1[1] - v0[1]*v1[0];
      if ((crossz < 0.0f && !this->ccw.getValue()) || 
          (crossz >= 0.0f && this->ccw.getValue())) {
        render = TRUE;
      }
    }
  }
  if (createcache) {
    state->pop();
    (void) SoCacheElement::setInvalid(storedinvalid);
  }
  if (render) inherited::GLRender(action);
}

void 
SmAnnotationWall::notify(SoNotList * list)
{
  if (PRIVATE(this)->cache) PRIVATE(this)->cache->invalidate();
  inherited::notify(list);
}

// *************************************************************************
