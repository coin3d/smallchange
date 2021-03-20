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
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/SbViewportRegion.h>
#include <SmallChange/nodes/SmVertexArrayShape.h>
#include <cstring>
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
      this->useifs = TRUE;
    }
  ~SmToVertexArrayShapeActionP() {
    delete this->vhash;
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
  SbBool useifs;

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
    if (useifs) {
      this->replaceIfs(path);
    }
    else {
      this->replaceVas(path);
    }
  }

  void replaceIfs(SoFullPath * path) {
    SoIndexedFaceSet * ifs = new SoIndexedFaceSet;
    SoVertexProperty * vp = new SoVertexProperty;
    vp->normalBinding = SoVertexProperty::PER_VERTEX_INDEXED;
    vp->materialBinding = SoVertexProperty::OVERALL;
    ifs->ref();
    ifs->vertexProperty = vp;

    if (this->hastexture) {
      vp->texCoord.setValues(0, this->tcoordlist.getLength(),
                          this->tcoordlist.getArrayPtr());
    }
    if (this->colorpervertex) {
      vp->orderedRGBA.setValues(0, this->colorlist.getLength(),
                                this->colorlist.getArrayPtr());
      vp->materialBinding = SoVertexProperty::PER_VERTEX_INDEXED;
    }
    else if (this->colorlist.getLength()) {
      uint32_t dummy = this->colorlist[0];
      vp->materialBinding = SoVertexProperty::OVERALL;
      vp->orderedRGBA.setValues(0, 1, &dummy);
    }
    vp->vertex.setValues(0, this->coordlist.getLength(),
                         this->coordlist.getArrayPtr());

    vp->normal.setValues(0, this->normallist.getLength(),
                         this->normallist.getArrayPtr());
    
    ifs->normalIndex.setNum(0);
    ifs->materialIndex.setNum(0);
    ifs->textureCoordIndex.setNum(0);

    int numtri = this->indices.getLength() / 3;
    ifs->coordIndex.setNum(numtri * 4);
    int32_t * ptr = ifs->coordIndex.startEditing();

    for (int i = 0; i < numtri; i++) {
      *ptr++ = this->indices[i*3];
      *ptr++ = this->indices[i*3+1];
      *ptr++ = this->indices[i*3+2];
      *ptr++ = -1;
    }
    ifs->coordIndex.finishEditing();

    SoNode * parent = path->getNodeFromTail(1);
    int idx = path->getIndexFromTail(0);
    
    path->pop();
    if (parent->isOfType(SoGroup::getClassTypeId())) {
      SoGroup * g = (SoGroup*)parent;
      g->replaceChild(idx, ifs);
    }
    else if (parent->isOfType(SoVRMLShape::getClassTypeId())) {
      SoVRMLShape * vs = (SoVRMLShape*) parent;
      vs->geometry = ifs;
    }
    path->push(idx);
    ifs->unrefNoDelete();
  }


  void replaceVas(SoFullPath * path) {
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

void 
SmToVertexArrayShapeAction::useIndexedFaceSet(const SbBool onoff)
{
  PRIVATE(this)->useifs = onoff;
}

#undef PRIVATE
