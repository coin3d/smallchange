/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use SmallChange with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

#include "Coinboard.h"
#include <Inventor/misc/SoState.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/elements/SoDiffuseColorElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/SbRotation.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/SbPlane.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/SbXfBox3f.h>
#include <stdlib.h>
#include <assert.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H

#include <GL/gl.h>

/*!
  \class Coinboard Coinboard.h
  \brief The Coinboard class is a shape node which faces geometry towards the camera.
  \ingroup nodes

  If the position field contains more than one value, the shape is
  rendered several times with a translation as specified in
  position. This makes it possible to have multiple equal billboards
  at different positions. Very useful for rendering particles or a
  forest of trees etc.

*/

/*!
  \enum Coinboard::ShapeType
  The type of shape in the coord field.
*/

/*!
  \var Coinboard::ShapeType Coinboard::TRIANGLES
  Draw triangles (GL_TRIANGLES).
*/

/*!
  \var Coinboard::ShapeType Coinboard::QUADS
  Draw quads (GL_QUADS).
*/

/*!
  \var Coinboard::ShapeType Coinboard::TRIANGLE_STRIP
  Draw triangle strip (GL_TRIANGLE_STRIP).
*/

/*!
  \var Coinboard::ShapeType Coinboard::TRIANGLE_FAN
  Draw triangle fan (GL_TRIANGLE_FAN).
*/

/*!
  \var SoMFVec3f Coinboard::position
  The positions where shapes should be replicated. Default is [0,0,0].
*/

/*!
  \var SoSFVec3f Coinboard::axisOfRotation
  Use this to lock rotation around one axis. Default is [0,1,0].
  Set to [0,0,0] to rotate freely.
*/

/*!
  \var SoSFInt32 Coinboard::frontAxis
  The axis which should be directed towards the camera. This can
  be either 0 (positive x-axis), 1 (positive y-axis) or 2 (positive z-axis).
*/

/*!
  \var SoMFVec3f Coinboard::coord
  The coordinates that will be sent to GL.
*/

/*! \var SoMFVec4f Coinboard::texCoord

  The texture coordinates per vertex. Will be used when you have as
  many texture coordinates as normal coordinates.
*/

/*!
  \var SoSFVec3f Coinboard::normal
  The normal per vertex. Will be used when you have as many normals
  as normal coordinates.
*/

/*!
  \var SoSFEnum Coinboard::shapeType
  Shape type. Default value is TRIANGLES.
*/

/*!
  \var SoMFInt32 Coinboard::numVertices
  To draw several shapes per position, supply the number of vertices
  for each shape here. The default is one value with -1, which means
  all coordinates should be used for the one shape specified by
  the coords field.
*/

SO_NODE_SOURCE(Coinboard);

