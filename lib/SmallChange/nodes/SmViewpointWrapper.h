#ifndef SM_VIEWPOINTWRAPPER_H
#define SM_VIEWPOINTWRAPPER_H

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

#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/SoLists.h>
#include <SmallChange/basic.h>

class SoPath;
class SoFullPath;
class SoPathSensor;
class SoSensor;
class SoFieldSensor;
class SoGetMatrixAction;

class SMALLCHANGE_DLL_API SmViewpointWrapper : public SoPerspectiveCamera {
  typedef SoPerspectiveCamera inherited;
  SO_NODE_HEADER(SmViewpointWrapper);
public:
  static void initClass(void);
  SmViewpointWrapper(void);
  
  void setSceneGraph(SoNode * root);
  
  static SbBool hasViewpoints(SoNode * root);

protected:
  ~SmViewpointWrapper();
  
private:
  void setViewpoint(SoPath * path);
  
  static void fieldsensor_cb(void * data, SoSensor * sensor);
  static void pathsensor_cb(void * data, SoSensor * sensor);
  static void set_bind_cb(void * data, SoSensor * sensor);

  void updateCamera(void);
  void updateViewpoint(void);
  
  void attachFieldSensors(void);
  void detachFieldSensors(void);
  

  void truncateLists(void);
  
  SoNodeList nodelist;
  SbPList set_bind_sensorlist;
  SoSearchAction sa;
  SoNode * scenegraph;
  SoFullPath * pathtoviewpoint;
  SoPathSensor * pathsensor;
  SoFieldSensor * positionsensor;
  SoFieldSensor * orientationsensor;
  SoFieldSensor * heightanglesensor;
  SoGetMatrixAction * gmaction;

  void attachSetBindSensors(void);
  void detachSetBindSensors(void);

  void sendBindEvents(SoNode * node, const SbBool onoff);
  void bindTopOfStack(void);

};

#endif //  SM_VIEWPOINTWRAPPER_H
