#ifndef SMSCENEMANAGER_H
#define SMSCENEMANAGER_H

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

#include <Inventor/SoSceneManager.h>

class SmSceneManagerP;
class SoCamera;

class SmSceneManager : public SoSceneManager {
  typedef SoSceneManager inherited;
public:
  SmSceneManager();
  virtual ~SmSceneManager();

  virtual void render(const SbBool clearwindow = TRUE,
                      const SbBool clearzbuffer = TRUE);
  virtual void render(SoGLRenderAction * action,
                      const SbBool initmatrices = TRUE,
                      const SbBool clearwindow = TRUE,
                      const SbBool clearzbuffer = TRUE);
  
  enum RenderMode {
    AS_IS,
    WIREFRAME,
    POINTS,
    WIREFRAME_OVERLAY,
    BOUNDING_BOX
  };
  
  enum StereoMode {
    MONO,
    RED_CYAN,
    RED_BLUE,
    QUAD_BUFFER,
    INTERLEAVED_ROWS,
    INTERLEAVED_COLUMNS
  };

  void setCamera(SoCamera * camera);
  SoCamera * getCamera(void) const;

  void setRenderMode(const RenderMode mode);
  RenderMode getRenderMode(void) const;
  void setStereoMode(const StereoMode mode);
  StereoMode getStereoMode(void) const;
  void setStereoOffset(const float offset);
  float getStereoOffset(void) const;
  
  void setTexturesEnabled(const SbBool onoff);
  SbBool isTexturesEnabled(void) const;

  void setWireframeOverlayColor(const SbColor & color);
  const SbColor & getWireframeOverlayColor(void) const;
  
private:

  void touch(void);
  void renderSingle(SoGLRenderAction * action,
                    SbBool initmatrices,
                    SbBool clearwindow,
                    SbBool clearzbuffer);
  SmSceneManagerP * pimpl;
};


#endif // SMSCENEMANAGER_H
