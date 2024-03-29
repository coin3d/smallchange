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

#include "SmHQSphere.h"
#include <Inventor/SbBSPTree.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoTextureCoordinateElement.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SbSphere.h>
#include <Inventor/SbVec4f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbLine.h>
#include <Inventor/system/gl.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <cstddef>
#include <cstring>
#include <Inventor/SbBasic.h> // COIN_MAJOR_VERSION, COIN_MINOR_VERSION

#include "../misc/SbList.h"



class SmHQSphereP {
public:
  SbBSPTree bsp;
  SbList <int32_t> idx;
  SbList <SbVec3f> coord;
  SbList <SbVec2f> texcoord;
  HQSphereGenerator generator;
  int currlevel;

  void genGeom(const int level);
};

#define PRIVATE(obj) obj->pimpl

// *************************************************************************

/*!
  \var SoSFInt32 SmHQSphere::level

  Decides the number of times each triangle in the tessellation of the
  sphere is recursively subdivided into 4 new triangles. At level 1,
  the sphere will be made from only 8 triangles.

  The total number of triangles making up the sphere will therefore be

  \verbatim
  numtriangles = 8 * (4 ^ (level - 1))
  \endverbatim

  So be careful when increasing the \a level field value, as the
  number of triangles making up the sphere will increase
  exponentially.
*/

// *************************************************************************

SO_NODE_SOURCE(SmHQSphere);

// *************************************************************************

/*!
  Constructor.
*/
SmHQSphere::SmHQSphere()
{
  SO_NODE_CONSTRUCTOR(SmHQSphere);
  SO_NODE_ADD_FIELD(radius, (1.0f));
  SO_NODE_ADD_FIELD(level, (5));

  PRIVATE(this) = new SmHQSphereP;
  PRIVATE(this)->currlevel = -1;
}

/*!
  Destructor.
*/
SmHQSphere::~SmHQSphere()
{
  delete PRIVATE(this);
}

/*!
  Required Coin method.
*/
void
SmHQSphere::initClass()
{
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    SO_NODE_INIT_CLASS(SmHQSphere, SoShape, "Shape");
  }
}

// *************************************************************************

void
SmHQSphere::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

  SoState * state = action->getState();

  SoMaterialBundle mb(action);
  mb.sendFirst();

  SbBool doTextures = FALSE;
  if (SoGLTextureEnabledElement::get(state)) {
    doTextures = TRUE;
  }
  SbBool sendNormals = !mb.isColorOnly() ||
    (SoTextureCoordinateElement::getType(state) == SoTextureCoordinateElement::FUNCTION);

  int l = this->level.getValue();
  if (l != PRIVATE(this)->currlevel) {
    PRIVATE(this)->genGeom(l);
    PRIVATE(this)->currlevel = l;
  }

  int n = PRIVATE(this)->idx.getLength();
  const int32_t * idx = PRIVATE(this)->idx.getArrayPtr();
  const SbVec3f * pts = PRIVATE(this)->coord.getArrayPtr();
  const SbVec2f * tc = PRIVATE(this)->texcoord.getArrayPtr();

  float r = this->radius.getValue();

  if (r != 1.0f) {
    glPushMatrix();
    glScalef(r, r, r);
  }

  SbBool varray = FALSE;
#if (COIN_MAJOR_VERSION > 2) || ((COIN_MAJOR_VERSION == 2) && (COIN_MINOR_VERSION > 1))
  const cc_glglue * glue = cc_glglue_instance(SoGLCacheContextElement::get(state));
  varray = cc_glglue_has_vertex_array(glue);
