/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion. All rights reserved.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

#ifndef ROT2HEADING_H
#define ROT2HEADING_H

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

#endif // ROT2HEADING_H
