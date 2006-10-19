#ifndef SMALLCHANGE_CAMERACONTROL_H
#define SMALLCHANGE_CAMERACONTROL_H

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

#include <Inventor/SbBasic.h>

class SoCamera;
class SbVec3f;
class SbVec3d;
class SbRotation;

void cam_reset_roll(SoCamera * camera, const SbVec3f & viewup);

void cam_seek_to_point(SoCamera * camera,
                       const SbVec3d & point,
                       const SbRotation & orientation,
                       const float seektime = 2.0f);

void cam_seek_to_node(SoCamera * camera,
                      const float seektime = 2.0f);

SbBool cam_is_seeking(void);

#endif // SMALLCHANGE_CAMERACONTROL_H
