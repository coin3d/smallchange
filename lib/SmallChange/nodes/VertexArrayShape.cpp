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
  \class SmVertexArrayShape
  \brief The SmVertexArrayShape class us used to render using OpenGL vertex arrays (and soon with vertex buffer objects).

*/

#include <SmallChange/nodes/SmVertexArrayShape.h>

#include <Inventor/misc/SoState.h>
#include <Inventor/bundles/SoTextureCoordinateBundle.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>

#include <Inventor/system/gl.h>
#include <Inventor/C/glue/gl.h>

#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoCoordinate4.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/nodes/SoTextureCoordinate3.h>
#include <Inventor/nodes/SoPackedColor.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/caches/SoNormalCache.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/details/SoFaceDetail.h>
#include <assert.h>

class SmVertexArrayShapeP {
public:
  SmVertexArrayShapeP(SmVertexArrayShape * master) : master(master) { }

public:
  SmVertexArrayShape * master;

  // needed when vertex buffer objects are used
  SbBool indexarraydirty;
  SbBool coordarraydirty;
  SbBool normalarraydirty;
  SbBool texcoordarraydirty;
  SbBool colorarraydirty;
  SbList <int32_t> indexlist;

  SbBool indexlistdirty;
  int numtriangles;
  void updateIndexList(void);
};

#define PRIVATE(obj) (obj)->pimpl
#define PUBLIC(obj) (obj)->master

SO_NODE_SOURCE(SmVertexArrayShape);

/*!
  Constructor.
*/
SmVertexArrayShape::SmVertexArrayShape()
{
  SO_NODE_CONSTRUCTOR(SmVertexArrayShape);

  PRIVATE(this) = new SmVertexArrayShapeP(this);
  PRIVATE(this)->indexlistdirty = TRUE;
  PRIVATE(this)->numtriangles = 0;
  PRIVATE(this)->indexarraydirty = TRUE;
  PRIVATE(this)->coordarraydirty = TRUE;
  PRIVATE(this)->normalarraydirty = TRUE;
  PRIVATE(this)->texcoordarraydirty = TRUE;
  PRIVATE(this)->colorarraydirty = TRUE;
  
  SO_NODE_ADD_FIELD(vertexIndex, (-1));
  SO_NODE_ADD_FIELD(vertexCoord, (NULL));
  SO_NODE_ADD_FIELD(vertexNormal, (NULL));
  SO_NODE_ADD_FIELD(vertexTexCoord, (NULL));
  SO_NODE_ADD_FIELD(vertexColor, (NULL));

  this->vertexIndex.setNum(0);
  this->vertexIndex.setDefault(TRUE);
}

/*!
  Destructor.
*/
SmVertexArrayShape::~SmVertexArrayShape()
{
  delete PRIVATE(this);
}

// doc from parent
void
SmVertexArrayShape::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(SmVertexArrayShape, SoShape, "Shape");
  }
}

// doc from parent
void
SmVertexArrayShape::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SoNode * node = this->vertexCoord.getValue();
  if (node == NULL) return;
  const int n = this->vertexIndex.getNum();
  if (n == 0) return;
  const int32_t * ptr = this->vertexIndex.getValues(0);

  center.setValue(0.0f, 0.0f, 0.0f);
  int numacc = 0;
  if (node->isOfType(SoCoordinate3::getClassTypeId())) {
    const SbVec3f * v = ((SoCoordinate3*)node)->point.getValues(0);
    const int numcoords = ((SoCoordinate3*)node)->point.getNum();
    for (int i = 0; i < n; i++) {
      int idx = ptr[i];
      if (idx >= 0 && idx < numcoords) {
        box.extendBy(v[idx]);
        center += v[idx];
        numacc++;
      }
    }
  }
  else if (node->isOfType(SoCoordinate4::getClassTypeId())) {
    SbVec3f tmp;
    
    const SbVec4f * v = ((SoCoordinate4*)node)->point.getValues(0);
    const int numcoords = ((SoCoordinate4*)node)->point.getNum();
    for (int i = 0; i < n; i++) {
      int idx = ptr[i];
      if (idx >= 0 && idx < numcoords) {
        SbVec4f h = v[idx];
        h.getReal(tmp);
        box.extendBy(tmp);
        center += tmp;
        numacc++;
      }
    }
  }
  if (numacc) center /= (float) numacc;
}

