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

#include <Inventor/draggers/SoTranslate1Dragger.h>
#include <Inventor/nodes/SoTranslation.h> 
#include <Inventor/nodes/SoSeparator.h> 
#include <Inventor/nodes/SoBaseColor.h> 
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoAntiSquish.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoSwitch.h>

#include "SmRangeTranslate1DraggerKit.h"

static void draggerCallback(void * classObject, SoDragger * dragger);
static void translationChangedCallback(void * classObject, SoSensor * sensor);

SO_KIT_SOURCE(SmRangeTranslate1DraggerKit);

class SmRangeTranslate1DraggerKitP {

public:
  SmRangeTranslate1DraggerKitP(SmRangeTranslate1DraggerKit * master) {
    this->master = master;
  }
  SmRangeTranslate1DraggerKit * master;

  void initDragger();
  void reshapeDragger();
 
  SoTranslate1Dragger * dragger;
  SoFieldSensor * draggerSensor;
  SoSeparator * draggerRoot;
  SoSwitch * frontArrowSwitch;
  SoSwitch * rearArrowSwitch;

};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

SmRangeTranslate1DraggerKit::SmRangeTranslate1DraggerKit()
{

  PRIVATE(this) = new SmRangeTranslate1DraggerKitP(this);

  SO_KIT_CONSTRUCTOR(SmRangeTranslate1DraggerKit);
  SO_KIT_ADD_CATALOG_ENTRY(topSep, SoSeparator, TRUE, this, "", TRUE);
  SO_KIT_ADD_FIELD(range, (SbVec2f(0, 1.0f)));
  SO_KIT_INIT_INSTANCE();


  PRIVATE(this)->draggerSensor = new SoFieldSensor(translationChangedCallback, PRIVATE(this));
  PRIVATE(this)->draggerSensor->setPriority(0);
  PRIVATE(this)->draggerSensor->attach(&this->translation);

  initSmRangeTranslate1Dragger();
  PRIVATE(this)->reshapeDragger();

}

SmRangeTranslate1DraggerKit::~SmRangeTranslate1DraggerKit()
{
  delete PRIVATE(this)->draggerSensor;
  PRIVATE(this)->draggerRoot->removeAllChildren();
  PRIVATE(this)->draggerRoot->unref();
}

void
SmRangeTranslate1DraggerKit::initClass(void)
{
  SO_KIT_INIT_CLASS(SmRangeTranslate1DraggerKit, SoDragger, "Dragger");
}

SbBool 
SmRangeTranslate1DraggerKit::affectsState(void) const
{
  return FALSE;
}

void
SmRangeTranslate1DraggerKit::initSmRangeTranslate1Dragger()
{
  PRIVATE(this)->dragger = new SoTranslate1Dragger;
  PRIVATE(this)->dragger->addMotionCallback(&draggerCallback, PRIVATE(this));

  PRIVATE(this)->draggerRoot = new SoSeparator;
  PRIVATE(this)->draggerRoot->ref();
  PRIVATE(this)->draggerRoot->addChild(PRIVATE(this)->dragger);

  PRIVATE(this)->frontArrowSwitch = new SoSwitch();
  PRIVATE(this)->rearArrowSwitch = new SoSwitch();

  setPart("topSep", PRIVATE(this)->draggerRoot);

}

