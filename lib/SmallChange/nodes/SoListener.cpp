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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_OPENAL

#include <SmallChange/nodes/SoListener.h>

#include <SmallChange/actions/SoAudioRenderAction.h>

#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>

#include <AL/al.h>
#include <AL/altypes.h>

#include <SmallChange/misc/ALTools.h>

SO_NODE_SOURCE(SoListener);

void SoListener::initClass()
{
  SO_NODE_INIT_CLASS(SoListener, SoNode, "Node");
};

SoListener::SoListener()
{
  SO_NODE_CONSTRUCTOR(SoListener);
  SO_NODE_ADD_FIELD(position, (0.0f, 0.0f, 1.0f));
  SO_NODE_ADD_FIELD(orientation, (SbRotation(SbVec3f(0.0f, 0.0f, 1.0f), 0.0f)));
  SO_NODE_ADD_FIELD(velocity, (0.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(gain, (1.0f));

};

SoListener::~SoListener()
{
};

void SoListener::audioRender(SoAudioRenderAction *action)
{
  ALint error;
  SbVec3f pos, worldpos;
  SbVec3f viewdir;
  SbVec3f viewup;
  ALfloat alfloat3[3];
  ALfloat alfloat6[6];

  pos = position.getValue();
  SoModelMatrixElement::get(action->getState()).multVecMatrix(pos, worldpos); 

#if 0
  // 20011206 thammer, kept for debugging purposes
//  float x, y, z;
//  worldpos.getValue(x, y, z);
//  printf("(%0.2f, %0.2f, %0.2f)\n", x, y, z);
#endif // 0

  SbVec3f2ALfloat3(alfloat3, worldpos);

  // Position ...
  alListenerfv(AL_POSITION, alfloat3);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoListener::audioRender",
                              "alListenerfv(AL_POSITION,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  // Velocity ...
  SbVec3f2ALfloat3(alfloat3, velocity.getValue());

  alListenerfv(AL_VELOCITY, alfloat3);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoListener::audioRender",
                              "alListenerfv(AL_VELOCITY,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  // Orientation ...
  this->orientation.getValue().multVec(SbVec3f(0,0,-1), viewdir);
  SbVec3f2ALfloat3(alfloat6, viewdir);

  this->orientation.getValue().multVec(SbVec3f(0,1,0), viewup);
  SbVec3f2ALfloat3(alfloat6+3, viewup);

  alListenerfv(AL_ORIENTATION,alfloat6);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoListener::audioRender",
                              "alListenerfv(AL_ORIENTATION,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  // Gain
  float gain = this->gain.getValue();
  gain = gain<0.0f ? 0.0f : ( gain>1.0f ? 1.0f : gain );  // clamp to [0.0f, 1.0f];
  alListenerf(AL_GAIN, gain);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoListener::audioRender",
                              "alListenerf(AL_GAIN,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
};


#endif // HAVE_OPENAL
