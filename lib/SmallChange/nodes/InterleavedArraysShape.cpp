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
  \class InterleavedArraysShape InterleavedArrayShape.h SmallChange/nodes/InterleavedArraysShape.h
  \brief The InterleavedArraysShape class is used to render static geometry very fast.
  \ingroup nodes

  It creates a VBO (vertex buffer object) vertex cache internally, and
  all vertex data will be interleaved for maximum rendering
  performance. If you change one vertex attribute, for instance the
  normals, the entire cache needs to be rebuilt, and this is why this
  shape is best suited for relatively static geometry.

  To achieve this caching and reduce overhead, it will only render the
  contents of its SoVertexShape::vertexProperty node, it will not pick
  up any vertex properties from the state.

  This shape only supports OVERALL and PER_VERTEX[_INDEXED]
  material/normal bindings. If some binding is set to something other 
  than OVERALL, it will be interpreted as PER_VERTEX[_INDEXED],
  if the SoVertexProperty node contains sufficient data to render
  that attribute per vertex.
*/

/*!
  \var SoSFEnum InterleavedArraysShape::type

  The type of geometry to render.
*/

/*!
  \var SoSFInt23 InterleavedArraysShape::vertexIndex

  Optional field for specifying vertex indices. If this field is
  empty, the shape will render all vertices in its SoVertexProperty
  node (using glDrawArrays()).
*/

/*!
  \enum InterleavedArraysShape::Type

  The type of primitives supported by this shape.
*/

/*!
  \var InterleavedArraysShape::Type InterleavedArraysShape::POINTS
  For rendering points.
*/

/*!
  \var InterleavedArraysShape::Type InterleavedArraysShape::LINES
  For rendering lines.
*/

/*!
  \var InterleavedArraysShape::Type InterleavedArraysShape::TRIANGLES
  For rendering triangles.
*/

/*!
  \var InterleavedArraysShape::Type InterleavedArraysShape::QUADS
  For rendering quads.
*/

/*!
  \var InterleavedArraysShape::Type InterleavedArraysShape::TRIANGLE_STRIP
  For rendering a triangle strip.
*/

/*!
  \var InterleavedArraysShape::Type InterleavedArraysShape::QUAD_STRIP
  For rendering a quad strip.
*/

#include "InterleavedArraysShape.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/bundles/SoTextureCoordinateBundle.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoTextureCoordinateElement.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SbSphere.h>
#include <Inventor/SbVec4f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/system/gl.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/details/SoLineDetail.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/details/SoFaceDetail.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoShapeHintsElement.h>
#include <Inventor/elements/SoShapeStyleElement.h>
#include <Inventor/misc/SoContextHandler.h>
#include <Inventor/elements/SoOverrideElement.h>
#include <cstddef>
#include <cstring>
#include <map>
#include <vector>

#if COIN_MAJOR_VERSION > 3 || (COIN_MAJOR_VERSION==3 && COIN_MINOR_VERSION >= 1)
#include <Inventor/elements/SoGLVertexAttributeElement.h>
#endif

#define PRIVATE(obj) obj->pimpl

// *************************************************************************

class InterleavedArraysShape::VBO {
public:
  VBO(const GLenum target = GL_ARRAY_BUFFER,
      const GLenum usage = GL_STATIC_DRAW);
  ~VBO();
  
  void setBufferData(const GLvoid * data, 
                     intptr_t size, 
                     uint32_t contextid);
  
  bool hasVBO(uint32_t contextid);
  void bindBuffer(uint32_t contextid);
  
private:
  void clear();
  static void context_destruction_cb(uint32_t context, void * userdata);
  static void vbo_delete(void * closure, uint32_t contextid);
  
  GLenum target;
  GLenum usage;

  // VBOs for different contexts
  std::map<uint32_t, GLuint> vbomap;
};

/*!
  \var SoMFInt32 InterleavedArraysShape::vertexIndex
*/

/*!
  \var SoSFEnum InterleavedArraysShape::type
*/

// *************************************************************************

SO_NODE_SOURCE(InterleavedArraysShape);

// *************************************************************************

class InterleavedArraysShape::Pimpl {
public:
  InterleavedArraysShape::VBO * vbo;
  InterleavedArraysShape::VBO * indexvbo;
  size_t normaloffset;
  size_t coloroffset;
  size_t texcoordoffset;
  size_t vertexsize;
  SbBool useshorts;