#endif // Coin version >= 2.2
  if (varray) {
#if (COIN_MAJOR_VERSION > 2) || ((COIN_MAJOR_VERSION==2) && (COIN_MINOR_VERSION > 1))
    cc_glglue_glVertexPointer(glue, 3, GL_FLOAT, 0,
                              (GLvoid*) pts);
    cc_glglue_glEnableClientState(glue, GL_VERTEX_ARRAY);

    if (sendNormals) {
      cc_glglue_glNormalPointer(glue, GL_FLOAT, 0,
                                (GLvoid*) pts);
      cc_glglue_glEnableClientState(glue, GL_NORMAL_ARRAY);
    }
    if (doTextures) {
      cc_glglue_glTexCoordPointer(glue, 2, GL_FLOAT, 0,
                                  (GLvoid*) tc);
      cc_glglue_glEnableClientState(glue, GL_TEXTURE_COORD_ARRAY);
    }
    cc_glglue_glDrawElements(glue, GL_TRIANGLES, n, GL_UNSIGNED_INT, idx);

    cc_glglue_glDisableClientState(glue, GL_VERTEX_ARRAY);
    if (sendNormals) cc_glglue_glDisableClientState(glue, GL_NORMAL_ARRAY);
    if (doTextures) cc_glglue_glDisableClientState(glue, GL_TEXTURE_COORD_ARRAY);
    SoGLCacheContextElement::shouldAutoCache(state, SoGLCacheContextElement::DONT_AUTO_CACHE);
#endif // Coin version >= 2.2
  }
  else {
    glBegin(GL_TRIANGLES);
    if (sendNormals && doTextures) {
      for (int i = 0; i < n; i++) {
        SbVec2f t = tc[idx[i]];
        glTexCoord2f(t[0], t[1]);
        SbVec3f v = pts[idx[i]];
        glNormal3f(v[0], v[1], v[2]);
        glVertex3f(v[0], v[1], v[2]);
      }
    }
    else if (sendNormals && !doTextures) {
      for (int i = 0; i < n; i++) {
        SbVec3f v = pts[idx[i]];
        glNormal3f(v[0], v[1], v[2]);
        glVertex3f(v[0], v[1], v[2]);
      }
    }
    else {
      for (int i = 0; i < n; i++) {
        SbVec3f v = pts[idx[i]];
        glVertex3f(v[0], v[1], v[2]);
      }
    }
    glEnd();  // GL_TRIANGLES
  }
  if (r != 1.0f) {
    glPopMatrix();
  }
}


void
SmHQSphere::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
  if (!this->shouldPrimitiveCount(action)) return;

  int l = this->level.getValue();
  if (l != PRIVATE(this)->currlevel) {
    PRIVATE(this)->genGeom(l);
    PRIVATE(this)->currlevel = l;
  }
  action->addNumTriangles(PRIVATE(this)->idx.getLength()/3);
}

// internal method used to add a sphere intersection to the ray pick
// action, and set the correct pp normal and texture coordinates
static void
try_add_intersection(SoRayPickAction * action, const SbVec3f & pt)
{
  if (action->isBetweenPlanes(pt)) {
    SoPickedPoint * pp = action->addIntersection(pt);
    if (pp) {
      SbVec3f normal = pt;
      normal.normalize();
      pp->setObjectNormal(normal);
      SbVec4f tc((float) (atan2(pt[0], pt[2]) * (1.0 / (2.0*M_PI)) + 0.5),
                 (float) (atan2(pt[1], sqrt(pt[0]*pt[0] + pt[2]*pt[2])) * (1.0/M_PI) + 0.5),
                 0.0f, 1.0f);
      pp->setObjectTextureCoords(tc);
    }
  }
}

void
SmHQSphere::rayPick(SoRayPickAction * action)
{
  if (!shouldRayPick(action)) return;

  action->setObjectSpace();
  const SbLine & line = action->getLine();
  SbSphere sphere(SbVec3f(0.0f, 0.0f, 0.0f), this->radius.getValue());
  SbVec3f enter, exit;
  if (sphere.intersect(line, enter, exit)) {
    try_add_intersection(action, enter);
    if (exit != enter) try_add_intersection(action, exit);
  }
}

void
SmHQSphere::generatePrimitives(SoAction * action)
{
  int l = this->level.getValue();
  if (l != PRIVATE(this)->currlevel) {
    PRIVATE(this)->genGeom(l);
    PRIVATE(this)->currlevel = l;
  }

  int n = PRIVATE(this)->idx.getLength();
  const int32_t * idx = PRIVATE(this)->idx.getArrayPtr();
  const SbVec3f * pts = PRIVATE(this)->coord.getArrayPtr();
  const SbVec2f * tc = PRIVATE(this)->texcoord.getArrayPtr();

  float r = this->radius.getValue();

  SoPrimitiveVertex vertex;
  this->beginShape(action, SoShape::TRIANGLES);

  for (int i = 0; i < n; i++) {
    vertex.setTextureCoords(tc[idx[i]]);
    SbVec3f p = pts[idx[i]];
    vertex.setNormal(p);
    p *= r;
    vertex.setPoint(p);
    this->shapeVertex(&vertex);
  }
  this->endShape();
}

void
SmHQSphere::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  float r = this->radius.getValue();

  // Allow negative values.
  if (r < 0.0f) r = -r;

  box.setBounds(SbVec3f(-r, -r, -r), SbVec3f(r, r, r));
  center.setValue(0.0f, 0.0f, 0.0f);
}

#undef PRIVATE

