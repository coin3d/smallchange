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
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoLevelOfDetail.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoCube.h>

#include "SmAxisKit.h"

SO_KIT_SOURCE(SmAxisKit);

class SmAxisKitP {

public:
  SmAxisKitP(SmAxisKit * master) {
    this->master = master;
  }

  SoFieldSensor * axisRangeSensor;
  SoFieldSensor * markerIntervalSensor;
  SoFieldSensor * textIntervalSensor;
  SoFieldSensor * digitsSensor;

  SoSeparator * axisRoot;
  SoSeparator * generateAxis(int LODlevel) const;
  SoLevelOfDetail * generateLOD(void) const;
  SoSeparator * setupMasterNodes(void) const;

private:
  SmAxisKit * master;
};

static void fieldsChangedCallback(void * classObject, SoSensor * sensor);

#define PRIVATE(p) ((p)->pimpl)
#define PUBLIC(p) ((p)->master)

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

  PRIVATE(this)->axisRoot->addChild(PRIVATE(this)->generateLOD());

  setPart("topSeparator", PRIVATE(this)->axisRoot);

  PRIVATE(this)->axisRangeSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->axisRangeSensor->setPriority(0);
  PRIVATE(this)->axisRangeSensor->attach(&this->axisRange);

  PRIVATE(this)->markerIntervalSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->markerIntervalSensor->setPriority(0);
  PRIVATE(this)->markerIntervalSensor->attach(&this->markerInterval);

  PRIVATE(this)->textIntervalSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->textIntervalSensor->setPriority(0);
  PRIVATE(this)->textIntervalSensor->attach(&this->textInterval);

  PRIVATE(this)->digitsSensor = new SoFieldSensor(fieldsChangedCallback,PRIVATE(this));
  PRIVATE(this)->digitsSensor->setPriority(0);
  PRIVATE(this)->digitsSensor->attach(&this->digits);
}