  enum {
    MAYBE, YES, NO
  };
  int transparency;
};

namespace {

  SoShape::TriangleShape toShape(InterleavedArraysShape::Type type) {
    switch (type) {
    case InterleavedArraysShape::POINTS:
      return SoShape::POINTS;
    case InterleavedArraysShape::LINES:
      return SoShape::LINES;
    case InterleavedArraysShape::TRIANGLES:
      return SoShape::TRIANGLES;
    case InterleavedArraysShape::QUADS:
      return SoShape::QUADS;
    case InterleavedArraysShape::QUAD_STRIP:
      return SoShape::QUAD_STRIP;
    case InterleavedArraysShape::TRIANGLE_STRIP:
      return SoShape::TRIANGLE_STRIP;
    default:
      assert(0 && "unknown type");
      break;
    }
    return (SoShape::TriangleShape) 0;
  }

  GLenum toGL(InterleavedArraysShape::Type type) {
    switch (type) {
    case InterleavedArraysShape::POINTS:
      return GL_POINTS;
    case InterleavedArraysShape::LINES:
      return GL_LINES;
    case InterleavedArraysShape::TRIANGLES:
      return GL_TRIANGLES;
    case InterleavedArraysShape::QUADS:
      return GL_QUADS;
    case InterleavedArraysShape::QUAD_STRIP:
      return GL_QUAD_STRIP;
    case InterleavedArraysShape::TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    default:
      assert(0 && "unknown type");
      break;
    }
    return (GLenum) 0;
  }  
}


// *************************************************************************

/*!
  Constructor.
*/
InterleavedArraysShape::InterleavedArraysShape()
{
  PRIVATE(this) = new Pimpl;
  PRIVATE(this)->vbo = NULL;
  PRIVATE(this)->indexvbo = NULL;
  PRIVATE(this)->transparency = Pimpl::MAYBE;

  SO_NODE_CONSTRUCTOR(InterleavedArraysShape);
  SO_NODE_ADD_FIELD(type, (TRIANGLES));
  SO_NODE_ADD_FIELD(vertexIndex, (0));
  this->vertexIndex.setNum(0);

  SO_NODE_DEFINE_ENUM_VALUE(Type, POINTS);
  SO_NODE_DEFINE_ENUM_VALUE(Type, LINES);
  SO_NODE_DEFINE_ENUM_VALUE(Type, TRIANGLES);
  SO_NODE_DEFINE_ENUM_VALUE(Type, QUADS);
  SO_NODE_DEFINE_ENUM_VALUE(Type, TRIANGLE_STRIP);
  SO_NODE_DEFINE_ENUM_VALUE(Type, QUAD_STRIP);
  SO_NODE_SET_SF_ENUM_TYPE(type, Type);
}

/*!
  Destructor.
*/
InterleavedArraysShape::~InterleavedArraysShape()
{
  delete PRIVATE(this)->vbo;
  delete PRIVATE(this)->indexvbo;
  delete PRIVATE(this);
}

/*!
  Required Coin method.
*/
void
InterleavedArraysShape::initClass()
{
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    SO_NODE_INIT_CLASS(InterleavedArraysShape, SoVertexShape, "VertexShape");
  }
}

// *************************************************************************

void 
InterleavedArraysShape::notify(SoNotList * l)
{
  SoField * f = l->getLastField();
  if (f == &this->vertexProperty) {
    PRIVATE(this)->transparency = Pimpl::MAYBE;
  }
  if (PRIVATE(this)->vbo) {
    delete PRIVATE(this)->vbo;
    PRIVATE(this)->vbo = NULL;
  }
  if (PRIVATE(this)->indexvbo) {
    delete PRIVATE(this)->indexvbo;
    PRIVATE(this)->indexvbo = NULL;
  }
  inherited::notify(l);
}

