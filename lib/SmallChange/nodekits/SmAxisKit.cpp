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

#include <Inventor/nodekits/SoShapeKit.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoLevelOfDetail.h>
#include <Inventor/sensors/SoFieldSensor.h>

#include "SmAxisKit.h"

SO_KIT_SOURCE(SmAxisKit);

class SmAxisKitP {

public:
  SmAxisKitP(SmAxisKit * master) {
    this->master = master;
  }

  SmAxisKit * master;

  SoFieldSensor * axisRangeSensor;
  SoFieldSensor * markerIntervalSensor;
  SoFieldSensor * markerWidthSensor;
  SoFieldSensor * textIntervalSensor;
  SoFieldSensor * digitsSensor;
  SoFieldSensor * axisNameSensor;
  SoFieldSensor * arrowColorSensor;

  SoSeparator * masterAxis;
  SoText2 * axisName;

  SoSeparator * axisRoot;
  SoSeparator * generateAxis(int LODlevel);
  void generateLOD();
  void setupMasterNodes();


};

static void fieldsChangedCallback(void * classObject, SoSensor * sensor);
static void axisNameChangedCallback(void * classObject, SoSensor * sensor);

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

SmAxisKit::SmAxisKit()
{
  PRIVATE(this) = new SmAxisKitP(this);

  SO_KIT_CONSTRUCTOR(SmAxisKit);

#if defined(__COIN__) 
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, TRUE, this, "", TRUE);
#else
#define EMPTY \x0 // Inventor doesn't handle ""...
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, TRUE, this, EMPTY, TRUE);
#undef EMPTY
#endif // OIV kit specification


  SO_KIT_ADD_FIELD(axisRange, (SbVec2f(0.0f, 30.0f)));
  SO_KIT_ADD_FIELD(markerInterval, (1.0f));
  SO_KIT_ADD_FIELD(markerWidth, (0.1f));
  SO_KIT_ADD_FIELD(textInterval, (5.0f));
  SO_KIT_ADD_FIELD(digits, (2));
  SO_KIT_ADD_FIELD(axisName, (SbString("")));
  SO_KIT_ADD_FIELD(arrowColor, (SbColor(0.7f, 1.0f, 0.7f)));

  SO_KIT_INIT_INSTANCE();
  
  PRIVATE(this)->axisRoot = new SoSeparator;
  PRIVATE(this)->axisRoot->ref();

  PRIVATE(this)->setupMasterNodes();
  PRIVATE(this)->generateLOD();

  setPart("topSeparator", PRIVATE(this)->axisRoot);

  PRIVATE(this)->axisNameSensor = new SoFieldSensor(axisNameChangedCallback,PRIVATE(this));
  PRIVATE(this)->axisNameSensor->setPriority(0);
  PRIVATE(this)->axisNameSensor->attach(&this->axisName);

  PRIVATE(this)->axisRangeSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->axisRangeSensor->setPriority(0);
  PRIVATE(this)->axisRangeSensor->attach(&this->axisRange);

  PRIVATE(this)->markerIntervalSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->markerIntervalSensor->setPriority(0);
  PRIVATE(this)->markerIntervalSensor->attach(&this->markerInterval);

  PRIVATE(this)->markerWidthSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->markerWidthSensor->setPriority(0);
  PRIVATE(this)->markerWidthSensor->attach(&this->markerInterval);

  PRIVATE(this)->textIntervalSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->textIntervalSensor->setPriority(0);
  PRIVATE(this)->textIntervalSensor->attach(&this->textInterval);

  PRIVATE(this)->digitsSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->digitsSensor->setPriority(0);
  PRIVATE(this)->digitsSensor->attach(&this->digits);

  PRIVATE(this)->arrowColorSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->arrowColorSensor->setPriority(0);
  PRIVATE(this)->arrowColorSensor->attach(&this->arrowColor);
}

SmAxisKit::~SmAxisKit()
{
  delete PRIVATE(this)->axisRangeSensor;
  delete PRIVATE(this)->markerIntervalSensor;
  delete PRIVATE(this)->markerWidthSensor;
  delete PRIVATE(this)->textIntervalSensor;
  delete PRIVATE(this)->digitsSensor;
  delete PRIVATE(this)->axisNameSensor;
  delete PRIVATE(this)->arrowColorSensor;
  PRIVATE(this)->axisRoot->removeAllChildren();
  PRIVATE(this)->axisRoot->unref();
}

void
SmAxisKit::initClass(void)
{
  SO_KIT_INIT_CLASS(SmAxisKit, SoBaseKit, "BaseKit");
}

SbBool 
SmAxisKit::affectsState(void) const
{
  return FALSE;
}

void 
SmAxisKitP::generateLOD()
{

  SoLevelOfDetail * LODnode = new SoLevelOfDetail;
 
  LODnode->screenArea.set1Value(0, 2000);
  LODnode->screenArea.set1Value(1, 1200);
  LODnode->screenArea.set1Value(2, 300);
  LODnode->screenArea.set1Value(3, 50);  
  
  for (int i=0;i<4;++i) 
    LODnode->addChild(this->generateAxis(i));
   
  this->axisRoot->addChild(LODnode);

}

