#ifndef SMALLCHANGE_SOLISTENER_H
#define SMALLCHANGE_SOLISTENER_H

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
#include <Inventor/fields/SoSFRotation.h>
#include <Inventor/fields/SoSFFloat.h>

class SoListener : public SoNode
{
  SO_NODE_HEADER(SoListener);
  friend class SoAudioRenderAction;

public:
  static void initClass();
  SoListener();
  SoSFVec3f position;
  SoSFRotation orientation;
  SoSFVec3f velocity;
  SoSFFloat gain;

protected:
  virtual void audioRender(class SoAudioRenderAction *action);
private:
  virtual ~SoListener();
};

#endif // !SMALLCHANGE_SOLISTENER_H