void
InterleavedArraysShape::createVBO(uint32_t contextid)
{
  if (!PRIVATE(this)->vbo) {
    PRIVATE(this)->vbo = new VBO();
  }
  PRIVATE(this)->normaloffset = 0;
  PRIVATE(this)->coloroffset = 0;
  PRIVATE(this)->texcoordoffset = 0;
  PRIVATE(this)->vertexsize = sizeof(SbVec3f);

  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  
  const SbVec3f * vptr = vp->vertex.getValues(0);
  const SbVec3f * nptr = NULL;
  const SbVec2f * tcptr = NULL;
  const uint32_t * cptr = NULL;

  const int n = vp->vertex.getNum();

  if (vp->normal.getNum() >= n) {
    PRIVATE(this)->normaloffset = PRIVATE(this)->vertexsize;
    PRIVATE(this)->vertexsize += sizeof(SbVec3f);
    nptr = vp->normal.getValues(0);
  }
  if (vp->orderedRGBA.getNum() >= n) {
    PRIVATE(this)->coloroffset = PRIVATE(this)->vertexsize;
    PRIVATE(this)->vertexsize += sizeof(uint32_t);
    cptr = vp->orderedRGBA.getValues(0);
  }
  if (vp->texCoord.getNum() >= n) {
    PRIVATE(this)->texcoordoffset = PRIVATE(this)->vertexsize;
    PRIVATE(this)->vertexsize += sizeof(SbVec2f);
    tcptr = vp->texCoord.getValues(0);
  }
  std::vector<unsigned char> buffer(PRIVATE(this)->vertexsize * n);
  unsigned char * dst = &buffer[0];

  SbBool bigendian = coin_host_get_endianness() == COIN_HOST_IS_BIGENDIAN;
  for (int i = 0; i < n; i++) {
    memcpy(dst, vptr+i, sizeof(SbVec3f));
    if (nptr) memcpy(dst + PRIVATE(this)->normaloffset, nptr+i, sizeof(SbVec3f));
    if (cptr) {
      memcpy(dst + PRIVATE(this)->coloroffset, cptr+i, sizeof(uint32_t));
      if (!bigendian) {
        uint32_t * dsti = (uint32_t*) (dst + PRIVATE(this)->coloroffset);
        *dsti = coin_hton_uint32(*dsti);
      }
    }
    if (tcptr) memcpy(dst + PRIVATE(this)->texcoordoffset, tcptr+i, sizeof(SbVec2f));
    dst += PRIVATE(this)->vertexsize;
  }
  PRIVATE(this)->vbo->setBufferData(&buffer[0], 
                                    buffer.size(),
                                    contextid);
}

void
InterleavedArraysShape::createIndexVBO(uint32_t contextid)
{
  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  PRIVATE(this)->useshorts = vp->vertex.getNum() <= 0xffff;

  const int n = this->vertexIndex.getNum();
  const int32_t * src = this->vertexIndex.getValues(0);
  
  if (!PRIVATE(this)->indexvbo) {
    PRIVATE(this)->indexvbo = new VBO(GL_ELEMENT_ARRAY_BUFFER);
  }
  if (PRIVATE(this)->useshorts) {
    std::vector<GLushort> buffer(n);
    GLushort * dst = &buffer[0];
    for (int i = 0; i < n; i++) {
      dst[i] = (GLushort) src[i];
    }    
    PRIVATE(this)->indexvbo->setBufferData(&buffer[0], n*sizeof(GLushort), contextid);
  }
  else {
    if (sizeof(int32_t) == sizeof(GLuint)) {
      PRIVATE(this)->indexvbo->setBufferData(src, n*sizeof(GLuint), contextid);
    }
    else {
      std::vector<GLuint> buffer(n);
      GLuint * dst = &buffer[0];
      for (int i = 0; i < n; i++) {
        dst[i] = (GLuint) src[i];
      }
      PRIVATE(this)->indexvbo->setBufferData(&buffer[0], n*sizeof(GLuint), contextid);
    }
  }
}

