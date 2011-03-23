#ifndef SMALLCHANGE_TRACK_H
#define SMALLCHANGE_TRACK_H

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

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoMFVec3d.h>
#include <Inventor/fields/SoMFTime.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFUShort.h>
#include <SmallChange/basic.h>

class SoAction;
class SoGLRenderAction;

class SMALLCHANGE_DLL_API SmTrack : public SoShape {
  typedef SoShape inherited;
  SO_NODE_HEADER(SmTrack);

public:
  static void initClass(void);
  SmTrack(void);

  SoMFTime timeStamps;
  SoMFVec3d track;
  SoSFFloat trackLength;

  SoSFUShort pointInterval;
  SoSFUShort lineInterval;
  SoSFUShort tickInterval;
  SoSFFloat tickSize;

  void append(const SbVec3d & pos,
              const SbTime & timestamp);

  /*!
       Convenience: Clears timeStamps, track and reset trackLength
  */
  void deleteValues();

protected:
  virtual void GLRender(SoGLRenderAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box,
                           SbVec3f & center);
  virtual void generatePrimitives(SoAction * action);

private:
  virtual ~SmTrack(void);

  class SmTrackP * pimpl;
};

#endif // SMALLCHANGE_TRACK_H
