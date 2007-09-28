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
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/caches/SoCache.h>
#include <SmallChange/nodes/SmTextureText2.h>
#include <string.h>
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
  SbList <int> axis1idx;
  SbList <int> axis2idx;

  void add_anno_text(SbList <int> & list,
                     const SbMatrix & projm, 
                     const float maxdist,
                     const SbVec3f * pos, int i0, int i1);
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
  SO_KIT_ADD_CATALOG_ENTRY(extraGeom, SoSeparator, TRUE, topSeparator, material, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(material, SoMaterial, TRUE, topSeparator, lineSet, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(lineSet, SoLineSet, FALSE, topSeparator, text, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(text, SmTextureText2, FALSE, topSeparator, "", FALSE);

  SO_KIT_ADD_FIELD(ccw, (TRUE));
  SO_KIT_ADD_FIELD(bottomLeft, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(bottomRight, (1.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(topRight, (1.0f, 1.0f, 0.0f));
  SO_KIT_ADD_FIELD(topLeft, (0.0f, 1.0f, 0.0f));
  SO_KIT_ADD_FIELD(axis1Annotation, (""));
  SO_KIT_ADD_FIELD(axis1AnnotationPos, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(axis2Annotation, (""));
  SO_KIT_ADD_FIELD(axis2AnnotationPos, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(annotationGap, (30.0f));

  this->axis1Annotation.setNum(0);
  this->axis1AnnotationPos.setNum(0);
  this->axis2Annotation.setNum(0);
  this->axis2AnnotationPos.setNum(0);

  this->axis1Annotation.setDefault(TRUE);
  this->axis1AnnotationPos.setDefault(TRUE);
  this->axis2Annotation.setDefault(TRUE);
  this->axis2AnnotationPos.setDefault(TRUE);

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
    inherited::getBoundingBox(action);
  }
}

void 
SmAnnotationWall::GLRender(SoGLRenderAction * action)
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

  int i;
  
  SbVec3f p[5];
  
  p[0] = this->bottomLeft.getValue();
  p[1] = this->bottomRight.getValue();
  p[2] = this->topRight.getValue();
  p[3] = this->topLeft.getValue();
  p[4] = p[0];

  // FIXME: consider using the actual bbox
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
      SbList <int> l1;
      SbList <int> l2;

      if (this->axis1AnnotationPos.getNum() >= 2) {
        l1.truncate(0);
        l1.append(0);
        l1.append(this->axis1AnnotationPos.getNum()-1);
        PRIVATE(this)->add_anno_text(l1, projmatrix,
                                     (this->annotationGap.getValue() * 2.0f) / maxsize, 
                                     this->axis1AnnotationPos.getValues(0), 
                                     0, this->axis1Annotation.getNum() - 1); 

      }

      if (this->axis2AnnotationPos.getNum() >= 2) {
        l2.truncate(0);
        l2.append(0);
        l2.append(this->axis2AnnotationPos.getNum()-1);
        PRIVATE(this)->add_anno_text(l2, projmatrix,
                                     (this->annotationGap.getValue() * 2.0f) / maxsize, 
                                     this->axis2AnnotationPos.getValues(0), 
                                     0, this->axis2Annotation.getNum() - 1); 
      }
      
      if (l1 != PRIVATE(this)->axis1idx || l2 != PRIVATE(this)->axis2idx) {
        SmTextureText2 * t = dynamic_cast<SmTextureText2*>(this->getAnyPart("text", TRUE));
        assert(t);
        t->justification = SmTextureText2::CENTER;
        t->position.setNum(l1.getLength() + l2.getLength());
        t->string.setNum(l1.getLength() + l2.getLength());
        SbVec3f * pos = t->position.startEditing();
        SbString * text = t->string.startEditing();
        for (i = 0; i < l1.getLength(); i++) {
          pos[i] = this->axis1AnnotationPos[l1[i]];
          text[i] = this->axis1Annotation.getValues(0)[i];
        }
        for (i = 0; i < l2.getLength(); i++) {
          pos[i+l1.getLength()] = this->axis2AnnotationPos[l2[i]];
          text[i+l2.getLength()] = this->axis2Annotation.getValues(0)[i];
        }
        t->position.finishEditing();
        t->string.finishEditing();

        PRIVATE(this)->axis1idx = l1;
        PRIVATE(this)->axis2idx = l2;        
      }
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
  PRIVATE(this)->axis1idx.truncate(0);
  PRIVATE(this)->axis2idx.truncate(1);
  inherited::notify(list);
}

// *************************************************************************

void 
SmAnnotationWallP::add_anno_text(SbList <int> & list,
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
  if (pos[1][2] < 1.0f) {
    SbBool add = FALSE;
    float len = 0.0f;
    if (pos[0][2] < 1.0f) {
      p[0][2] = 0.0f;
      p[1][2] = 0.0f;
      len = (p[1]-p[0]).length();
    }
    else if (pos[2][2] < 1.0f) {
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
