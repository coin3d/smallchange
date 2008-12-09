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

/*!
  \class SmTextureText2Collector nodes/SmTextureText2Collector.h
  \brief The SmTextureText2Collector node is a group node which optimizes SmTextureText2 rendering.

  Since SmTextureText2 has antialiased rendering, it needs to render
  the text in a separate blending pass in Coin. This makes rendering
  slow if you have lots of SmTextureText2 nodes in your scene graph,
  and you use the SORTED_OBJECT_BLEND or DELAYED_BLEND transparency
  modes.

  SmTextureText2 will also invalidate render and bbox caches every
  time the camera moves. In addition, you might get quite a lot of
  OpenGL state changes when rendering the SmTextureText2 nodes
  inbetween other geometry.

  To solve these problem, it's possible to use this node to optimize text
  rendering. It will collect all strings from all SmTextureText2 nodes
  in its subgraph, and render all strings before exiting.

  A typical scene graph might look like this:

  \code
  Separator {
    PerspectiveCamera { } # the camera needs to be in front of the node
    SmTextureText2Collector {
      ...
      SmTextureText2 {...}
    }
  }

  \endcode

  Please note that this node will only be able to optimize SmTextureText2 nodes
  where the number of postions equals the number of strings.
*/

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
#include <Inventor/SbPlane.h>
#include <assert.h>

SO_NODE_SOURCE(SmTextureText2Collector);

SmTextureText2Collector::SmTextureText2Collector(void)
{
  SO_NODE_CONSTRUCTOR(SmTextureText2Collector);
  SO_NODE_ADD_FIELD(depthMask, (false));
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

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDepthMask(this->depthMask.getValue());
    glBegin(GL_QUADS);

    const SbPlane & nearplane = vv.getPlane(0.0f);

    for (size_t i = 0; i < items.size(); i++) {
      float dist = -nearplane.getDistance(items[i].worldpos);
      if ((dist < 0.0f) ||
          ((items[i].maxdist > 0.0f) && (dist > items[i].maxdist))) continue;

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

      short ymin = short(-currentfont->getDescent());

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
    glPopAttrib();
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
                                    const float maxdist,
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
  item.maxdist = maxdist;
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
  return false;
}

SoElement *
SmTextureText2CollectorElement::copyMatchInfo(void) const
{
  assert(0 && "should never be called");
  return NULL;
}