/*!
  Constructor.
*/
Coinboard::Coinboard()
{
  SO_NODE_CONSTRUCTOR(Coinboard);
  SO_NODE_ADD_FIELD(axisOfRotation, (0.0f, 1.0f, 0.0f));
  SO_NODE_ADD_FIELD(frontAxis, (2));
  SO_NODE_ADD_FIELD(position, (0.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(coord, (0.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(texCoord, (0.0f, 0.0f, 0.0f, 1.0f));
  SO_NODE_ADD_FIELD(normal, (0.0f, 0.0f, 1.0f));
  SO_NODE_ADD_FIELD(numVertices, (-1));
  SO_NODE_ADD_FIELD(shapeType, (TRIANGLES));

  SO_NODE_DEFINE_ENUM_VALUE(ShapeType, TRIANGLES);
  SO_NODE_DEFINE_ENUM_VALUE(ShapeType, QUADS);
  SO_NODE_DEFINE_ENUM_VALUE(ShapeType, TRIANGLE_STRIP);
  SO_NODE_DEFINE_ENUM_VALUE(ShapeType, TRIANGLE_FAN);
  SO_NODE_SET_SF_ENUM_TYPE(shapeType, ShapeType);
}

/*!
  Destructor.
*/
Coinboard::~Coinboard()
{
}

/*!
  Required Coin method.
*/
void
Coinboard::initClass()
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(Coinboard, SoShape, "Shape");
  }
}

void
Coinboard::render(const SbVec3f & offset,
                  const SbVec3f * coords, const int n,
                  const SbVec4f * texcoords)
{
  if (texcoords) {
    for (int i = 0; i < n; i++) {
      glTexCoord4fv(texcoords[i].getValue());
      glVertex3fv((coords[i]+offset).getValue());
    }
  }
  else {
    for (int i = 0; i < n; i++) {
      glVertex3fv((coords[i]+offset).getValue());
    }
  }
}

void
Coinboard::render(const SbMatrix & transform,
                  const SbVec3f * coords, const int n,
                  const SbVec4f * texcoords)
{
  SbVec3f tmp;
  if (texcoords) {
    for (int i = 0; i < n; i++) {
      glTexCoord4fv(texcoords[i].getValue());
      transform.multVecMatrix(coords[i], tmp);
      glVertex3fv(tmp.getValue());
    }
  }
  else {
    for (int i = 0; i < n; i++) {
      transform.multVecMatrix(coords[i], tmp);
      glVertex3fv(tmp.getValue());
    }
  }
}

/*!
  This is a Coin method. It renders all shapes facing the camera.
*/
void
Coinboard::GLRender(SoGLRenderAction * action)
{
  int num = this->position.getNum();
  int numcoords = this->coord.getNum();
  if (num == 0 || numcoords < 3) return;

  const int32_t * verts = this->numVertices.getValues(0);
  int numv = this->numVertices.getNum();
  if (numv == 1 && verts[0] == -1) numv = 0;
  else if (numv && numv != num) return;

  if (!this->shouldGLRender(action)) return;

  const SbVec3f * pos = this->position.getValues(0);
  const SbVec3f * coords = this->coord.getValues(0);
  const SbVec4f * texcoords = NULL;

  GLenum type;
  switch ((ShapeType) this->shapeType.getValue()) {
  case TRIANGLES: type = GL_TRIANGLES; break;
  case QUADS: type = GL_QUADS; break;
  case TRIANGLE_STRIP: type = GL_TRIANGLE_STRIP; break;
  default: // avoid compiler warnings
  case TRIANGLE_FAN: type = GL_TRIANGLE_FAN; break;
  }

  SoState * state = action->getState();

  SbBool doTextures = SoGLTextureEnabledElement::get(state);
  if (doTextures) {
    texcoords = this->texCoord.getValues(0);
  }
  SoMaterialBundle mb(action);
  mb.sendFirst();

  SbBool neednormal = SoLightModelElement::get(state) != SoLightModelElement::BASE_COLOR;
  SbVec3f normal = this->normal.getValue();

  const SbMatrix & mm = SoModelMatrixElement::get(state);

  SbVec3f toviewer;
  SbVec3f cameray(0.0f, 1.0f, 0.0f);
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  toviewer = - vv.getProjectionDirection();
  mm.inverse().multDirMatrix(toviewer, toviewer);
  toviewer.normalize();

  SbVec3f rotaxis = this->axisOfRotation.getValue();

  if (rotaxis == SbVec3f(0.0f, 0.0f, 0.0f)) {
    SbMatrix viewmat = SoViewingMatrixElement::get(state).inverse();

    SbMatrix mymat = mm;
    mymat[0][0] = viewmat[0][0];
    mymat[0][1] = viewmat[0][1];
    mymat[0][2] = viewmat[0][2];
    mymat[1][0] = viewmat[1][0];
    mymat[1][1] = viewmat[1][1];
    mymat[1][2] = viewmat[1][2];
    mymat[2][0] = viewmat[2][0];
    mymat[2][1] = viewmat[2][1];
    mymat[2][2] = viewmat[2][2];
    mymat[3][0] = 0.0f;
    mymat[3][1] = 0.0f;
    mymat[3][2] = 0.0f;

    state->push();
    SoModelMatrixElement::set(state, this, mymat);

    mymat = SoViewingMatrixElement::get(state);
    mymat[3][0] = 0.0f;
    mymat[3][1] = 0.0f;
    mymat[3][2] = 0.0f;
    mymat.multLeft(mm);
    SbVec3f tmp;

    if (neednormal) glNormal3fv(normal.getValue());

    if (type == GL_TRIANGLES || type == GL_QUADS) {
      glBegin(type);
      for (int i = 0; i < num; i++) {
        mymat.multVecMatrix(pos[i], tmp);
        if (numv) {
          this->render(tmp, coords, verts[i], texcoords);
          coords += verts[i];
          if (texcoords) texcoords += verts[i];
        }
        else {
          this->render(tmp, coords, numcoords, texcoords);
        }
      }
      glEnd();
    }
    else {
      for (int i = 0; i < num; i++) {
        mymat.multVecMatrix(pos[i], tmp);
        glBegin(type);
        if (numv) {
          this->render(tmp, coords, verts[i], texcoords);
          coords += verts[i];
          if (texcoords) texcoords += verts[i];
        }
        else {
          this->render(tmp, coords, numcoords, texcoords);
        }
        glEnd();
      }
    }
    state->pop();
  }
  else {
    int axisnum = this->frontAxis.getValue();
    SbVec3f zaxis(0.0f, 0.0f, 0.0f);
    zaxis[axisnum] = 1.0f;
    SbPlane plane(rotaxis.cross(toviewer), 0.0f);
    const SbVec3f n = plane.getNormal();
    SbVec3f vecinplane = zaxis - n * n[axisnum];
    vecinplane.normalize();
    if (vecinplane.dot(toviewer) < 0.0f) vecinplane = - vecinplane;
    float angle = acos(SbClamp(vecinplane.dot(zaxis), -1.0f, 1.0f));
    if (n[axisnum] > 0.0f) angle = -angle;
    SbRotation rot(rotaxis, angle);

    if (neednormal) {
      rot.multVec(normal, normal);
      glNormal3fv(normal.getValue());
    }

    if (type == GL_TRIANGLES || type == GL_QUADS) {
      glBegin(type);
      for (int i = 0; i < num; i++) {
        SbMatrix mat, tmp;
        mat.setTranslate(pos[i]);
        tmp.setRotate(rot);
        mat.multLeft(tmp);
        if (numv) {
          this->render(mat, coords, verts[i], texcoords);
          coords += verts[i];
          if (texcoords) texcoords += verts[i];
        }
        else {
          this->render(mat, coords, numcoords, texcoords);
        }
      }
      glEnd();
    }
    else {
      for (int i = 0; i < num; i++) {
        SbMatrix mat, tmp;
        mat.setTranslate(pos[i]);
        tmp.setRotate(rot);
        mat.multLeft(tmp);

        glBegin(type);
        if (numv) {
          this->render(mat, coords, verts[i], texcoords);
          coords += verts[i];
          if (texcoords) texcoords += verts[i];
        }
        else {
          this->render(mat, coords, numcoords, texcoords);
        }
        glEnd();
      }
    }
  }
}

/*!
  This is a Coin method. It should generate primitives, but is currently not implemented.
*/
void
Coinboard::generatePrimitives(SoAction * action)
{
}

/*!
  This is a Coin method. It calculates the bounding box of the fron
  facing shapes.
*/
void
Coinboard::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  int num = this->position.getNum();
  int numcoords = this->coord.getNum();

  const SbVec3f * pos = this->position.getValues(0);
  const SbVec3f * coords = this->coord.getValues(0);

  box.makeEmpty();
  for (int i = 0; i < num; i++) {
    for (int j = 0; j < numcoords; j++) {
      box.extendBy(coords[j]+pos[i]);
    }
  }
  center = box.getCenter();
}

/*!
  This is a Coin method. It should return the number of primitives,
  but is currently not implemented.
*/
void
Coinboard::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
}

void
Coinboard::generate(const SbMatrix & matrix,
                    const SbVec3f * coords, const int n,
                    const SbVec4f * texcoords)
{
}


void
Coinboard::generate(const SbVec3f & offset,
                    const SbVec3f * coords, const int n,
                    const SbVec4f * texcoords)
{
}
