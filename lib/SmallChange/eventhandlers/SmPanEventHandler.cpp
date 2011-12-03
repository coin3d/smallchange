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

#include "../misc/cameracontrol.h"

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
  if (cam == NULL) return;

  SbVec2f dp = currpos - prevpos;
  cam_spin(cam, dp, this->kit->viewUp.getValue());
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
  
  SbPlane panningplane = SbPlane(SbVec3f(0, 0, 1), float(utmpos[2]));
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