void
InterleavedArraysShape::GLRender(SoGLRenderAction * action)
{
  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  if (!vp) return;
  SoState * state = action->getState();
  Binding mbind = this->findMaterialBinding(state);
  SbBool transparency = FALSE;
  
  if (mbind == PER_VERTEX) {
    if (PRIVATE(this)->transparency == Pimpl::MAYBE) {
      PRIVATE(this)->transparency = Pimpl::NO;
      const uint32_t * rgba = vp->orderedRGBA.getValues(0);
      const int n = vp->orderedRGBA.getNum();

      for (int i = 0; i < n; i++) {
        if ((rgba[i] & 0xff) != 0xff) {
          PRIVATE(this)->transparency = Pimpl::YES;
          break;
        }
      }
    }
    if (PRIVATE(this)->transparency == Pimpl::YES) {
      transparency = TRUE;
    }
  }

  if (transparency) {
    state->push();
    SoShapeStyleElement::setTransparentMaterial(state, TRUE);
  }
  if (this->shouldGLRender(action)) {
    SoMaterialBundle mb(action);
    mb.sendFirst();
    
    SbBool doTextures = vp->texCoord.getNum();
    Binding nbind = this->findNormalBinding(state);
    
    uint32_t contextid = (uint32_t) action->getCacheContext();
    const cc_glglue * glue = cc_glglue_instance((int) contextid);

    if (cc_glglue_has_vertex_array(glue) &&
        cc_glglue_has_vertex_buffer_object(glue)) {
      if (!PRIVATE(this)->vbo || !PRIVATE(this)->vbo->hasVBO(contextid)) {
        this->createVBO(contextid);
      }
      PRIVATE(this)->vbo->bindBuffer(contextid);
      
      this->enableArrays(action, 
                         nbind == PER_VERTEX,
                         mbind == PER_VERTEX,
                         doTextures);
    
      if (this->vertexIndex.getNum()) {
        if (!PRIVATE(this)->indexvbo || !PRIVATE(this)->indexvbo->hasVBO(contextid)) {
          this->createIndexVBO(contextid);
        }
        PRIVATE(this)->indexvbo->bindBuffer(contextid);
        
        cc_glglue_glDrawElements(glue,
                                 toGL((Type) this->type.getValue()),
                                 this->vertexIndex.getNum(),
                                 PRIVATE(this)->useshorts ? 
                                 GL_UNSIGNED_SHORT :
                                 GL_UNSIGNED_INT,
                                 NULL);
        cc_glglue_glBindBuffer(glue, GL_ELEMENT_ARRAY_BUFFER, 0);
      }
      else {
        cc_glglue_glDrawArrays(glue, 
                               toGL((Type) this->type.getValue()),
                               0,
                               vp->vertex.getNum());
      }
      this->disableArrays(action,
                          nbind == PER_VERTEX,
                          mbind == PER_VERTEX,
                          doTextures);
      cc_glglue_glBindBuffer(glue, GL_ARRAY_BUFFER, 0);
      
      SoGLCacheContextElement::shouldAutoCache(state,
                                               SoGLCacheContextElement::DONT_AUTO_CACHE);
    }
    else {
      // fall back to immediate mode rendering
      const SbVec3f * vptr = vp->vertex.getValues(0);
      const SbVec3f * nptr = NULL;
      const SbVec2f * tcptr = NULL;
      const uint32_t * cptr = NULL;
      const int nv = vp->vertex.getNum();
      if (nbind == PER_VERTEX) {
        nptr = vp->normal.getValues(0);
      }
      if (mbind == PER_VERTEX) {
        cptr = vp->orderedRGBA.getValues(0);
      }
      if (vp->texCoord.getNum() >= nv) {
        tcptr = vp->texCoord.getValues(0);
      }
      glBegin(toGL((Type) this->type.getValue()));
      if (this->vertexIndex.getNum()) {
        const int n = this->vertexIndex.getNum();
        const int32_t * idx = this->vertexIndex.getValues(0);
        for (int j = 0; j < n; j++) {
          const int i = idx[j];
          if (cptr) {
            uint32_t c = cptr[i];
            glColor4ub(c>>24, (c>>16)&0xff, (c>>8)&0xff, c&0xff);
          }
          if (tcptr) glTexCoord2fv((const GLfloat*) &tcptr[i]);
          if (nptr) glNormal3fv((const GLfloat*) &nptr[i]);
          glVertex3fv((const GLfloat*) &vptr[i]);
        }
      }
      else {
        for (int i = 0; i < nv; i++) {
          if (cptr) {
            uint32_t c = cptr[i];
            glColor4ub(c>>24, (c>>16)&0xff, (c>>8)&0xff, c&0xff);
          }
          if (tcptr) glTexCoord2fv((const GLfloat*) &tcptr[i]);
          if (nptr) glNormal3fv((const GLfloat*) &nptr[i]);
          glVertex3fv((const GLfloat*) &vptr[i]);
        }
      }
      glEnd();
    }
  }
  if (transparency) state->pop();
}

