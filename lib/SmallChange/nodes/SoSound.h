#ifndef SMALLCHANGE_SOSOUND_H
#define SMALLCHANGE_SOSOUND_H

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use SmallChange with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFNode.h>
#include <Inventor/fields/SoSFBool.h>

#include <SmallChange/actions/SoAudioRenderAction.h>

class SoTimerSensor;

class SbAsyncBuffer;

class SoSound : 
  public SoNode
{
  typedef SoNode inherited;
  SO_NODE_HEADER(SoSound);

  friend class SoAudioRenderAction;

public:

  static void initClass();
  SoSound();

  SoSFVec3f  direction;
  SoSFFloat  intensity;
  SoSFVec3f  location;
  SoSFFloat  maxBack;
  SoSFFloat  maxFront;
  SoSFFloat  minBack;
  SoSFFloat  minFront;
  SoSFFloat  priority;
  SoSFNode   source;
  SoSFBool   spatialize;

protected:

  virtual void audioRender(SoAudioRenderAction *action);

protected:
  class SoSoundP *sosound_impl;
  friend class SoSoundP;

private:

  virtual ~SoSound();

};

#endif // !SMALLCHANGE_SOSOUND_H
