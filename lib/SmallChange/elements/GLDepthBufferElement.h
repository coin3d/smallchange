/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_GLDEPTHBUFFERELEMENT_H
#define COIN_GLDEPTHBUFFERELEMENT_H

#include <Inventor/elements/SoElement.h>
#include <Inventor/elements/SoSubElement.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

class GLDepthBufferElement : public SoElement {
  typedef SoElement inherited;

  SO_ELEMENT_HEADER(GLDepthBufferElement);
public:
  static void initClass(void);
protected:
  virtual ~GLDepthBufferElement();

public:
  enum Func {
    NEVER,
    ALWAYS,
    LESS,
    LEQUAL,
    EQUAL,
    GEQUAL,
    GREATER,
    NOTEQUAL
  };

  static void set(SoState * state, const Func func, const SbBool enable);
  virtual void init(SoState * state);
  virtual void pop(SoState * state,
                   const SoElement * prevTopElement);

  virtual SbBool matches(const SoElement * element) const;
  virtual SoElement * copyMatchInfo(void) const;

private:
  void updategl(void) const;
  SbBool enable;
  Func func;
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif

#endif // !COIN_GLDEPTHBUFFERELEMENT_H
