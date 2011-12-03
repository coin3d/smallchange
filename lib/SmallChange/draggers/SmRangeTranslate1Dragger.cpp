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

#include <Inventor/draggers/SoTranslate1Dragger.h>
#include <Inventor/nodes/SoTranslation.h> 
#include <Inventor/nodes/SoSeparator.h> 
#include <Inventor/nodes/SoBaseColor.h> 
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoAntiSquish.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoSwitch.h>

#include "SmRangeTranslate1Dragger.h"

static void draggerCallback(void * classObject, SoDragger * dragger);
static void translationChangedCallback(void * classObject, SoSensor * sensor);

SO_KIT_SOURCE(SmRangeTranslate1Dragger);

class SmRangeTranslate1DraggerP {

public:
  SmRangeTranslate1DraggerP(SmRangeTranslate1Dragger * master) {
    this->master = master;
  }
  SmRangeTranslate1Dragger * master;

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

SmRangeTranslate1Dragger::SmRangeTranslate1Dragger()
{

  PRIVATE(this) = new SmRangeTranslate1DraggerP(this);

  SO_KIT_CONSTRUCTOR(SmRangeTranslate1Dragger);

  SO_KIT_ADD_CATALOG_ENTRY(topSep, SoSeparator, TRUE, this, "", TRUE);

  SO_KIT_ADD_FIELD(range, (SbVec2f(0, 1.0f)));
  SO_KIT_ADD_FIELD(translation, (SbVec3f(0, 0, 0)));

  SO_KIT_INIT_INSTANCE();


  PRIVATE(this)->draggerSensor = new SoFieldSensor(translationChangedCallback, PRIVATE(this));
  PRIVATE(this)->draggerSensor->setPriority(0);
  PRIVATE(this)->draggerSensor->attach(&this->translation);

  initSmRangeTranslate1Dragger();
  PRIVATE(this)->reshapeDragger();

}

SmRangeTranslate1Dragger::~SmRangeTranslate1Dragger()
{
  delete PRIVATE(this)->draggerSensor;
  PRIVATE(this)->draggerRoot->removeAllChildren();
  PRIVATE(this)->draggerRoot->unref();
}

void
SmRangeTranslate1Dragger::initClass(void)
{
  SO_KIT_INIT_CLASS(SmRangeTranslate1Dragger, SoDragger, "Dragger");
}

SbBool 
SmRangeTranslate1Dragger::affectsState(void) const
{
  return FALSE;
}

void
SmRangeTranslate1Dragger::initSmRangeTranslate1Dragger()
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
SmRangeTranslate1DraggerP::reshapeDragger()
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
  
  SmRangeTranslate1DraggerP * thisp = (SmRangeTranslate1DraggerP *) classObject;
  SoTranslate1Dragger * tdragger = (SoTranslate1Dragger *) dragger;

  SbVec3f tf = tdragger->translation.getValue();

  thisp->frontArrowSwitch->whichChild = SO_SWITCH_ALL;
  thisp->rearArrowSwitch->whichChild = SO_SWITCH_ALL;
  
  if (tf[0] > thisp->master->range.getValue()[1]) { // Is it above?
    tf[0] = thisp->master->range.getValue()[1];
    thisp->frontArrowSwitch->whichChild = SO_SWITCH_NONE;
  } 
  else if (tf[0] < thisp->master->range.getValue()[0]) { // Is it below?
    tf[0] = thisp->master->range.getValue()[0];
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
  SmRangeTranslate1DraggerP * thisp = (SmRangeTranslate1DraggerP *) classObject;
  SoFieldSensor * fsens = (SoFieldSensor *) sensor;
  SoSFVec3f * field = (SoSFVec3f *) fsens->getAttachedField();

  thisp->dragger->translation.setValue(field->getValue());
}