void
InterleavedArraysShape::enableArrays(SoGLRenderAction * action,
                                     SbBool normals,
                                     SbBool colors,
                                     SbBool texcoords)
{
  SoState * state = action->getState();
  const cc_glglue * glue = cc_glglue_instance((int) action->getCacheContext());

  if (colors) {
    cc_glglue_glColorPointer(glue, 4, GL_UNSIGNED_BYTE,
                             PRIVATE(this)->vertexsize, 
                             (const GLvoid*) PRIVATE(this)->coloroffset);
    cc_glglue_glEnableClientState(glue, GL_COLOR_ARRAY);
  }
  if (texcoords) {
    cc_glglue_glTexCoordPointer(glue, 2, GL_FLOAT, 
                                PRIVATE(this)->vertexsize,
                                (const GLvoid*) PRIVATE(this)->texcoordoffset);
    cc_glglue_glEnableClientState(glue, GL_TEXTURE_COORD_ARRAY);
  }
  if (normals) {
    cc_glglue_glNormalPointer(glue, GL_FLOAT, 
                              PRIVATE(this)->vertexsize,
                              (const GLvoid*) PRIVATE(this)->normaloffset);
    cc_glglue_glEnableClientState(glue, GL_NORMAL_ARRAY);
  }
  cc_glglue_glVertexPointer(glue, 3, GL_FLOAT,
                            PRIVATE(this)->vertexsize,
                            NULL);
  cc_glglue_glEnableClientState(glue, GL_VERTEX_ARRAY);

#if COIN_MAJOR_VERSION > 3 || (COIN_MAJOR_VERSION==3 && COIN_MINOR_VERSION >= 1) 
  SoGLVertexAttributeElement::getInstance(state)->enableVBO(action);
#endif
}

void
InterleavedArraysShape::disableArrays(SoGLRenderAction * action,
                                      SbBool normals,
                                      SbBool colors,
                                      SbBool texcoords)
{
  SoState * state = action->getState();
  const cc_glglue * glue = cc_glglue_instance((int) action->getCacheContext());
  if (colors) {
    cc_glglue_glDisableClientState(glue, GL_COLOR_ARRAY);
  }
  if (texcoords) {
    cc_glglue_glDisableClientState(glue, GL_TEXTURE_COORD_ARRAY);
  }
  if (normals) {
    cc_glglue_glDisableClientState(glue, GL_NORMAL_ARRAY);
  }
  cc_glglue_glDisableClientState(glue, GL_VERTEX_ARRAY);

#if COIN_MAJOR_VERSION > 3 || (COIN_MAJOR_VERSION==3 && COIN_MINOR_VERSION >= 1) 
  SoGLVertexAttributeElement::getInstance(state)->disableVBO(action);
#endif
}

void
InterleavedArraysShape::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
  if (!this->shouldPrimitiveCount(action)) return;

  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  if (!vp) return;

  int numv = vp->vertex.getNum();
  if (this->vertexIndex.getNum()) {
    numv = this->vertexIndex.getNum();
  }
  
  switch((Type) this->type.getValue()) {
  case POINTS:
    action->addNumPoints(numv);
    break;
  case LINES:
    action->addNumLines(numv/2);
    break;
  case TRIANGLES:
    action->addNumTriangles(numv/3);
    break;
  case QUADS:
    action->addNumTriangles((numv/4)*2);
    break;
  case TRIANGLE_STRIP:
    action->addNumTriangles(numv-2);
    break;
  case QUAD_STRIP:
    action->addNumTriangles(numv-2);
    break;
  default:
    assert(0 && "unknown type");
    break;
  }
}

