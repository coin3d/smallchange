/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2002 by Systems in TCB. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  version 2.1 as published by the Free Software Foundation. See the
 *  file LICENSE.LGPL at the root directory of the distribution for
 *  more details.
 *
 *  If you want to use Coin for applications not compatible with the
 *  LGPL, please contact SIM to acquire a Professional Edition license.
 *
 *  Systems in TCB, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

/*!
  \class SoTCBCurve SoTCBCurve.h Inventor/nodes/SoTCBCurve.h
  \brief The SoTCBCurve class is a node for representing smooth curves. 
  \ingroup nodes

  The TCB-type curve guarantees that all controlpoints are touched by the
  curve. If no timestamps are specified, all timeintervals are set to equal
  length. Coordinates are read from the state. 

  Note that the list of timestamps MUST be sorted in increasing order. 
*/

#include "SoTCBCurve.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/details/SoLineDetail.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

//FIXME: These lines didn't compile with msvs...? torbjorv 07052002
/*#if HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H*/
#include <windows.h>
#include <GL/gl.h>


/*!
  \var SoSFInt32 SoTCBCurve::numControlpoints
  Number of control points to use in this NURBS curve.

  \var SoMFTime SoTCBCurve::timestamp
  The timestamps for the curve. This table must contain either 0 elements
  or exactly numControlpoints elements. Nothing in between. This list MUST
  be sorted in increasing order. 

*/


// *************************************************************************

SO_NODE_SOURCE(SoTCBCurve);

// *************************************************************************

class SoTCBCurveP {
public:
  SoTCBCurveP(SoTCBCurve * master) {
    this->master = master;
  }

