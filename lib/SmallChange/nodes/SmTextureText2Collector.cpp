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
#include "SmTextureFont.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoGLTextureImageElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoGLTextureCoordinateElement.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/system/gl.h>
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
  SO_NODE_INIT_CLASS(SmTextureText2Collector, SoSeparator, "Separator");
  SO_ENABLE(SoGLRenderAction, SmTextureText2CollectorElement);
}
  
void 
SmTextureText2Collector::GLRenderInPath(SoGLRenderAction * action)
{
  SoState * state = action->getState();  
  SmTextureText2CollectorElement::startCollecting(state, false);
  inherited::GLRenderInPath(action);
  (void) SmTextureText2CollectorElement::finishCollecting(state);
}

// convert normalized screen space coordinates into pixel screen
// space. Values that are too far from the viewport are culled.
static SbBool get_screenpoint_pixels(const SbVec3f & screenpoint,
                                     const SbVec2s & vpsize,
                                     SbVec2s & sp)
{
  float sx = screenpoint[0] * vpsize[0];
  float sy = screenpoint[1] * vpsize[1];

  // FIXME: just assume we won't have strings larger than 3000 pixels
  const float limit = 3000.0f;

  if ((sx > -limit) &&
      (sy > -limit) &&
      (sx < limit) &&
      (sy < limit)) {
    sp = SbVec2s(static_cast<short>(sx), static_cast<short>(sy));
    return TRUE;
  }
  return FALSE;
}

void 
SmTextureText2Collector::GLRenderBelowPath(SoGLRenderAction * action)
{
  SoState * state = action->getState();  
  SoCacheElement::invalidate(state);
  
  // for Coin-3
  // bool transppass = action->isRenderingTranspPaths(); 

  bool transppass = action->handleTransparency(FALSE);
  
  SmTextureText2CollectorElement::startCollecting(state, !transppass);
  inherited::GLRenderBelowPath(action);

  const std::vector<SmTextureText2CollectorElement::TextItem> & items = 
    SmTextureText2CollectorElement::finishCollecting(state);

  if (!transppass && items.size()) {
    // FIXME: sort items based on font
    state->push();
    
    SbMatrix normalize(0.5f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.5f, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.5f, 0.5f, 0.0f, 1.0f);
    SbMatrix projmatrix =
      SoViewingMatrixElement::get(state) *
      SoProjectionMatrixElement::get(state) *
      normalize;

    const SbViewVolume & vv = SoViewVolumeElement::get(state);
    const SbViewportRegion & vp = SoViewportRegionElement::get(state);
    const SbVec2s vpsize = vp.getViewportSizePixels();
    
    // Set up new view volume
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, vpsize[0], 0, vpsize[1], -1.0f, 1.0f);

    // set up texture and rendering 
    const SmTextureFont::FontImage * currentfont = items[0].font;
    SoLightModelElement::set(state, SoLightModelElement::BASE_COLOR); 
    SoTextureQualityElement::set(state, 0.3f);
    SoGLTextureImageElement::set(state, this,
                                 currentfont->getGLImage(),
                                 SoTextureImageElement::MODULATE,
                                 SbColor(1.0f, 1.0f, 1.0f));
    SoLazyElement::setVertexOrdering(state, SoLazyElement::CCW);
    SoGLTextureCoordinateElement::setTexGen(state, this, NULL);

    SoGLTextureEnabledElement::set(state, this, TRUE);
    SoLazyElement::enableBlending(state, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SoMaterialBundle mb(action);
    mb.sendFirst();

    glBegin(GL_QUADS);

    for (size_t i = 0; i < items.size(); i++) {
      glColor4fv(items[i].color.getValue());
      int len = items[i].text.getLength();
      if (len == 0) continue;
      const unsigned char * sptr =
        reinterpret_cast<const unsigned char *>(items[i].text.getString());

      if (items[i].font != currentfont) {
        glEnd();
        currentfont = items[i].font;
        SoGLTextureImageElement::set(state, this,
                                     currentfont->getGLImage(),
                                     SoTextureImageElement::MODULATE,
                                     SbColor(1.0f, 1.0f, 1.0f));        
        SoGLLazyElement::getInstance(state)->send(state, 
                                                  SoLazyElement::GLIMAGE_MASK);

        glBegin(GL_QUADS);
      }
      SbVec3f screenpoint;
      projmatrix.multVecMatrix(items[i].worldpos, screenpoint);

      int ymin = -currentfont->getLeading();
      
      switch (items[i].vjustification) {
      case SmTextureText2::BOTTOM:
        break;
      case SmTextureText2::TOP:
        ymin -= currentfont->getAscent();
        break;
      case SmTextureText2::VCENTER:
        ymin -= currentfont->getAscent() / 2;
        break;
      default:
        assert(0 && "unknown alignment");
        break;
      }
      short w = (short) currentfont->stringWidth(items[i].text); 
    
      SbVec2s sp;
      if (!get_screenpoint_pixels(screenpoint, vpsize, sp)) continue;

      SbVec2s n0 = SbVec2s(sp[0],
                           sp[1] + ymin);
      
      switch (items[i].justification) {
      case SmTextureText2::LEFT:
        break;
      case SmTextureText2::RIGHT:
        n0[0] -= w;
        break;
      case SmTextureText2::CENTER:
        n0[0] -= w/2;
        break;
      default:
        assert(0 && "unknown alignment");
      break;
      }
      currentfont->renderString(items[i].text, SbVec3f(n0[0], n0[1], screenpoint[2]), false);
    }
    glEnd();
    SoGLLazyElement::getInstance(state)->reset(state, 
                                               SoLazyElement::DIFFUSE_MASK);
    
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    state->pop();
  }
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
  this->storeitems = false;
}

void 
SmTextureText2CollectorElement::startCollecting(SoState * state, const bool storeitems)
{
  SmTextureText2CollectorElement * elem = 
    static_cast<SmTextureText2CollectorElement*> 
    (state->getElementNoPush(classStackIndex));
  elem->items.clear();
  elem->collecting = true;
  elem->storeitems = storeitems;
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
				    const SbString & text,
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
  if (!elem->storeitems) return;

  TextItem item;
  item.text = text;
  item.font = font;
  item.color = color;
  item.worldpos = worldpos;
  item.justification = j;
  item.vjustification = vj;

  elem->items.push_back(item);
}

const std::vector <SmTextureText2CollectorElement::TextItem> & 
SmTextureText2CollectorElement::finishCollecting(SoState * state)
{
  SmTextureText2CollectorElement * elem = 
    static_cast<SmTextureText2CollectorElement*> 
    (state->getElementNoPush(classStackIndex));
  elem->collecting = false;
  elem->storeitems = false;
  return elem->items;
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