// doc from parent
void
SmVertexArrayShape::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;
  SoState * state = action->getState();
  
  const cc_glglue * glue = cc_glglue_instance(SoGLCacheContextElement::get(state));
  if (!cc_glglue_has_vertex_array(glue)) return; // FIXME: render using normal OpenGL

  SoCoordinate3 * coord3 = NULL ;
  SoCoordinate4 * coord4 = NULL;
  SoTextureCoordinate2 * texcoord2 = NULL;
  SoTextureCoordinate3 * texcoord3 = NULL;
  SoNormal * normal = NULL;
  SoBaseColor * basecolor = NULL;
  SoPackedColor * packedcolor = NULL;

  SoNode * node;
  node = this->vertexCoord.getValue();
  if (node && node->isOfType(SoCoordinate3::getClassTypeId())) {
    coord3 = (SoCoordinate3*) node;
  }
  else if (node && node->isOfType(SoCoordinate4::getClassTypeId())) {
    coord4 = (SoCoordinate4*) node;
  }
  else return; // no data
  
  node = this->vertexNormal.getValue();
  if (node && node->isOfType(SoNormal::getClassTypeId())) {
    normal = (SoNormal*) node;
  }
  node = this->vertexTexCoord.getValue();
  if (node && node->isOfType(SoTextureCoordinate2::getClassTypeId())) {
    texcoord2 = (SoTextureCoordinate2*) node;
  }
  else if (node && node->isOfType(SoTextureCoordinate3::getClassTypeId())) {
    texcoord3 = (SoTextureCoordinate3*) node;
  }
  node = this->vertexColor.getValue();
  if (node && node->isOfType(SoBaseColor::getClassTypeId())) {
    basecolor = (SoBaseColor*) node;
  }
  else if (node && node->isOfType(SoPackedColor::getClassTypeId())) {
    packedcolor = (SoPackedColor*) node;
  }

  SoMaterialBundle mb(action);
  mb.sendFirst();

  if (coord3) {
    cc_glglue_glVertexPointer(glue, 3, GL_FLOAT, 0, 
                              (GLvoid*) coord3->point.getValues(0));
  }
  else {
    cc_glglue_glVertexPointer(glue, 4, GL_FLOAT, 0, 
                              (GLvoid*) coord4->point.getValues(0));
  }
  cc_glglue_glEnableClientState(glue, GL_VERTEX_ARRAY);
  
  if (normal) {
    cc_glglue_glNormalPointer(glue, GL_FLOAT, 0, 
                              (GLvoid*) normal->vector.getValues(0));
    cc_glglue_glEnableClientState(glue, GL_NORMAL_ARRAY);
  }
  if (basecolor || packedcolor) {
    if (basecolor) {
      cc_glglue_glColorPointer(glue, 3, GL_FLOAT, 0, 
                               (GLvoid*) basecolor->rgb.getValues(0));
    }
    else {
      cc_glglue_glColorPointer(glue, 4, GL_UNSIGNED_BYTE, 0,
                               (GLvoid*) packedcolor->orderedRGBA.getValues(0));
    }
    cc_glglue_glEnableClientState(glue, GL_COLOR_ARRAY);
  }
  if (texcoord2 || texcoord3) {
    if (texcoord2) {
      cc_glglue_glTexCoordPointer(glue, 2, GL_FLOAT, 0,
                                  (GLvoid*) texcoord2->point.getValues(0));
    }
    else {
      cc_glglue_glTexCoordPointer(glue, 3, GL_FLOAT, 0,
                                  (GLvoid*) texcoord3->point.getValues(0));
    }
    cc_glglue_glEnableClientState(glue, GL_TEXTURE_COORD_ARRAY);
  }

  if (PRIVATE(this)->indexlistdirty) {
    PRIVATE(this)->updateIndexList();
  }
  
  const int32_t * ptr = PRIVATE(this)->indexlist.getArrayPtr();
  const int32_t * endptr =  ptr + PRIVATE(this)->indexlist.getLength();
  
  while (ptr < endptr) {
    int32_t type = *ptr++;
    int32_t len = *ptr++;
    
    cc_glglue_glDrawElements(glue, (GLenum) (-type - 1), len, GL_UNSIGNED_INT, ptr);
    ptr += len;
  }

  cc_glglue_glDisableClientState(glue, GL_VERTEX_ARRAY);
  if (normal) cc_glglue_glDisableClientState(glue, GL_NORMAL_ARRAY);
  if (basecolor || packedcolor) cc_glglue_glDisableClientState(glue, GL_COLOR_ARRAY);
  if (texcoord2 || texcoord3) cc_glglue_glDisableClientState(glue, GL_TEXTURE_COORD_ARRAY);
  SoGLCacheContextElement::shouldAutoCache(state, SoGLCacheContextElement::DONT_AUTO_CACHE);
}

