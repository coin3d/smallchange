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

/*!
  \class SoLODExtrusion SoLODExtrusion.h
  \brief Extrusion node with auto level of detail per spine segment

  Renders an extrusion of \e crossSection along \e spine, for
  all \e spine segments closer to camera than \e lodDistance1. Between
  \e lodDistance1 and \e lodDistance2 , \e spine segments are rendered
  as simple lines. Beyond \e lodDistance2 nothing is rendered. 
  To render lines to infinity, set \e lodDistance2 to less than zero.
  (the default value for \e lodDistance2 is -1.0).

  The node currently renders the extrusion with per vertex normal
  vectors. Per (spine) vertex material is not yet supported, nor
  are texture coordinates.

  \sa SoVRMLExtrusion
*/

/*! \var SoSFBool SoLODExtrusion::beginCap
  Whether or not to render \e crossSection as a polygon at the
  beginning of the extrusion.
 */

/*! \var SoSFBool SoLODExtrusion::endCap
  Whether or not to render \e crossSection as a polygon at the
  end of the extrusion.
 */

/*! \var SoSFBool SoLODExtrusion::ccw
  Whether or not \e crossSection vertices are ordered counter-clockwise
  or not.
 */

/*! \var SoSFFloat SoLODExtrusion::creaseAngle
  Crease angle for normal vector calculation.
 */

/*! \var SoMFVec2f SoLODExtrusion::crossSection
  Cross section of the extrusion, given as a list of 2D vertices.
 */

/*! \var SoMFVec3f SoLODExtrusion::spine
  The central spine of the extrusion, given as a list of 3D
  vertices.
 */

/*! \var SoMFVec3f SoLODExtrusion::color
  If color.getNum() == spine.getNum(), the extrusion is colored
  per spine point.
 */

/*! \var SoSFFloat SoLODExtrusion::radius
  If radius is set to a value greater than zero, \e crossSection
  will be set to represent a circle of radius \e radius. Any other
  setting of \e crossSection is ignored as long as \e radius is
  greater than zero. Default value for \e radius is -1.0.
 */

/*! \var SoSFInt32 SoLODExtrusion::circleSegmentCount
  How many line segments will be used to represent a circular
  cross section. Relevant only when \e radius is greater than
  zero.
 */

/*! \var SoSFFloat SoLODExtrusion::lodDistance1
  The distance from camera where extrusion rendering is replaced
  by line rendering.
 */

/*! \var SoSFFloat SoLODExtrusion::lodDistance2
  The distance from camera where line rendering will be replaced
  by nothing, if \e lodDistance2 is greater than zero. If 
  \e lodDistance2 is less than zero, lines will be rendered to
  infinity (or to the end of the spine, or to the far clipping
  plane, whichever is closer to camera). Default value for
  \e lodDistance2 is -1.0.
 */

/*!
  \var SoSFVec3f SoLODExtrusion::zAxis
  
  Makes it possible to lock the extrusion Z-axis to always have this 
  value. The extrusion coordinate system will need to be orthonormal,
  of course, so the Z-axis will be transformed to accout for this.
  Default value is (0, 0, 0), which means that the Z-axis is not 
  locked.
*/

#include "SoLODExtrusion.h"
// #include <Inventor/nodes/SoSubNodeP.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/misc/SoNormalGenerator.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/bundles/SoTextureCoordinateBundle.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/elements/SoTextureCoordinateElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/SbTesselator.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/details/SoFaceDetail.h>
#include <float.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>


//
// needed to avoid warnings generated by SbVec3f::normalize
//
static float
my_normalize(SbVec3f & vec)
{
  float len = vec.length();
  if (len > FLT_EPSILON) {
    vec /= len;
  }
  return len;
}


#ifndef DOXYGEN_SKIP_THIS
class SoLODExtrusionP {
public:
  SoLODExtrusionP(SoLODExtrusion * master)
    :master(master),
     coord(32),
     tcoord(32),
     idx(32),
     color_idx(32),
     segidx(32),
     striplens(32),
     gen(NULL),
     tess(tess_callback, this),
     dirty(TRUE)
  {
  }

