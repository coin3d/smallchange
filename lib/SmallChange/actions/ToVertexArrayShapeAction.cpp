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

#include "SmToVertexArrayShapeAction.h"
#include <Inventor/SbName.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoVertexShape.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoTextureImageElement.h>
#include <Inventor/VRMLnodes/SoVRMLShape.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/nodes/SoPackedColor.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/SbViewportRegion.h>
#include <SmallChange/nodes/SmVertexArrayShape.h>
#include <string.h>
#include "../misc/SbHash.h"

SO_ACTION_SOURCE(SmToVertexArrayShapeAction);

void
SmToVertexArrayShapeAction::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_ACTION_INIT_CLASS(SmToVertexArrayShapeAction, SoAction);
  }
}

class sm_vavertex {
public:
  SbVec3f v;
  SbVec3f n;
  SbVec2f tc;
  uint32_t col;

  // needed for SbHash
  operator unsigned long(void) const;
  int operator==(const sm_vavertex & ov) const;
};

sm_vavertex::operator unsigned long(void) const
{
  int size = sizeof(*this);
  unsigned long key = 0;
  const unsigned char * ptr = (const unsigned char *) this;
  for (int i = 0; i < size; i++) {
    int shift = (i%4) * 8;
    key ^= (ptr[i]<<shift);
  }
  return key;
}

int
sm_vavertex::operator==(const sm_vavertex & ov) const
{
  return memcmp(this, &ov, sizeof(sm_vavertex)) == 0;
}

class SmToVertexArrayShapeActionP {
public:
  SmToVertexArrayShapeActionP(void)
    : cbaction(SbViewportRegion(640, 480)) { 
    cbaction.addTriangleCallback(SoShape::getClassTypeId(), triangle_cb, this);
    cbaction.addPreCallback(SoVertexShape::getClassTypeId(),
                            pre_shape_cb, this);
    this->vhash = new SbHash <int32_t, sm_vavertex>;
  }

  static SoCallbackAction::Response pre_shape_cb(void * userdata, SoCallbackAction * action, const SoNode * node) {
    SmToVertexArrayShapeActionP * thisp = (SmToVertexArrayShapeActionP*)userdata;
    SoState * state = action->getState();
    SoLazyElement * lelem = SoLazyElement::getInstance(state);
    
    thisp->numdiffuse = lelem->getNumDiffuse();
    thisp->numtransp = lelem->getNumTransparencies();
    if (lelem->isPacked()) {
      thisp->packedptr = lelem->getPackedPointer();
      thisp->diffuseptr = NULL;
      thisp->transpptr = NULL;
    }
    else {
      thisp->packedptr = NULL;
      thisp->diffuseptr = lelem->getDiffusePointer();
      thisp->transpptr = lelem->getTransparencyPointer();
    }
    thisp->colorpervertex = FALSE;
    // just store diffuse color with index 0
    uint32_t col;
    if (thisp->packedptr) {
      col = thisp->packedptr[0];
    }
    else {
      SbColor tmpc = thisp->diffuseptr[0];
      float tmpt = thisp->transpptr[0];
      col = tmpc.getPackedValue(tmpt);
    }
    thisp->firstcolor = col;   

    SbVec2s dummysize;
    int dummync;
    thisp->hastexture = SoTextureImageElement::getImage(state, dummysize, dummync) != NULL;
    return SoCallbackAction::CONTINUE;
  }
  
  static void triangle_cb(void * userdata, SoCallbackAction * action,
                          const SoPrimitiveVertex * v1,
                          const SoPrimitiveVertex * v2,
                          const SoPrimitiveVertex * v3) {
    SmToVertexArrayShapeActionP * thisp = (SmToVertexArrayShapeActionP*) userdata;

    const SoPrimitiveVertex * vp[] = {v1,v2,v3};
    sm_vavertex v;
    for (int i = 0; i < 3; i++) {
      // FIXME: support more properties per-vertex
      v.v = vp[i]->getPoint();
      v.n = vp[i]->getNormal();
      SbVec4f tc = vp[i]->getTextureCoords();
      if (tc[3] != 0.0f && tc[3] != 1.0f) {
        tc[0] /= tc[3];
        tc[1] /= tc[3];
      }
      v.tc = SbVec2f(tc[0], tc[1]);

      int midx = vp[i]->getMaterialIndex();
      uint32_t col;
      if (thisp->packedptr) {
        col = thisp->packedptr[SbClamp(midx, 0, thisp->numdiffuse)];
      }
      else {
        SbColor tmpc = thisp->diffuseptr[SbClamp(midx,0,thisp->numdiffuse)];
        float tmpt = thisp->transpptr[SbClamp(midx,0,thisp->numtransp)];
        col = tmpc.getPackedValue(tmpt);
      }
      if (col != thisp->firstcolor) thisp->colorpervertex = TRUE;
      v.col = col;

      int32_t idx;
      if (!thisp->vhash->get(v, idx)) {
        idx = thisp->coordlist.getLength();
        thisp->coordlist.append(v.v);
        thisp->normallist.append(v.n);
        thisp->tcoordlist.append(v.tc);
        thisp->colorlist.append(v.col);
        thisp->vhash->put(v, idx);
      }
      thisp->indices.append(idx);
    }
  }