void
SmHQSphereP::genGeom(const int level)
{
  int i, n;
  this->generator.generate(SbClamp(level, 1, 12), this->bsp, this->idx);
  const SbVec3f * pts = this->bsp.getPointsArrayPtr();
  this->texcoord.truncate(0, TRUE);
  this->coord.truncate(0, TRUE);

  n = this->bsp.numPoints();
  for (i = 0; i < n; i++) {
    this->coord.append(pts[i]);
  }
  this->bsp.clear();
  pts = this->coord.getArrayPtr();

  // generate texcoords for all vertices
  for (i = 0; i < n; i++) {
    SbVec3f pt = pts[i];
    SbVec2f tc((float) (atan2(pt[0], pt[2]) * (1.0 / (2.0*M_PI)) + 0.5),
               (float) (atan2(pt[1], sqrt(pt[0]*pt[0] + pt[2]*pt[2])) * (1.0/M_PI) + 0.5));

    tc[0] = SbClamp(tc[0], 0.0f, 1.0f);
    tc[1] = SbClamp(tc[1], 0.0f, 1.0f);
    // so that right-side-of-texture can be detected below
    if (tc[0] >= 0.99999f) tc[0] = 0.0f;
    this->texcoord.append(tc);
  }

  // detect triangles that are on the back side of the sphere, on the
  // right side, with at least one vertex on the right-side-edge of
  // the texture. Fix texture coordinates for those triangles
  n = this->idx.getLength();
  int32_t * iptr = (int32_t*) this->idx.getArrayPtr();

#define FLTCOMPARE(x, y) (fabs((x)-(y)) < 0.000001f)

  SbVec3f p[3];
  SbVec2f t[3];
  int cnt = 0;
  int j;
  for (i = 0; i < n; i += 3) {
    SbBool rightside = FALSE;
    for (j = 0; j < 3; j++) {
      p[j] = pts[iptr[i+j]];
      t[j] = this->texcoord[iptr[i+j]];
      if (p[j][0] > 0.000001f) rightside = TRUE;
      if (p[j][2] > 0.000001f) break; // not on the back of the sphere
    }
    if (rightside && (j == 3) &&
        (FLTCOMPARE(p[0][0], 0.0f) || FLTCOMPARE(p[1][0], 0.0f) || FLTCOMPARE(p[2][0], 0.0f))) {
      // just create new vertices for these triangles
      int len = this->coord.getLength();
      for (j = 0; j < 3; j++) {
        iptr[i+j] = len+j;
        this->coord.append(p[j]);
        if (t[j][0] <= 0.000001f) t[j][0] = 1.0f;
        this->texcoord.append(t[j]);
      }
    }
  }
#undef FLTCOMPARE
}

void
HQSphereGenerator::init(void)
{
#if 0 // octahedron looks better
  /* Twelve vertices of icosahedron on unit sphere */
  const float tau = 0.8506508084f; /* t=(1+sqrt(5))/2, tau=t/sqrt(1+t^2)  */
  const float one =0.5257311121f; /* one=1/sqrt(1+t^2) , unit sphere     */

  const SbVec3f ZA(tau, one, 0.0f);
  const SbVec3f ZB(-tau, one, 0.0f);
  const SbVec3f ZC(-tau, -one, 0.0f);
  const SbVec3f ZD(tau, -one, 0.0f);
  const SbVec3f YA(one, 0.0f ,  tau);
  const SbVec3f YB(one, 0.0f , -tau);
  const SbVec3f YC(-one, 0.0f , -tau);
  const SbVec3f YD(-one, 0.0f ,  tau);
  const SbVec3f XA(0.0f, tau, one);
  const SbVec3f XB(0.0f, -tau, one);
  const SbVec3f XC(0.0f, -tau, -one);
  const SbVec3f XD(0.0f, tau, -one);

  /* Structure for unit icosahedron */
  const triangle triangles[] = {
    triangle(YA, XA, YD),
    triangle(YA, YD, XB),
    triangle(YB, YC, XD),
    triangle(YB, XC, YC),
    triangle(ZA, YA, ZD),
    triangle(ZA, ZD, YB),
    triangle(ZC, YD, ZB),
    triangle(ZC, ZB, YC),
    triangle(XA, ZA, XD),
    triangle(XA, XD, ZB),
    triangle(XB, XC, ZD),
    triangle(XB, ZC, XC),
    triangle(XA, YA, ZA),
    triangle(XD, ZA, YB),
    triangle(YA, XB, ZD),
    triangle(YB, ZD, XC),
    triangle(YD, XA, ZB),
    triangle(YC, ZB, XD),
    triangle(YD, ZC, XB),
    triangle(YC, XC, ZC)
  };
  this->orgobject =
    new object(20, triangles);
#else
  const SbVec3f XPLUS(1,  0,  0);
  const SbVec3f XMIN(-1,  0,  0);
  const SbVec3f YPLUS(0,  1,  0);
  const SbVec3f YMIN(0, -1,  0);
  const SbVec3f ZPLUS(0,  0,  1);
  const SbVec3f ZMIN(0,  0, -1);

  /* Vertices of a unit octahedron */
  triangle triangles[] = {
    triangle(XPLUS, ZPLUS, YPLUS),
    triangle(YPLUS, ZPLUS, XMIN),
    triangle(XMIN , ZPLUS, YMIN),
    triangle(YMIN , ZPLUS, XPLUS),
    triangle(XPLUS, YPLUS, ZMIN),
    triangle(YPLUS, XMIN , ZMIN),
    triangle(XMIN , YMIN , ZMIN),
    triangle(YMIN , XPLUS, ZMIN)
  };
  this->orgobject =
    new object(8, triangles);
#endif
};

