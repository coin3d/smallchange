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
  \class SoPointCloud SoPointCloud.h Inventor/nodes/SoPointCloud.h
  \brief The SoPointCloud class is used to display a set of 3D points.
  \ingroup nodes

  When a point is closer to the camera than a specified distance,
  it will be rendered as a quad.

  This node either uses the coordinates currently on the state
  (typically set up by a leading SoCoordinate3 node in the scenegraph)
  or from a SoVertexProperty node attached to this node to render a
  set of 3D points.

  The SoPointCloud::numPoints field specifies the number of points in
  the coordinate set which should be rendered (or otherwise handled by
  traversal actions).
*/

#include "SoPointCloud.h"

#include <Inventor/misc/SoState.h>
#include <Inventor/bundles/SoTextureCoordinateBundle.h>
#include <Inventor/SoPrimitiveVertex.h>

#include <Inventor/actions/SoGLRenderAction.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H

#include <GL/gl.h>

#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/elements/SoNormalBindingElement.h>
#include <Inventor/elements/SoMaterialBindingElement.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/caches/SoNormalCache.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/SbPlane.h>

#if COIN_DEBUG
#include <Inventor/errors/SoDebugError.h>
#endif // COIN_DEBUG

/*!
  \var SoSFInt32 SoPointCloud::numPoints

  Used to specify number of points in the point cloud. Coordinates for
  the points will be taken from the state stack's set of 3D
  coordinates, typically set up by a leading SoCoordinate3 node.

  If this field is equal to -1 (the default value) \e all coordinates
  currently on the state will be rendered or otherwise handled by
  traversal actions.

  SoPointCloud inherits the field SoNonIndexedShape::startIndex, which
  specifies the start index for points from the current state set of
  coordinates. Please note that this field has been obsoleted, but is
  still provided for compatibility.

*/

/*!
  \var  SoSFFloat detailDistance

  The distance (int the local coordinate system) when the points
  will be rendered as quads.
*/

/*!
  \var SoSFFloat radius
  
  The radius of the points when rendered as quads.
*/

SO_NODE_SOURCE(SoPointCloud);

/*!
  Constructor.
*/
SoPointCloud::SoPointCloud()
{
  SO_NODE_CONSTRUCTOR(SoPointCloud);

  SO_NODE_ADD_FIELD(numPoints, (-1));
  SO_NODE_ADD_FIELD(detailDistance, (10.0f));
  SO_NODE_ADD_FIELD(xSize, (0.1f));
  SO_NODE_ADD_FIELD(ySize, (0.1f));
  SO_NODE_ADD_FIELD(zSize, (0.1f));
  SO_NODE_ADD_FIELD(shape, (CUBE));
  SO_NODE_ADD_FIELD(mode, (DISTANCE_BASED));

  SO_NODE_DEFINE_ENUM_VALUE(Shape, BILLBOARD_DIAMOND);
  SO_NODE_DEFINE_ENUM_VALUE(Shape, CUBE);

  SO_NODE_DEFINE_ENUM_VALUE(Mode, ALWAYS_POINTS);
  SO_NODE_DEFINE_ENUM_VALUE(Mode, DISTANCE_BASED);
  SO_NODE_DEFINE_ENUM_VALUE(Mode, ALWAYS_SHAPE);

  SO_NODE_SET_SF_ENUM_TYPE(mode, Mode);
  SO_NODE_SET_SF_ENUM_TYPE(shape, Shape);
}

/*!
  Destructor.
*/
SoPointCloud::~SoPointCloud()
{
}

// doc from parent
void
SoPointCloud::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(SoPointCloud, SoNonIndexedShape, "NonIndexedShape");
  }
}

// doc from parent
void
SoPointCloud::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  inherited::computeCoordBBox(action, this->numPoints.getValue(), box, center);

  const float rx = this->xSize.getValue() * 0.5f;
  const float ry = this->ySize.getValue() * 0.5f;
  const float rz = this->zSize.getValue() * 0.5f;

  SbVec3f & bmin = box.getMin();
  SbVec3f & bmax = box.getMax();
  
  bmin[0] -= rx;
  bmin[1] -= ry;
  bmin[2] -= rz;

  bmax[0] += rx;
  bmax[1] += ry;
  bmax[2] += rz;
}