  SoLODExtrusion * master;
  SbList <SbVec3f> coord;
  SbList <SbVec2f> tcoord;
  SbList <int> idx;     // vertex index
  SbList <int> color_idx;     // per coord color index (into master->color), matches coord list
  SbList <int> segidx;  // index into idx, for each spine segment
  SbList <int32_t> striplens;  // lengths of tri-strips
  SbList <float> spinelens;    // geometric length of each spine segment, precalculated
  SoNormalGenerator * gen;
  SbTesselator tess;
  SbBool dirty;

  static void tess_callback(void *, void *, void *, void *);
  void generateCoords(void);
  void generateNormals(void);
  void renderSegidx( const int, const SbBool);
  void makeCircleCrossSection( const float, const int);
};
#endif // DOXYGEN_SKIP_THIS

#undef THIS
#define THIS this->pimpl


SO_NODE_SOURCE(SoLODExtrusion);

void
SoLODExtrusion::initClass(void) // static
{
  SO_NODE_INIT_CLASS(SoLODExtrusion, SoShape, "SoShape");
}

SoLODExtrusion::SoLODExtrusion(void)
{
  THIS = new SoLODExtrusionP(this);

  SO_NODE_CONSTRUCTOR(SoLODExtrusion);

  SO_NODE_ADD_FIELD(beginCap, (TRUE));
  SO_NODE_ADD_FIELD(endCap, (TRUE));
  SO_NODE_ADD_FIELD(ccw, (FALSE));
  SO_NODE_ADD_FIELD(creaseAngle, (0.0f));

  SO_NODE_ADD_FIELD(crossSection, (0.0f, 0.0f));
  this->crossSection.setNum(5);
  SbVec2f * cs = this->crossSection.startEditing();
  cs[0] = SbVec2f(1.0f, 1.0f);
  cs[1] = SbVec2f(1.0f, -1.0f);
  cs[2] = SbVec2f(-1.0f, -1.0f);
  cs[3] = SbVec2f(-1.0f, 1.0f);
  cs[4] = SbVec2f(1.0f, 1.0f);
  this->crossSection.finishEditing();
  this->crossSection.setDefault(TRUE);

  SO_NODE_ADD_FIELD(spine, (0.0f, 0.0f, 0.0f));
  this->spine.setNum(2);
  this->spine.set1Value(1, 0.0f, 1.0f, 0.0f);
  this->spine.setDefault(TRUE);

  SO_NODE_ADD_FIELD(color, (1.0f, 0.0f, 0.0f));

  SO_NODE_ADD_FIELD(radius, (-1.0f));
  SO_NODE_ADD_FIELD(circleSegmentCount, (10));
  SO_NODE_ADD_FIELD(lodDistance1, (1000.0));
  SO_NODE_ADD_FIELD(lodDistance2, (-1.0));   // default lines to infinity
  SO_NODE_ADD_FIELD(zAxis, (0.0f, 0.0f, 0.0f));
}

SoLODExtrusion::~SoLODExtrusion()
{
  delete THIS->gen;
  delete THIS;
}


