/**************************************************************************\
 *
 *  Copyright (C) 1998-2004 by Systems in Motion. All rights reserved.
 *
\**************************************************************************/

#include "SmVesselKit.h"

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
#include <Inventor/C/basic.h>
#include <Inventor/SbBSPTree.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <SmallChange/nodes/UTMPosition.h>
#include <SmallChange/nodes/AutoFile.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodekits/SoNodeKitListPart.h>


class SmVesselKitP {
public:
};


// convenience define to access private data
#define PRIVATE(obj) (obj)->pimpl

SO_KIT_SOURCE(SmVesselKit);

/*!
  Constructor. 
*/
SmVesselKit::SmVesselKit(void) 
{
  PRIVATE(this) = new SmVesselKitP;

  SO_KIT_CONSTRUCTOR(SmVesselKit);
  
  SO_KIT_ADD_FIELD(oceanKit, (NULL)); 
  SO_KIT_ADD_FIELD(size, (40.0, 10.0));
  SO_KIT_ADD_FIELD(pitchInertia, (1.0));
  SO_KIT_ADD_FIELD(pitchResistance, (1.0));
  SO_KIT_ADD_FIELD(pitchBalance, (1.0));
  SO_KIT_ADD_FIELD(rollInertia, (1.0));
  SO_KIT_ADD_FIELD(rollResistance, (1.0));
  SO_KIT_ADD_FIELD(rollBalance, (1.0));
  
  SO_KIT_INIT_INSTANCE();
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

// doc in parent
void 
SmVesselKit::GLRender(SoGLRenderAction * action)
{
  // TODO: update inherited pitch and roll fields based on oceanKit geometry
  inherited::GLRender(action);
}