void
InterleavedArraysShape::generatePrimitives(SoAction * action)
{
  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  if (!vp) return;

  Type type = (Type) this->type.getValue();
  SoState * state = action->getState();
  const SbVec3f * vptr = vp->vertex.getValues(0);
  const SbVec3f * nptr = NULL;
  const SbVec2f * tcptr = NULL;
  const uint32_t * cptr = NULL;

  const int nv = vp->vertex.getNum();

  if (this->findNormalBinding(state) == PER_VERTEX) {
    nptr = vp->normal.getValues(0);
  }
  if (this->findMaterialBinding(state) == PER_VERTEX) {
    cptr = vp->orderedRGBA.getValues(0);
  }
  if (vp->texCoord.getNum() >= nv) {
    tcptr = vp->texCoord.getValues(0);
  }
  SbVec3f dummynormal(0.0f, 0.0f, 1.0f);
  const SbVec3f * currnormal = &dummynormal;
  if (nptr) currnormal = nptr;
  
  SoPrimitiveVertex vertex;
  SoFaceDetail faceDetail;
  SoLineDetail lineDetail;

  SoDetail * detail = &faceDetail;
  if (type == LINES) detail = &lineDetail;
  else if (type == POINTS) detail = NULL;
  
  SoPointDetail pointDetail;  
  vertex.setDetail(&pointDetail);
  vertex.setNormal(*currnormal);

  this->beginShape(action, toShape((Type)this->type.getValue()),
                   detail);
  if (this->vertexIndex.getNum()) {
    const int ni = this->vertexIndex.getNum();
    const int32_t * vidx = this->vertexIndex.getValues(0);
    for (int i = 0; i < ni; i++) {
      int idx = vidx[i];
      if (nptr) {
        pointDetail.setNormalIndex(idx);
        currnormal = &nptr[idx];
        vertex.setNormal(*currnormal);
      }
      if (cptr) {
        pointDetail.setMaterialIndex(idx);
        vertex.setMaterialIndex(idx);
      }
      if (tcptr) {
        pointDetail.setTextureCoordIndex(idx);
        vertex.setTextureCoords(tcptr[idx]);
      }
      pointDetail.setCoordinateIndex(idx);
      vertex.setPoint(vptr[idx]);
      this->shapeVertex(&vertex);
      
      if (this->shouldIncrementFaceIndex(type, i)) {
        faceDetail.incFaceIndex();
        lineDetail.incLineIndex();
      }
    }
  }
  else {
    for (int idx = 0; idx < nv; idx++) {
      if (nptr) {
        pointDetail.setNormalIndex(idx);
        currnormal = &nptr[idx];
        vertex.setNormal(*currnormal);
      }
      if (cptr) {
        pointDetail.setMaterialIndex(idx);
        vertex.setMaterialIndex(idx);
      }
      if (tcptr) {
        pointDetail.setTextureCoordIndex(idx);
        vertex.setTextureCoords(tcptr[idx]);
      }
      pointDetail.setCoordinateIndex(idx);
      vertex.setPoint(vptr[idx]);
      this->shapeVertex(&vertex);
      if (this->shouldIncrementFaceIndex(type, idx)) {
        faceDetail.incFaceIndex();
        lineDetail.incLineIndex();
      }
    }
  }
  this->endShape();
}

SbBool
InterleavedArraysShape::shouldIncrementFaceIndex(Type type, int i) const
{
  switch (type) {
  case POINTS:
    return TRUE;
    break;
  case LINES:
    return (i & 1) ? TRUE : FALSE;
    break;
  case TRIANGLES:
    return ((i % 3) == 2) ? TRUE : FALSE;
    break;
  case QUADS:
    return ((i % 4) == 3) ? TRUE : FALSE;
    break;
  case TRIANGLE_STRIP:
    return (i > 2) ? TRUE : FALSE;
    break;
  case QUAD_STRIP:
    return ((i > 3) && (i & 1)) ? TRUE : FALSE;
    break;
  default:
    assert(0 && "unknown type");
    break;
  }
  return FALSE;
}

SoPickedPoint * 
InterleavedArraysShape::tryPickTriangle(SoRayPickAction * action,
                                        const SbVec3f * vertices,
                                        int v0, int v1, int v2,
                                        SbVec3f & barycentric)
{
  SbVec3f intersection;
  SbBool front;
  
  if (action->intersect(vertices[v0], vertices[v1], vertices[v2],
                        intersection, barycentric, front)) {

    if (action->isBetweenPlanes(intersection)) {
      if (SoShapeHintsElement::getVertexOrdering(action->getState()) ==
          SoShapeHintsElement::CLOCKWISE) {
        front = !front;
      }
#if COIN_MAJOR_VERSION > 3 || (COIN_MAJOR_VERSION==3 && COIN_MINOR_VERSION >= 1)
      return action->addIntersection(intersection, front);
#else
      return action->addIntersection(intersection);
#endif
    }
  }
  return NULL;
}

