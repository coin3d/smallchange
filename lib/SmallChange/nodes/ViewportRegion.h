/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

#ifndef COIN_VIEWPORTREGION_H
#define COIN_VIEWPORTREGION_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/fields/SoSFVec2f.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFColor.h>
#include <Inventor/fields/SoSFBool.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

class ViewportRegion : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(ViewportRegion);

public:
  static void initClass(void);

  SoSFVec2f origin;
  SoSFVec2f size;
  SoSFBool flipX;
  SoSFBool flipY;
  SoSFBool clampSize;

  SoSFVec2f pixelOrigin;
  SoSFVec2f pixelSize;
  SoSFBool clearDepthBuffer;
  SoSFBool clearColorBuffer;
  SoSFColor clearColor;

  ViewportRegion(void);

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void getMatrix(SoGetMatrixAction * action);
  virtual void handleEvent(SoHandleEventAction * action);
  virtual void pick(SoPickAction * action);

protected:
  virtual ~ViewportRegion();
  virtual void notify(SoNotList * list);

private:
  SbBool usepixelsize;
  SbBool usepixelorigin;
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif // win

#endif // COIN_VIEWPORTREGION_H
