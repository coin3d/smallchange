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
  \class SoTCBCurve SoTCBCurve.h Inventor/nodes/SoTCBCurve.h
  \brief The SoTCBCurve class is a node for representing smooth curves.
  \ingroup nodes

  The TCB-type curve guarantees that all controlpoints are touched by
  the curve. If no timestamps are specified, all timeintervals are set
  to equal length. Coordinates are read from the state.

  Note that the list of timestamps \e must be sorted in increasing order.

  Example usage:
  \code
  #include <Inventor/Qt/SoQt.h>
  #include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
  #include <SmallChange/nodes/SoTCBCurve.h>
  #include <Inventor/nodes/SoCoordinate3.h>
  #include <Inventor/nodes/SoSeparator.h>
      
  int
  main(int argc, char* argv[])
  {
    QWidget * mainwin = SoQt::init(argv[0]);
  
    SoTCBCurve::initClass();
  
    SoSeparator * root = new SoSeparator;
    root->ref();
      
    const SbVec3f coordset[] = {
      SbVec3f(0, 0, 0),
      SbVec3f(1, 1, 0),
      SbVec3f(2, 0, 0),
      SbVec3f(3, -1, 0),
      SbVec3f(4, 0, 0)
    };
  
    SoCoordinate3 * coords = new SoCoordinate3;
    coords->point.setValues(0, sizeof(coordset) / sizeof(coordset[0]), coordset);
  
    root->addChild(coords);
  
    SoTCBCurve * curve = new SoTCBCurve;
    curve->numControlpoints = coords->point.getNum();
  
    // XXX this should really have been unnecessary, but due to a bug in
    // SoTCBCurve vs spec... XXX
  #if 1
    for (int i=0; i < coords->point.getNum(); i++) {
      curve->timestamp.set1Value(i, SbTime((double)i));
    }
  #endif
  
    root->addChild(curve);
  
    SoQtExaminerViewer * viewer = new SoQtExaminerViewer(mainwin);
    viewer->setSceneGraph(root);
      
    SoQt::show(mainwin);
    SoQt::mainLoop();
  
    delete viewer;
    root->unref();
      
    return 0;
  }
  \endcode
*/

#include "SoTCBCurve.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/details/SoLineDetail.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef __COIN__
#include <Inventor/system/gl.h>
#else // SGI/TGS Inventor
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>
#endif // SGI/TGS Inventor


/*!
  \var SoSFInt32 SoTCBCurve::numControlpoints

  Number of control points to use in this NURBS curve.
*/
/*!
  \var SoMFTime SoTCBCurve::timestamp

  The timestamps for the curve. This table must contain either 0
  elements or exactly numControlpoints elements. Nothing in
  between. This list \e must be sorted in increasing order.
*/


// *************************************************************************

SO_NODE_SOURCE(SoTCBCurve);

// *************************************************************************

class SoTCBCurveP {
public:
  SoTCBCurveP(SoTCBCurve * master)
  {
    this->master = master;
    this->linesPerSegment = 0;
  }

  int linesPerSegment;

