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

  // NOTE: Padding the bbox and the points used against the clipper to avoid
  // culling/clipping annotation text. This is an issue for labels that protrude
  // into neighboring tiles when doing tiled rendering, a la SoOffscreenRenderer.
  SbMatrix translate;
  translate.setTranslate(bbox.getCenter());
  SbMatrix scale;
  scale.setScale(2.0f);
  SbMatrix grow = translate.inverse() * scale * translate;
  bbox.transform(grow);
  for (int i = 0; i < 5; i++) grow.multVecMatrix(p[i], p[i]);
  
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
    for (i = 0; i < 6; i++) {
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