void
SoLODExtrusion::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

  this->updateCache();

  SoState * state = action->getState();
  SoMaterialBundle mb(action);
  mb.sendFirst();

  // Find camera position in local coordinate space
  SbVec3f cameralocal;
  const SbMatrix &tempmat = SoModelMatrixElement::get(state);
  const SbMatrix &matrix = tempmat.inverse();
  const SbViewVolume &vv = SoViewVolumeElement::get(state);
  matrix.multVecMatrix(vv.getProjectionPoint(), cameralocal);

  int spinelength = this->spine.getNum();
  SbBool use_color;
  if( spinelength == this->color.getNum())
    use_color = TRUE;
  else
    use_color = FALSE;

  const SbVec3f * sv = this->spine.getValues(0);
  const SbVec3f * colorv = this->color.getValues(0);
  const float * lengths = this->pimpl->spinelens.getArrayPtr();
  int lodmode, i = 0;
  float dist, ld1dist, ld2dist, accdist;

  while (i < spinelength-1) {
    accdist = 0.0;
    dist = (cameralocal - sv[i]).length();
    ld1dist = SbAbs(dist - this->lodDistance1.getValue());
    ld2dist = SbAbs(dist - this->lodDistance2.getValue());
    if (dist < this->lodDistance1.getValue()) {
      lodmode = 0;
    }
    else if (this->lodDistance2.getValue() > 0.0) {
      if (dist < this->lodDistance2.getValue()) {
	lodmode = 1;
      }
      else { 
	lodmode = 2;
      }
    }
    else {
      lodmode = 1;
    }
    switch (lodmode) {
    case 0:    // render extrusion until above lodDistance1
      if (i == 0)
	THIS->renderSegidx(i, use_color);  // render begin cap
      while (accdist < ld1dist && i < spinelength-1) {
	THIS->renderSegidx(i + 1, use_color);
	accdist += lengths[i];
	i++;
      }
      if (i == spinelength - 1) { 
	THIS->renderSegidx(i+1, use_color);  // render end cap
      }
      break;
    case 1:    // render line until crossing a lodDistance
      {
        SbBool wasenabled = glIsEnabled(GL_LIGHTING);
        if (wasenabled) glDisable(GL_LIGHTING);
        glBegin(GL_LINE_STRIP);
        while (accdist < ld1dist && accdist < ld2dist && i < spinelength-1) {
          if( use_color )
            glColor3fv((const GLfloat*)colorv[i].getValue());
          glVertex3fv((const GLfloat*)sv[i].getValue());
          accdist += lengths[i];
          i++;
        }
        if( use_color )
          glColor3fv((const GLfloat*)colorv[i].getValue());
        glVertex3fv((const GLfloat*)sv[i].getValue());
        glEnd();
        if (wasenabled) glEnable(GL_LIGHTING);
      }
      break;
    case 2:    // render nothing until beneath lodDistance2
      while (accdist < ld2dist && i < spinelength-1) {
	accdist += lengths[i];
	i++;
      }
      break;
    default:
      i = spinelength + 1;  // exit loop on error
      break;
    }
  } 
}

void
SoLODExtrusion::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
  this->updateCache();
  action->addNumTriangles(THIS->idx.getLength() / 4);
}

void
SoLODExtrusion::computeBBox(SoAction * action,
                             SbBox3f & box,
                             SbVec3f & center)
{
  this->updateCache();

  int num = THIS->coord.getLength();
  const SbVec3f * coords = THIS->coord.getArrayPtr();

  box.makeEmpty();
  while (num--) {
    box.extendBy(*coords++);
  }

  num = this->spine.getNum();
  coords = this->spine.getValues(0);
  while(num--) {
    box.extendBy(*coords++);
  }

  if (!box.isEmpty()) center = box.getCenter();
}

void
SoLODExtrusion::generatePrimitives(SoAction * action)
{
  this->updateCache();
  
  // FIXME: generate triangles

  /*
  const SbVec3f * normals = THIS->gen.getNormals();
  const SbVec2f * tcoords = THIS->tcoord.getArrayPtr();
  const SbVec3f * coords = THIS->coord.getArrayPtr();
  const int32_t * iptr = THIS->idx.getArrayPtr();
  const int32_t * endptr = iptr + THIS->idx.getLength();

  SoPrimitiveVertex vertex;
  SoFaceDetail faceDetail;
  SoPointDetail pointDetail;

  vertex.setDetail(&pointDetail);

  int idx;
  int nidx = 0;
  this->beginShape(action, TRIANGLES, &faceDetail);
  while (iptr < endptr) {
    idx = *iptr++;
    while (idx >= 0) {
      pointDetail.setNormalIndex(nidx++);
      pointDetail.setCoordinateIndex(idx);
      vertex.setNormal(*normals++);
      vertex.setTextureCoords(tcoords[idx]);
      vertex.setPoint(coords[idx]);
      this->shapeVertex(&vertex);
      idx = *iptr++;
    }
  }
  this->endShape();
  */
}

void
SoLODExtrusion::updateCache(void)
{
  if (THIS->dirty) {
    THIS->generateCoords();
    THIS->generateNormals();
    THIS->dirty = FALSE;
  }
}

/*!
  Overloaded to disable geometry cache.
*/
void
SoLODExtrusion::notify(SoNotList * list)
{
  THIS->dirty = TRUE;
  inherited::notify(list);
}


SoDetail * 
SoLODExtrusion::createTriangleDetail(SoRayPickAction * action,
                                      const SoPrimitiveVertex * v1,
                                      const SoPrimitiveVertex * v2,
                                      const SoPrimitiveVertex * v3,
                                      SoPickedPoint * pp)
{
  // no triangle detail for Extrusion
  return NULL;
}