  int linesPerSegment;

private:
  SoTCBCurve * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************


/*!
  Constructor.
*/
SoTCBCurve::SoTCBCurve(void)
{
  PRIVATE(this) = new SoTCBCurveP(this);

  SO_NODE_CONSTRUCTOR(SoTCBCurve);

  SO_NODE_ADD_FIELD(numControlpoints, (0));
}// Constructor




/*!
  Destructor.
*/
SoTCBCurve::~SoTCBCurve()
{
  delete PRIVATE(this);
}// Destructor




// Doc from parent class.
void
SoTCBCurve::initClass(void)
{
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_NODE_INIT_CLASS(SoTCBCurve, SoShape, "Shape");
}



// Doc from parent class.
void 
SoTCBCurve::GLRender(SoGLRenderAction *action)
{
  SoState * state = action->getState();
  if (!shouldGLRender(action)) return;

  if (numControlpoints.getValue() < 2) return;

  const SoCoordinateElement *coordElement= SoCoordinateElement::getInstance(state);
  const SbVec3f * coords =  coordElement->getArrayPtr3();

 
  //---- Rendering...
  // FIXME: I guess things like material, normals, texturecoords, drawstyle etc 
  // should be stuffed here somewhere.
  // But I can't find any examples or reasonable text in that stupid book. This is probably trivial for 
  // the other guys here. 20020612 torbjorv

  glColor3f(1.0, 1.0, 1.0);
  glDisable(GL_LIGHTING);
  glBegin(GL_LINE_STRIP);


  // Renders each curvesegment as [complexity*100] lines, using a for-loop. Mark 
  // that this baby guarantees the curve always touches perfectly all controlpoints
  // (avoiding floating point errors)
  PRIVATE(this)->linesPerSegment = (int)(this->getComplexityValue(action)*100.0);

  glVertex3f(coords[0][0], coords[0][1], coords[0][2]);
  for (int segment = 0; segment < numControlpoints.getValue() - 1; segment++) {

    float timestep  = (timestamp[segment + 1] - timestamp[segment]).getValue()/PRIVATE(this)->linesPerSegment;
    float time      = timestamp[segment].getValue() + timestep;
    SbVec3f vec;

    for (int i = 0; i < PRIVATE(this)->linesPerSegment - 1; i++) {
      TCB(coords, timestamp, numControlpoints.getValue(), time, vec);
      glVertex3f(vec[0], vec[1], vec[2]);
      
      time += timestep;
    }// for

    glVertex3f(coords[segment + 1][0], coords[segment + 1][1], coords[segment + 1][2]);
  }// for

  glEnd();
  glEnable(GL_LIGHTING);
}// GLRender








// Doc from parent class.
void
SoTCBCurve::generatePrimitives(SoAction * action)
{
  SoState * state = action->getState();

  const SoCoordinateElement *coordElement= SoCoordinateElement::getInstance(state);
  const SbVec3f * coords =  coordElement->getArrayPtr3();
  

  //---- Generating...
  // FIXME: I guess things like material, normals, texturecoords, drawstyle 
  // etc should be stuffed here somewhere. But I can't find any examples 
  // or reasonable text in that stupid book. This is probably trivial for 
  // the other guys here. 20020612 torbjorv

  SoLineDetail lineDetail;
  SoPointDetail pointDetail;

  beginShape(action, SoShape::LINE_STRIP, &lineDetail);

  // Generates each curvesegment as [resolution] lines using a for-loop. Mark 
  // that this baby guarantees the curve always touches perfectly all controlpoints
  // (avoiding floating point errors)
  
  SoPrimitiveVertex pv;
  SbVec3f point;
  point.setValue(coords[0][0], coords[0][1], coords[0][2]);
  pv.setPoint(point);
  pv.setDetail(&pointDetail);
  shapeVertex(&pv);

  if (numControlpoints.getValue() > 1)
    for (int segment = 0; segment < numControlpoints.getValue() - 1; segment++) {

      float timestep  = (timestamp[segment + 1] - timestamp[segment]).getValue()/PRIVATE(this)->linesPerSegment;
      float time      = timestamp[segment].getValue() + timestep;
      SbVec3f vec;

      for (int i = 0; i < PRIVATE(this)->linesPerSegment - 1; i++) {
        TCB(coords, timestamp, numControlpoints.getValue(), time, vec);
        point.setValue(vec[0], vec[1], vec[2]);

        pv.setPoint(point);
        shapeVertex(&pv);
        lineDetail.incPartIndex();
      
        time += timestep;
      }// for

      point.setValue(coords[segment + 1][0], coords[segment + 1][1], coords[segment + 1][2]);
      pv.setPoint(point);
      shapeVertex(&pv);
      lineDetail.incLineIndex();
      lineDetail.setPartIndex(0);
    }// for

  endShape();
}//generatePrimitives





// Doc from parent class.
void
SoTCBCurve::computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center)
{
  /*
    I'm not sure how accurate such a boundingbox must be, as this could
    be implemented very fast by using the controlpoints. But the curve will most
    likely run outside this box. 20020613 torbjorv
  */

  SoState * state = action->getState();
  
  const SoCoordinateElement *coordElement= SoCoordinateElement::getInstance(state);
  const SbVec3f * coords =  coordElement->getArrayPtr3();
 
  //---- Generating geometry...
  float minx = coords[0][0];
  float miny = coords[0][1];
  float minz = coords[0][2];
  float maxx = coords[0][0];
  float maxy = coords[0][1];
  float maxz = coords[0][2];

  if (numControlpoints.getValue() > 1)
    for (int segment = 0; segment < numControlpoints.getValue() - 1; segment++) {

      float timestep  = (timestamp[segment + 1] - timestamp[segment]).getValue()/PRIVATE(this)->linesPerSegment;
      float time      = timestamp[segment].getValue() + timestep;
      SbVec3f vec;

      for (int i = 0; i < PRIVATE(this)->linesPerSegment - 1; i++) {
        TCB(coords, timestamp, numControlpoints.getValue(), time, vec);
        glVertex3f(vec[0], vec[1], vec[2]);

        if (vec[0] < minx) minx = vec[0];
        if (vec[1] < miny) miny = vec[1];
        if (vec[2] < minz) minz = vec[2];
        if (vec[0] < maxx) maxx = vec[0];
        if (vec[1] < maxy) maxy = vec[1];
        if (vec[2] < maxz) maxz = vec[2];
      
        time += timestep;
      }// for

      if (coords[segment + 1][0] < minx) minx = coords[segment + 1][0];
      if (coords[segment + 1][1] < miny) miny = coords[segment + 1][1];
      if (coords[segment + 1][2] < minz) minz = coords[segment + 1][2];
      if (coords[segment + 1][0] > maxx) maxx = coords[segment + 1][0];
      if (coords[segment + 1][1] > maxy) maxy = coords[segment + 1][1];
      if (coords[segment + 1][2] > maxz) maxz = coords[segment + 1][2];
    }// for

  box.setBounds(minx, miny, minz, maxx, maxy, maxz);
  center.setValue(0.5*(minx + maxx), 0.5*(miny + maxy), 0.5*(minz + maxz));

  return;
}//computeBBox




  
/*!
  Static function to interpolate values along a curve. 

  This function is based on the Lightwave SDK (ftp.newtek.com) and calculates a 
  TCB-type curve. Code for handling continuity/tension/bias is removed (values = 1). 
  Quit ironic as this is how the curve got it's name. :) The timestamp-table must
  be sorted in increasing order. Timevalues specified outside the range of timestamps
  will be clipped to the nearest value. 

  This function is totally independent of the rest of the class, and may be used for 
  general curvecalculations. 
*/
void 
SoTCBCurve::TCB(const float coords[][3], const float tStamps[], int numControlpoints, float time, float &x, float &y, float &z)
{
  if (numControlpoints == 0) return;

  if (time < tStamps[0]) time = tStamps[0];
  if (time > tStamps[numControlpoints - 1]) time = tStamps[numControlpoints - 1];

  int k1, k2;
  int c1, c2;
  float h1, h2, h3, h4;
  float t, t2, t3;

  float adj0, adj1;
  float d10_x, d10_y, d10_z;
  float dd0_x, dd0_y, dd0_z;
  float ds1_x, ds1_y, ds1_z;

  //---- Find object's position and angles...
  if ( numControlpoints > 1 ) {

    //---- Find segment...
    k2 = 0;
    c1 = numControlpoints - 1;
    for (c2 = 0; c2 < c1; c2++)
      if (tStamps[c2] <= time) k2++;
    k1 = k2 - 1;

    //---- Calculating t = (T - T0)/(T1 - T0)
    t = (float)(time - (float)tStamps[k1])/(tStamps[k2] - tStamps[k1]);

    d10_x = coords[k2][0] - coords[k1][0];
    d10_y = coords[k2][1] - coords[k1][1];
    d10_z = coords[k2][2] - coords[k1][2];

    t2 = t*t;
    t3 = t2*t;

    //---- Calculating some magic stuff...
    h1 = 1.0 - (3*t2 - 2*t3);
    h2 = 3*t2 - 2*t3;
    h3 = t3 - 2*t2 + t;
    h4 = t3 - t2;

    if (k1 != 0) 
      adj0 = (float)(tStamps[k2] - tStamps[k1])/(tStamps[k2] - tStamps[k1 - 1]);
    if (k2 != (numControlpoints - 1)) 
      adj1 = (float)(tStamps[k2] - tStamps[k1])/(tStamps[k2+1] - tStamps[k1]);

    //---- Calculating TCB-values...
    if (k1 == 0) {
      dd0_x = d10_x;
      dd0_y = d10_y;
      dd0_z = d10_z;
    }//if
    else {
      dd0_x = adj0*((coords[k1][0] - coords[k1 - 1][0]) + d10_x);
      dd0_y = adj0*((coords[k1][1] - coords[k1 - 1][1]) + d10_y);
      dd0_z = adj0*((coords[k1][2] - coords[k1 - 1][2]) + d10_z);
    }//else

    if (k2 == (numControlpoints - 1)) {
      ds1_x = d10_x;
      ds1_y = d10_y;
      ds1_z = d10_z;
    }//if
    else {
      ds1_x = adj1*((coords[k2 + 1][0] - coords[k2][0]) + d10_x);
      ds1_y = adj1*((coords[k2 + 1][1] - coords[k2][1]) + d10_y);
      ds1_z = adj1*((coords[k2 + 1][2] - coords[k2][2]) + d10_z);
    }//else

    x = coords[k1][0]*h1 + coords[k2][0]*h2 + dd0_x*h3 + ds1_x*h4;
    y = coords[k1][1]*h1 + coords[k2][1]*h2 + dd0_y*h3 + ds1_y*h4;
    z = coords[k1][2]*h1 + coords[k2][2]*h2 + dd0_z*h3 + ds1_z*h4;
  }//if
  else {
    x = coords[0][0];
    y = coords[0][1];
    z = coords[0][2];
  }//else if only one controlpoint...

}//TCB