void 
InterleavedArraysShape::rayPick(SoRayPickAction * action)
{
  if (!shouldRayPick(action)) return;
  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  if (!vp || !vp->vertex.getNum()) return;

  Type type = (Type) this->type.getValue();

  if (type == TRIANGLES || type == QUADS) {
    // do optimized picking for these primitives
    SoState * state = action->getState();

    const SbVec3f * vertices = vp->vertex.getValues(0);
    const int primsize = type == TRIANGLES ? 3 : 4;

    const int n = this->vertexIndex.getNum() ? this->vertexIndex.getNum() : vp->vertex.getNum();
    const int32_t * vidx = this->vertexIndex.getNum() ? this->vertexIndex.getValues(0) : NULL;

    Binding mbind = this->findMaterialBinding(state);
    Binding nbind = this->findNormalBinding(state);

    for (int i = 0; i < n; i += primsize) {
      int i0,i1,i2,i3;
      if (vidx) {
        i0 = vidx[i];
        i1 = vidx[i+1];
        i2 = vidx[i+2];
        i3 = -1;
        if (primsize == 4) i3 = vidx[i+3];
      }
      else {
        i0 = i;
        i1 = i+1;
        i2 = i+2;
        i3 = -1;
        if (primsize == 4) i3 = i+3;
      }
      SbVec3f bary;
      SoPickedPoint * pp = this->tryPickTriangle(action, vertices, i0, i1, i2, bary);

      int b0 = i0;
      int b1 = i1;
      int b2 = i2;

      if (!pp && i3 >= 0) {
        pp = this->tryPickTriangle(action, vertices, i0, i2, i3, bary);
        b1 = i2;
        b2 = i3;
      }
      if (pp) {
        int lookup[] = {i0,i1,i2,i3};
        SoFaceDetail * facedetail = new SoFaceDetail;
        facedetail->setFaceIndex(i / primsize);
        facedetail->setNumPoints(primsize);
        for (int j = 0; j < primsize; j++) {
          SoPointDetail * pd = facedetail->getPoints() + j;
          pd->setCoordinateIndex(lookup[j]);
          if (nbind == PER_VERTEX) {
            pd->setNormalIndex(lookup[j]);
          }
          if (mbind == PER_VERTEX) {
            pd->setMaterialIndex(lookup[j]);
          }
          if (vp->texCoord.getNum() >= vp->vertex.getNum()) {
            pd->setTextureCoordIndex(lookup[j]);
          }
        }
        pp->setDetail(facedetail, this);
        
        if (this->findNormalBinding(state) == PER_VERTEX) {
          const SbVec3f * nptr = vp->normal.getValues(0);
          SbVec3f n =
            nptr[b0] * bary[0] +
            nptr[b1] * bary[1] +
            nptr[b2] * bary[2];
          n.normalize();
          pp->setObjectNormal(n);
        }
        if (vp->texCoord.getNum() >= vp->vertex.getNum()) {
          const SbVec2f * tcptr = vp->texCoord.getValues(0);

          SbVec2f tc =
            tcptr[b0] * bary[0] +
            tcptr[b1] * bary[1] +
            tcptr[b2] * bary[2];

          pp->setObjectTextureCoords(SbVec4f(tc[0], tc[1], 0.0f, 1.0f));
          
          float maxval = bary[0];
          int maxi = b0;
          if (bary[1] > maxval) {
            maxi = b1;
            maxval = bary[1];
          }
          if (bary[2] > maxval) {
            maxi = b2;
          }
          pp->setMaterialIndex(b2);
        }
      }
    }
  }
  else {
    inherited::rayPick(action);
  }
}

void
InterleavedArraysShape::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  box.makeEmpty();
  if (!vp) return;
  
  const SbVec3f * src = vp->vertex.getValues(0);
  const int n = vp->vertex.getNum();
  for (int i = 0; i < n; i++) {
    box.extendBy(src[i]);
  }
  center = box.getCenter();
}

// *************************************************************************
// VBO class, copied from Coin, but changed a bit to avoid storing data twice

/*!
  Constructor
*/
InterleavedArraysShape::VBO::VBO(const GLenum target, const GLenum usage)
  : target(target),
    usage(usage)
{
  SoContextHandler::addContextDestructionCallback(context_destruction_cb, this);
}

InterleavedArraysShape::VBO::~VBO()
{
  SoContextHandler::removeContextDestructionCallback(context_destruction_cb, this);
  this->clear();
}