void
SmRangeTranslate1DraggerKitP::reshapeDragger()
{

  SoSeparator * newDraggerSep = new SoSeparator;
  SoSeparator * newDraggerActiveSep = new SoSeparator;
  
  SoShapeHints * draggerHint = new SoShapeHints;
  draggerHint->vertexOrdering = SoShapeHints::CLOCKWISE;
  draggerHint->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
  newDraggerSep->addChild(draggerHint);
  newDraggerActiveSep->addChild(draggerHint);

  SoBaseColor * headColor = new SoBaseColor;
  headColor->rgb.setValue(0.4f, 0.7f, 0.6f);

  SoBaseColor * defaultColor = new SoBaseColor;
  SoBaseColor * activeColor = new SoBaseColor;
  defaultColor->rgb.setValue(0.4f, 0.7f, 0.4f);
  activeColor->rgb.setValue(0.6f, 1.0f, 0.6f);  
  newDraggerSep->addChild(defaultColor);
  newDraggerActiveSep->addChild(activeColor);
  
  SoSeparator * frontHeadSep = new SoSeparator;
  SoSeparator * rearHeadSep = new SoSeparator;

  SoCoordinate3 * arrowBodyCoords = new SoCoordinate3;
  SoCoordinate3 * arrowHeadCoordsFront = new SoCoordinate3;
  SoCoordinate3 * arrowHeadCoordsRear = new SoCoordinate3;
  
  arrowBodyCoords->point.set1Value(0, 1.0, -1.0f, 0.0f);
  arrowBodyCoords->point.set1Value(1, -1.0f, -1.0f, 0.0f);
  arrowBodyCoords->point.set1Value(2, -1.0f, 1.0f, 0.0f);
  arrowBodyCoords->point.set1Value(3, 1.0f, 1.0f, 0.0f);

  arrowHeadCoordsFront->point.set1Value(0, 0.1f, 1.0f, 0.0f);
  arrowHeadCoordsFront->point.set1Value(1, 1.35f, 0.0f, 0.0f);
  arrowHeadCoordsFront->point.set1Value(2, 0.1f, -1.0f, 0.0f);

  arrowHeadCoordsRear->point.set1Value(0, -0.1f, 1.0f, 0.0f);
  arrowHeadCoordsRear->point.set1Value(1, -1.35f, 0.0f, 0.0f);
  arrowHeadCoordsRear->point.set1Value(2, -0.1f, -1.0f, 0.0f);

  SoFaceSet * arrowBodySet = new SoFaceSet;
  SoFaceSet * arrowHeadSet = new SoFaceSet;

  arrowBodySet->numVertices.setValue(4);
  arrowHeadSet->numVertices.setValue(3);

  SoTranslation * arrowTransFront = new SoTranslation;
  SoTranslation * arrowTransRear = new SoTranslation;
  arrowTransFront->translation.setValue(1,0,0);
  arrowTransRear->translation.setValue(-2,0,0);

  SoAntiSquish * antiSquish = new SoAntiSquish;
  antiSquish->sizing = SoAntiSquish::Y;

  frontHeadSep->addChild(this->frontArrowSwitch);
  rearHeadSep->addChild(this->rearArrowSwitch);

  newDraggerSep->addChild(arrowBodyCoords);
  newDraggerSep->addChild(arrowBodySet);
  newDraggerSep->addChild(arrowTransFront);
  newDraggerSep->addChild(frontHeadSep);
  newDraggerSep->addChild(arrowTransRear);
  newDraggerSep->addChild(rearHeadSep);
   
  newDraggerActiveSep->addChild(arrowBodyCoords);
  newDraggerActiveSep->addChild(arrowBodySet);
  newDraggerActiveSep->addChild(arrowTransFront);
  newDraggerActiveSep->addChild(frontHeadSep);
  newDraggerActiveSep->addChild(arrowTransRear);
  newDraggerActiveSep->addChild(rearHeadSep);

  this->frontArrowSwitch->addChild(antiSquish);
  this->frontArrowSwitch->addChild(headColor);
  this->frontArrowSwitch->addChild(arrowHeadCoordsFront);
  this->frontArrowSwitch->addChild(arrowHeadSet);

  this->rearArrowSwitch->addChild(antiSquish);
  this->rearArrowSwitch->addChild(headColor);
  this->rearArrowSwitch->addChild(arrowHeadCoordsRear);
  this->rearArrowSwitch->addChild(arrowHeadSet); 
  
  this->dragger->setPart("feedback",newDraggerSep);
  this->dragger->setPart("feedbackActive",newDraggerActiveSep);

  this->dragger->setPart("translator",newDraggerSep);
  this->dragger->setPart("translatorActive",newDraggerActiveSep);

  this->frontArrowSwitch->whichChild = SO_SWITCH_ALL;
  this->rearArrowSwitch->whichChild = SO_SWITCH_ALL;


}

static void
draggerCallback(void * classObject, SoDragger * dragger)
{
  
  SmRangeTranslate1DraggerKitP * thisp = (SmRangeTranslate1DraggerKitP *) classObject;
  SoTranslate1Dragger * tdragger = (SoTranslate1Dragger *) dragger;

  SbVec3f tf = tdragger->translation.getValue();

  thisp->frontArrowSwitch->whichChild = SO_SWITCH_ALL;
  thisp->rearArrowSwitch->whichChild = SO_SWITCH_ALL;
  
  if (tf[0] > thisp->master->range[0][1]) { // Is it above?
    tf[0] = thisp->master->range[0][1];
    thisp->frontArrowSwitch->whichChild = SO_SWITCH_NONE;
  } 
  else if (tf[0] < thisp->master->range[0][0]) { // Is it below?
    tf[0] = thisp->master->range[0][0];
    thisp->rearArrowSwitch->whichChild = SO_SWITCH_NONE;
  } 
  
  thisp->draggerSensor->detach();
  tdragger->translation.setValue(tf);
  thisp->master->translation.setValue(tf);
  thisp->draggerSensor->attach(&thisp->master->translation);

}

static void 
translationChangedCallback(void * classObject, SoSensor * sensor)
{

  SmRangeTranslate1DraggerKitP * thisp = (SmRangeTranslate1DraggerKitP *) classObject;
  SoFieldSensor * fsens = (SoFieldSensor *) sensor;
  SoSFVec3f * field = (SoSFVec3f *) fsens->getAttachedField();

  thisp->dragger->translation.setValue(field->getValue());
  
}