/*!
  Static function to interpolate values along a curve. 

  This function is based on the Lightwave SDK (ftp.newtek.com) and calculates a 
  TCB-type curve. Code for handling continuity/tension/bias is removed (values = 1). 
  Quit ironic as this is how the curve got it's name. :) The timestamp-table must
  be sorted in increasing order. Timevalues specified outside the range of timestamps
  will be clipped to the nearest value. 

  This function is totally independent of the rest of the class, and may be used for 
  general curvecalculations. 
*/
void 
SoTCBCurve::TCB(const SoMFVec3f &vec, const SoMFTime &timestamp, const SbTime &time, SbVec3f &res)
{
  TCB(vec.getValues(0), timestamp, vec.getNum(), time, res);
}//TCB


 
 
/*!
  Static function to interpolate values along a curve. 

  This function is based on the Lightwave SDK (ftp.newtek.com) and calculates a 
  TCB-type curve. Code for handling continuity/tension/bias is removed (values = 1). 
  Quit ironic as this is how the curve got it's name. :) The timestamp-table must
  be sorted in increasing order. Timevalues specified outside the range of timestamps
  will be clipped to the nearest value. 

  This function is totally independent of the rest of the class, and may be used for 
  general curvecalculations. 
*/
void 
SoTCBCurve::TCB(const SbVec3f * vec, const SoMFTime &timestamp, int numControlpoints, const SbTime &time, SbVec3f &res)
{
  if (numControlpoints == 0) return;

  int k1, k2;
  int c1, c2;
  float h1, h2, h3, h4;
  float t, t2, t3;

  float adj0, adj1;
  float d10_x, d10_y, d10_z;
  float dd0_x, dd0_y, dd0_z;
  float ds1_x, ds1_y, ds1_z;

  //---- Find object's position and angles...
  if ( numControlpoints > 1 ) {

    //---- Find segment...
    k2 = 0;
    c1 = numControlpoints - 1;
    for (c2 = 0; c2 < c1; c2++)
      if (timestamp[c2] <= time) k2++;
    k1 = k2 - 1;

    //---- Calculating t = (T - T0)/(T1 - T0)
    t = (time - timestamp[k1])/(timestamp[k2] - timestamp[k1]);

    d10_x = vec[k2][0] - vec[k1][0];
    d10_y = vec[k2][1] - vec[k1][1];
    d10_z = vec[k2][2] - vec[k1][2];

    t2 = t*t;
    t3 = t2*t;

    //---- Calculating some magic stuff...
    h1 = 1.0 - (3*t2 - 2*t3);
    h2 = 3*t2 - 2*t3;
    h3 = t3 - 2*t2 + t;
    h4 = t3 - t2;

    if (k1 != 0) 
      adj0 = (timestamp[k2] - timestamp[k1])/(timestamp[k2] - timestamp[k1 - 1]);
    if (k2 != (numControlpoints - 1)) 
      adj1 = (timestamp[k2] - timestamp[k1])/(timestamp[k2+1] - timestamp[k1]);

    //---- Calculating TCB-values...
    if (k1 == 0) {
      dd0_x = d10_x;
      dd0_y = d10_y;
      dd0_z = d10_z;
    }//if
    else {
      dd0_x = adj0*((vec[k1][0] - vec[k1 - 1][0]) + d10_x);
      dd0_y = adj0*((vec[k1][1] - vec[k1 - 1][1]) + d10_y);
      dd0_z = adj0*((vec[k1][2] - vec[k1 - 1][2]) + d10_z);
    }//else

    if (k2 == (numControlpoints - 1)) {
      ds1_x = d10_x;
      ds1_y = d10_y;
      ds1_z = d10_z;
    }//if
    else {
      ds1_x = adj1*((vec[k2 + 1][0] - vec[k2][0]) + d10_x);
      ds1_y = adj1*((vec[k2 + 1][1] - vec[k2][1]) + d10_y);
      ds1_z = adj1*((vec[k2 + 1][2] - vec[k2][2]) + d10_z);
    }//else

    res[0] = vec[k1][0]*h1 + vec[k2][0]*h2 + dd0_x*h3 + ds1_x*h4;
    res[1] = vec[k1][1]*h1 + vec[k2][1]*h2 + dd0_y*h3 + ds1_y*h4;
    res[2] = vec[k1][2]*h1 + vec[k2][2]*h2 + dd0_z*h3 + ds1_z*h4;
  }//if
  else {
    res[0] = vec[0][0];
    res[1] = vec[0][1];
    res[2] = vec[0][2];
  }//else if only one controlpoint...
}//TCB



/*!
  Returns an integer with the number of lines per segment rendered at the previous pass.
*/
int SoTCBCurve::getLinesPerSegment()
{
  return PRIVATE(this)->linesPerSegment;
}// getLinesPerSegment
