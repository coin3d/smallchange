/**************************************************************************\
 *
 *  Copyright (C) 1998-2004 by Systems in Motion. All rights reserved.
 *
\**************************************************************************/

#ifndef DYNAMIC_OBJECT_H
#define DYNAMIC_OBJECT_H

#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFName.h>
#include <Inventor/fields/SoSFVec3d.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFRotation.h>
#include <Inventor/C/basic.h>
#include <Inventor/SbVec3f.h>

// Application must set this cb 
typedef int (*dok_elevation_cb_type)(double easting, double northing, float &elevation);

class DynamicObjectKitP;
class SoSensor;

class DynamicObjectKit : public SoBaseKit {
  typedef SoBaseKit inherited;
  
  SO_KIT_HEADER(DynamicObjectKit);
  
  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(utmPosition);
  SO_KIT_CATALOG_ENTRY_HEADER(relativePosition);
  SO_KIT_CATALOG_ENTRY_HEADER(rotation);
  SO_KIT_CATALOG_ENTRY_HEADER(geometry);
  SO_KIT_CATALOG_ENTRY_HEADER(shape);
  SO_KIT_CATALOG_ENTRY_HEADER(stdRotation);
  SO_KIT_CATALOG_ENTRY_HEADER(target);
  SO_KIT_CATALOG_ENTRY_HEADER(modelRotation);
  SO_KIT_CATALOG_ENTRY_HEADER(scale);
  SO_KIT_CATALOG_ENTRY_HEADER(offset);
  SO_KIT_CATALOG_ENTRY_HEADER(file);
  SO_KIT_CATALOG_ENTRY_HEADER(childList);
  
public:
  DynamicObjectKit(void);
  
  SoSFBool isThreadSafe;
  SoSFBool hasRelativeElevation;
  SoSFBool hasRelativePosition;
  SoSFVec3d position;
  SoSFFloat heading;
  SoSFFloat pitch;
  SoSFFloat roll;
  SoSFString objectName;
  SoSFName objectId;
  SoSFVec3f cameraOffset;
  SoSFRotation cameraRotation;
  
  static void initClass(void);
  static void setElevationCallback(dok_elevation_cb_type cbfunc);
  
  void setOrientation(float heading, float pitch, float roll);
  void setGeometryVisibility(SbBool visibility);
  SbBool getGeometryVisibility(void);
  DynamicObjectKit * getObjectByObjectId(const SbName objectId);
  DynamicObjectKit * addObject(DynamicObjectKit * newObject, const SbName parentId);
  DynamicObjectKit * removeObject(const SbName objectId);
  
protected:
  virtual ~DynamicObjectKit();

public:
  
  void preRender(SoAction * action);

  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void GLRender(SoGLRenderAction * action);

  void reset(void);

private:
  
  static void field_change_cb(void * closure, SoSensor *);

  void updateScene(void);

  DynamicObjectKitP * pimpl;

};

#endif // !DYNAMIC_OBJECT_H
