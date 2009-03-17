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
  \class SoAngle1Dragger SoAngle1Dragger.h Smallchange/draggers/
  SoAngle1Dragger.h
  \brief The SoAngle1Dragger class is for rotating geometry around a 
  single axis.
  \ingroup draggers

  Use an instance of this dragger class in your scenegraph to let the
  end-users of your application rotate geometry around a pre-defined
  axis vector in 3D.

  For the dragger orientation and positioning itself, use some kind of
  transformation node in your scenegraph, as usual.

  This class offers a field angle, given in radians. This field holds
  the current angle, in un-normalized values. In other words, it holds
  the number of rotations the dragger has done. 
*/

#include "SoAngle1Dragger.h"
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/projectors/SbCylinderPlaneProjector.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/nodes/SoMaterial.h>



/*!
  \var SoSFRotation SoAngle1Dragger::angle

  This field is continuously updated to contain the angle (in radians) 
  of the current direction vector of the dragger.

  The application programmer using this dragger in his scenegraph
  should connect the relevant node fields in the scene to this field
  to make them follow the dragger orientation.

  The angle hold un-normalized values. In other words, it holds the number
  of rotations the dragger has done, in addition to the orientation.
*/


/*!
  \var SoFieldSensor * SoAngle1Dragger::fieldSensor
  \internal
*/
/*!
  \var SbCylinderProjector * SoAngle1Dragger::cylinderProj
  \internal
*/
/*!
  \var SbBool SoAngle1Dragger::userProj
  \internal
*/

// *************************************************************************

SO_KIT_SOURCE(SoAngle1Dragger);

// *************************************************************************

class SoAngle1DraggerP {
public:
  SoAngle1DraggerP(SoAngle1Dragger * master) {
    this->master = master;

  }

  static void angleCallback(void *data, SoSensor *sensor);

  float lastAngle;
  float adder;
  float startDrag;

