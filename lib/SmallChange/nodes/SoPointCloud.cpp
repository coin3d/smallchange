/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
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
#include <Inventor/elements/SoGLLightModelElement.h>
#include <Inventor/caches/SoNormalCache.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoGLShadeModelElement.h>
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
  SO_NODE_ADD_FIELD(radius, (0.1f));
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

  const float r = this->radius.getValue();

  SbVec3f & bmin = box.getMin();
  SbVec3f & bmax = box.getMax();
  
  bmin[0] -= r;
  bmin[1] -= r;
  bmin[2] -= r;

  bmax[0] += r;
  bmax[1] += r;
  bmax[2] += r;
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

// doc from parent
void
SoPointCloud::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();

  SbBool didpush = FALSE;
  if (this->vertexProperty.getValue()) {
    state->push();
    didpush = TRUE;
    this->vertexProperty.getValue()->GLRender(action);
  }

  const SoCoordinateElement * tmp;
  const SbVec3f * normals;
  SbBool needNormals =
    (SoLightModelElement::get(state) !=
     SoLightModelElement::BASE_COLOR);

  SoVertexShape::getVertexData(state, tmp, normals,
                               needNormals);

  if (normals == NULL && needNormals) {
    needNormals = FALSE;
    if (!didpush) {
      state->push();
      didpush = TRUE;
    }
    SoLightModelElement::set(state, SoLightModelElement::BASE_COLOR);
  }

  if (!this->shouldGLRender(action)) {
    if (didpush)
      state->pop();
    return;
  }

  SbBool wasopen = state->isCacheOpen();
  // close the cache, since we don't create a cache dependency on
  // the model matrix or the view volume elements
  state->setCacheOpen(FALSE);
  const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix inversemm = mm.inverse();
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  state->setCacheOpen(wasopen);

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
  float r = this->radius.getValue();
  xaxis *= r;
  yaxis *= r;
  zaxis *= r;
  
  // render in two loops to avoid frequent OpenGL state changes

  SoGLShadeModelElement::forceSend(state, TRUE);

  int i;
  glBegin(GL_QUADS);
  SbVec3f v;

  for (i = 0; i < numpts; i++) {
    v = coords->get3(idx+i);
    float dist = - nearplane.getDistance(v);
    if (dist <= distlimit) {
      glVertex3fv((v - yaxis).getValue());
      glVertex3fv((v + xaxis).getValue());
      glVertex3fv((v + yaxis).getValue());
      if (mbind == PER_VERTEX) mb.send(i, TRUE);
      glVertex3fv((v - xaxis).getValue());
    }
  }
  glEnd();

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

  if (didpush)
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
SoPointCloud::getBoundingBox(SoGetBoundingBoxAction *action)
{
  inherited::getBoundingBox(action);
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
