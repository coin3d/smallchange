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

// FIXME: this code does not compile as it is. I'm not sure whether
// that is due to bit-rot or if it was just never completed. It
// /looks/ fairly complete, but check with Torbjørn Vik before
// attempting to fix. 20030109 mortene.

#include <Inventor/SoDB.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include "../nodes/SoTCBCurve.h"
#include "SoCameraPathEditKit.h"
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/draggers/SoDragPointDragger.h>
#include <Inventor/draggers/SoTranslate1Dragger.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/details/SoLineDetail.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include "../SoText2Set/SoText2Set.h"
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/SbTime.h>
#include <Inventor/nodes/SoFile.h>
#include "SoAngle1Manip.h"
#include "SoAngle1Dragger.h"
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoLightModel.h>



SO_KIT_SOURCE(SoCameraPathEditKit);


void
SoCameraPathEditKit::initClass()
{
  SO_KIT_INIT_CLASS(SoCameraPathEditKit, SoBaseKit, "BaseKit");

  SoTCBCurve::initClass();
  SoText2Set::initClass();
  SoAngle1Dragger::initClass();
  SoAngle1Manip::initClass();
}




SoCameraPathEditKit::SoCameraPathEditKit()
{

  SO_KIT_CONSTRUCTOR(SoCameraPathEditKit);

  // Fields
  SO_NODE_ADD_FIELD(timestampVisible, (TRUE));
  SO_NODE_ADD_FIELD(timestampEnabled, (FALSE));
  SO_NODE_ADD_FIELD(timeScale, (1));
  SO_NODE_ADD_FIELD(activePoint, (0));

  SO_NODE_ADD_FIELD(position, ((0, 0, 0)));
  SO_NODE_ADD_FIELD(orientation, ((0, 0, 0)));
  SO_NODE_ADD_FIELD(timestamp, (0.0));
  SO_NODE_ADD_FIELD(editMode, (POSITION));

  timestamp.setNum(0);
  position.setNum(0);
  orientation.setNum(0);

  // Catalog
  SO_KIT_ADD_CATALOG_ENTRY(rootSeparator, SoSeparator, FALSE, this, "", FALSE);

  // Sub rootSeparator
  SO_KIT_ADD_CATALOG_ENTRY(eventCallback, SoEventCallback, FALSE, rootSeparator, draggerSeparator, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(draggerSeparator, SoSeparator, FALSE, rootSeparator, feedbackSeparator, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(feedbackSeparator, SoSeparator, FALSE, rootSeparator, "", FALSE);

  // Sub draggerSeparator
  SO_KIT_ADD_CATALOG_ENTRY(draggerPick, SoPickStyle, TRUE, draggerSeparator, draggerDraw, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(draggerDraw, SoDrawStyle, TRUE, draggerSeparator, draggerTransform, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(draggerTransform, SoTransform, FALSE, draggerSeparator, draggerSwitch, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(draggerSwitch, SoSwitch, FALSE, draggerSeparator, "", FALSE);

  // Sub draggerSwitch
  SO_KIT_ADD_CATALOG_ENTRY(timeDragger, SoTranslate1Dragger, TRUE, draggerSwitch, posDragger, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(posDragger, SoDragPointDragger, TRUE, draggerSwitch, orDragger, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(orDragger, SoSeparator, FALSE, draggerSwitch, "", FALSE);

  // Sub orDragger
  SO_KIT_ADD_CATALOG_ENTRY(orLightModel, SoLightModel, TRUE, orDragger, headingMaterial, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(headingMaterial, SoMaterial, TRUE, orDragger, heading, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(heading, SoAngle1Manip, TRUE, orDragger, pitchMaterial, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(pitchMaterial, SoMaterial, TRUE, orDragger, pitchRotate, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(pitchRotate, SoRotation, TRUE, orDragger, pitch, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(pitch, SoAngle1Manip, TRUE, orDragger, bankMaterial, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(bankMaterial, SoMaterial, TRUE, orDragger, bankRotate, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(bankRotate, SoRotation, TRUE, orDragger, bank, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(bank, SoAngle1Manip, TRUE, orDragger, "", TRUE);

  // Sub feedbackSeparator
  SO_KIT_ADD_CATALOG_ENTRY(coordinates, SoCoordinate3, TRUE, feedbackSeparator, tagSeparator, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(tagSeparator, SoSeparator, FALSE, feedbackSeparator, curveSeparator, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(curveSeparator, SoSeparator, FALSE, feedbackSeparator, objectSeparator, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(objectSeparator, SoSeparator, FALSE, feedbackSeparator, "", FALSE);

  // Sub tagSeparator
  SO_KIT_ADD_CATALOG_ENTRY(tagDrawStyle, SoDrawStyle, TRUE, tagSeparator, tags, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(tags, SoText2Set, TRUE, tagSeparator, "", TRUE);

  // Sub curveSeparator
  SO_KIT_ADD_CATALOG_ENTRY(curveDrawStyle, SoDrawStyle, TRUE, curveSeparator, controlpoints, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(controlpoints, SoPointSet, TRUE, curveSeparator, curve, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(curve, SoTCBCurve, TRUE, curveSeparator, "", TRUE);

  // Sub objectSeparator
  SO_KIT_ADD_CATALOG_ENTRY(objectTransform, SoTransform, TRUE, objectSeparator, objectDrawStyle, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(objectDrawStyle, SoDrawStyle, TRUE, objectSeparator, objectModel, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(objectModel, SoFile, TRUE, objectSeparator, "", TRUE);

  SO_KIT_INIT_INSTANCE();


  SoFieldSensor *activePointSensor = new SoFieldSensor(activePointCB, this);
  activePointSensor->attach(&activePoint);


  //---- Setting up initial data for the curve
  SoDrawStyle * drawStyle = new SoDrawStyle;
  drawStyle->pointSize = 5;
  drawStyle->lineWidth = 1;
  setPart("curveDrawStyle", drawStyle);


  insertPosition(SbVec3f(-10, -10, -10), SbTime(0.0));
  insertPosition(SbVec3f(10, 10, 10), SbTime(10.0));
  SoCoordinate3 * coordinates = (SoCoordinate3 *)this->getAnyPart("coordinates", TRUE);


  SoPointSet * controlpoints = (SoPointSet *)this->getAnyPart("controlpoints", TRUE);
  setPart("controlpoints", controlpoints);

  SoTCBCurve * curve = (SoTCBCurve *)this->getAnyPart("curve", TRUE);
  curve->setTimestampSrc(&timestamp);


  //---- Setting up draggers... 
  SoSwitch* draggerSwitch = (SoSwitch*)this->getAnyPart("draggerSwitch", TRUE);
  draggerSwitch->whichChild = 0;

  SoDrawStyle *draggerDraw = (SoDrawStyle *)this->getAnyPart("draggerDraw", TRUE);
  draggerDraw->style = SoDrawStyle::FILLED;

  SoDragPointDragger * posDragger = (SoDragPointDragger *)this->getAnyPart("posDragger", TRUE);;
  SoFieldSensor * posSensor = new SoFieldSensor(posCallback, this);
  posSensor->attach(&posDragger->translation);
  posDragger->addFinishCallback(dragposFinishCB, this);

  SoTranslate1Dragger * timeDragger = (SoTranslate1Dragger *)this->getAnyPart("timeDragger", TRUE);
  SoFieldSensor * timeSensor = new SoFieldSensor(timeCallback, this);
  timeSensor->attach(&timeDragger->translation);
  timeDragger->addStartCallback(dragtimeStartCB, this);
  timeDragger->addFinishCallback(dragtimeFinishCB, this);

  SoTransform * draggerTransform = (SoTransform*)this->getAnyPart("draggerTransform", TRUE);

  SoAngle1Manip * heading = (SoAngle1Manip*) this->getAnyPart("heading", TRUE);
  SoAngle1Manip * pitch = (SoAngle1Manip*) this->getAnyPart("pitch", TRUE);
  SoAngle1Manip * bank = (SoAngle1Manip*) this->getAnyPart("bank", TRUE);

  SoAngle1Dragger * draggerHeading = (SoAngle1Dragger *)heading->getDragger();
  SoAngle1Dragger * draggerPitch = (SoAngle1Dragger *)pitch->getDragger();
  SoAngle1Dragger * draggerBank = (SoAngle1Dragger *)bank->getDragger();

  SoFieldSensor * headingSensor = new SoFieldSensor(orCallback, this);
  SoFieldSensor * pitchSensor = new SoFieldSensor(orCallback, this);
  SoFieldSensor * bankSensor = new SoFieldSensor(orCallback, this);
  headingSensor->attach(&draggerHeading->angle);
  pitchSensor->attach(&draggerPitch->angle);
  bankSensor->attach(&draggerBank->angle);


  SoRotation * pitchRotate = (SoRotation *)this->getAnyPart("pitchRotate", TRUE);
  pitchRotate->rotation.setValue(SbVec3f(0, 0, 1), 0.5*3.1459);
  SoRotation * bankRotate = (SoRotation *)this->getAnyPart("bankRotate", TRUE);
  bankRotate->rotation.setValue(SbVec3f(1, 0, 0), 0.5*3.1459);

  draggerTransform->translation = coordinates->point[activePoint.getValue()];

  SoText2Set * tags = (SoText2Set *)this->getAnyPart("tags", TRUE);
  tags->justification = SoText2Set::CENTER;
  tags->displacement = SbVec2f(0.0, 20.0);
  buildTags();
  activePoint = 0;


  SoMaterial * headingMaterial = new SoMaterial;
  SoMaterial * pitchMaterial = new SoMaterial;
  SoMaterial * bankMaterial = new SoMaterial;

  headingMaterial->diffuseColor = SbColor(1.0, 0.0, 0.0);
  // headingMaterial->setOverride(TRUE);
  pitchMaterial->diffuseColor = SbColor(0.0, 1.0, 0.0);
  // pitchMaterial->setOverride(TRUE);
  bankMaterial->diffuseColor = SbColor(0.0, 0.0, 1.0);
  // bankMaterial->setOverride(TRUE);

  draggerHeading->setPart("material", headingMaterial);
  draggerPitch->setPart("material", pitchMaterial);
  draggerBank->setPart("material", bankMaterial);

  SoLightModel * orLightModel = (SoLightModel *)this->getAnyPart("orLightModel", TRUE);
  orLightModel->model = SoLightModel::BASE_COLOR;


  //---- Feedback object
  SoFile * file = (SoFile *)this->getAnyPart("objectModel", TRUE);
  file->name.set("cameraModel.iv");

  SoDrawStyle * objectDrawStyle = (SoDrawStyle *)this->getAnyPart("objectDrawStyle", TRUE);
  objectDrawStyle->style = SoDrawStyle::LINES;

  SoTransform * objectTransform = (SoTransform *)this->getAnyPart("objectTransform", TRUE);
  SbMatrix m1, m2;
  SbRotation r;
  r.setValue(SbVec3f(0, 1, 0), -0.5*3.1459);
  m1.setRotate(r);
  r.setValue(SbVec3f(1, 0, 0), -0.5*3.1459);
  m2.setRotate(r);
  m1.multLeft(m2);
  // objectTransform->rotation = m1;

  //---- Setting up sensors and callbacks
  SoFieldSensor * activePointCB = new SoFieldSensor(timeCallback, this);
  timeSensor->attach(&timeDragger->translation);

  SoEventCallback * eventCallback = (SoEventCallback*) this->getAnyPart("eventCallback", TRUE);
  eventCallback->addEventCallback(SoMouseButtonEvent::getClassTypeId(), pickCallback, this);
  eventCallback->addEventCallback(SoKeyboardEvent::getClassTypeId(), keyCallback, this);

}



void SoCameraPathEditKit::timeCallback(void * data, SoSensor * sensor)
{
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;
  SoTCBCurve * curve = SO_GET_PART(thisp, "curve", SoTCBCurve);
  SoCoordinate3 * coords = SO_GET_PART(thisp, "coordinates", SoCoordinate3);
  SoTranslate1Dragger *timeDragger = SO_GET_PART(thisp, "timeDragger", SoTranslate1Dragger);

  SbVec3f translation = timeDragger->translation.getValue();
  float distance = translation[0]*thisp->timeScale.getValue();

  thisp->setTimestamp(thisp->activePoint.getValue(), thisp->dragStarttime + distance);
  thisp->buildTags();
}




void SoCameraPathEditKit::orCallback(void * data, SoSensor * sensor)
{
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;

  SoAngle1Manip * heading = (SoAngle1Manip*)thisp->getAnyPart("heading", TRUE);
  SoAngle1Manip * pitch = (SoAngle1Manip*)thisp->getAnyPart("pitch", TRUE);
  SoAngle1Manip * bank = (SoAngle1Manip*)thisp->getAnyPart("bank", TRUE);

  SoTransform * objectTransform = (SoTransform*)thisp->getAnyPart("objectTransform", TRUE);

  SoAngle1Dragger * draggerHeading = (SoAngle1Dragger *)heading->getDragger();
  SoAngle1Dragger * draggerPitch = (SoAngle1Dragger *)pitch->getDragger();
  SoAngle1Dragger * draggerBank = (SoAngle1Dragger *)bank->getDragger();

  SbVec3f tmp;
  tmp[0] = draggerHeading->angle.getValue();
  tmp[1] = draggerPitch->angle.getValue();
  tmp[2] = draggerBank->angle.getValue();



  SbRotation headingRot, pitchRot, bankRot;
  headingRot.setValue(SbVec3f(0, 1, 0), draggerHeading->angle.getValue());
  pitchRot.setValue(SbVec3f(1, 0, 0), -draggerPitch->angle.getValue());
  bankRot.setValue(SbVec3f(0, 0, 1), draggerBank->angle.getValue());

  SbMatrix headingMat, pitchMat, bankMat, totMat;
  headingMat.makeIdentity();
  pitchMat.makeIdentity();
  bankMat.makeIdentity();
  headingMat.setRotate(headingRot);
  pitchMat.setRotate(pitchRot);
  bankMat.setRotate(bankRot);

  totMat.makeIdentity();
  totMat.multRight(bankMat);
  totMat.multRight(pitchMat);
  totMat.multRight(headingMat);

  objectTransform->rotation = totMat;

  thisp->orientation.set1Value(thisp->activePoint.getValue(), tmp);
}




void SoCameraPathEditKit::posCallback(void * data, SoSensor * sensor)
{
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;

  SoCoordinate3 * coordinates = SO_GET_PART(thisp, "coordinates", SoCoordinate3);
  SoDragPointDragger * posDragger = SO_GET_PART(thisp, "posDragger", SoDragPointDragger);
  SoTransform * draggerTransform = SO_GET_PART(thisp, "draggerTransform", SoTransform);
  SoTransform * objectTransform = SO_GET_PART(thisp, "objectTransform", SoTransform);

  SbVec3f vec = posDragger->translation.getValue() + draggerTransform->translation.getValue();
  thisp->position.set1Value(thisp->activePoint.getValue(), vec);
  coordinates->point.set1Value(thisp->activePoint.getValue(), vec);
  objectTransform->translation = vec;


}





void SoCameraPathEditKit::pickCallback(void * data, SoEventCallback * eventCallback)
{
  int i;

  SoHandleEventAction * action = eventCallback->getAction();
  const SoPickedPointList & plist = action->getPickedPointList();
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;
  SoCoordinate3 * coordinates = SO_GET_PART(thisp, "coordinates", SoCoordinate3);
  SoPointSet * controlpoints = SO_GET_PART(thisp, "controlpoints", SoPointSet);
  SoDragPointDragger * posDragger = SO_GET_PART(thisp, "posDragger", SoDragPointDragger);
  SoTranslate1Dragger * timeDragger = SO_GET_PART(thisp, "timeDragger", SoTranslate1Dragger);
  SoTCBCurve * curve = SO_GET_PART(thisp, "curve", SoTCBCurve);
  SoTransform * draggerTransform = SO_GET_PART(thisp, "draggerTransform", SoTransform);
  SoPickedPoint * pick;

  // Handle left mousebutton event
  if (SoMouseButtonEvent::isButtonPressEvent(eventCallback->getEvent(), SoMouseButtonEvent::BUTTON1))
    {

      // Search for a coordinate
      for (i = 0; i < plist.getLength(); i++)
        {
          pick = plist[i];

          // Handle picking of a controlpoint
          if (((SoFullPath*)pick->getPath())->getTail() == controlpoints)
            {
              SoPointDetail * pointDetail = (SoPointDetail*)pick->getDetail();
              thisp->activePoint = pointDetail->getCoordinateIndex();
              return;
            }
        }


      // Search for the curve
      for (i = 0; i < plist.getLength(); i++)
        {
          pick = plist[i];

          // Handle adding of a new controlpoint
          // The timestamp value for the newly generated controlpoint will not be very
          // accurate. It depends on the complexitylevel of the curve as the partindex is
          // used for interpolating the timevalue. 
          if (((SoFullPath*)pick->getPath())->getTail() == curve)
            {
              const SoLineDetail * curvedetails = (SoLineDetail*)pick->getDetail(curve);

              SbVec3f pickedPoint = pick->getPoint();
              float vec3f[][3] = { {pickedPoint[0], pickedPoint[1], pickedPoint[2]} };


              // timestamp are specified
              if (thisp->timestamp.getNum() != 0) {
                float k = (curvedetails->getPartIndex() + 0.5)/((float)curve->getLinesPerSegment());
                SbTime timeA = thisp->timestamp[curvedetails->getLineIndex()];
                SbTime timeB = thisp->timestamp[curvedetails->getLineIndex() + 1];
                SbTime delta = timeB - timeA;
                SbTime t = (timeA + k*delta).getValue();
                thisp->insertPosition(pickedPoint, t);
              }
              else
                thisp->insertPosition(pickedPoint, curvedetails->getLineIndex() + 1);

              thisp->buildTags();
              return;
            }
        }

    }


  if (SoMouseButtonEvent::isButtonPressEvent(eventCallback->getEvent(), SoMouseButtonEvent::BUTTON2))
    {

      for (i = 0; i < plist.getLength(); i++)
        {
          pick = plist[i];

          // Handle deleting of a controlpoint
          if (((SoFullPath*)pick->getPath())->getTail() == controlpoints)
            {
              if (thisp->numControlpoints <= 2) return;

              SoPointDetail * pointDetail = (SoPointDetail*)pick->getDetail();
              int deleteidx = pointDetail->getCoordinateIndex();

              thisp->deleteControlpoint(deleteidx);
              thisp->buildTags();
            }
        }
    }
}


SoCameraPathEditKit::~SoCameraPathEditKit()
{
}




void SoCameraPathEditKit::buildTag(int tagidx)
{
  SoText2Set * tags = SO_GET_PART(this, "tags", SoText2Set);
  SbString s;

  if (timestamp[tagidx] < 0.0) {
    SbTime tmp = -timestamp[tagidx];
    s = tmp.format("-%h:%m:%s:%i");
  }
  else
    s = timestamp[tagidx].format("%h:%m:%s:%i");

  tags->strings.setValues(tagidx, 1, &s);
}





void SoCameraPathEditKit::buildTags()
{
  SoTCBCurve * curve = SO_GET_PART(this, "curve", SoTCBCurve);
  SoText2Set * tags = SO_GET_PART(this, "tags", SoText2Set);

  tags->strings.deleteValues(0);
  tags->strings.insertSpace(0, curve->numControlpoints.getValue());

  for (int i = curve->numControlpoints.getValue() - 1; i >= 0; i--)
    buildTag(i);
}



void SoCameraPathEditKit::keyCallback(void * data, SoEventCallback * eventCallback)
{
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;

  SoTranslate1Dragger * timeDragger = SO_GET_PART(thisp, "timeDragger", SoTranslate1Dragger);
  SoCoordinate3 * coords = SO_GET_PART(thisp, "coordinates", SoCoordinate3);
  SoTCBCurve * curve = SO_GET_PART(thisp, "curve", SoTCBCurve);

  if (SoKeyboardEvent::isKeyPressEvent(eventCallback->getEvent(), SoKeyboardEvent::SPACE) == TRUE) {
    thisp->flipEditmode();
  }

}





void SoCameraPathEditKit::flipEditmode()
{
  if (editMode.getValue() == POSITION)
    setEditmode(ORIENTATION);

  else
    if (editMode.getValue() == ORIENTATION)
      setEditmode(TIME);

    else
      if (editMode.getValue() == TIME)
        setEditmode(POSITION);
}


void SoCameraPathEditKit::setEditmode(Editmode mode)
{
  SoSwitch *draggerSwitch = (SoSwitch *)this->getAnyPart("draggerSwitch", TRUE);

  switch (editMode.getValue())
    {
    case ORIENTATION:
      draggerSwitch->whichChild = 2;
      break;
    case TIME:
      draggerSwitch->whichChild = 0;
      break;
    case POSITION:
      draggerSwitch->whichChild = 1;
      break;
    }
  editMode = mode;
}




//-------------------- CALLBACKS -------------------------------
void SoCameraPathEditKit::activePointCB(void * data, SoSensor * sensor)
{
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;

  thisp->updateDraggers();
}



// Saves the time-value before the dragging starts
void SoCameraPathEditKit::dragtimeStartCB(void *data, SoDragger *dragger)
{
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;
  thisp->dragStarttime = thisp->timestamp[thisp->activePoint.getValue()].getValue();
}



void SoCameraPathEditKit::dragtimeFinishCB(void *data, SoDragger *dragger)
{
  SoTranslate1Dragger * timeDragger = (SoTranslate1Dragger *)dragger;
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;
  thisp->dragStarttime = thisp->timestamp[thisp->activePoint.getValue()].getValue();
  timeDragger->translation = SbVec3f(0, 0, 0);
}



void SoCameraPathEditKit::dragposFinishCB(void *data, SoDragger *dragger)
{
  SoDragPointDragger * posDragger = (SoDragPointDragger *)dragger;
  SoCameraPathEditKit * thisp = (SoCameraPathEditKit *)data;
  SoTransform * draggerTransform = (SoTransform *)thisp->getAnyPart("draggerTransform", TRUE);

  posDragger->translation = SbVec3f(0, 0, 0);
  draggerTransform->translation = thisp->position[thisp->activePoint.getValue()];
}







void SoCameraPathEditKit::updateDraggers()
{
  SoDragPointDragger * posDragger = (SoDragPointDragger *)this->getAnyPart("posDragger", TRUE);
  SoTranslate1Dragger * timeDragger = (SoTranslate1Dragger *)this->getAnyPart("timeDragger", TRUE);
  SoTransform * draggerTransform = (SoTransform *)this->getAnyPart("draggerTransform", TRUE);
  SoTransform * objectTransform = (SoTransform *)this->getAnyPart("objectTransform", TRUE);
  SoAngle1Manip * heading = (SoAngle1Manip *)this->getAnyPart("heading", TRUE);
  SoAngle1Manip * pitch = (SoAngle1Manip *)this->getAnyPart("pitch", TRUE);
  SoAngle1Manip * bank = (SoAngle1Manip *)this->getAnyPart("bank", TRUE);

  posDragger->translation.enableNotify(FALSE);
  // posDragger->translation = SbVec3f(0, 0, 0);
  posDragger->translation.enableNotify(TRUE);

  timeDragger->translation.enableNotify(FALSE);
  // timeDragger->translation = SbVec3f(0, 0, 0);
  timeDragger->translation.enableNotify(TRUE);

  SoAngle1Dragger *headingDragger = (SoAngle1Dragger *)heading->getDragger();
  SoAngle1Dragger *pitchDragger = (SoAngle1Dragger *)pitch->getDragger();
  SoAngle1Dragger *bankDragger = (SoAngle1Dragger *)bank->getDragger();

  draggerTransform->translation = position[activePoint.getValue()];
  objectTransform->translation = position[activePoint.getValue()];


  if ((headingDragger->angle.getValue() != orientation[activePoint.getValue()][0]) ||
      (pitchDragger->angle.getValue() != orientation[activePoint.getValue()][1]) ||
      (bankDragger->angle.getValue() != orientation[activePoint.getValue()][2]))
    {
      headingDragger->setAngle(orientation[activePoint.getValue()][0]);
      pitchDragger->setAngle(orientation[activePoint.getValue()][1]);
      bankDragger->setAngle(orientation[activePoint.getValue()][2]);

      SbRotation headingRot = heading->rotation.getValue();
      SbRotation pitchRot = pitch->rotation.getValue();
      SbRotation bankRot = bank->rotation.getValue();
      SbMatrix headingMat, pitchMat, bankMat, totMat;
      headingMat.makeIdentity();
      pitchMat.makeIdentity();
      bankMat.makeIdentity();

      headingMat.setRotate(headingRot);
      pitchMat.setRotate(pitchRot);
      bankMat.setRotate(bankRot);

      totMat.makeIdentity();
      totMat.multRight(bankMat);
      totMat.multRight(pitchMat);
      totMat.multRight(headingMat);

      SbMatrix m1, m2;
      SbRotation r;
      r.setValue(SbVec3f(0, 1, 0), -0.5*3.1459);
      m1.setRotate(r);
      r.setValue(SbVec3f(1, 0, 0), -0.5*3.1459);
      m2.setRotate(r);
      m1.multLeft(m2);
      totMat.multRight(m1);

      objectTransform->rotation = totMat;
    }
}





//------------------------- ------------------------
void SoCameraPathEditKit::insertPosition(int idx, const SbVec3f &pos)
{
  //Avoid inserting beyond scope of array
  if (idx > numControlpoints) idx = numControlpoints;
  if (idx < 0) idx = 0;

  // No timestamp specified, so let's calculate the midpoint between
  // the pre and post element of the one to insert
  SbTime t;
  if (idx == numControlpoints) {
    t = timestamp[numControlpoints - 1];
  }
  else if (idx == 0) {
    t = timestamp[0];
  }
  else {
    int pre, post;
    pre = idx - 1;
    post = idx;
    t = (timestamp[post] + timestamp[pre])*0.5;
  }

  // Calculate interpolated orientation
  SbVec3f or;
  SoTCBCurve::TCB(orientation, timestamp, t, or);

  insertControlpoint(idx, pos, or);
}



// pushes the element at position [idx] down
void SoCameraPathEditKit::insertOrientation(int idx, const SbVec3f &or)
{
  //Avoid inserting beyond scope of array
  if (idx > numControlpoints) idx = numControlpoints;
  if (idx < 0) idx = 0;

  // No timestamp specified, so let's calculate the midpoint between
  // the pre and post element of the one to insert
  SbTime t;
  if (idx == numControlpoints) {
    t = timestamp[numControlpoints - 1];
  }
  else if (idx == 0) {
    t = timestamp[0];
  }
  else {
    int pre, post;
    pre = idx - 1;
    post = idx;
    t = (timestamp[post] + timestamp[pre])*0.5;
  }


  // Calculate interpolated position
  SbVec3f pos;
  SoTCBCurve::TCB(position, timestamp, t, pos);

  insertControlpoint(idx, pos, or);
}




void SoCameraPathEditKit::insertControlpoint(int idx, const SbVec3f &pos, const SbVec3f &or)
{
  SoCoordinate3 * coordinates = (SoCoordinate3 *)this->getAnyPart("coordinates", TRUE);
  SoPointSet * controlpoints = (SoPointSet *)this->getAnyPart("controlpoints", TRUE);
  SoTCBCurve * curve = (SoTCBCurve *)this->getAnyPart("curve", TRUE);

  //Avoid inserting beyond scope of array
  if (idx > numControlpoints) idx = numControlpoints;
  if (idx < 0) idx = 0;

  coordinates->point.insertSpace(idx, 1);
  timestamp.insertSpace(idx, 1);
  position.insertSpace(idx, 1);
  orientation.insertSpace(idx, 1);


  // Timestamps are enabled/user specified, so we'll have to calculate an
  // "interpolated" value for the new controlpoint
  SbTime t;
  if (timestampEnabled.getValue()) {
    if (numControlpoints == 0)
      t = 0.0;
    else {
      SbTime pre, post;
      if (idx == 0)
        pre = timestamp[0] - 1.0f*timeScale.getValue();
      else
        pre = timestamp[idx - 1];

      if (idx == numControlpoints)
        post = timestamp[idx - 1] + 1.0f*timeScale.getValue();
      else
        post = timestamp[idx + 1];

      t = (post - pre)*0.5;
    }
  }


  // Timestamps are not enabled/user specified, so let's generate linear
  // timestamps (equal intervals between each controlpoint)
  else {
    for (int i = 0; i < numControlpoints; i++)
      timestamp.set1Value(i, float(i)*timeScale.getValue());

    t = idx;
  }


  // Save the values
  setControlpoint(idx, pos, or, t);
  numControlpoints++;
  curve->numControlpoints = numControlpoints;
  controlpoints->numPoints = numControlpoints;
}







int SoCameraPathEditKit::insertControlpoint(const SbVec3f &pos, const SbVec3f &or, const SbTime &time)
{
  SoCoordinate3 * coordinates = (SoCoordinate3 *)this->getAnyPart("coordinates", TRUE);
  SoPointSet * controlpoints = (SoPointSet *)this->getAnyPart("controlpoints", TRUE);
  SoTCBCurve * curve = (SoTCBCurve *)this->getAnyPart("curve", TRUE);

  coordinates->point.insertSpace(0, 1);
  position.insertSpace(0, 1);
  timestamp.insertSpace(0, 1);
  orientation.insertSpace(0, 1);
  numControlpoints++;
  curve->numControlpoints = numControlpoints;
  controlpoints->numPoints = numControlpoints;
  activePoint = activePoint.getValue() + 1;

  int idx = setTimestamp(0, time);
  setPosition(idx, pos);
  setOrientation(idx, or);

  return idx;
}




int SoCameraPathEditKit::insertControlpoint(const SbTime &time)
{
  // Calculating interpolated orientation
  SbVec3f or;
  SoTCBCurve::TCB(orientation, timestamp, time, or);

  // Calculating interpolated position
  SbVec3f pos;
  SoTCBCurve::TCB(position, timestamp, time, pos);

  return insertControlpoint(pos, or, time);
}



int SoCameraPathEditKit::insertPosition(const SbVec3f &pos, const SbTime &time)
{
  // Calculating interpolated orientation
  SbVec3f or;
  SoTCBCurve::TCB(orientation, timestamp, time, or);

  return insertControlpoint(pos, or, time);
}



int SoCameraPathEditKit::insertOrientation(const SbVec3f &or, const SbTime &time)
{
  // Calculating interpolated position
  SbVec3f pos;
  SoTCBCurve::TCB(position, timestamp, time, pos);

  return insertControlpoint(pos, or, time);
}



void SoCameraPathEditKit::deleteControlpoint(int idx)
{
  SoCoordinate3 * coordinates = (SoCoordinate3 *)this->getAnyPart("coordinates", TRUE);
  SoPointSet * controlpoints = (SoPointSet *)this->getAnyPart("controlpoints", TRUE);
  SoTCBCurve * curve = (SoTCBCurve *)this->getAnyPart("curve", TRUE);

  coordinates->point.deleteValues(idx, 1);
  position.deleteValues(idx, 1);
  timestamp.deleteValues(idx, 1);
  orientation.deleteValues(idx, 1);

  numControlpoints--;
  controlpoints->numPoints = numControlpoints;
  curve->numControlpoints = numControlpoints;

  if ((activePoint.getValue() >= idx) && (activePoint.getValue() != 0)) {
    activePoint = activePoint.getValue() - 1;
  }
}




void SoCameraPathEditKit::setPosition(int idx, const SbVec3f &pos)
{
  SoCoordinate3 * coordinates = (SoCoordinate3 *)this->getAnyPart("coordinates", TRUE);

  coordinates->point.set1Value(idx, pos);
  position.set1Value(idx, pos);

  if (idx == activePoint.getValue())
    updateDraggers();
}



void SoCameraPathEditKit::setOrientation(int idx, const SbVec3f &or)
{
  orientation.set1Value(idx, or);

  if (idx == activePoint.getValue())
    updateDraggers();
}







// Be aware that this fx does not sort the controlpoints! 
void SoCameraPathEditKit::setControlpoint(int idx, const SbVec3f &pos, const SbVec3f &or, const SbTime &time)
{
  position.set1Value(idx, pos);
  orientation.set1Value(idx, or);
  timestamp.set1Value(idx, time);
}


// Assumes that the timestamp-list already's sorted
// The index must be within the tables range: [ 0, timestamp.getNum() >
// Returns the new index of the point (-1 if the specified idx is stupid)
int SoCameraPathEditKit::setTimestamp(int idx, const SbTime &time)
{
  if ((idx >= numControlpoints) || (idx < 0)) return -1;

  SoCoordinate3 * coords = SO_GET_PART(this, "coordinates", SoCoordinate3);

  SbVec3f tmpPos = position[idx];
  SbVec3f tmpOr = orientation[idx];

  for (int i = 0; (i < timestamp.getNum()) && (time > timestamp[i]); i++);

  if (i != idx) {
    timestamp.insertSpace(i, 1);
    coords->point.insertSpace(i, 1);
    position.insertSpace(i, 1);
    orientation.insertSpace(i, 1);

    /* if (activePoint.getValue() >= i)
       activePoint = activePoint.getValue() + 1;*/
  }

  timestamp.setValues(i, 1, &time);
  coords->point.setValues(i, 1, &tmpPos);
  position.setValues(i, 1, &tmpPos);
  orientation.setValues(i, 1, &tmpOr);

  if (i < idx) {
    timestamp.deleteValues(idx + 1, 1);
    coords->point.deleteValues(idx + 1, 1);
    position.deleteValues(idx + 1, 1);
    orientation.deleteValues(idx + 1, 1);
  }
  else
    if (i > idx) {
      timestamp.deleteValues(idx, 1);
      coords->point.deleteValues(idx, 1);
      position.deleteValues(idx, 1);
      orientation.deleteValues(idx, 1);
      i--;
    }

  if (activePoint.getValue() == idx) {
    activePoint = i;
  }
  else
    if ((activePoint.getValue() <= i) &&
        (activePoint.getValue() > idx))
      {
        activePoint = activePoint.getValue() - 1;
      }
    else
      if ((activePoint.getValue() >= i) &&
          (activePoint.getValue() < idx))
        {
          activePoint = activePoint.getValue() + 1;
        }

  return i;
}
