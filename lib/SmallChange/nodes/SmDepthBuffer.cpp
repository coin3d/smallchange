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
  \class SmDepthBuffer SmDepthBuffer.h
  \brief The SmDepthBuffer class is a node used to control the GL depth buffer.

  \ingroup nodes

  It is basically a direct mapping of glDepthFunc(). In addition it is possible
  to enable/disable depth buffer and clear the depth buffer when the node is
  traversed.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include "SmDepthBuffer.h"
#include <Inventor/actions/SoGLRenderAction.h>

#ifdef __COIN__
#include <Inventor/system/gl.h>
#else // SGI/TGS Inventor
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>
#endif // SGI/TGS Inventor

/*!
  \enum SmDepthBuffer::Func
  Enumeration for the various depth functions.
*/

/*!
  \var SmDepthBuffer::Func SmDepthBuffer::NEVER
  Never passes.
*/

/*!
  \var SmDepthBuffer::Func SmDepthBuffer::ALWAYS
  Always passes.
*/

/*!
  \var SmDepthBuffer::Func SmDepthBuffer::LESS
  Passes if the incoming depth value is less than the stored depth value.
*/

/*!
  \var SmDepthBuffer::Func SmDepthBuffer::LEQUAL
  Passes if the incoming depth value is less than or equal to the stored depth value.
*/

/*!
  \var SmDepthBuffer::Func SmDepthBuffer::EQUAL
  Passes if the incoming depth value is equal to the stored depth value.
*/

/*!
  \var SmDepthBuffer::Func SmDepthBuffer::GEQUAL
  Passes if the incoming depth value is greater than or equal to the stored depth value.
*/

/*!
  \var SmDepthBuffer::Func SmDepthBuffer::GREATER
  Passes if the incoming depth value is greater than the stored depth value.
*/

/*!
  \var SmDepthBuffer::Func SmDepthBuffer::NOTEQUAL
  Passes if the incoming depth value is not equal to the stored depth value.
*/

/*!
  \var SoSFEnum SmDepthBuffer::func

  Which depth function to use. Defaults to LESS.
*/

/*!
  \var SoSFBool SmDepthBuffer::enable

  Enable depth buffer writes. Defaults to TRUE.
*/

/*!
  \var SoSFFloat SmDepthBuffer::clearNow

  If TRUE, clear buffer when node is traversed. Default is FALSE.
*/


SO_NODE_SOURCE(SmDepthBuffer);

/*!
  Constructor.
*/
SmDepthBuffer::SmDepthBuffer(void)
{
  SO_NODE_CONSTRUCTOR(SmDepthBuffer);

  SO_NODE_ADD_FIELD(func, (SmDepthBuffer::LESS));
  SO_NODE_ADD_FIELD(enable, (TRUE));
  SO_NODE_ADD_FIELD(clearNow, (FALSE));

  SO_NODE_DEFINE_ENUM_VALUE(Func, NEVER);
  SO_NODE_DEFINE_ENUM_VALUE(Func, ALWAYS);
  SO_NODE_DEFINE_ENUM_VALUE(Func, LESS);
  SO_NODE_DEFINE_ENUM_VALUE(Func, LEQUAL);
  SO_NODE_DEFINE_ENUM_VALUE(Func, EQUAL);
  SO_NODE_DEFINE_ENUM_VALUE(Func, GEQUAL);
  SO_NODE_DEFINE_ENUM_VALUE(Func, GREATER);
  SO_NODE_DEFINE_ENUM_VALUE(Func, NOTEQUAL);
  SO_NODE_SET_SF_ENUM_TYPE(func, Func);
}

/*!
  Destructor.
*/
SmDepthBuffer::~SmDepthBuffer()
{
}

/*!
  Required Coin method.
*/
void
SmDepthBuffer::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    GLDepthBufferElement::initClass();
    SO_NODE_INIT_CLASS(SmDepthBuffer, SoNode, "Node");
    SO_ENABLE(SoGLRenderAction, GLDepthBufferElement);
  }
}

/*!
  Coin method.
*/
void
SmDepthBuffer::GLRender(SoGLRenderAction * action)
{
  GLDepthBufferElement::set(action->getState(),
                            (GLDepthBufferElement::Func) this->func.getValue(),
                            this->enable.getValue());
  if (this->clearNow.getValue()) glClear(GL_DEPTH_BUFFER_BIT);
}
