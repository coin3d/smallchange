#ifndef SMALLCHANGE_ROT2HEADING_H
#define SMALLCHANGE_ROT2HEADING_H

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

#include <Inventor/engines/SoSubEngine.h>
#include <Inventor/engines/SoEngineOutput.h>
#include <Inventor/fields/SoMFRotation.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoSFBool.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

class Rot2Heading : public SoEngine {
  typedef SoEngine inherited;
  
  SO_ENGINE_HEADER(Rot2Heading);
  
public:
  
  SoSFBool inverse;
  SoMFRotation rotation;
  
  SoEngineOutput heading;         // SoMFVec3f

  static void initClass(void);
  Rot2Heading(void);

protected:

  virtual ~Rot2Heading();

private:  
  virtual void evaluate();
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif // win

#endif // !SMALLCHANGE_ROT2HEADING_H
