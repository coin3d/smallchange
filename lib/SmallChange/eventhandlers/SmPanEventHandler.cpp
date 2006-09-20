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


/*!
  \class SmPanEventHandler SmPanEventHandler.h
  \brief Google Earth type navigation mode
  \ingroup eventhandlers

  FIXME: doc
*/


#include "SmPanEventHandler.h"
#include <SmallChange/nodes/UTMCamera.h>
#include <Inventor/SbLine.h>
#include <Inventor/SbPlane.h>
#include <Inventor/projectors/SbSphereSheetProjector.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/events/SoEvent.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMotion3Event.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/events/SoButtonEvent.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>

class SmPanEventHandlerP {
public:
  SbVec2f lastmouseposition;
  SbBool dragenabled;
  SbBool spinenabled;
  SbBool zoomenabled;
  SbSphereSheetProjector * spinprojector;
};

#define PRIVATE(obj) (obj)->pimpl
#define PUBLIC(obj) (obj)->master

SO_NODE_SOURCE(SmPanEventHandler);

void 
SmPanEventHandler::initClass(void)
{
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    SO_NODE_INIT_CLASS(SmPanEventHandler, SmEventHandler, "SmEventHandler");
  }
}

SmPanEventHandler::SmPanEventHandler(void)
{
  SO_NODE_CONSTRUCTOR(SmPanEventHandler);
  SO_NODE_ADD_FIELD(zoomSpeed, (0.2f));

  PRIVATE(this) = new SmPanEventHandlerP;

  PRIVATE(this)->dragenabled = FALSE;
  PRIVATE(this)->spinenabled = FALSE;
  PRIVATE(this)->zoomenabled = FALSE;
  PRIVATE(this)->spinprojector = new SbSphereSheetProjector(SbSphere(SbVec3f(0, 0, 0), 0.8f));
  SbViewVolume volume;
  volume.ortho(-1, 1, -1, 1, -1, 1);
  PRIVATE(this)->spinprojector->setViewVolume(volume);
}


SmPanEventHandler::~SmPanEventHandler()
{
  delete PRIVATE(this)->spinprojector;
  delete PRIVATE(this);
}

void 
SmPanEventHandler::handleEvent(SoHandleEventAction * action)
{
  const SoEvent * ev = action->getEvent();
  const SoType type(ev->getTypeId());

  // Mouse Button handling
  if (type.isDerivedFrom(SoMouseButtonEvent::getClassTypeId())) {
    const SoMouseButtonEvent * const event = (const SoMouseButtonEvent *) ev;
    const int button = event->getButton();
    const SbBool press = event->getState() == SoButtonEvent::DOWN ? TRUE : FALSE;

    switch (button) {
    case SoMouseButtonEvent::BUTTON1:
      PRIVATE(this)->dragenabled = press;
      break;
    case SoMouseButtonEvent::BUTTON2:
      PRIVATE(this)->spinenabled = press;
      break;
    case SoMouseButtonEvent::BUTTON3:
      PRIVATE(this)->zoomenabled = press;
      break;
    case SoMouseButtonEvent::BUTTON4:
      this->zoom(1.0f + this->zoomSpeed.getValue());
      break;
    case SoMouseButtonEvent::BUTTON5:
      this->zoom(1.0f - this->zoomSpeed.getValue());
      break;
    default:
      break;
    }
  }

  // Mouse Movement handling
  if (type.isDerivedFrom(SoLocation2Event::getClassTypeId())) {
    const SoLocation2Event * const event = (const SoLocation2Event *) ev;

    const SbVec2f prevnormalized = PRIVATE(this)->lastmouseposition;
    const SbVec2s size(this->getGLSize());
    const SbVec2s pos(event->getPosition());
    const SbVec2f posn((float) pos[0] / (float) SbMax((int)(size[0] - 1), 1),
                       (float) pos[1] / (float) SbMax((int)(size[1] - 1), 1));
    
    PRIVATE(this)->lastmouseposition = posn;

    if (PRIVATE(this)->dragenabled)
      this->pan(posn, prevnormalized);

    if (PRIVATE(this)->spinenabled) {
      this->spin(posn, prevnormalized);
    }      
    if (PRIVATE(this)->zoomenabled) {
      this->zoom(posn, prevnormalized);
    }      
  }
}

