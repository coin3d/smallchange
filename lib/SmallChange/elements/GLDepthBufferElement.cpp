/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class GLDepthBufferElement Inventor/elements/GLDepthBufferElement.h
  \brief The SoGLDrawStyleElement controls the OpenGL depth buffer.
*/

#include "GLDepthBufferElement.h"

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>
#include <assert.h>

SO_ELEMENT_SOURCE(GLDepthBufferElement);

/*!
  This static method initializes static data for the
  GLDepthBufferElement class.
*/

void
GLDepthBufferElement::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_ELEMENT_INIT_CLASS(GLDepthBufferElement, inherited);
  }
}

/*!
  The destructor.
*/

GLDepthBufferElement::~GLDepthBufferElement(void)
{
}

/*!
  Internal Coin method.
*/
void
GLDepthBufferElement::init(SoState * state)
{
  inherited::init(state);
  this->enable = TRUE;
  this->func = LESS;
  this->updategl();
}

/*!
  Internal Coin method.
*/
void
GLDepthBufferElement::pop(SoState * state,
                          const SoElement * prevTopElement)
{
  this->updategl();
  inherited::pop(state, prevTopElement);
}

/*!
  Set this element's values.
*/
void 
GLDepthBufferElement::set(SoState * state, const Func func, const SbBool enable)
{
  GLDepthBufferElement * elem = (GLDepthBufferElement*)
    SoElement::getElement(state, classStackIndex);
  elem->func = func;
  elem->enable = enable;
  elem->updategl();
}

/*!
  Internal Coin method.
*/
SbBool 
GLDepthBufferElement::matches(const SoElement * element) const
{
  const GLDepthBufferElement * elem = (const GLDepthBufferElement*) element;
  return (elem->func == this->func) && (elem->enable == this->enable); 
}

/*!
  Internal Coin method.
*/
SoElement * 
GLDepthBufferElement::copyMatchInfo(void) const
{
  GLDepthBufferElement * elem = (GLDepthBufferElement*)
    this->getTypeId().createInstance();
  elem->func = this->func;
  elem->enable = this->enable;
  return elem;
}

void 
GLDepthBufferElement::updategl(void) const
{
  if (this->enable) {
    glEnable(GL_DEPTH_TEST);
    switch (this->func) {
    case NEVER: glDepthFunc(GL_NEVER); break;
    case ALWAYS: glDepthFunc(GL_ALWAYS); break;
    case LESS: glDepthFunc(GL_LESS); break;
    case LEQUAL: glDepthFunc(GL_LEQUAL); break;
    case EQUAL: glDepthFunc(GL_EQUAL); break;
    case GEQUAL: glDepthFunc(GL_GEQUAL); break;
    case GREATER: glDepthFunc(GL_GREATER); break;
    case NOTEQUAL: glDepthFunc(GL_NOTEQUAL); break; 
    }
  }
  else glDisable(GL_DEPTH_TEST);
}