// Callback from SoGLCacheContextElement
void
InterleavedArraysShape::VBO::vbo_delete(void * closure, uint32_t contextid)
{
  const cc_glglue * glue = cc_glglue_instance((int) contextid);
  GLuint id = (GLuint) ((uintptr_t) closure);
  cc_glglue_glDeleteBuffers(glue, 1, &id);
}

void
InterleavedArraysShape::VBO::clear()
{
  for (std::map<uint32_t, GLuint>::const_iterator it = this->vbomap.begin();
       it != this->vbomap.end();
       ++it) {
    uintptr_t id = (uintptr_t) it->second;
    SoGLCacheContextElement::scheduleDeleteCallback(it->first, vbo_delete, 
                                                    (void*) id);    
  }
  this->vbomap.clear();
}

void
InterleavedArraysShape::VBO::setBufferData(const GLvoid * data, 
                                           intptr_t size,
                                           uint32_t contextid)
{
  assert(!this->hasVBO(contextid));
  GLuint buffer;
  const cc_glglue * glue = cc_glglue_instance((int) contextid);
  cc_glglue_glGenBuffers(glue, 1, &buffer);
  cc_glglue_glBindBuffer(glue, this->target, buffer);
  cc_glglue_glBufferData(glue, 
                         this->target,
                         size,
                         data,
                         this->usage);
  this->vbomap[contextid] = buffer;
  // reset/disable current VBO
  cc_glglue_glBindBuffer(glue, this->target, 0);
}

bool 
InterleavedArraysShape::VBO::hasVBO(uint32_t contextid)
{
  const std::map<uint32_t, GLuint>::const_iterator it =
    this->vbomap.find(contextid);
  return it != this->vbomap.end();
}

void
InterleavedArraysShape::VBO::bindBuffer(uint32_t contextid)
{
  assert(this->hasVBO(contextid));
  const cc_glglue * glue = cc_glglue_instance((int) contextid);
  cc_glglue_glBindBuffer(glue, this->target, this->vbomap[contextid]);
}

void
InterleavedArraysShape::VBO::context_destruction_cb(uint32_t context, void * userdata)
{
  GLuint buffer;
  VBO * thisp = (VBO*) userdata;
  const cc_glglue * glue = cc_glglue_instance((int) context);

  std::map<uint32_t, GLuint>::iterator it = thisp->vbomap.find(context);
  if (it != thisp->vbomap.end()) {
    GLuint buffer = it->second;
    cc_glglue_glDeleteBuffers(glue, 1, &buffer);
    thisp->vbomap.erase(it);
  }
}

InterleavedArraysShape::Binding
InterleavedArraysShape::findMaterialBinding(SoState * const state) const
{
  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  if (!vp) return OVERALL;

  Binding binding = OVERALL;

  if (vp->materialBinding.getValue() != SoVertexProperty::OVERALL) {
    binding = PER_VERTEX;
  }
  if (SoOverrideElement::getMaterialBindingOverride(state)) {
    SoMaterialBindingElement::Binding matbind =
      SoMaterialBindingElement::get(state);
    
    switch (matbind) {
    case SoMaterialBindingElement::OVERALL:
      binding = OVERALL;
      break;
    default:
      binding = PER_VERTEX;
    }
  }
  if (binding == PER_VERTEX &&
      vp->orderedRGBA.getNum() < vp->vertex.getNum()) {
    binding = OVERALL;
  }
  return binding;
}

InterleavedArraysShape::Binding
InterleavedArraysShape::findNormalBinding(SoState * const state) const
{
  SoVertexProperty * vp = (SoVertexProperty*) this->vertexProperty.getValue();
  if (!vp) return OVERALL;

  Binding binding = OVERALL;

  if (vp->normalBinding.getValue() != SoVertexProperty::OVERALL) {
    binding = PER_VERTEX;
  }
  if (SoOverrideElement::getNormalBindingOverride(state)) {
    SoNormalBindingElement::Binding matbind =
      SoNormalBindingElement::get(state);
    
    switch (matbind) {
    case SoNormalBindingElement::OVERALL:
      binding = OVERALL;
      break;
    default:
      binding = PER_VERTEX;
    }
  }
  if (binding == PER_VERTEX &&
      vp->normal.getNum() < vp->vertex.getNum()) {
    binding = OVERALL;
  }
  return binding;
}