// Internal method which translates the current material binding
// found on the state to a material binding for this node.
// PER_PART, PER_FACE, PER_VERTEX and their indexed counterparts
// are translated to PER_VERTEX binding. OVERALL means overall
// binding for point set also, of course. The default material
// binding is OVERALL.
SoPointCloud::Binding
SoPointCloud::findMaterialBinding(SoState * const state) const
{
  Binding binding = OVERALL;
  if (SoMaterialBindingElement::get(state) !=
      SoMaterialBindingElement::OVERALL) binding = PER_VERTEX;
  return binding;
}

//
// the 6 quads in the cube
//
static int cube_vindices[] =
{
  0, 1, 3, 2,
  5, 4, 6, 7,
  1, 5, 7, 3,
  4, 0, 2, 6,
  4, 5, 1, 0,
  2, 3, 7, 6
};

static float cube_normals[] =
{
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, -1.0f,
  -1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, -1.0f, 0.0f
};

static void
generate_cube_vertices(SbVec3f *varray,
                       const SbVec3f & center,
                       const float w,
                       const float h,
                       const float d)
{
  for (int i = 0; i < 8; i++) {
    varray[i].setValue((i&1) ? -w : w,
                       (i&2) ? -h : h,
                       (i&4) ? -d : d);
    varray[i] += center;
  }
}

static void
render_cube(const SbVec3f & center,
            const float width,
            const float height,
            const float depth)
{
  SbVec3f varray[8];
  generate_cube_vertices(varray,
                         center,
                         width * 0.5f,
                         height * 0.5f,
                         depth * 0.5f);
  int * iptr = cube_vindices;

  for (int i = 0; i < 6; i++) { // 6 quads
    glNormal3fv((const GLfloat*)&cube_normals[i*3]);
    for (int j = 0; j < 4; j++) {
      glVertex3fv((const GLfloat*)&varray[*iptr++]);
    }
  }
}


// doc from parent
void
SoPointCloud::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  state->push();

  if (this->vertexProperty.getValue()) {
    this->vertexProperty.getValue()->GLRender(action);
  }

  const SoCoordinateElement * tmp;
  const SbVec3f * normals;
  SbBool needNormals = FALSE;

  SoVertexShape::getVertexData(state, tmp, normals,
                               needNormals);

  Shape rendershape = (Shape) this->shape.getValue();

  if (!this->shouldGLRender(action)) {
    state->pop();
    return;
  }
  
  SbBool waslightingenabled = glIsEnabled(GL_LIGHTING);
  SbBool wascullingenabled = glIsEnabled(GL_CULL_FACE);
  SbBool islightingenabled = waslightingenabled;
  SbBool iscullingenabled = wascullingenabled;

  const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix inversemm = mm.inverse();
  const SbViewVolume & vv = SoViewVolumeElement::get(state);

  SbVec3f zaxis = vv.getProjectionDirection();
  SbVec3f yaxis = vv.getViewUp();
  SbVec3f xaxis = yaxis.cross(zaxis);

  inversemm.multDirMatrix(xaxis, xaxis);
  inversemm.multDirMatrix(yaxis, yaxis);

  // move near plane to object space
  SbPlane nearplane = vv.getPlane(0.0f);
  // transform plane into this node's coordinate system
  nearplane.transform(inversemm);

  const SoGLCoordinateElement * coords = (SoGLCoordinateElement *)tmp;

  Binding mbind = this->findMaterialBinding(action->getState());

  SoMaterialBundle mb(action);
  mb.sendFirst(); // make sure we have the correct material

  int32_t idx = this->startIndex.getValue();
  int32_t numpts = this->numPoints.getValue();
  if (numpts < 0) numpts = coords->getNum() - idx;
  
  float distlimit = this->detailDistance.getValue();

  float r = this->xSize.getValue() * 0.5f;
  xaxis *= r;
  yaxis *= r;
  zaxis *= r;
  
  float dx = this->xSize.getValue();
  float dy = this->ySize.getValue();
  float dz = this->zSize.getValue();

  // render in two loops to avoid frequent OpenGL state changes

  // FIXME: add test here when adding more shapes
  if (rendershape == CUBE) {
    if (!islightingenabled) {
      glEnable(GL_LIGHTING);
      islightingenabled = TRUE;
    }
  }
  else if (islightingenabled) {
    glDisable(GL_LIGHTING);
    islightingenabled = FALSE;
  }
  
  if (rendershape == BILLBOARD_DIAMOND) {
    if (iscullingenabled) {
      glDisable(GL_CULL_FACE);
      iscullingenabled = FALSE;
    }
  }

  int i;
  glBegin(GL_QUADS);
  SbVec3f v;
  
  for (i = 0; i < numpts; i++) {
    v = coords->get3(idx+i);
    float dist = - nearplane.getDistance(v);
    if (dist <= distlimit) {
      if (rendershape == CUBE) {
        if (mbind == PER_VERTEX) mb.send(i, TRUE);
        render_cube(v, dx, dy, dz);
      }
      else if (rendershape == BILLBOARD_DIAMOND) {
        glVertex3fv((v - yaxis).getValue());
        glVertex3fv((v + xaxis).getValue());
        glVertex3fv((v + yaxis).getValue());
        if (mbind == PER_VERTEX) mb.send(i, TRUE);
        glVertex3fv((v - xaxis).getValue());
      }
    }
  }
  glEnd();

  if (islightingenabled) {
    glDisable(GL_LIGHTING);
    islightingenabled = FALSE;
  }


  glBegin(GL_POINTS);
  for (i = 0; i < numpts; i++) {
    v = coords->get3(idx+i);
    float dist = - nearplane.getDistance(v);
    if (dist > distlimit) {
      if (mbind == PER_VERTEX) mb.send(i, TRUE);
      glVertex3f(v[0], v[1], v[2]);
    }
  }
  glEnd();

  if (waslightingenabled && !islightingenabled) {
    glEnable(GL_LIGHTING);
  }
  else if (!waslightingenabled && islightingenabled) {
    glDisable(GL_LIGHTING);
  }

  if (iscullingenabled && !wascullingenabled) {
    glDisable(GL_CULL_FACE);
  }
  if (!iscullingenabled && wascullingenabled) {
    glEnable(GL_CULL_FACE);
  }
  state->pop();
}