#undef THIS
#ifndef DOXYGEN_SKIP_THIS

void
SoLODExtrusionP::generateCoords(void)
{
  int cursegidx = 0;

  this->coord.truncate(0);
  this->color_idx.truncate(0);
  this->tcoord.truncate(0);
  this->idx.truncate(0);
  this->segidx.truncate(0);
  this->striplens.truncate(0);
  this->spinelens.truncate(0);

  // Create circular cross section if radius > 0.0
  if (this->master->radius.getValue() > 0.0) {
    this->makeCircleCrossSection(this->master->radius.getValue(),
                                 this->master->circleSegmentCount.getValue());
  }
  if (this->master->crossSection.getNum() == 0 ||
      this->master->spine.getNum() == 0) return;
  
  SbMatrix matrix = SbMatrix::identity();

  int i, j, numcross;
  SbBool connected = FALSE;   // is cross section closed
  SbBool closed = FALSE;      // is spine closed
  numcross = this->master->crossSection.getNum();
  const SbVec2f * cross =  master->crossSection.getValues(0);
  if (cross[0] == cross[numcross-1]) {
    connected = TRUE;
    numcross--;
  }

  int numspine = master->spine.getNum();
  const SbVec3f * spine = master->spine.getValues(0);
  if (spine[0] == spine[numspine-1]) {
    closed = TRUE;
    //    numspine--;
  }

  SbVec3f zaxis = this->master->zAxis.getValue();
  SbBool dolockz = zaxis != SbVec3f(0.0f, 0.0f, 0.0f);
  if (dolockz) zaxis.normalize();

  SbVec3f X, Y, Z;
  SbVec3f prevX(1.0f, 0.0f, 0.0f);
  SbVec3f prevY(0.0f, 1.0f, 0.0f);
  SbVec3f prevZ(0.0f, 0.0f, 1.0f);

  //  int numorient = this->master->orientation.getNum();
  //  const SbRotation * orient = this->master->orientation.getValues(0);
  int numorient = 0;
  const SbRotation * orient = NULL;

  //  int numscale = this->master->scale.getNum();
  //  const SbVec2f * scale = this->master->scale.getValues(0);
  int numscale = 0;
  const SbVec2f * scale = NULL;

  int reversecnt = 0;

  // loop through all spines
  for (i = 0; i < numspine; i++) {
    if (i < numspine-1) {
      this->spinelens.append( (spine[i+1]-spine[i]).length() );
    }
    if (closed) {
      if (i > 0)
        Y = spine[i+1] - spine[i-1];
      else
        Y = spine[1] - spine[numspine-1];
    }
    else {
      if (i == 0) Y = spine[1] - spine[0];
      else if (i == numspine-1) Y = spine[numspine-1] - spine[numspine-2];
      else Y = spine[i+1] - spine[i-1];
    }

    if (my_normalize(Y) <= FLT_EPSILON) {
      if (prevY[1] < 0.0f)
        Y = SbVec3f(0.0f, -1.0f, 0.0f);
      else
        Y = SbVec3f(0.0f, 1.0f, 0.0f);
    }
    
    if (dolockz) {
      Z = zaxis;
      X = Y.cross(Z);
      X.normalize();
      Z = X.cross(Y);
      Z.normalize();
    }
    else {
      SbVec3f z0, z1;
      
      if (closed) {
        if (i > 0) {
          z0 = spine[i+1] - spine[i];
          z1 = spine[i-1] - spine[i];
        }
        else {
          z0 = spine[1] - spine[0];
          z1 = spine[numspine-1] - spine[0];
        }
      }
      else {
        if (numspine == 2) {
          z0 = SbVec3f(1.0f, 0.0f, 0.0f);
          z1 = Y;
          Z = z0.cross(z1);
          if (my_normalize(Z) <= FLT_EPSILON) {
            z0 = SbVec3f(0.0f, 1.0f, 0.0f);
            z1 = Y;
          }
        }
        else if (i == 0) {
          z0 = spine[2] - spine[1];
          z1 = spine[0]-spine[1];
        }
        else if (i == numspine-1) {
          z0 = spine[numspine-1] - spine[numspine-2];
          z1 = spine[numspine-3]-spine[numspine-2];
        }
        else {
          z0 = spine[i+1] - spine[i];
          z1 = spine[i-1] - spine[i];
        }
      }
      
      my_normalize(z0);
      my_normalize(z1);
      
      // test if spine segments are (almost) parallel. If they are, the
      // cross product will not be reliable, and we should just use the
      // previous Z-axis instead.
      if (z0.dot(z1) < -0.999f) {
        Z = prevZ;
      }
      else {
        Z = z0.cross(z1);
      }
      
      if (my_normalize(Z) <= FLT_EPSILON || SbAbs(Y.dot(Z)) > 0.5f) {
        Z = SbVec3f(0.0f, 0.0f, 0.0f);
        
        int bigy = 0;
        float bigyval = Y[0];
        if (SbAbs(Y[1]) > SbAbs(bigyval)) {
          bigyval = Y[1];
          bigy = 1;
        }
        if (SbAbs(Y[2]) > SbAbs(bigyval)) {
          bigy = 2;
          bigyval = Y[2];
        }
        Z[(bigy+1)%3] = bigyval > 0.0f ? 1.0f : -1.0f;
        
        // make Z perpendicular to Y
        X = Y.cross(Z);
        my_normalize(X);
        Z = X.cross(Y);
        my_normalize(Z);
      }
      
      X = Y.cross(Z);
      my_normalize(X);
      
      if (i > 0 && (Z.dot(prevZ) <= 0.5f || X.dot(prevX) <= 0.5f)) {
        // if change is fairly large, try to find the most appropriate
        // axis. This will minimize change from spine-point to
        // spine-point
        SbVec3f v[4];
        v[0] = X;
        v[1] = -X;
        v[2] = Z;
        v[3] = -Z;
        
        float maxdot = v[0].dot(prevZ);
        int maxcnt = 0;
        for (int cnt = 1; cnt < 4; cnt++) {
          float dot = v[cnt].dot(prevZ);
          if (dot > maxdot) {
            maxdot = dot;
            maxcnt = cnt;
          }
        }
        Z = v[maxcnt];
        X = Y.cross(Z);
        my_normalize(X);
      }
    }

    prevX = X;
    prevY = Y;
    prevZ = Z;

    matrix[0][0] = X[0];
    matrix[0][1] = X[1];
    matrix[0][2] = X[2];
    matrix[0][3] = 0.0f;

    matrix[1][0] = Y[0];
    matrix[1][1] = Y[1];
    matrix[1][2] = Y[2];
    matrix[1][3] = 0.0f;

    matrix[2][0] = Z[0];
    matrix[2][1] = Z[1];
    matrix[2][2] = Z[2];
    matrix[2][3] = 0.0f;

    matrix[3][0] = spine[i][0];
    matrix[3][1] = spine[i][1];
    matrix[3][2] = spine[i][2];
    matrix[3][3] = 1.0f;

    int cnt = 0;
    if (X[0] < 0.0f) cnt++;
    if (X[1] < 0.0f) cnt++;
    if (X[2] < 0.0f) cnt++;
    if (Y[0] < 0.0f) cnt++;
    if (Y[1] < 0.0f) cnt++;
    if (Y[2] < 0.0f) cnt++;
    if (Z[0] < 0.0f) cnt++;
    if (Z[1] < 0.0f) cnt++;
    if (Z[2] < 0.0f) cnt++;

    if (cnt & 1) reversecnt--;
    else reversecnt++;

    if (numorient) {
      SbMatrix rmat;
      orient[SbMin(i, numorient-1)].getValue(rmat);
      matrix.multLeft(rmat);
    }

    if (numscale) {
      SbMatrix smat = SbMatrix::identity();
      SbVec2f s = scale[SbMin(i, numscale-1)];
      smat[0][0] = s[0];
      smat[2][2] = s[1];
      matrix.multLeft(smat);
    }

    for (j = 0; j < numcross; j++) {
      SbVec3f c;
      c[0] = cross[j][0];
      c[1] = 0.0f;
      c[2] = cross[j][1];

      matrix.multVecMatrix(c, c);
      this->coord.append(c);
      this->tcoord.append(SbVec2f(float(j)/float(numcross-1),
                                  float(i)/float(closed ? numspine : numspine-1)));
      this->color_idx.append(i);
    }
  }

  // this macro makes the code below more readable
#define ADD_TRIANGLE(i0, j0, i1, j1, i2, j2) \
  do { \
    if (reversecnt < 0) { \
      this->idx.append((i2)*numcross+(j2)); \
      this->idx.append((i1)*numcross+(j1)); \
      this->idx.append((i0)*numcross+(j0)); \
      this->idx.append(-1); \
    } \
    else { \
      this->idx.append((i0)*numcross+(j0)); \
      this->idx.append((i1)*numcross+(j1)); \
      this->idx.append((i2)*numcross+(j2)); \
      this->idx.append(-1); \
    } \
  } while (0)

#define ADD_VERTEX(i0, j0) \
  do { \
    this->idx.append((i0)*numcross+(j0)); \
  } while (0)

  // create begin cap, walls and end cap
  for (i = 0; i < numspine-1; i++) {
    // create beginCap polygon if first segment
    if (i == 0) {
      this->segidx.append(cursegidx);
      int cnt0 = 0;
      int cnt1 = numcross-1;
      
      for (j = 0; j < numcross; j++) {
        if (j & 1) {
          ADD_VERTEX(i, cnt0);
          cnt0++;
        }
        else {
          ADD_VERTEX(i, cnt1);
          cnt1--;
        }
      }
      cursegidx += numcross;
      this->striplens.append(numcross);
    }
    // Create walls
    this->segidx.append( cursegidx );
    for (j = 0; j < numcross; j++) {
      ADD_VERTEX(i, j);
      ADD_VERTEX(i+1, j);
      cursegidx += 2;
    }
    if (connected) {
      ADD_VERTEX(i, 0);
      ADD_VERTEX(i+1, 0);
      cursegidx += 2;
      this->striplens.append( 2 + numcross * 2 );
    } 
    else {
      this->striplens.append( numcross * 2 );
    }
    //  this->idx.append(-1);
    //  cursegidx += 1;
    // create endCap polygon if last segment
    if (i == numspine - 2) {
      this->segidx.append(cursegidx);
      int cnt0 = 0;
      int cnt1 = numcross-1;
      for (j = 0; j < numcross; j++) {
        if (!(j & 1)) {
          ADD_VERTEX(i+1, cnt0);
          cnt0++;
        }
        else {
          ADD_VERTEX(i+1, cnt1);
          cnt1--;
            
        }
      }
      cursegidx += numcross;
      this->striplens.append(numcross);
    }
  }
#undef ADD_TRIANGLE
#undef ADD_VERTEX
  //  printf("generateCoords: done, segidx length = %d \n", this->segidx.getLength() );
}

