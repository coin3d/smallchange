#ifndef SMALLCHANGE_VIEWPORTREGION_H
#define SMALLCHANGE_VIEWPORTREGION_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/fields/SoSFVec2f.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFColor.h>
#include <Inventor/fields/SoSFBool.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

#include <SmallChange/basic.h>

class ViewportRegionP;


class SMALLCHANGE_DLL_API ViewportRegion : public SoNode {
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
  virtual void callback(SoCallbackAction * action);

protected:
  virtual ~ViewportRegion();
  virtual void notify(SoNotList * list);

private:
  ViewportRegionP * pimpl;
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif // win

#endif // !SMALLCHANGE_VIEWPORTREGION_H
