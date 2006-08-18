/**************************************************************************\
 *
 *  Copyright (C) 1998-2004 by Systems in Motion. All rights reserved.
 *
\**************************************************************************/

#include <Inventor/SbBasic.h>
#include "SmVesselKit.h"
#if defined(__COIN__) && (COIN_MAJOR_VERSION >= 3)

#include "SmOceanKit.h"

#include <math.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoPackedColor.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoNormalBinding.h>
#include <Inventor/nodes/SoInfo.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/SbColor.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SbTime.h>
#include <Inventor/C/basic.h>
#include <Inventor/SbBSPTree.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <SmallChange/nodes/UTMPosition.h>
#include <SmallChange/elements/UTMElement.h>
#include <SmallChange/nodes/AutoFile.h>
#include <SmallChange/nodekits/SmTrackPointKit.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodekits/SoNodeKitListPart.h>


class SmVesselKitP {
public:
  SbVec3d heading2SbVec3d(float heading);
  float getWaveSlope(SoGLRenderAction * action, SmOceanKit * ok, SbVec3d & pos, float heading, float length, float & avgElevation);
  SbVec2f getTranslation(float heading, float speed, float dt);
  SbTime lasttime;
  SoFieldSensor * positionsensor;
  SmTrackPointKit * track;
};

static void position_cb(void * closure, SoSensor *)
{
  SmVesselKit * thisp = (SmVesselKit *) closure;
  assert(thisp && thisp->isOfType(SmVesselKit::getClassTypeId()));
  thisp->savePosition();
}

// convenience define to access private data
#define PRIVATE(obj) (obj)->pimpl

SO_KIT_SOURCE(SmVesselKit);

/*!
  Constructor. 
*/
SmVesselKit::SmVesselKit(void) 
{
  PRIVATE(this) = new SmVesselKitP;
  PRIVATE(this)->lasttime = SbTime::getTimeOfDay();

  SO_KIT_CONSTRUCTOR(SmVesselKit);
  
  SO_KIT_ADD_FIELD(oceanKit, (NULL)); 
  SO_KIT_ADD_FIELD(size, (40.0, 10.0));
  SO_KIT_ADD_FIELD(speed, (0.0));
  SO_KIT_ADD_FIELD(lastDatumTime, (0.0));
  SO_KIT_ADD_FIELD(maxExtrapolationTime, (10.0));
  SO_KIT_ADD_FIELD(pitchInertia, (1.0));
  SO_KIT_ADD_FIELD(pitchResistance, (1.0));
  SO_KIT_ADD_FIELD(pitchBalance, (1.0));
  SO_KIT_ADD_FIELD(rollInertia, (1.0));
  SO_KIT_ADD_FIELD(rollResistance, (1.0));
  SO_KIT_ADD_FIELD(rollBalance, (1.0));
  SO_KIT_ADD_FIELD(trackLength, (22.0f));
  
  SO_KIT_INIT_INSTANCE();

  PRIVATE(this)->track = new SmTrackPointKit;
  PRIVATE(this)->track->trackLength.connectFrom(&this->trackLength);
  SoSwitch * geo = (SoSwitch*)this->getAnyPart("geometry", TRUE);
  geo->addChild(PRIVATE(this)->track);

  PRIVATE(this)->positionsensor = new SoFieldSensor(position_cb, this);
  PRIVATE(this)->positionsensor->attach(&this->position);
}

/*!
  Destructor
*/
SmVesselKit::~SmVesselKit()
{
  delete PRIVATE(this);
}

// Documented in superclass
void
SmVesselKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SmVesselKit, SmDynamicObjectKit, "SmDynamicObjectKit");
  }
}

void
SmVesselKit::savePosition(void)
{
  PRIVATE(this)->track->addTrackPoint(this->position.getValue());
}

// doc in parent
void 
SmVesselKit::GLRender(SoGLRenderAction * action)
{
  float elevation = 0.0;
  SbVec3d utmpos = this->position.getValue();
  SmOceanKit * ok = (SmOceanKit*)this->oceanKit.getValue();
  if (ok && ok->isOfType(SmOceanKit::getClassTypeId())) {
    float e1, e2;
    float pitch = PRIVATE(this)->getWaveSlope(action, ok, utmpos, this->heading.getValue(), this->size.getValue()[0], e1);
    float roll = -1.0 * PRIVATE(this)->getWaveSlope(action, ok, utmpos, this->heading.getValue()+90.0, this->size.getValue()[1], e2);
    elevation = (e1+e2)/2.0;
    this->pitch.setValue(pitch);
    this->roll.setValue(roll);
  }
  SbTime now = SbTime::getTimeOfDay();
  SbVec2f motion(0,0);
  if (now.getValue() - this->lastDatumTime.getValue().getValue() < this->maxExtrapolationTime.getValue() ) {
    double dt = now.getValue() - PRIVATE(this)->lasttime.getValue();
    PRIVATE(this)->lasttime = now;
    motion = PRIVATE(this)->getTranslation(this->heading.getValue(), this->speed.getValue(), dt);
  }

  this->position.enableNotify(FALSE);
  this->position.setValue(SbVec3d(utmpos[0]+motion[0], utmpos[1]+motion[1], elevation));
  this->position.enableNotify(TRUE);
  inherited::GLRender(action);
}

SbVec3d
SmVesselKitP::heading2SbVec3d(float heading)
{
  return SbVec3d( sin(heading*M_PI/180.0), cos(heading*M_PI/180.0), 0.0);
}

float 
SmVesselKitP::getWaveSlope(SoGLRenderAction * action, SmOceanKit * ok, SbVec3d & pos, float heading, float length, float & avgElevation)
{
  double cx, cy, cz;
  UTMElement::getReferencePosition(action->getState(), cx, cy, cz);
  SbVec3d campos(cx, cy, cz);
  SbVec3d p0 = pos - campos;
  SbVec3d v = this->heading2SbVec3d(heading);
  SbVec3d v1 = v * length / - 2.0;
  SbVec3d p1 = p0 + v1;
  float p1e = ok->getElevation(p1[0], p1[1]) / 2.0;  // FIXME: workaround for SmOceanKit bug? preng 2006-03-13
  SbVec3d v2 = v * length / 2.0;
  SbVec3d p2 = p0 + v2;
  float p2e = ok->getElevation(p2[0], p2[1]) / 2.0;  // FIXME: workaround for SmOceanKit bug? preng 2006-03-13
  float elevdiff = p2e - p1e;
  float slopeangle = atan(elevdiff/length) * 180.0 / M_PI;
  avgElevation = (p1e+p2e)/2.0;
  return slopeangle;
}

SbVec2f
SmVesselKitP::getTranslation(float heading, float speed, float dt)
{
  SbVec3d v = this->heading2SbVec3d(heading);
  v *= speed * 1.852 / 3.6;  // From knots to km/h to m/s
  v *= dt;
  return SbVec2f(v[0], v[1]);
}


#endif // temporary compile fix