// doc from parent
void
SmVertexArrayShape::getPrimitiveCount(SoGetPrimitiveCountAction *action)
{
  if (!this->shouldPrimitiveCount(action)) return;

  if (PRIVATE(this)->indexlistdirty) {
    PRIVATE(this)->updateIndexList();
  }
  action->addNumTriangles(PRIVATE(this)->numtriangles);
}

// doc from parent
void
SmVertexArrayShape::generatePrimitives(SoAction * action)
{
  if (PRIVATE(this)->indexlistdirty) {
    PRIVATE(this)->updateIndexList();
  }

  SoCoordinate3 * coord3 = NULL ;
  SoCoordinate4 * coord4 = NULL;
  SoTextureCoordinate2 * texcoord2 = NULL;
  SoTextureCoordinate3 * texcoord3 = NULL;
  SoNormal * normal = NULL;
  SoBaseColor * basecolor = NULL;
  SoPackedColor * packedcolor = NULL;

  SoNode * node;
  node = this->vertexCoord.getValue();
  if (node && node->isOfType(SoCoordinate3::getClassTypeId())) {
    coord3 = (SoCoordinate3*) node;
  }
  else if (node && node->isOfType(SoCoordinate4::getClassTypeId())) {
    coord4 = (SoCoordinate4*) node;
  }
  else return; // no data
  
  node = this->vertexNormal.getValue();
  if (node && node->isOfType(SoNormal::getClassTypeId())) {
    normal = (SoNormal*) node;
  }
  node = this->vertexTexCoord.getValue();
  if (node && node->isOfType(SoTextureCoordinate2::getClassTypeId())) {
    texcoord2 = (SoTextureCoordinate2*) node;
  }
  else if (node && node->isOfType(SoTextureCoordinate3::getClassTypeId())) {
    texcoord3 = (SoTextureCoordinate3*) node;
  }
  node = this->vertexColor.getValue();
  if (node && node->isOfType(SoBaseColor::getClassTypeId())) {
    basecolor = (SoBaseColor*) node;
  }
  else if (node && node->isOfType(SoPackedColor::getClassTypeId())) {
    packedcolor = (SoPackedColor*) node;
  }

  const SbVec3f * normals = normal ? normal->vector.getValues(0) : NULL;
  const SbVec3f * c3 = coord3 ? coord3->point.getValues(0) : NULL;
  const SbVec4f * c4 = coord4 ? coord4->point.getValues(0) : NULL;
  const SbVec2f * tc2 = texcoord2 ? texcoord2->point.getValues(0) : NULL;
  const SbVec3f * tc3 = texcoord3 ? texcoord3->point.getValues(0) : NULL;

  SoPrimitiveVertex vertex;
  SoPointDetail pointDetail;
  SoFaceDetail faceDetail;
  vertex.setDetail(&pointDetail);
  SbVec3f dummynormal(0,0,1);
  const SbVec3f *currnormal = &dummynormal;
  if (normals) currnormal = normals;
  vertex.setNormal(*currnormal);

  const int32_t * ptr = PRIVATE(this)->indexlist.getArrayPtr();
  const int32_t * endptr =  ptr + PRIVATE(this)->indexlist.getLength();

  while (ptr < endptr) {
    int32_t type = *ptr++;
    int32_t len = *ptr++;

    SoShape::TriangleShape mode;
    switch (type) {
    case POINTS:
      mode = SoShape::POINTS;
      break;
    case LINES:
      mode = SoShape::LINES;
      break;
    case LINE_LOOP:
      assert(0 && "not supported yet");
      //      mode = SoShape::LINE_LOOP;
    case LINE_STRIP:
      mode = SoShape::LINE_STRIP;
      break;
    case TRIANGLES:
      mode = SoShape::TRIANGLES;
      break;
    case TRIANGLE_STRIP:
      mode = SoShape::TRIANGLE_STRIP;
      break;
    case TRIANGLE_FAN:
      mode = SoShape::TRIANGLE_FAN;
      break;
    case QUADS:
      mode = SoShape::QUADS;
      break;
    case QUAD_STRIP:
      mode = SoShape::QUAD_STRIP;
      break;
    case POLYGON:
      mode = SoShape::POLYGON;
      break;
    default:
      assert(0 && "should not get here");
    }
    this->beginShape(action, mode, &faceDetail);
    while (len--) {
      int idx = *ptr++;
      pointDetail.setMaterialIndex(idx);
      vertex.setMaterialIndex(idx);
      if (normals) vertex.setNormal(normals[idx]);
      pointDetail.setNormalIndex(idx);
      pointDetail.setTextureCoordIndex(idx);
      pointDetail.setCoordinateIndex(idx);
      if (tc2) {
        vertex.setTextureCoords(SbVec4f(tc2[idx][0], tc2[idx][1], 0.0f, 1.0f));
      }
      else if (tc3) {
        vertex.setTextureCoords(SbVec4f(tc3[idx][0], tc3[idx][1], tc3[idx][2], 1.0f));
      }
      if (c3) {
        vertex.setPoint(c3[idx]);
      }
      else {
        SbVec3f t;
        c4[idx].getReal(t);
        vertex.setPoint(t);
      }
      this->shapeVertex(&vertex);
    }
    this->endShape(); 
  }
}

