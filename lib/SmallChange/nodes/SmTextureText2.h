#ifndef SM_TEXTURE_TEXT2_H
#define SM_TEXTURE_TEXT2_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFEnum.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API SmTextureText2 : public SoShape {
  typedef SoShape inherited;
  
  SO_NODE_HEADER(SmTextureText2);

public:
  static void initClass(void);
  SmTextureText2(void);
  
  enum Justification {
    LEFT = 1,
    RIGHT,
    CENTER
  };

  SoMFString string;
  SoSFEnum justification;
  SoMFVec3f position;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~SmTextureText2();

  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:

  void renderString(const SbString * s, 
                    const int numstring,
                    const SbVec3f & pos,
                    const SbViewVolume & vv,
                    const SbViewportRegion & vp,
                    const SbMatrix & projmatrix,
                    const SbMatrix & modelmatrix,
                    const SbMatrix & invmodelmatrix);

  void oldRenderString(const SbString * s, 
                       const int numstring,
                       const SbVec3f & pos,
                       const SbViewVolume & vv,
                       const SbViewportRegion & vp,
                       const SbMatrix & projmatrix,
                       const SbMatrix & modelmatrix,
                       const SbMatrix & invmodelmatrix);

  static unsigned char * create_texture(void);
  static void get_text_pixmap_position(const int idx, int & x, int & y);
  static void render_text(unsigned char * dst,
                          const int idx, const int x, const int y,
                          const unsigned char value,
                          const unsigned char alpha);
};

#endif
