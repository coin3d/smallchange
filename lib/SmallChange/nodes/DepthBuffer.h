/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SODEPTHBUFFER_H
#define COIN_SODEPTHBUFFER_H

#include <SmallChange/elements/GLDepthBufferElement.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/nodes/SoSubNode.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

class DepthBuffer : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(DepthBuffer);

public:
  static void initClass(void);
  DepthBuffer(void);

  enum Func {
    NEVER = GLDepthBufferElement::NEVER,
    ALWAYS = GLDepthBufferElement::ALWAYS,
    LESS = GLDepthBufferElement::LESS,
    LEQUAL = GLDepthBufferElement::LEQUAL,
    EQUAL = GLDepthBufferElement::EQUAL,
    GEQUAL = GLDepthBufferElement::GEQUAL,
    GREATER = GLDepthBufferElement::GREATER,
    NOTEQUAL = GLDepthBufferElement::NOTEQUAL
  };

  SoSFEnum func;
  SoSFBool enable;
  SoSFBool clearNow;

  virtual void GLRender(SoGLRenderAction * action);

protected:
  virtual ~DepthBuffer();
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif // win

#endif // !COIN_SODRAWSTYLE_H
