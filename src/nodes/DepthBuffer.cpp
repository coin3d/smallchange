/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class DepthBuffer DepthBuffer.h
  \brief The DepthBuffer class is a node used to control the GL depth buffer. 
  \ingroup nodes

  It is basically a direct mapping of glDepthFunc(). In addition it is possible
  to enable/disable depth buffer and clear the depth buffer when the node is
  traversed.
*/

#include "DepthBuffer.h"
#include <Inventor/actions/SoGLRenderAction.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>

/*!
  \enum DepthBuffer::Func
  Enumeration for the various depth functions.
*/

/*!
  \var DepthBuffer::Func DepthBuffer::NEVER
  Never passes.
*/

/*!
  \var DepthBuffer::Func DepthBuffer::ALWAYS
  Always passes.
*/

/*!
  \var DepthBuffer::Func DepthBuffer::LESS
  Passes if the incoming depth value is less than the stored depth value.
*/

/*!
  \var DepthBuffer::Func DepthBuffer::LEQUAL
  Passes if the incoming depth value is less than or equal to the stored depth value.
*/

/*!
  \var DepthBuffer::Func DepthBuffer::EQUAL
  Passes if the incoming depth value is equal to the stored depth value.
*/

/*!
  \var DepthBuffer::Func DepthBuffer::GEQUAL
  Passes if the incoming depth value is greater than or equal to the stored depth value.
*/

/*!
  \var DepthBuffer::Func DepthBuffer::GREATER
  Passes if the incoming depth value is greater than the stored depth value.
*/

/*!
  \var DepthBuffer::Func DepthBuffer::NOTEQUAL
  Passes if the incoming depth value is not equal to the stored depth value.
*/

/*!
  \var SoSFEnum DepthBuffer::func

  Which depth function to use. Defaults to LESS.
*/

/*!
  \var SoSFBool DepthBuffer::enable

  Enable depth buffer writes. Defaults to TRUE.
*/

/*!
  \var SoSFFloat DepthBuffer::clearNow

  If TRUE, clear buffer when node is traversed. Default is FALSE.
*/


SO_NODE_SOURCE(DepthBuffer);

/*!
  Constructor.
*/
DepthBuffer::DepthBuffer(void)
{
  SO_NODE_CONSTRUCTOR(DepthBuffer);

  SO_NODE_ADD_FIELD(func, (DepthBuffer::LESS));
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
DepthBuffer::~DepthBuffer()
{
}

/*!
  Required Coin method.
*/
void
DepthBuffer::initClass(void)
{
  SO_NODE_INIT_CLASS(DepthBuffer, SoNode, "Node");
  SO_ENABLE(SoGLRenderAction, GLDepthBufferElement);
}

/*!
  Coin method.
*/
void
DepthBuffer::GLRender(SoGLRenderAction * action)
{
  GLDepthBufferElement::set(action->getState(), 
                            (GLDepthBufferElement::Func) this->func.getValue(),
                            this->enable.getValue());
  if (this->clearNow.getValue()) glClear(GL_DEPTH_BUFFER_BIT);
}