  SoFieldSensor * fieldSensor;
  SbCylinderProjector * cylinderProj;
  SbBool userProj;

private:
  SoAngle1Dragger * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************




// doc in superclass
void
SoAngle1Dragger::initClass(void)
{
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_KIT_INIT_CLASS(SoAngle1Dragger, SoDragger, "Dragger");
}

/*!
  Default constructor, sets up the dragger nodekit catalog with the
  interaction geometry.
 */
SoAngle1Dragger::SoAngle1Dragger(void)
{
  PRIVATE(this) = new SoAngle1DraggerP(this);

  SO_KIT_CONSTRUCTOR(SoAngle1Dragger);

  SO_KIT_ADD_CATALOG_ENTRY(rotatorSwitch, SoSwitch, FALSE, geomSeparator, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(rotator, SoSeparator, FALSE, rotatorSwitch,  activeRotator, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(activeRotator, SoSeparator, FALSE, rotatorSwitch, "", TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(material, SoMaterial, TRUE, rotator, geometry, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(geometry, SoSeparator,TRUE, rotator, "", TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(activeMaterial, SoMaterial, TRUE, activeRotator, activeGeometry, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(activeGeometry, SoSeparator, TRUE, activeRotator, "", TRUE);

  //FIXME: Set up a defaultbuffer for the definitions. torbjorv 07042002
  if (SO_KIT_IS_FIRST_INSTANCE()) {
    SoInteractionKit::readDefaultParts("angle1Dragger.iv",
                                       NULL,
                                       0);
  }// if


  SO_KIT_ADD_FIELD(angle, (0.0f));

  SO_KIT_INIT_INSTANCE();

  // initialize default parts
  this->setPartAsDefault("geometry",        "angle1Geometry");
  this->setPartAsDefault("activeGeometry",  "angle1Geometry");
  this->setPartAsDefault("activeMaterial",  "angle1ActiveMaterial");
  this->setPartAsDefault("material",        "angle1Material");

  // initialize switch values
  SoSwitch *sw;
  sw = SO_GET_ANY_PART(this, "rotatorSwitch", SoSwitch);
  sw->whichChild = 0;

  // setup projector
  PRIVATE(this)->cylinderProj = new SbCylinderPlaneProjector();
  PRIVATE(this)->userProj = FALSE;

  // setup callbacks
  this->addStartCallback(SoAngle1Dragger::startCB);
  this->addMotionCallback(SoAngle1Dragger::motionCB);
  this->addFinishCallback(SoAngle1Dragger::doneCB);

  PRIVATE(this)->fieldSensor = new SoFieldSensor(PRIVATE(this)->angleCallback, this);
  PRIVATE(this)->fieldSensor->setPriority(0);

  this->setUpConnections(TRUE, TRUE);
}

/*!
  Protected destructor.

  (Dragger classes are derived from SoBase, so they are reference
  counted and automatically destroyed when their reference count goes
  to 0.)
 */
SoAngle1Dragger::~SoAngle1Dragger()
{
  delete PRIVATE(this)->fieldSensor;
  if (!PRIVATE(this)->userProj) delete PRIVATE(this)->cylinderProj;
}

// Doc in superclass.
SbBool
SoAngle1Dragger::setUpConnections(SbBool onoff, SbBool doitalways)
{
  if (!doitalways && this->connectionsSetUp == onoff) return onoff;

  SbBool oldval = this->connectionsSetUp;

  if (onoff) {
    inherited::setUpConnections(onoff, doitalways);

    if (PRIVATE(this)->fieldSensor->getAttachedField() != &angle) {
      PRIVATE(this)->fieldSensor->attach(&angle);
    }
  }
  else {
    if (PRIVATE(this)->fieldSensor->getAttachedField() != NULL) {
      PRIVATE(this)->fieldSensor->detach();
    }
    inherited::setUpConnections(onoff, doitalways);
  }
  this->connectionsSetUp = onoff;
  return oldval;
}






/*!
  Replace the default cylinder projection strategy. You may want to do
  this if you change the dragger geometry, for instance.

  The default cylinder projection is an SbCylinderPlaneProjector.

  \sa SbCylinderSectionProjector, SbCylinderSheetProjector
*/
void
SoAngle1Dragger::setProjector(SbCylinderProjector * p)
{
  if (!PRIVATE(this)->userProj) delete PRIVATE(this)->cylinderProj;
  PRIVATE(this)->cylinderProj = p;
}




/*!
  Returns projector instance used for converting from user interaction
  dragger movements to 3D dragger re-orientation.

  \sa setProjector()
*/
const SbCylinderProjector *
SoAngle1Dragger::getProjector(void) const
{
  return PRIVATE(this)->cylinderProj;
}




// Doc in superclass.
void
SoAngle1Dragger::copyContents(const SoFieldContainer * fromfc,
                                         SbBool copyconnections)
{
  inherited::copyContents(fromfc, copyconnections);
  
  assert(fromfc->isOfType(SoAngle1Dragger::getClassTypeId()));
  SoAngle1Dragger *from = (SoAngle1Dragger *)fromfc;
  if (!PRIVATE(this)->userProj) {
    delete PRIVATE(this)->cylinderProj; 
  }
  PRIVATE(this)->cylinderProj = NULL;
  
  if (PRIVATE(from)->cylinderProj) {
    PRIVATE(this)->cylinderProj = (SbCylinderProjector*) 
      PRIVATE(from)->cylinderProj->copy();
  }
  else {
    // just create a new one
    PRIVATE(this)->cylinderProj = new SbCylinderPlaneProjector();
  }
  // we copied or created a new one, and need to delete it.
  PRIVATE(this)->userProj = FALSE;
}

/*! \internal */
void
SoAngle1Dragger::startCB(void * f, SoDragger * d)
{
  SoAngle1Dragger *thisp = (SoAngle1Dragger*)d;
  thisp->dragStart();
}

/*! \internal */
void
SoAngle1Dragger::motionCB(void * f, SoDragger * d)
{
  SoAngle1Dragger *thisp = (SoAngle1Dragger*)d;
  thisp->drag();
}

/*! \internal */
void
SoAngle1Dragger::doneCB(void * f, SoDragger * d)
{
  SoAngle1Dragger *thisp = (SoAngle1Dragger*)d;
  thisp->dragFinish();
}

/*! \internal
  Called when dragger is selected (picked) by the user.
*/
void
SoAngle1Dragger::dragStart(void)
{
  SoSwitch *sw;
  sw = SO_GET_ANY_PART(this, "rotatorSwitch", SoSwitch);
  SoInteractionKit::setSwitchValue(sw, 1);

  SbVec3f hitPt = this->getLocalStartingPoint();
  SbLine line(SbVec3f(0.0f, 0.0f, 0.0f), SbVec3f(0.0f, 1.0f, 0.0f));
  SbVec3f ptOnLine = line.getClosestPoint(hitPt);
  PRIVATE(this)->cylinderProj->setCylinder(SbCylinder(line, (ptOnLine-hitPt).length()));

  PRIVATE(this)->cylinderProj->setViewVolume(this->getViewVolume());
  PRIVATE(this)->cylinderProj->setWorkingSpace(this->getLocalToWorldMatrix());

  switch (this->getFrontOnProjector()) {
  case FRONT:
    PRIVATE(this)->cylinderProj->setFront(TRUE);
    break;
  case BACK:
    PRIVATE(this)->cylinderProj->setFront(TRUE);
    break;
  default: // avoid warnings
  case USE_PICK:
    PRIVATE(this)->cylinderProj->setFront(PRIVATE(this)->cylinderProj->isPointInFront(hitPt));
    break;
  }

  PRIVATE(this)->lastAngle = 0;
  PRIVATE(this)->adder = 0;
  PRIVATE(this)->startDrag = angle.getValue();

}

/*! \internal
  Called when user drags the mouse after picking the dragger.
*/
void
SoAngle1Dragger::drag(void)
{
  PRIVATE(this)->cylinderProj->setViewVolume(this->getViewVolume());
  PRIVATE(this)->cylinderProj->setWorkingSpace(this->getLocalToWorldMatrix());


  SbVec3f projPt = PRIVATE(this)->cylinderProj->project(this->getNormalizedLocaterPosition());
  SbVec3f startPt = this->getLocalStartingPoint();
  SbRotation rot = PRIVATE(this)->cylinderProj->getRotation(startPt, projPt);

  this->setMotionMatrix(this->appendRotation(this->getStartMotionMatrix(),
                                             rot, SbVec3f(0.0f, 0.0f, 0.0f)));

  // Calculating that stupid angle myself
  SbMatrix mat;
  rot.getValue(mat);
  SbVec3f vec = SbVec3f(0, 0, 1);
  mat.multVecMatrix(vec, vec); 

  float tmp = static_cast<float>(atan2(vec[0], vec[2]));
  float diff = PRIVATE(this)->lastAngle - tmp;
  if (fabs(diff) > ((350.0*2*M_PI)/360.0)) {
    if (PRIVATE(this)->lastAngle < 0)
      PRIVATE(this)->adder -= (float) (2.0*M_PI);
    else
      PRIVATE(this)->adder += (float) (2.0*M_PI);
  }//if

  PRIVATE(this)->lastAngle = tmp;
  PRIVATE(this)->fieldSensor->detach();
  angle = PRIVATE(this)->startDrag + tmp + PRIVATE(this)->adder;
  PRIVATE(this)->fieldSensor->attach(&angle);

}





/*! \internal
  Called when mouse button is released after picking and interacting
  with the dragger.
*/
void
SoAngle1Dragger::dragFinish(void)
{
  SoSwitch *sw;
  sw = SO_GET_ANY_PART(this, "rotatorSwitch", SoSwitch);
  SoInteractionKit::setSwitchValue(sw, 0);
}



void SoAngle1DraggerP::angleCallback(void *data, SoSensor *sensor)
{
  SoAngle1Dragger * thisp = (SoAngle1Dragger *)data;

//  thisp->setAngle(thisp->angle.getValue());

  SbMatrix matrix = thisp->getMotionMatrix();

  SbVec3f t, s;
  SbRotation r, so;

  matrix.getTransform(t, r, s, so);
  r.setValue(SbVec3f(0, 1, 0), thisp->angle.getValue());
  matrix.setTransform(t, r, s, so);
  thisp->setMotionMatrix(matrix);

}//axisCallback