void
HQSphereGenerator::generate(const int maxlevel,
                            SbBSPTree & bsp,
                            SbList <int> & idx)
{
  int level;

  bsp.clear();
  idx.truncate(0, TRUE);
  object * old = this->orgobject;

  /* Subdivide each starting triangle (maxlevel - 1) times */
  for (level = 1; level < maxlevel; level++) {
    /* Allocate a new object */
    /* FIXME: Valgrind reports an 8-byte memory leak here. 20030404 mortene. */
    object * newobj = new object(old->npoly * 4, NULL);

    /* Subdivide each triangle in the old approximation and normalize
     *  the new points thus generated to lie on the surface of the unit
     *  sphere.
     * Each input triangle with vertices labelled [0,1,2] as shown
     *  below will be turned into four new triangles:
     *
     *                      Make new points
     *                          a = (0+2)/2
     *                          b = (0+1)/2
     *                          c = (1+2)/2
     *        1
     *       /\             Normalize a, b, c
     *      /  \
     *    b/____\ c         Construct new triangles
     *    /\    /\              [0,b,a]
     *   /  \  /  \             [b,1,c]
     *  /____\/____\            [a,b,c]
     * 0      a     2           [a,c,2]
     */
    for (int i = 0; i < old->npoly; i++) {
      triangle *oldt = &old->poly[i];
      triangle *newt = &newobj->poly[i*4];
      SbVec3f a, b, c;

      a = normalize(midpoint(oldt->pt[0], oldt->pt[2]));
      b = normalize(midpoint(oldt->pt[0], oldt->pt[1]));
      c = normalize(midpoint(oldt->pt[1], oldt->pt[2]));

      newt->pt[0] = oldt->pt[0];
      newt->pt[1] = b;
      newt->pt[2] = a;
      newt++;

      newt->pt[0] = b;
      newt->pt[1] = oldt->pt[1];
      newt->pt[2] = c;
      newt++;

      newt->pt[0] = a;
      newt->pt[1] = b;
      newt->pt[2] = c;
      newt++;

      newt->pt[0] = a;
      newt->pt[1] = c;
      newt->pt[2] = oldt->pt[2];
    }

    if (old != this->orgobject) delete old;
    /* Continue subdividing new triangles */
    old = newobj;
  }
  this->convert(old, bsp, idx);
  if (old != this->orgobject) delete old;
}

SbVec3f
HQSphereGenerator::normalize(const SbVec3f & p)
{
  float mag;

  SbVec3f r = p;
  mag = r[0] * r[0] + r[1] * r[1] + r[2] * r[2];
  if (mag != 0.0f) {
    mag = (float) (1.0 / sqrt(mag));
    r[0] *= mag;
    r[1] *= mag;
    r[2] *= mag;
  }
  return r;
}

/* Return the midpoint on the line between two points */
SbVec3f
HQSphereGenerator::midpoint(const SbVec3f & a, const SbVec3f & b)
{
  SbVec3f r;

  r[0] = (a[0] + b[0]) * 0.5f;
  r[1] = (a[1] + b[1]) * 0.5f;
  r[2] = (a[2] + b[2]) * 0.5f;

  return r;
}

void
HQSphereGenerator::convert(object * obj, SbBSPTree & bsp, SbList <int> & idx)
{
  triangle * t = obj->poly;
  int n = obj->npoly;

  for (int i = 0; i < n; i++) {
    for (int j = 2; j >= 0; j--) {
      idx.append(bsp.addPoint(t->pt[j]));
    }
    t++;
  }
}