void 
SmVertexArrayShape::notify(SoNotList * l)
{
  SoField * f = l->getLastField();
  if (f == &this->vertexIndex) {
    PRIVATE(this)->indexarraydirty = TRUE;
    PRIVATE(this)->indexlistdirty = TRUE;
  }
  else if (f == &this->vertexCoord) {
    PRIVATE(this)->coordarraydirty = TRUE;
  }
  else if (f == &this->vertexNormal) {
    PRIVATE(this)->normalarraydirty = TRUE;
  }
  else if (f == &this->vertexTexCoord) {
    PRIVATE(this)->texcoordarraydirty = TRUE;
  }
  else if (f == &this->vertexColor) {
    PRIVATE(this)->colorarraydirty = TRUE;
  }
}

void 
SmVertexArrayShapeP::updateIndexList(void)
{
  this->indexlistdirty = FALSE;
  this->indexlist.truncate(0);
  this->numtriangles = 0;
  const int n = PUBLIC(this)->vertexIndex.getNum();;
  if (n == 0) return;
  const int32_t * src = PUBLIC(this)->vertexIndex.getValues(0);
  const int32_t * end = src + n;

  do {
    int type = *src++;
    assert(type < 0);
    this->indexlist.append(type);
    int i = 0;
    int lenidx = this->indexlist.getLength();
    this->indexlist.append(0); // insert a temporary value 
    while (src < end && *src >= 0) { 
      i++;
      this->indexlist.append(*src++);
    }
    this->numtriangles = i - 2;
    this->indexlist[lenidx] = i; // set the correct len value
  } while (src < end);
}

#undef PRIVATE