SmAxisKit::~SmAxisKit()
{
  delete PRIVATE(this)->axisRangeSensor;
  delete PRIVATE(this)->markerIntervalSensor;
  delete PRIVATE(this)->textIntervalSensor;
  delete PRIVATE(this)->digitsSensor;

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

SoLevelOfDetail *
SmAxisKitP::generateLOD(void) const
{
  SoLevelOfDetail * LODnode = new SoLevelOfDetail; 
  LODnode->screenArea.set1Value(0, 24000);
  LODnode->screenArea.set1Value(1, 12000);
  LODnode->screenArea.set1Value(2, 6000);
  LODnode->screenArea.set1Value(3, 2000);  
  LODnode->screenArea.set1Value(4, 800);  
  
  for (int i=0;i<5;++i) 
    LODnode->addChild(this->generateAxis(i));

  return LODnode;
}

SoSeparator *
SmAxisKitP::setupMasterNodes(void) const
{
  // Setting up the nodes which are used for every LOD   
  float range = (PUBLIC(this)->axisRange.getValue()[1] - PUBLIC(this)->axisRange.getValue()[0]);

  // Axis
  SoSeparator * masterAxis = new SoSeparator;
  SoSeparator * sep1 = new SoSeparator;
  SoTranslation * trans1 = new SoTranslation;
  SoTranslation * centerTrans = new SoTranslation;
  SoBaseColor * axisColor = new SoBaseColor;

  // Making an invisible axis out of a cube to make sure the LOD will
  // work properly
  SoSeparator * invisibleCubeSep = new SoSeparator;
  SoDrawStyle * drawStyle = new SoDrawStyle;
  drawStyle->style = SoDrawStyleElement::INVISIBLE;
  SoCube * invisibleCube = new SoCube;
  invisibleCube->width.setValue(range);

  invisibleCubeSep->addChild(drawStyle);
  invisibleCubeSep->addChild(invisibleCube);
  sep1->addChild(invisibleCubeSep);

  SoCoordinate3 * axisCoords = new SoCoordinate3;
  SoLineSet * axisLine = new SoLineSet;  
  axisLine->numVertices = 2;
  axisCoords->point.set1Value(0, 0.0f, 0.0f, 0.0f);
  axisCoords->point.set1Value(1, range, 0.0f, 0.0f);

  axisColor->rgb.setValue(1.0f, 1.0f, 1.0f);
  trans1->translation.setValue(range/2, 0.0f, 0.0f);
  centerTrans->translation.setValue(-range/2, 0.0f, 0.0f);

  sep1->addChild(centerTrans);
  sep1->addChild(axisColor);
  sep1->addChild(axisCoords);
  sep1->addChild(axisLine);
    
  SoSeparator * axisnamesep = new SoSeparator;
  SoTranslation * trans3 = new SoTranslation;
  trans3->translation.setValue(0.0f, 2.0f, 0.0f);

  SoText2 * newaxisname = new SoText2;  
  newaxisname->string.connectFrom(&PUBLIC(this)->axisName);
    
  axisnamesep->addChild(axisColor);
  axisnamesep->addChild(trans1);
  axisnamesep->addChild(newaxisname);

  masterAxis->addChild(sep1);
  masterAxis->addChild(axisnamesep);
  return masterAxis;
}

SoSeparator * 
SmAxisKitP::generateAxis(int LODlevel) const
{

  SoSeparator * root = new SoSeparator;
  float range = (PUBLIC(this)->axisRange.getValue()[1] - PUBLIC(this)->axisRange.getValue()[0]);
 
  SoTranslation * centerTrans = new SoTranslation;
  centerTrans->translation.setValue(-range/2, 0.0f, 0.0f);

  root->addChild(this->setupMasterNodes());

  // Markers
  SoSeparator * sep3 = new SoSeparator;
  SoLineSet * marker1 = new SoLineSet;
  SoLineSet * marker2 = new SoLineSet;
  SoCoordinate3 * markerCoords1 = new SoCoordinate3;
  SoCoordinate3 * markerCoords2 = new SoCoordinate3;
  SoTranslation * mtrans1 = new SoTranslation;
  SoTranslation * mtrans2 = new SoTranslation;
  SoBaseColor * markerColor = new SoBaseColor;

  markerCoords1->point.set1Value(0, 0.0f, 0.0f, 0.0f);
  markerCoords1->point.set1Value(1, 0.0f, 0.2f, 0.0f);
  markerCoords2->point.set1Value(0, 0.0f, 0.0f, 0.0f);
  markerCoords2->point.set1Value(1, 0.0f, 0.4f, 0.0f);

  marker1->numVertices = 2;
  marker2->numVertices = 2;

  mtrans1->translation.setValue(PUBLIC(this)->markerInterval.getValue(), 0.0f, 0.0f);
 
  markerColor->rgb.setValue(1.0f, 1.0f, 0.7f);

  sep3->addChild(centerTrans);
  sep3->addChild(markerColor);

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
    default:
      skip = TRUE;
      break;
    }

    if (counter == 5) {
      if (!skip) {
        sep3->addChild(markerCoords2);
        sep3->addChild(marker2);
      }
      counter = 0;
    } 
    else {
      if (!skip) {
        sep3->addChild(markerCoords1);
        sep3->addChild(marker1);
      }
    }

    sep3->addChild(mtrans1);
    pos += PUBLIC(this)->markerInterval.getValue();
    ++counter;
    ++lodskipper;
  }
  root->addChild(sep3);

  // Text
  SoSeparator * textsep = new SoSeparator;
  SoTranslation * ttrans1 = new SoTranslation;
  SoTranslation * ttrans2 = new SoTranslation;

  ttrans1->translation.setValue(PUBLIC(this)->textInterval.getValue(), 0.0f, 0.0f);
  ttrans2->translation.setValue(0.0f, 1.0f, 0.0f);  

  textsep->addChild(centerTrans);
  textsep->addChild(ttrans2);

  SbString tmpstr;
  const char * tmptext = (tmpstr.sprintf("%%.%df", PUBLIC(this)->digits.getValue())).getString();  
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
      if ((lodskipper & 1) || (lodskipper & 2) || (lodskipper & 4))
        skip = TRUE;      
      break;
    case 4:
      skip = TRUE;
      break;
    default:
      break;
    }
    
    SbString mtext;
    SoText2 * markerText = new SoText2;
    markerText->string.setValue(mtext.sprintf(tmptext, (pos + PUBLIC(this)->axisRange.getValue()[0])));

    if (!skip) 
      textsep->addChild(markerText);
    textsep->addChild(ttrans1);

    pos += PUBLIC(this)->textInterval.getValue();
    ++lodskipper;

  }

  root->addChild(textsep);
   
  return root;

}

static void fieldsChangedCallback(void * classObject, SoSensor * sensor)
{
  SmAxisKitP * thisp = (SmAxisKitP *) classObject;  // Fetch caller object

  thisp->axisRoot->removeAllChildren();
  thisp->axisRoot->addChild(thisp->generateLOD());
}
