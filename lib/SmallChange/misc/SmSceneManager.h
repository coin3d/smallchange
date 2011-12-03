#ifndef SMSCENEMANAGER_H
#define SMSCENEMANAGER_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include <Inventor/SoSceneManager.h>
#include <SmallChange/basic.h>

class SmSceneManagerP;
class SoCamera;

class SMALLCHANGE_DLL_API SmSceneManager : public SoSceneManager {
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
    HIDDEN_LINE,
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