void
SoLODExtrusionP::generateNormals(void)
{
#if 0 // reset() is only available in Coin-2
  if (this->gen) this->gen->reset(this->master->ccw.getValue());
  else this->gen = new SoNormalGenerator(this->master->ccw.getValue());
#else // only Coin-2
  delete this->gen;
  this->gen = new SoNormalGenerator(this->master->ccw.getValue());
#endif // Coin-1 fix

  const SbVec3f * vertices = this->coord.getArrayPtr();

  const int32_t * ptr = this->striplens.getArrayPtr();
  const int32_t * start = ptr;
  const int32_t * end = ptr + this->striplens.getLength();

  const int32_t * idxptr = this->idx.getArrayPtr();

  int cnt = 0;
  while (ptr < end) {
    int num = *ptr++ - 3;
    assert(num >= 0);
    SbVec3f striptri[3];
    striptri[0] = vertices[idxptr[cnt++]];
    striptri[1] = vertices[idxptr[cnt++]];
    striptri[2] = vertices[idxptr[cnt++]];
    this->gen->triangle(striptri[0], striptri[1], striptri[2]);
    SbBool flag = FALSE;
    while (num--) {
      if (flag) striptri[1] = striptri[2];
      else striptri[0] = striptri[2];
      flag = !flag;
      striptri[2] = vertices[idxptr[cnt++]];
      this->gen->triangle(striptri[0], striptri[1], striptri[2]);
    }
  }

  this->gen->generate(this->master->creaseAngle.getValue(),
                      this->striplens.getArrayPtr(),
                      this->striplens.getLength());
}