  SbHash <int32_t, sm_vavertex> * vhash;
  SoCallbackAction cbaction;
  SoSearchAction sa;
  SbList <SbVec3f> coordlist;
  SbList <SbVec2f> tcoordlist;
  SbList <SbVec3f> normallist;
  SbList <uint32_t> colorlist;
  SbList <int32_t> indices;

  int numdiffuse;
  int numtransp;
  const uint32_t * packedptr;
  const SbColor * diffuseptr;
  const float * transpptr;
  SbBool colorpervertex;
  uint32_t firstcolor;
  SbBool hastexture;

  void init(void) {
    coordlist.truncate(0);
    tcoordlist.truncate(0);
    normallist.truncate(0);
    colorlist.truncate(0);
    indices.truncate(0);
    delete this->vhash;
    this->vhash = new SbHash<int32_t, sm_vavertex>;
  }
  void replaceNode(SoFullPath * path) {
    SmVertexArrayShape * vas = new SmVertexArrayShape;
    vas->ref();

    if (this->hastexture) {
      SoTextureCoordinate2 * tc = new SoTextureCoordinate2;
      vas->vertexTexCoord = tc;
      tc->point.setValues(0, this->tcoordlist.getLength(),
                          this->tcoordlist.getArrayPtr());
    }
    if (this->colorpervertex) {
#if 1 // packed color
      SoPackedColor * pc = new SoPackedColor;
      vas->vertexColor = pc;
      pc->orderedRGBA.setValues(0, this->colorlist.getLength(),
                                this->colorlist.getArrayPtr());
#else // base color
      SoBaseColor * bc = new SoBaseColor;
      bc->rgb.setNum(this->colorlist.getLength());
      SbColor * c = bc->rgb.startEditing();
      for (int i = 0; i < this->colorlist.getLength(); i++) {
        float dummy;
        c[i].setPackedValue(this->colorlist[i], dummy);
      }
      bc->rgb.finishEditing();
      vas->vertexColor = bc;

#endif // packed or base color 
    }

    SoCoordinate3 * c = new SoCoordinate3;
    vas->vertexCoord = c;
    SoNormal * n = new SoNormal;
    vas->vertexNormal = n;

    c->point.setValues(0, this->coordlist.getLength(),
                       this->coordlist.getArrayPtr());

    n->vector.setValues(0, this->normallist.getLength(),
                        this->normallist.getArrayPtr());
    
    vas->vertexIndex.setNum(1+this->indices.getLength());
    int32_t * ptr = vas->vertexIndex.startEditing();
    *ptr++ = SmVertexArrayShape::TRIANGLES;
    for (int i = 0; i < this->indices.getLength(); i++) {
      *ptr++ = this->indices[i];
    }
    vas->vertexIndex.finishEditing();

    SoNode * parent = path->getNodeFromTail(1);
    int idx = path->getIndexFromTail(0);
    
    path->pop();
    if (parent->isOfType(SoGroup::getClassTypeId())) {
      SoGroup * g = (SoGroup*)parent;
      g->replaceChild(idx, vas);
    }
    else if (parent->isOfType(SoVRMLShape::getClassTypeId())) {
      SoVRMLShape * vs = (SoVRMLShape*) parent;
      vs->geometry = vas;
    }
    path->push(idx);
    vas->unrefNoDelete();
  }
};


#define PRIVATE(obj) (obj)->pimpl

SmToVertexArrayShapeAction::SmToVertexArrayShapeAction(void) 
{
  SO_ACTION_CONSTRUCTOR(SmToVertexArrayShapeAction);
  PRIVATE(this) = new SmToVertexArrayShapeActionP;
}

SmToVertexArrayShapeAction::~SmToVertexArrayShapeAction(void)
{
  delete PRIVATE(this);
}

// Documented in superclass.
void 
SmToVertexArrayShapeAction::apply(SoNode * root)
{
  PRIVATE(this)->sa.setType(SoVertexShape::getClassTypeId());
  PRIVATE(this)->sa.setSearchingAll(TRUE);
  PRIVATE(this)->sa.setInterest(SoSearchAction::ALL);
  PRIVATE(this)->sa.apply(root);
  SoPathList & pl = PRIVATE(this)->sa.getPaths();
  for (int i = 0; i < pl.getLength(); i++) {
    this->apply(pl[i]);
  }
  PRIVATE(this)->sa.reset();
}


// Documented in superclass.
void 
SmToVertexArrayShapeAction::apply(SoPath * path)
{
  PRIVATE(this)->init();
  PRIVATE(this)->cbaction.apply(path);  
  PRIVATE(this)->replaceNode((SoFullPath*) path);
}

// Documented in superclass.
void 
SmToVertexArrayShapeAction::apply(const SoPathList & pathlist, SbBool obeysrules)
{
  for (int i = 0; i < pathlist.getLength(); i++) {
    this->apply(pathlist[i]);
  }
}

// Documented in superclass.
void
SmToVertexArrayShapeAction::beginTraversal(SoNode * node)
{  
  assert(0 && "should never get here");
}

#undef PRIVATE