void
SmPanEventHandler::zoom(const SbVec2f & currpos,
                        const SbVec2f & prevpos)
{
  float delta = float(exp((currpos[1] - prevpos[1]) * 20.0f));
  this->zoom(delta);
}

void
SmPanEventHandler::zoom(const float delta)
{
  SoCamera * cam = this->getCamera();
  if (!cam->isOfType(UTMCamera::getClassTypeId())) {
    return;
  }

  UTMCamera * utm = (UTMCamera*) cam;
  SbVec3d utmpos = utm->utmposition.getValue();
  
  const float oldfocaldist = cam->focalDistance.getValue();
  const float newfocaldist = oldfocaldist * delta;

  SbVec3f direction;
  cam->orientation.getValue().multVec(SbVec3f(0, 0, -1), direction);

  const SbVec3f oldpos = cam->position.getValue();
  const SbVec3f newpos = oldpos + (newfocaldist - oldfocaldist) * -direction;

  SbVec3f val(newpos-oldpos);
  utm->utmposition = utmpos + SbVec3d(val[0], val[1], val[2]);
  cam->focalDistance = newfocaldist;
}


void 
SmPanEventHandler::spin(const SbVec2f & currpos,
                        const SbVec2f & prevpos)
{
  SoCamera * cam = this->getCamera();
  if (!cam->isOfType(UTMCamera::getClassTypeId())) {
    return;
  }

  SbVec3f to = PRIVATE(this)->spinprojector->project(prevpos);
  SbVec3f from = PRIVATE(this)->spinprojector->project(currpos);

  SbRotation rot = PRIVATE(this)->spinprojector->getRotation(from, to);

  SbVec3f axis;
  float radians;
  rot.getValue(axis, radians);

  UTMCamera * utm = (UTMCamera*) cam;
  SbVec3d utmpos = utm->utmposition.getValue();

  if (cam == NULL) return;
      
  // Find global coordinates of focal point.
  SbVec3f direction;
  cam->orientation.getValue().multVec(SbVec3f(0, 0, -1), direction);
  SbVec3f focalpoint = cam->position.getValue() +
    cam->focalDistance.getValue() * direction;
      
  // Set new orientation value by accumulating the new rotation.
  cam->orientation = rot * cam->orientation.getValue();
      
  // Reposition camera so we are still pointing at the same old focal point.
  cam->orientation.getValue().multVec(SbVec3f(0, 0, -1), direction);
  SbVec3f val((focalpoint-cam->focalDistance.getValue()*direction)-cam->position.getValue());
  utm->utmposition = utmpos + SbVec3d(val[0], val[1], val[2]);
}


void
SmPanEventHandler::pan(const SbVec2f & currpos, 
                       const SbVec2f & prevpos)
{
  if (currpos == prevpos) return; // useless invocation

  // Find projection points for the last and current mouse coordinates.
  SoCamera * cam = this->getCamera();
  if (!cam->isOfType(UTMCamera::getClassTypeId())) {
    return;
  }

  UTMCamera * utm = (UTMCamera*) cam;
  SbVec3d utmpos = utm->utmposition.getValue();

  SbViewVolume vv = cam->getViewVolume(this->getGLAspectRatio());
  SbLine line;
  vv.projectPointToLine(currpos, line);
  SbVec3f current_planept;
  
  SbPlane panningplane = SbPlane(SbVec3f(0, 0, 1), utmpos[2]);
  panningplane.intersect(line, current_planept);
  vv.projectPointToLine(prevpos, line);
  SbVec3f old_planept;
  panningplane.intersect(line, old_planept);
  
  // Reposition camera according to the vector difference between the
  // projected points.
  SbVec3f val(-(old_planept-current_planept));
  utm->utmposition = utmpos + SbVec3d(val[0], val[1], val[2]);
}

#undef PRIVATE
#undef PUBLIC
