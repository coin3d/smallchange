/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2008 by Systems in Motion.  All rights reserved.
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

#include "SmTextureText2Collector.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <assert.h>

SO_NODE_SOURCE(SmTextureText2Collector);

SmTextureText2Collector::SmTextureText2Collector(void)
{
  SO_NODE_CONSTRUCTOR(SmTextureText2Collector);
}

SmTextureText2Collector::~SmTextureText2Collector()
{
}
  
void 
SmTextureText2Collector::initClass(void)
{
  SO_NODE_INIT_CLASS(SmTextureText2Collector, SoGroup, "Group");
  SO_ENABLE(SoGLRenderAction, SmTextureText2CollectorElement);
}
  
void 
SmTextureText2Collector::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  SmTextureText2CollectorElement::startCollecting(state);
  inherited::GLRender(action);
  SmTextureText2CollectorElement::finishCollecting(state);
}

/**************************************************************************/

SO_ELEMENT_SOURCE(SmTextureText2CollectorElement);

void 
SmTextureText2CollectorElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(SmTextureText2CollectorElement, inherited);
}

SmTextureText2CollectorElement::~SmTextureText2CollectorElement()
{
}
 
void 
SmTextureText2CollectorElement::init(SoState * state)
{
  this->collecting = false;
}

void 
SmTextureText2CollectorElement::startCollecting(SoState * state)
{
  SmTextureText2CollectorElement * elem = 
    static_cast<SmTextureText2CollectorElement*> 
    (state->getElementNoPush(classStackIndex));
  elem->items.clear();
  elem->collecting = true;
}
 
bool 
SmTextureText2CollectorElement::isCollecting(SoState * state)
{
  SmTextureText2CollectorElement * elem = 
    static_cast<SmTextureText2CollectorElement*> 
    (state->getElementNoPush(classStackIndex));
  return elem->collecting;
}

void
SmTextureText2CollectorElement::add(SoState * state,
				    const char * text,
				    const SmTextureFont::FontImage * font,
				    const SbVec3f & worldpos,
				    const SbColor4f & color,
				    SmTextureText2::Justification j,
				    SmTextureText2::VerticalJustification vj)
{
  SmTextureText2CollectorElement * elem = 
    static_cast<SmTextureText2CollectorElement*> 
    (state->getElementNoPush(classStackIndex));

  assert(elem->collecting);

  TextItem item;
  item.text = text;
  item.font = font;
  item.color = color;
  item.worldpos = worldpos;
  item.justification = j;
  item.vjustification = vj;

  elem->items.push_back(item);
}

void 
SmTextureText2CollectorElement::finishCollecting(SoState * state)
{
  SmTextureText2CollectorElement * elem = 
    static_cast<SmTextureText2CollectorElement*> 
    (state->getElementNoPush(classStackIndex));
  elem->collecting = false;

  // FIXME: render items
}

SbBool
SmTextureText2CollectorElement::matches(const SoElement * elt) const
{
  assert(0 && "should never be called");
}

SoElement *
SmTextureText2CollectorElement::copyMatchInfo(void) const
{
  assert(0 && "should never be called");
  return NULL;
}