void
SmAxisKitP::setupMasterNodes()
{

  // Setting up the nodes which are used for every LOD 
  
  float range = (this->master->axisRange[0][1] - this->master->axisRange[0][0]);

  // Axis
  masterAxis = new SoSeparator;
  SoSeparator * sep1 = new SoSeparator;
  SoCylinder * axisCylinder = new SoCylinder;
  SoTranslation * trans1 = new SoTranslation;
  SoTranslation * trans2 = new SoTranslation;
  SoCone * arrow = new SoCone;
  SoBaseColor * axisColor = new SoBaseColor;
  SoBaseColor * arrowColor = new SoBaseColor;
  SoComplexity * complexity1 = new SoComplexity;

  axisCylinder->height.setValue(range);
  axisCylinder->radius.setValue(0.5f);
  axisCylinder->parts.setValue(SoCylinder::SIDES | SoCylinder::BOTTOM);
  axisColor->rgb.setValue(1.0f, 1.0f, 1.0f);
  arrow->bottomRadius.setValue(0.7f);
  arrow->height.setValue(3.5f);
  trans1->translation.setValue(0.0f, range/2, 0.0f);
  trans2->translation.setValue(0.0f, 1.75f, 0.0f);
  complexity1->value.setValue(0.2f);
  arrowColor->rgb.setValue(this->master->arrowColor[0]);

  sep1->addChild(complexity1);
  sep1->addChild(axisColor);
  sep1->addChild(trans1);
  sep1->addChild(axisCylinder);
  sep1->addChild(trans1);
  sep1->addChild(trans2);
  sep1->addChild(arrowColor);
  sep1->addChild(arrow);
    
  SoSeparator * axisnamesep = new SoSeparator;
  SoTranslation * trans3 = new SoTranslation;
  trans3->translation.setValue(0.0f, 2.0f, 0.0f);

  axisName = new SoText2;  
  axisName->string.setValue(this->master->axisName[0]);
    
  axisnamesep->addChild(axisColor);
  axisnamesep->addChild(trans1);
  axisnamesep->addChild(trans1);
  axisnamesep->addChild(trans2);
  axisnamesep->addChild(trans3);
  axisnamesep->addChild(axisName);

  masterAxis->addChild(sep1);
  masterAxis->addChild(axisnamesep);

}

SoSeparator * 
SmAxisKitP::generateAxis(int LODlevel)
{

  SoSeparator * root = new SoSeparator;
  float range = (this->master->axisRange[0][1] - this->master->axisRange[0][0]);
 
  root->addChild(this->masterAxis);

  // Markers
  SoSeparator * sep3 = new SoSeparator;
  SoCube * marker1 = new SoCube;
  SoCube * marker2 = new SoCube;
  SoTranslation * mtrans1 = new SoTranslation;
  SoTranslation * mtrans2 = new SoTranslation;
  SoBaseColor * markerColor = new SoBaseColor;

  marker1->height.setValue(this->master->markerWidth[0]);
  marker1->width.setValue(0.1f);
  marker1->depth.setValue(0.7f);
  
  marker2->height.setValue(this->master->markerWidth[0]);
  marker2->width.setValue(0.1f);
  marker2->depth.setValue(1.2f);

  mtrans1->translation.setValue(0.0f, this->master->markerInterval[0], 0.0f);
  mtrans2->translation.setValue(0.0f, 0.0f, 0.7f);

  markerColor->rgb.setValue(1.0f, 1.0f, 0.7f);

  sep3->addChild(markerColor);
  sep3->addChild(mtrans2);

  int lodskipper = 0;
  SbBool skip;
  float pos = 0.0f;
  int counter = 0;

  while (pos <= range) {    
    
    skip = FALSE;
    switch (LODlevel) {      
    case 0:
      break;
    case 1:
      if (lodskipper & 1) 
        skip = TRUE;      
      break;
    case 2:
      if ((lodskipper & 1) || (lodskipper & 2))
        skip = TRUE;      
      break;
    case 3:
      skip = TRUE;
      break;
    default:
      break;
    }

    if (counter == 5) {
      if(!skip)
        sep3->addChild(marker2);
      counter = 0;
    } 
    else {
      if(!skip)
        sep3->addChild(marker1);
    }

    sep3->addChild(mtrans1);
    pos += this->master->markerInterval[0];
    ++counter;
  }
  root->addChild(sep3);

  // Text
  SoSeparator * textsep = new SoSeparator;
  SoTranslation * ttrans1 = new SoTranslation;
  SoTranslation * ttrans2 = new SoTranslation;

  ttrans1->translation.setValue(0.0f, this->master->textInterval[0], 0.0f);
  ttrans2->translation.setValue(0.0f, 0.0f, 1.4f);  
  textsep->addChild(ttrans2);

  SbString tmpstr;
  const char * tmptext = (tmpstr.sprintf("%%.%df", this->master->digits[0])).getString();  
  pos = 0.0f;
  lodskipper = 0;
  
  while (pos <= range) {

    skip = FALSE;
    switch (LODlevel) {      
    case 0:
      break;
    case 1:
      if (lodskipper & 1) 
        skip = TRUE;      
      break;
    case 2:
      if ((lodskipper & 1) || (lodskipper & 2))
        skip = TRUE;      
      break;
    case 3:
      skip = TRUE;
      break;
    default:
      break;
    }
    
    SbString mtext;
    SoText2 * markerText = new SoText2;
    markerText->string.setValue(mtext.sprintf(tmptext, (pos + this->master->axisRange[0][0])));

    if (!skip) 
      textsep->addChild(markerText);
    textsep->addChild(ttrans1);

    pos += this->master->textInterval[0];
    ++lodskipper;

  }

  root->addChild(textsep);
   
  return root;

}

static void fieldsChangedCallback(void * classObject, SoSensor * sensor)
{
  SmAxisKitP * thisp = (SmAxisKitP *) classObject;  // Fetch caller object
  thisp->axisRoot->removeAllChildren();
  thisp->generateLOD();
}

static void axisNameChangedCallback(void * classObject, SoSensor * sensor)
{
  SmAxisKitP * thisp = (SmAxisKitP *) classObject;  // Fetch caller object
  thisp->axisName->string.setValue(thisp->master->axisName[0]);
}
