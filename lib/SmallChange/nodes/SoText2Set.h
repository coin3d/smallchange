/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOTEXT2SET_H
#define COIN_SOTEXT2SET_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFTime.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFVec2f.h>
#include <Inventor/SbVec2f.h>

class SoText2Set : public SoShape
{
  typedef SoShape inherited;

  SO_NODE_HEADER(SoText2Set);

public:

  enum Justification {
    LEFT = 1,
    RIGHT,
    CENTER
  };

  // Fields
  SoSFEnum    justification;
  SoMFString  strings;
  SoSFVec2f   displacement;



  static void initClass(void);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

  SoText2Set(void);

protected:
  virtual ~SoText2Set();

  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  friend class SoText2SetP;
  class SoText2SetP * pimpl;
};

#endif // !COIN_SOTEXT2SET_H