void
SoLODExtrusionP::tess_callback(void * v0, void * v1, void * v2, void * data)
{
  SoLODExtrusionP * thisp = (SoLODExtrusionP*) data;
  thisp->idx.append((int) v0);
  thisp->idx.append((int) v1);
  thisp->idx.append((int) v2);
  thisp->idx.append(-1);
}


//
// Render triangles for segidx[index]
// Assumes per vertex normals, no materials or texture coords
//
void 
SoLODExtrusionP::renderSegidx( const int index, const SbBool use_color )
{
  //  printf("renderSegidx: segidx=%d \n", index);

  assert( index >=0 && index < this->segidx.getLength() );

  const int * siv = this->segidx.getArrayPtr();
  const int * iv = this->idx.getArrayPtr();
  const SbVec3f * cv = this->coord.getArrayPtr();
  const SbVec3f * nv = this->gen->getNormals();
  const SbColor * colorv = this->master->color.getValues(0);
  const int * coloridx = this->color_idx.getArrayPtr();
  int vcnt = this->coord.getLength();
  int startindex = siv[index];
  int stopindex;
  if (index < segidx.getLength()-1)
    stopindex = siv[index+1];
  else
    stopindex = idx.getLength();

  int curidx = startindex;
  int newmode;
  int32_t v1, v2, v3, v4, v5 = 0; // v5 init unnecessary, but kills a compiler warning.
  int32_t nv1, nv2, nv3;

  if( iv[curidx+3] < 0) {  /* Triangle */
    printf("renderSegidx: triangles. \n");
    glBegin(GL_TRIANGLES);
    while( curidx < stopindex ) {
      v1 = iv[curidx++];
      v2 = iv[curidx++];
      v3 = iv[curidx++];
      assert(v1 >= 0 && v2 >= 0 && v3 >= 0);
      assert(v1 < vcnt && v2 < vcnt && v3 < vcnt);
      v4 = iv[curidx++];
      assert( v4 < 0);  /* Triangle means every 4th index = -1  */
      
      /* vertex 1 *********************************************************/
      if( use_color )
	glColor3fv((const GLfloat*)colorv[coloridx[v1]].getValue());
      glNormal3fv((const GLfloat*)nv[v1].getValue());
      glVertex3fv((const GLfloat*)cv[v1].getValue());
      
      /* vertex 2 *********************************************************/
      if( use_color )
	glColor3fv((const GLfloat*)colorv[coloridx[v2]].getValue());
      glNormal3fv((const GLfloat*)nv[v2].getValue());
      glVertex3fv((const GLfloat*)cv[v2].getValue());
      
      /* vertex 3 *********************************************************/
      if( use_color )
	glColor3fv((const GLfloat*)colorv[coloridx[v3]].getValue());
      glNormal3fv((const GLfloat*)nv[v3].getValue());
      glVertex3fv((const GLfloat*)cv[v3].getValue());
    }
    glEnd(); /* draw triangles */
  } 
  else {   /* Tristrip(s) */
    glBegin(GL_TRIANGLE_STRIP);
    while( curidx < stopindex ) {
      v1 = iv[curidx];
      //      assert(v1 >= 0 && v1 < vcnt);
      if( use_color ) {
	glColor3fv((const GLfloat*)colorv[coloridx[v1]].getValue());
      }
      glNormal3fv((const GLfloat*)nv[curidx].getValue());
      glVertex3fv((const GLfloat*)cv[v1].getValue());
      curidx++;
    }
    glEnd(); /* draw tristrip*/
  }
}

void
SoLODExtrusionP::makeCircleCrossSection(const float radius, const int segments)
{
  SbList <SbVec2f> templist;
  templist.truncate(0);
  float angle = 2.0 * M_PI / float(segments);
  templist.append( SbVec2f( 0.0, radius ) );
  for( int i=1; i < segments; i++) {
    templist.append( SbVec2f( radius*sin(float(i)*angle), radius*cos(float(i)*angle)) );
  }
  templist.append( SbVec2f( 0.0, radius ) );

  // disable notify so that we don't send notification messages while
  // rendering (not good for multipipe rendering).
  SbBool old = this->master->crossSection.enableNotify(FALSE);
  this->master->crossSection.setValues( 0, templist.getLength(), templist.getArrayPtr() );
  this->master->crossSection.enableNotify(FALSE);
}

#endif // DOXYGEN_SKIP_THIS

