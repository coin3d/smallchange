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

/*!
  \class GLDepthBufferElement GLDepthBufferElement.h Inventor/elements/GLDepthBufferElement.h
  \brief The SoGLDrawStyleElement controls the OpenGL depth buffer.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include "GLDepthBufferElement.h"

#ifdef __COIN__
#include <Inventor/system/gl.h>
#else // SGI/TGS Inventor
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>
#endif // SGI/TGS Inventor

#include <cassert>

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
  this->func = LEQUAL;
  this->updategl();
}


void
GLDepthBufferElement::push(SoState * state)
{
  GLDepthBufferElement * prev = (GLDepthBufferElement*)this->getNextInStack();
  this->enable = prev->enable;
  this->func = prev->func;
  prev->capture(state);
}
/*!
  Internal Coin method.
*/
void
GLDepthBufferElement::pop(SoState * state,
                          const SoElement * prevTopElement)
{
  GLDepthBufferElement * prev = (GLDepthBufferElement*)prevTopElement;
  if (this->enable != prev->enable || this->func != prev->func) {
    this->updategl();
  }
}

/*!
  Set this element's values.
*/
void 
GLDepthBufferElement::set(SoState * state, const Func func, const SbBool enable)
{
  GLDepthBufferElement * elem = (GLDepthBufferElement*)
    SoElement::getElement(state, classStackIndex);

  if (func != elem->func || enable != elem->enable) {
    elem->enable = enable;
    elem->func = func;
    elem->updategl();
  }
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
