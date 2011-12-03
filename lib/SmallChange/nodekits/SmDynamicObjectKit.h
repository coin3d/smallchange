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
#include <SmallChange/basic.h>

// Application must set this cb 
typedef int (*dok_elevation_cb_type)(double easting, double northing, float &elevation);

class SmDynamicObjectKitP;
class SoSensor;

class SMALLCHANGE_DLL_API SmDynamicObjectKit : public SoBaseKit {
  typedef SoBaseKit inherited;
  
  SO_KIT_HEADER(SmDynamicObjectKit);
  
  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(utmPosition);
  SO_KIT_CATALOG_ENTRY_HEADER(relativePosition);
  SO_KIT_CATALOG_ENTRY_HEADER(rotation);
  SO_KIT_CATALOG_ENTRY_HEADER(geometry);
  SO_KIT_CATALOG_ENTRY_HEADER(shape);
  SO_KIT_CATALOG_ENTRY_HEADER(fileShape);
  SO_KIT_CATALOG_ENTRY_HEADER(stdRotation);
  SO_KIT_CATALOG_ENTRY_HEADER(target);
  SO_KIT_CATALOG_ENTRY_HEADER(modelRotation);
  SO_KIT_CATALOG_ENTRY_HEADER(scale);
  SO_KIT_CATALOG_ENTRY_HEADER(offset);
  SO_KIT_CATALOG_ENTRY_HEADER(file);
  SO_KIT_CATALOG_ENTRY_HEADER(childList);
  
public:
  SmDynamicObjectKit(void);
  
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
  SmDynamicObjectKit * getObjectByObjectId(const SbName objectId);
  SmDynamicObjectKit * addObject(SmDynamicObjectKit * newObject, const SbName parentId);
  SmDynamicObjectKit * removeObject(const SbName objectId);
  
protected:
  virtual ~SmDynamicObjectKit();

public:
  
  void preRender(SoAction * action);

  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  void reset(void);

private:
  
  static void field_change_cb(void * closure, SoSensor *);
  void updateScene(void);
  SmDynamicObjectKitP * pimpl;

};

#endif // !DYNAMIC_OBJECT_H
