/**************************************************************************\
 *
 *  Copyright (C) 1998-2004 by Systems in Motion. All rights reserved.
 *
\**************************************************************************/

#include "SmDynamicObjectKit.h"

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

// Application must set this cb (for relative elevation to work)
static dok_elevation_cb_type dok_elevation_cb = NULL;

class SmDynamicObjectKitP {
public:
  SbBool needupdate;
  
  SoFieldSensor * relativeElevationSensor;
  SoFieldSensor * relativePositionSensor;
  SoFieldSensor * positionSensor;
  SoFieldSensor * headingSensor;
  SoFieldSensor * pitchSensor;
  SoFieldSensor * rollSensor;
};


// convenience define to access private data
#define PRIVATE(obj) (obj)->pimpl

SO_KIT_SOURCE(SmDynamicObjectKit);

/*!
  Constructor. 
*/
SmDynamicObjectKit::SmDynamicObjectKit(void) 
{
  PRIVATE(this) = new SmDynamicObjectKitP;
  PRIVATE(this)->needupdate = FALSE;

  SO_KIT_CONSTRUCTOR(SmDynamicObjectKit);
  
  SO_KIT_ADD_FIELD(isThreadSafe, (FALSE));
  SO_KIT_ADD_FIELD(hasRelativeElevation, (FALSE));
  SO_KIT_ADD_FIELD(hasRelativePosition, (FALSE));
  SO_KIT_ADD_FIELD(position, (0.0, 0.0, 0.0));
  SO_KIT_ADD_FIELD(heading, (0.0));
  SO_KIT_ADD_FIELD(pitch, (0.0));
  SO_KIT_ADD_FIELD(roll, (0.0));
  SO_KIT_ADD_FIELD(objectName, (""));
  SO_KIT_ADD_FIELD(objectId, (""));
  SO_KIT_ADD_FIELD(cameraOffset, (0.0, 0.0, 0.0));
  SO_KIT_ADD_FIELD(cameraRotation, (SbRotation(SbVec3f(1.0, 0.0, 0.0), 0.0)));
  
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(utmPosition, UTMPosition, TRUE, topSeparator, relativePosition, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(relativePosition, SoTranslation, TRUE, topSeparator, rotation, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(rotation, SoRotation, TRUE, topSeparator, geometry, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(geometry, SoSwitch, TRUE, topSeparator, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(shape, SoSeparator, TRUE, geometry, fileShape, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(fileShape, SoSeparator, TRUE, geometry, childList, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(target, SoInfo, TRUE, fileShape, stdRotation, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(stdRotation, SoRotation, FALSE, fileShape, modelRotation, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(modelRotation, SoRotation, TRUE, fileShape, scale, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(scale, SoScale, TRUE, fileShape, offset, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(offset, SoTranslation, TRUE, fileShape, file, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(file, AutoFile, TRUE, fileShape, "", TRUE);
  SO_KIT_ADD_CATALOG_LIST_ENTRY(childList, SoSeparator, TRUE, geometry, "", SmDynamicObjectKit, TRUE);
  
  SO_KIT_INIT_INSTANCE();
  
  PRIVATE(this)->relativeElevationSensor = new SoFieldSensor(field_change_cb, this);
  PRIVATE(this)->relativeElevationSensor->setPriority(0);
  PRIVATE(this)->relativeElevationSensor->attach(&this->hasRelativeElevation);

  PRIVATE(this)->relativePositionSensor = new SoFieldSensor(field_change_cb, this);
  PRIVATE(this)->relativePositionSensor->setPriority(0);
  PRIVATE(this)->relativePositionSensor->attach(&this->hasRelativePosition);
  
  PRIVATE(this)->positionSensor = new SoFieldSensor(field_change_cb, this);
  PRIVATE(this)->positionSensor->setPriority(0);
  PRIVATE(this)->positionSensor->attach(&this->position);

  PRIVATE(this)->headingSensor = new SoFieldSensor(field_change_cb, this);
  PRIVATE(this)->headingSensor->setPriority(0);
  PRIVATE(this)->headingSensor->attach(&this->heading);

  PRIVATE(this)->pitchSensor = new SoFieldSensor(field_change_cb, this);
  PRIVATE(this)->pitchSensor->setPriority(0);
  PRIVATE(this)->pitchSensor->attach(&this->pitch);

  PRIVATE(this)->rollSensor = new SoFieldSensor(field_change_cb, this);
  PRIVATE(this)->rollSensor->setPriority(0);
  PRIVATE(this)->rollSensor->attach(&this->roll);
  
  this->setGeometryVisibility(TRUE);
  SoRotation * rn = new SoRotation;
  rn->rotation.setValue(SbVec3f(1,0,0), M_PI/2.0);
  this->setAnyPart("stdRotation", rn);
}

void
SmDynamicObjectKit::field_change_cb(void * closure, SoSensor *)
{
  SmDynamicObjectKit * thisp = (SmDynamicObjectKit*) closure;
  PRIVATE(thisp)->needupdate = TRUE;
}

/*!
  Destructor
*/
SmDynamicObjectKit::~SmDynamicObjectKit()
{
  PRIVATE(this)->relativeElevationSensor->detach();
  delete PRIVATE(this)->relativeElevationSensor;
  PRIVATE(this)->relativePositionSensor->detach();
  delete PRIVATE(this)->relativePositionSensor;
  PRIVATE(this)->positionSensor->detach();
  delete PRIVATE(this)->positionSensor;
  PRIVATE(this)->headingSensor->detach();
  delete PRIVATE(this)->headingSensor;
  PRIVATE(this)->pitchSensor->detach();
  delete PRIVATE(this)->pitchSensor;
  PRIVATE(this)->rollSensor->detach();
  delete PRIVATE(this)->rollSensor;
  delete PRIVATE(this);
}

// Documented in superclass
void
SmDynamicObjectKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SmDynamicObjectKit, SoBaseKit, "BaseKit");
  }
}

/*!
  Init callback function used to get terrain elevation when 
  object hasRelativeElevation.
  
  This method need only be called once.
*/
void 
SmDynamicObjectKit::setElevationCallback(dok_elevation_cb_type cbfunc)
{
  assert(cbfunc);
  dok_elevation_cb = cbfunc;
}

/*!
  Reset the kit. All elements and nodes will be removed.
*/
void 
SmDynamicObjectKit::reset(void)
{
  // just overwrite with new, empty nodes. The old ones will be deleted
  this->setAnyPart("utmposition", NULL, TRUE);
  this->setAnyPart("position", NULL, TRUE);
  this->setAnyPart("rotation", NULL, TRUE);
  this->setAnyPart("file", NULL, TRUE);
  this->setAnyPart("childList", NULL, TRUE);
  SoRotation * rn = new SoRotation;
  rn->rotation.setValue(SbVec3f(1,0,0), M_PI/2.0);
  this->setAnyPart("stdRotation", rn);
  this->setGeometryVisibility(TRUE);
  
  PRIVATE(this)->needupdate = FALSE;
}

// doc in parent
void 
SmDynamicObjectKit::getBoundingBox(SoGetBoundingBoxAction * action)
{
  this->updateScene();
  inherited::getBoundingBox(action);
}

/*!
  Method needed for thread safe rendering. If multiple threads are used to
  render a scene graph containing this nodekit, you must set the threadSafe
  field to TRUE, and use an SoCallbackAction to call this method before 
  rendering the scene graph.
*/
void
SmDynamicObjectKit::preRender(SoAction * action)
{
  this->updateScene();
}

// doc in parent
void 
SmDynamicObjectKit::GLRender(SoGLRenderAction * action)
{
  // if (!this->isThreadSafe.getValue()) this->preRender(action);
  updateScene();
  inherited::GLRender(action);
}

void 
SmDynamicObjectKit::updateScene(void)
{
  SbVec3d thispos = this->position.getValue();
  if (dok_elevation_cb) {
    if (this->hasRelativeElevation.getValue()) {
      float elevation = thispos[2];
      float terrain_elev;
      if (dok_elevation_cb(thispos[0], thispos[1], terrain_elev)) {
        thispos[2] = elevation + terrain_elev;
        PRIVATE(this)->needupdate = TRUE;
      }
    }
  }
  if (!PRIVATE(this)->needupdate) return;
  if (this->hasRelativePosition.getValue()) {
    // Relative position, use relativePosition part
    this->setAnyPart("utmPosition", NULL, TRUE);
    SoTranslation * pos = (SoTranslation *)this->getAnyPart("relativePosition", TRUE);
    assert(pos);
    SbVec3f posvec;
    posvec[0] = thispos[0];
    posvec[1] = thispos[1];
    posvec[2] = thispos[2];
    pos->translation.setValue(posvec);
  }
  else {
    // Absolute position, use utmPosition part
    this->setAnyPart("relativePosition", NULL, TRUE);
    UTMPosition * pos = (UTMPosition *)this->getAnyPart("utmPosition", TRUE);
    assert(pos);
    pos->utmposition.setValue(thispos);
  }
  // Set rotation from heading, pitch, roll fields
  SbRotation h(SbVec3f(0,0,1), -1 * (M_PI * this->heading.getValue() / 180.0));
  SbRotation p(SbVec3f(1,0,0), (M_PI * this->pitch.getValue() / 180.0));
  SbRotation r(SbVec3f(0,1,0), (M_PI * this->roll.getValue() / 180.0));
  SbRotation newrot = r*p*h;
  SoRotation * rot = (SoRotation *)this->getAnyPart("rotation", TRUE);
  assert(rot);
  rot->rotation.setValue(newrot);
  PRIVATE(this)->needupdate = FALSE;
}


/*!
  Set the 'orientation' part.
 */
void 
SmDynamicObjectKit::setOrientation(float heading, float pitch, float roll)
{
  this->heading.setValue(heading);
  this->pitch.setValue(pitch);
  this->roll.setValue(roll);
}

/*!
  Hide or show file geometry and all children (see 'childList').
 */
void 
SmDynamicObjectKit::setGeometryVisibility(SbBool visibility)
{
  SoSwitch * geo = (SoSwitch*)this->getAnyPart("geometry", TRUE);
  assert(geo);
  if (visibility)
    geo->whichChild.setValue(SO_SWITCH_ALL);
  else
    geo->whichChild.setValue(SO_SWITCH_NONE);
}

/*!
 */
SbBool 
SmDynamicObjectKit::getGeometryVisibility(void)
{
  SoSwitch * geo = (SoSwitch*)this->getAnyPart("geometry", FALSE);
  if (!geo)
    return FALSE;
  else 
    return geo->whichChild.getValue() == SO_SWITCH_ALL ? TRUE : FALSE;
}

/*!
  Find nodekit (this or one of it's descendants) by objectId.
  Only the first nodekit with matching objectId is returned.

  This mechanism (in a small way) duplicates the node name functionality
  in OI, for two reasons:
  - cannot limit id to node name acceptable by Coin, and
  - cannot limit id to not duplicate node name already used elsewhere
    in the scene graph.
*/
SmDynamicObjectKit *
SmDynamicObjectKit::getObjectByObjectId(const SbName objectId)
{
  if (objectId == this->objectId.getValue())
    return this;
  SmDynamicObjectKit * found = NULL;
  SoNodeKitListPart * children = (SoNodeKitListPart *)this->getAnyPart("childList", FALSE);
  if (children) {
    for (int i=0; i<children->getNumChildren() && !found; i++) {
      found = ((SmDynamicObjectKit *)(children->getChild(i)))->getObjectByObjectId(objectId);
    }
  }
  return found;
}

/*!  
  \param newObject Pointer to object to be added
  \param parentId ObjectId of desired parent object
  \return Pointer to parent if successful (ie parent was found) otherwise NULL

  Add a new object to the object hierarchy. \a parentId controls
  where in the object hierarchy the new object is inserted. Note that
  "" is default objectId for new SmDynamicObjectKit instances; calling
  this method on the root object with \a parentId = "" means new
  object will be added directly under the root object (since root object
  always has default id).
  
  \a newObject is only added once, as a child of the first found object matching
  \a parentId.
  
  If no existing nodekit has objectId == \a parentId, \a newObject is
  not added to the node hierarchy, and NULL is returned.

  \sa getObjectByObjectId
*/
SmDynamicObjectKit *
SmDynamicObjectKit::addObject(SmDynamicObjectKit * newObject, const SbName parentId)
{
  assert(newObject);
  assert(newObject->objectId.getValue() != SbName("") && "All objects must have an id");
  if (parentId == this->objectId.getValue()) {
    SoNodeKitListPart * children = SO_GET_PART(this, "childList", SoNodeKitListPart);
    assert(children);
    const char * type_name = children->getTypeId().getName().getString();
    children->addChild(newObject);
    if (parentId != SbName(""))
      newObject->hasRelativePosition.setValue(TRUE);
    return this;
  }
  else {
    SmDynamicObjectKit * inserted = NULL;
    SoNodeKitListPart * children = SO_GET_PART(this, "childList", SoNodeKitListPart);
    if (children) {
      for (int i=0; i<children->getNumChildren() && !inserted; i++) {
        inserted = ((SmDynamicObjectKit *)(children->getChild(i)))->addObject(newObject, parentId);
      }
    }
    return inserted;
  }
}

/*!
  \param objectId ObjectId of object to be removed
  
  Returns pointer to removed object, or NULL if no object with
  \a objectId was found.
*/
SmDynamicObjectKit * 
SmDynamicObjectKit::removeObject(const SbName objectId)
{
  assert(objectId != SbName("") && "You may not remove the root node");

  SmDynamicObjectKit * removed = NULL, * child;
  SoNodeKitListPart * children = SO_GET_PART(this, "childList", SoNodeKitListPart);
  if (children) {
    for (int i=0; i<children->getNumChildren() && !removed; i++) {
      child = (SmDynamicObjectKit *)(children->getChild(i));
      if (objectId == child->objectId.getValue()) {
        child->ref();
        children->removeChild(i);
        child->unrefNoDelete();
        removed = child;
      }
      else {
        removed = child->removeObject(objectId);
      }
    }
  }
  return removed;
}

