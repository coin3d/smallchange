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

#include "SmViewpointWrapper.h"
#include <Inventor/SoPath.h>
#include <Inventor/sensors/SoPathSensor.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/fields/SoSFBool.h>        
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/VRMLnodes/SoVRMLViewpoint.h>
#include <assert.h>

// FIXME: make a generic class to handle bindable nodes, pederb 2004-09-28

void
SmViewpointWrapper::initClass(void)
{
  SO_NODE_INIT_CLASS(SmViewpointWrapper, SoPerspectiveCamera, "PerspectiveCamera");
}

SO_NODE_SOURCE(SmViewpointWrapper);

SmViewpointWrapper::SmViewpointWrapper(void)
{
  SO_NODE_CONSTRUCTOR(SmViewpointWrapper);
  this->scenegraph = NULL;
  this->pathtoviewpoint = NULL;
  this->positionsensor = new SoFieldSensor(fieldsensor_cb, this);
  this->orientationsensor = new SoFieldSensor(fieldsensor_cb, this);
  this->heightanglesensor = new SoFieldSensor(fieldsensor_cb, this);
  this->pathsensor = new SoPathSensor(pathsensor_cb, this);
  this->attachFieldSensors();
  this->gmaction = new SoGetMatrixAction(SbViewportRegion(100, 100));
}

SmViewpointWrapper::~SmViewpointWrapper(void)
{
  this->pathsensor->detach();
  delete this->pathsensor;
  if (this->pathtoviewpoint) this->pathtoviewpoint->unref();
  this->detachFieldSensors();
  delete this->positionsensor;
  delete this->orientationsensor;
  delete this->heightanglesensor;
  delete this->gmaction;
  if (this->scenegraph) this->scenegraph->unref();
}

void 
SmViewpointWrapper::sendBindEvents(SoNode * node, const SbBool onoff)
{
  SoSFBool * isBound = (SoSFBool*) node->getField("isBound");
  SoSFTime * bindTime = (SoSFTime*) node->getField("bindTime");
  if (isBound && bindTime) {
    isBound->setValue(onoff);
    bindTime->setValue(SbTime::getTimeOfDay());
  }
}

void 
SmViewpointWrapper::setViewpoint(SoPath * path)
{
  if (this->pathtoviewpoint) {
    this->sendBindEvents(this->pathtoviewpoint->getTail(), FALSE);
    this->pathtoviewpoint->unref();
    this->pathsensor->detach();
  }
  this->pathtoviewpoint = (SoFullPath*) path;
  if (path) {
    path->ref();
    this->pathsensor->attach(path); 
    this->updateCamera();
    this->sendBindEvents(this->pathtoviewpoint->getTail(), TRUE);

    // calculate a focal distance. It's not possible to specify a
    // focal point in VRML97. We just set it to the center of the
    // scene.
    SbViewportRegion vp(640, 480);
    SoGetBoundingBoxAction bboxaction(vp);
    bboxaction.apply(path->getHead());
    SbVec3f center = bboxaction.getCenter();
    
    float dist = (this->position.getValue() - center).length();
    // avoid focalDistance ~= 0
    if (dist < 0.1f) dist = 0.1f;
    this->focalDistance = dist; 
  }
}

void 
SmViewpointWrapper::updateCamera(void)
{
  if (this->pathtoviewpoint == NULL) return;

  this->detachFieldSensors();

  SoVRMLViewpoint * vp = (SoVRMLViewpoint*)
    this->pathtoviewpoint->getTail();
  assert(vp->getTypeId() == SoVRMLViewpoint::getClassTypeId());
  
  this->gmaction->apply(this->pathtoviewpoint);
  SbVec3f pos = vp->position.getValue();
  float angle = vp->fieldOfView.getValue();

  SbRotation rot = vp->orientation.getValue();
  SbMatrix m;
  m.setRotate(rot);  
  this->gmaction->getMatrix().multVecMatrix(pos, pos);
  m.multRight(this->gmaction->getInverse());
  m.multLeft(this->gmaction->getMatrix());
  rot.setValue(m);
  
  this->position = pos;
  this->orientation = rot;
  this->heightAngle = angle;

  this->attachFieldSensors();
}

void
SmViewpointWrapper::updateViewpoint(void)
{
  if (this->pathtoviewpoint == NULL) return;
  this->pathsensor->detach();
  
  SoVRMLViewpoint * vp = (SoVRMLViewpoint*)
    this->pathtoviewpoint->getTail();
  assert(vp->getTypeId() == SoVRMLViewpoint::getClassTypeId());
  
  this->gmaction->apply(this->pathtoviewpoint);
  SbVec3f pos = this->position.getValue();
  float angle = this->heightAngle.getValue();

  SbRotation rot = this->orientation.getValue();
  SbMatrix m;
  m.setRotate(rot);  
  this->gmaction->getInverse().multVecMatrix(pos, pos);
  m.multRight(this->gmaction->getMatrix());
  m.multLeft(this->gmaction->getInverse());
  rot.setValue(m);
  
  vp->position = pos;
  vp->orientation = rot;
  vp->fieldOfView = angle;

  this->pathsensor->attach(this->pathtoviewpoint); 
}