// Documented in superclass.
SbBool
SoPointCloud::generateDefaultNormals(SoState *, SoNormalCache * nc)
{
  // Overridden to clear normal cache, as it's not possible to
  // generate a normal for a point.
  nc->set(0, NULL);
  return TRUE;
}

// Documented in superclass.
SbBool
SoPointCloud::generateDefaultNormals(SoState * state, SoNormalBundle * bundle)
{
  // Overridden to avoid (faulty) compiler warnings with some version
  // of g++.
  return FALSE;
}

// doc from parent
void
SoPointCloud::getPrimitiveCount(SoGetPrimitiveCountAction *action)
{
  if (!this->shouldPrimitiveCount(action)) return;
  int num = this->numPoints.getValue();
  if (num < 0) {
    SoVertexProperty *vp = (SoVertexProperty*) this->vertexProperty.getValue();
    if (vp && vp->vertex.getNum()) {
      num = vp->vertex.getNum() - this->startIndex.getValue();
    }
    else {
      const SoCoordinateElement *coordelem =
        SoCoordinateElement::getInstance(action->getState());
      num = coordelem->getNum() - this->startIndex.getValue();
    }
  }
  action->addNumPoints(num);
}

// doc from parent
void
SoPointCloud::generatePrimitives(SoAction *action)
{
  SoState * state = action->getState();

  if (this->vertexProperty.getValue()) {
    state->push();
    this->vertexProperty.getValue()->doAction(action);
  }

  const SoCoordinateElement *coords;
  const SbVec3f * normals;
  SbBool needNormals = FALSE;

  SoVertexShape::getVertexData(action->getState(), coords, normals,
                               needNormals);

  Binding mbind = this->findMaterialBinding(action->getState());

  SoPrimitiveVertex vertex;
  SoPointDetail pointDetail;
  vertex.setDetail(&pointDetail);

  int32_t idx = this->startIndex.getValue();
  int32_t numpts = this->numPoints.getValue();
  if (numpts < 0) numpts = coords->getNum() - idx;

  int matnr = 0;
  int texnr = 0;
  int normnr = 0;

  this->beginShape(action, SoShape::POINTS);
  for (int i = 0; i < numpts; i++) {
    if (mbind == PER_VERTEX) {
      pointDetail.setMaterialIndex(matnr);
      vertex.setMaterialIndex(matnr++);
    }
    pointDetail.setCoordinateIndex(idx);
    vertex.setPoint(coords->get3(idx++));
    this->shapeVertex(&vertex);
  }
  this->endShape();

  if (this->vertexProperty.getValue())
    state->pop();
}