  int getLinesPerSegment(SoAction * action);

private:
  SoTCBCurve * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

/*! Constructor. */
SoTCBCurve::SoTCBCurve(void)
{
  PRIVATE(this) = new SoTCBCurveP(this);

  SO_NODE_CONSTRUCTOR(SoTCBCurve);

  SO_NODE_ADD_FIELD(numControlpoints, (0));
  SO_NODE_ADD_FIELD(timestamp, (SbTime(0.0)));

  this->timestamp.setNum(0);
}

/*! Destructor. */
SoTCBCurve::~SoTCBCurve()
{
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoTCBCurve::initClass(void)
{
  SO_NODE_INIT_CLASS(SoTCBCurve, SoShape, "Shape");
}

// Doc from parent class.
void
SoTCBCurve::GLRender(SoGLRenderAction *action)
{
  SoState * state = action->getState();
  if (!this->shouldGLRender(action)) return;

  if (this->numControlpoints.getValue() < 2) return;

  const SoCoordinateElement *coordElement= SoCoordinateElement::getInstance(state);
  const SbVec3f * coords = coordElement->getArrayPtr3();


  //---- Rendering...

  // FIXME: I guess things like material, normals, texturecoords,
  // drawstyle etc should be stuffed here somewhere.  But I can't find
  // any examples or reasonable text in that stupid book. This is
  // probably trivial for the other guys here. 20020612 torbjorv

  // Renders each curvesegment as [complexity*100] lines, using a
  // for-loop. Mark that this baby guarantees the curve always touches
  // perfectly all controlpoints (avoiding floating point errors)
  PRIVATE(this)->linesPerSegment = PRIVATE(this)->getLinesPerSegment(action);
  if (PRIVATE(this)->linesPerSegment == 0) { return; }

  glColor3f(1.0, 1.0, 1.0);
  glDisable(GL_LIGHTING);
  glBegin(GL_LINE_STRIP);

  glVertex3f(coords[0][0], coords[0][1], coords[0][2]);
  for (int segment = 0; segment < this->numControlpoints.getValue() - 1; segment++) {

    // FIXME: should handle timestamp.getNum()==0 as a special case,
    // and just distribute timevalues uniformly. 20030108 mortene.
    assert(segment + 1 < this->timestamp.getNum());

    float timestep = (float) (this->timestamp[segment + 1] - this->timestamp[segment]).getValue()/PRIVATE(this)->linesPerSegment;
    float time = (float) this->timestamp[segment].getValue() + timestep;
    SbVec3f vec;

    // FIXME: investigate what happens if linesPerSegment==1. 20030109 mortene.
    for (int i = 0; i < PRIVATE(this)->linesPerSegment - 1; i++) {
      SoTCBCurve::TCB(coords, this->timestamp, this->numControlpoints.getValue(), time, vec);
      glVertex3f(vec[0], vec[1], vec[2]);

      time += timestep;
    }

    glVertex3f(coords[segment + 1][0], coords[segment + 1][1], coords[segment + 1][2]);
  }

  glEnd();
  glEnable(GL_LIGHTING);
}

// Doc from parent class.
void
SoTCBCurve::generatePrimitives(SoAction * action)
{
  SoState * state = action->getState();

  const SoCoordinateElement *coordElement= SoCoordinateElement::getInstance(state);
  const SbVec3f * coords = coordElement->getArrayPtr3();


  //---- Generating...

  // FIXME: I guess things like material, normals, texturecoords,
  // drawstyle etc should be stuffed here somewhere. But I can't find
  // any examples or reasonable text in that stupid book. This is
  // probably trivial for the other guys here. 20020612 torbjorv

  SoLineDetail lineDetail;
  SoPointDetail pointDetail;

  this->beginShape(action, SoShape::LINE_STRIP, &lineDetail);

  // Generates each curvesegment as [resolution] lines using a for-loop. Mark
  // that this baby guarantees the curve always touches perfectly all controlpoints
  // (avoiding floating point errors)

  SoPrimitiveVertex pv;
  SbVec3f point;
  point.setValue(coords[0][0], coords[0][1], coords[0][2]);
  pv.setPoint(point);
  pv.setDetail(&pointDetail);
  this->shapeVertex(&pv);

  if (this->numControlpoints.getValue() > 1)
    for (int segment = 0; segment < this->numControlpoints.getValue() - 1; segment++) {

      // FIXME: should handle timestamp.getNum()==0 as a special
      // case. 20030108 mortene.
      assert(segment + 1 < this->timestamp.getNum());

      float timestep = (float) (this->timestamp[segment + 1] - this->timestamp[segment]).getValue()/PRIVATE(this)->linesPerSegment;
      float time = (float) this->timestamp[segment].getValue() + timestep;
      SbVec3f vec;

      for (int i = 0; i < PRIVATE(this)->linesPerSegment - 1; i++) {
        SoTCBCurve::TCB(coords, this->timestamp, this->numControlpoints.getValue(), time, vec);
        point.setValue(vec[0], vec[1], vec[2]);

        pv.setPoint(point);
        this->shapeVertex(&pv);
        lineDetail.incPartIndex();

        time += timestep;
      }

      point.setValue(coords[segment + 1][0], coords[segment + 1][1], coords[segment + 1][2]);
      pv.setPoint(point);
      this->shapeVertex(&pv);
      lineDetail.incLineIndex();
      lineDetail.setPartIndex(0);
    }

  this->endShape();
}

// Doc from parent class.
void
SoTCBCurve::computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center)
{
  /*
    I'm not sure how accurate such a boundingbox must be, as this
    could be implemented very fast by using the controlpoints. But the
    curve will most likely run outside this box. 20020613 torbjorv
  */

  // FIXME: the checks for when to _not_ compute a bbox below should
  // be copied to generatePrimitives() and GLRender(). 20030109 mortene.

  if (this->numControlpoints.getValue() == 0) return;

  SoState * state = action->getState();
  const SoCoordinateElement * coordElement = SoCoordinateElement::getInstance(state);
  if ((coordElement == NULL) || (coordElement->getNum() == 0)) return;
  assert(this->numControlpoints.getValue() <= coordElement->getNum());

  // FIXME: should handle timestamp.getNum()==0 as a special
  // case. 20030108 mortene.
  assert(this->timestamp.getNum() == this->numControlpoints.getValue());

  const SbVec3f * coords = coordElement->getArrayPtr3();

  box.extendBy(coords[0]);

  for (int segment = 0; segment < this->numControlpoints.getValue() - 1; segment++) {
    float timestep = (float) (this->timestamp[segment + 1] - this->timestamp[segment]).getValue()/PRIVATE(this)->linesPerSegment;
    float time = (float) this->timestamp[segment].getValue() + timestep;
    SbVec3f vec;

    for (int i = 0; i < PRIVATE(this)->linesPerSegment - 1; i++) {
      SoTCBCurve::TCB(coords, this->timestamp, this->numControlpoints.getValue(), time, vec);
      box.extendBy(vec);
      time += timestep;
    }

    box.extendBy(coords[segment + 1]);
  }

  center = box.getCenter();

  return;
}

/*!
  Static function to interpolate values along a curve.

  This function is based on the Lightwave SDK (ftp.newtek.com) and
  calculates a TCB-type curve. Code for handling continuity / tension
  / bias is removed (values = 1).  Quite ironic as this is how the
  curve got it's name. :) The timestamp-table must be sorted in
  increasing order. Time values specified outside the range of
  timestamps will be clipped to the nearest value.

  This function is totally independent of the rest of the class, and
  may be used for general curvecalculations.
*/
void
SoTCBCurve::TCB(const SbVec3f * vec, const SoMFTime &timestamp,
                const int numControlpoints, const SbTime time, SbVec3f &res)
{
  assert(numControlpoints > 0);

  if (numControlpoints == 1) {
    res = vec[0];
    return;
  }

  //---- Find segment...
  int k = -1;
  for (int i = 0; i < numControlpoints - 1; i++) {
    if (timestamp[i] <= time) k++;
  }

  //---- Calculating t = (T - T0)/(T1 - T0)
  float t = (float) ((time - timestamp[k])/(timestamp[k + 1] - timestamp[k]));

  //---- Calculating curve-location.

  SbVec3f d10 = vec[k + 1] - vec[k];
  SbVec3f dd0, ds1;

  if (k == 0) {
    dd0 = d10;
  }
  else {
    const float adj = (float)
      ((timestamp[k + 1] - timestamp[k]) / (timestamp[k + 1] - timestamp[k - 1]));

    dd0 = adj * (vec[k] - vec[k - 1] + d10);
  }

  if (k + 2 == numControlpoints) {
    ds1 = d10;
  }
  else {
    const float adj = (float)
      ((timestamp[k + 1] - timestamp[k]) / (timestamp[k + 2] - timestamp[k]));

    ds1 = adj * (vec[k + 2] - vec[k + 1] + d10);
  }

  //---- Parametrization constants.
  const float t2 = t*t;
  const float t3 = t2*t;
  const float h1 = 1.0f - (3*t2 - 2*t3);
  const float h2 = 3*t2 - 2*t3;
  const float h3 = t3 - 2*t2 + t;
  const float h4 = t3 - t2;

  res = vec[k] * h1 + vec[k + 1] * h2 + dd0 * h3 + ds1 * h4;
}

/*!
  Returns an integer with the number of lines per segment rendered at
  the previous pass.
*/
int
SoTCBCurve::getLinesPerSegment(void)
{
  // FIXME: try to get rid of this method. 20030109 mortene.

  return PRIVATE(this)->linesPerSegment;
}

/*!
  Returns an integer with the number of lines per segment to render.
*/
int
SoTCBCurveP::getLinesPerSegment(SoAction * action)
{
  // FIXME: the magic number (100.0f) should really be replaced with
  // something based on the screenspace the curve occupies
  // instead. 20030109 mortene.
  return (int)(PUBLIC(this)->getComplexityValue(action) * 100.0f);
}