void 
SmViewpointWrapper::attachFieldSensors(void)
{
  this->positionsensor->attach(&this->position);
  this->orientationsensor->attach(&this->orientation);
  this->heightanglesensor->attach(&this->heightAngle);
}

void 
SmViewpointWrapper::detachFieldSensors(void)
{
  this->positionsensor->detach();
  this->orientationsensor->detach();
  this->heightanglesensor->detach();
}

void 
SmViewpointWrapper::fieldsensor_cb(void * data, SoSensor * sensor)
{
  SmViewpointWrapper * thisp = (SmViewpointWrapper *) data;
  thisp->updateViewpoint();
}

void 
SmViewpointWrapper::pathsensor_cb(void * data, SoSensor * sensor)
{
  SmViewpointWrapper * thisp = (SmViewpointWrapper *) data;
  thisp->updateCamera();
}

void 
SmViewpointWrapper::set_bind_cb(void * data, SoSensor * sensor)
{
  SmViewpointWrapper * thisp = (SmViewpointWrapper *) data;
  int idx = thisp->set_bind_sensorlist.find(sensor);
  
  if (idx >= 0) {
    SoNode * node = thisp->nodelist[idx];
    node->ref();
    SoSFBool * set_bind = (SoSFBool*) node->getField(SbName("set_bind"));
    assert(set_bind);
    if (set_bind->getValue()) { // TRUE event
      // check if node is already bound
      if (thisp->pathtoviewpoint && thisp->pathtoviewpoint->getTail() == node) return;
      thisp->nodelist.remove(idx);
      thisp->nodelist.insert(node, 0);
      
      thisp->bindTopOfStack();
    }
    else if (idx == 0) { // FALSE event to top-of-stack
      thisp->nodelist.remove(0);
      thisp->nodelist.append(node); // place the node at the bottom of the stack

      thisp->bindTopOfStack();
    }
    node->unrefNoDelete();
  }
}

void
SmViewpointWrapper::bindTopOfStack(void)
{
  if (this->nodelist.getLength()) {
    this->sa.setNode(this->nodelist[0]);
    this->sa.setInterest(SoSearchAction::FIRST);
    this->sa.apply(this->scenegraph);
    if (this->sa.getPath()) {
      this->setViewpoint(this->sa.getPath());
    }
    this->sa.reset();
  }
}

SbBool 
SmViewpointWrapper::hasViewpoints(SoNode * root)
{
  SoSearchAction sa;
  sa.setInterest(SoSearchAction::ALL);
  sa.setType(SoVRMLViewpoint::getClassTypeId());
  sa.apply(root);
  return sa.getPaths().getLength() > 0;
}

void
SmViewpointWrapper::setSceneGraph(SoNode * root)
{
  this->truncateLists();
  this->setViewpoint(NULL);
  if (this->scenegraph) this->scenegraph->unref();
  this->scenegraph = root;
  if (this->scenegraph) {
    this->scenegraph->ref();
    this->sa.setInterest(SoSearchAction::ALL);
    this->sa.setType(SoVRMLViewpoint::getClassTypeId());
    this->sa.apply(root);
    SoPathList & pl = this->sa.getPaths();
    if (pl.getLength()) {
      for (int i = 0; i < pl.getLength(); i++) {
        SoFullPath * p = (SoFullPath*) pl[i];
        if (p->getTail()->isOfType(SoVRMLViewpoint::getClassTypeId())) {
          this->nodelist.append(p->getTail());
          this->set_bind_sensorlist.append(new SoFieldSensor(set_bind_cb, this));
        }
      }
      this->attachSetBindSensors();
      // bind the first Viewpoint
      SoSFBool * set_bind = (SoSFBool*) this->nodelist[0]->getField(SbName("set_bind"));
      assert(set_bind);
      set_bind->setValue(TRUE);
    }
    this->sa.reset();
  }
}

void 
SmViewpointWrapper::truncateLists(void)
{
  this->detachSetBindSensors();
  this->nodelist.truncate(0);
  for (int i = 0; i < this->set_bind_sensorlist.getLength(); i++) {
    SoFieldSensor * fs = (SoFieldSensor*) this->set_bind_sensorlist[i];
    delete fs;
  }
}

void 
SmViewpointWrapper::attachSetBindSensors(void)
{
  assert(this->nodelist.getLength() == this->set_bind_sensorlist.getLength());
  for (int i = 0; i < this->nodelist.getLength(); i++) {
    SoVRMLViewpoint * vp = (SoVRMLViewpoint*) this->nodelist[i];
    SoFieldSensor * fs = (SoFieldSensor*) this->set_bind_sensorlist[i];
    fs->attach(vp->getField(SbName("set_bind")));
  }
}

void 
SmViewpointWrapper::detachSetBindSensors(void)
{
  for (int i = 0; i < this->set_bind_sensorlist.getLength(); i++) {
    SoFieldSensor * fs = (SoFieldSensor*) this->set_bind_sensorlist[i];
    fs->detach();
  }
}

