#ifndef SM_ANNOTATION_AXIS_H
#define SM_ANNOTATION_AXIS_H

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

class SmAnnotationAxisP;

#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFBool.h>
#include <SmallChange/basic.h>

class SoSensor;

class SMALLCHANGE_DLL_API SmAnnotationAxis : public SoBaseKit {
  
  typedef SoBaseKit inherited;
  SO_KIT_HEADER(SmAnnotationAxis);
  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(text);
  SO_KIT_CATALOG_ENTRY_HEADER(axisSwitch);
  SO_KIT_CATALOG_ENTRY_HEADER(noAxis);
  SO_KIT_CATALOG_ENTRY_HEADER(axisSep);
  SO_KIT_CATALOG_ENTRY_HEADER(axisMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(axisLineSet);
  
public:

  SmAnnotationAxis(void);
  static void initClass(void);

  SoMFString annotation;
  SoMFVec3f annotationPos;
  SoSFFloat annotationGap;
  SoSFVec3f annotationOffset;

  SoSFBool renderAxis;
  SoSFVec3f axisTickSize;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  
protected:  
  virtual void notify(SoNotList * list);
  virtual ~SmAnnotationAxis();

private:
  static void regen_geometry(void * userdata, SoSensor * s);
  SmAnnotationAxisP * pimpl;
};

#endif // SM_ANNOTATION_AXIS_H

