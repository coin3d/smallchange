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

/*!
  \class SmSceneManager SmallChange/misc/SmSceneManager.h

  The SmSceneManager class extends SoSceneManger with render style functionality.

  This class makes it possible to easily control the draw style of a
  scene. It also has support for controlling stereo rendering.
*/

#include "SmSceneManager.h"
#include <Inventor/elements/SoDrawStyleElement.h>
#include <Inventor/elements/SoComplexityTypeElement.h>
#include <Inventor/elements/SoPolygonOffsetElement.h>
#include <Inventor/elements/SoMaterialBindingElement.h>
#include <Inventor/elements/SoOverrideElement.h>
#include <Inventor/elements/SoTextureOverrideElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/nodes/SoInfo.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/SoFullPath.h>
#include <Inventor/system/gl.h>
#include <cassert>

class SmSceneManagerP {
public:
  SmSceneManager * master;
  SmSceneManager::RenderMode rendermode;
  SmSceneManager::StereoMode stereomode;

  SbBool texturesenabled;
  float stereooffset;
  SoInfo * dummynode;
  SoSearchAction searchaction;
  SoCamera * camera;
  SoColorPacker colorpacker;
  SbColor overlaycolor;
  
  void clearBuffers(SbBool color, SbBool depth) {
    const SbColor bgcol = this->master->getBackgroundColor();
    GLbitfield mask = 0;
    if (color) mask |= GL_COLOR_BUFFER_BIT;
    if (depth) mask |= GL_DEPTH_BUFFER_BIT;
    glClearColor(bgcol[0], bgcol[1], bgcol[2], 0.0f);
    glClear(mask);
  }

  SoCamera * getCamera(void) {
    if (this->camera) return this->camera;
    
    this->searchaction.setType(SoCamera::getClassTypeId());
    this->searchaction.setInterest(SoSearchAction::FIRST);
    SbBool old = SoBaseKit::isSearchingChildren();
    SoBaseKit::setSearchingChildren(TRUE);
    this->searchaction.apply(master->getSceneGraph());
    SoBaseKit::setSearchingChildren(old);
    SoFullPath * path = (SoFullPath*) this->searchaction.getPath();
    if (path) {
      SoNode * tail = path->getTail();
      this->searchaction.reset();
      return (SoCamera*) tail;
    }
    return NULL;
  }
};

#define PRIVATE(obj) obj->pimpl

/*!
  Constructor.
*/
SmSceneManager::SmSceneManager()
{
  PRIVATE(this) = new SmSceneManagerP;
  PRIVATE(this)->master = this;
  PRIVATE(this)->rendermode = AS_IS;
  PRIVATE(this)->stereomode = MONO;
  PRIVATE(this)->stereooffset = 0.1f;
  PRIVATE(this)->texturesenabled = TRUE;
  PRIVATE(this)->camera = NULL;
  PRIVATE(this)->overlaycolor = SbColor(1.0f, 0.0f, 0.0f);
  PRIVATE(this)->dummynode = new SoInfo;
  PRIVATE(this)->dummynode->ref();
}

/*!
  Destructor
*/
SmSceneManager::~SmSceneManager()
{
  PRIVATE(this)->dummynode->unref();
  if (PRIVATE(this)->camera) {
    PRIVATE(this)->camera->unref();
  }
  delete PRIVATE(this);
}

// doc in superclass
void 
SmSceneManager::render(const SbBool clearwindow,
                       const SbBool clearzbuffer)
{
  inherited::render(clearwindow, clearzbuffer);
}

// doc in superclass
void 
SmSceneManager::render(SoGLRenderAction * action,
                       const SbBool initmatrices,
                       const SbBool clearwindow,
                       const SbBool clearzbuffer)
{

  if (PRIVATE(this)->stereomode == MONO) {
    this->renderSingle(action, initmatrices, clearwindow, clearzbuffer);
  }
  else {
    SoCamera * camera = PRIVATE(this)->getCamera();
    if (!camera) return;

    PRIVATE(this)->clearBuffers(TRUE, TRUE);
    camera->setStereoAdjustment(PRIVATE(this)->stereooffset);
    camera->setStereoMode(SoCamera::LEFT_VIEW);
    
    switch (PRIVATE(this)->stereomode) {      
    case RED_CYAN:
    case RED_BLUE:
      glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
      break;
    case QUAD_BUFFER:
    case INTERLEAVED_ROWS:
    case INTERLEAVED_COLUMNS:
      assert(0 && "not implemented yet");
      break;
    default:
      assert(0 && "unknown stereo mode");
      break;
    }
    this->renderSingle(action, initmatrices, FALSE, FALSE);

    camera->setStereoMode(SoCamera::RIGHT_VIEW);
    switch (PRIVATE(this)->stereomode) {      
    case RED_CYAN:      
      glClear(GL_DEPTH_BUFFER_BIT);
      glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
      break;
    case RED_BLUE:
      glClear(GL_DEPTH_BUFFER_BIT);
      glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);
      break;
    case QUAD_BUFFER:
    case INTERLEAVED_ROWS:
    case INTERLEAVED_COLUMNS:
      assert(0 && "not implemented yet");
      break;
    default:
      assert(0 && "unknown stereo mode");
      break;
    }
    this->renderSingle(action, initmatrices, FALSE, FALSE);

    // restore
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    camera->setStereoMode(SoCamera::MONOSCOPIC);
  }
}

/*!  
  Sets the camera to be used. If you do not set a camera, the
  manager will search the scene graph for a camera (every frame).
  Set the camera here to avoid this search.
*/
void 
SmSceneManager::setCamera(SoCamera * camera)
{
  if (PRIVATE(this)->camera) {
    PRIVATE(this)->camera->unref();
  }
  PRIVATE(this)->camera = camera;
  if (camera) camera->ref();
}

/*!
  Returns the current camera. If no camera has been set, the current
  scene graph will be searched, and the first active camera will be
  returned.
*/
SoCamera * 
SmSceneManager::getCamera(void) const
{
  return PRIVATE(this)->getCamera();
}

/*!
  Sets the render mode.
*/
void 
SmSceneManager::setRenderMode(const RenderMode mode)
{
  PRIVATE(this)->rendermode = mode;
}

/*!
  Returns the current render mode.
*/
SmSceneManager::RenderMode 
SmSceneManager::getRenderMode(void) const
{
  return PRIVATE(this)->rendermode;
}

/*!
  Sets the stereo mode.
*/
void 
SmSceneManager::setStereoMode(const StereoMode mode)
{
  PRIVATE(this)->stereomode = mode;
  this->touch();
}

/*!
  Returns the current stereo mode.
*/
SmSceneManager::StereoMode 
SmSceneManager::getStereoMode(void) const
{
  return PRIVATE(this)->stereomode;
}

/*!
  Sets the stereo offset used when doing stereo rendering.
*/
void 
SmSceneManager::setStereoOffset(const float offset)
{
  PRIVATE(this)->stereooffset = offset;
  this->touch();
}

/*!
  Returns the current stereo offset.
*/
float 
SmSceneManager::getStereoOffset(void) const
{
  return PRIVATE(this)->stereooffset;
}

/*!
  Enable/disable textures when rendering.
*/
void 
SmSceneManager::setTexturesEnabled(const SbBool onoff)
{
  PRIVATE(this)->texturesenabled = onoff;
  this->touch();
}

/*!
  Returns whether textures are enabled or not.
*/
SbBool 
SmSceneManager::isTexturesEnabled(void) const
{
  return PRIVATE(this)->texturesenabled;
}

/*!
  Sets the color of the lines in WIREFRAME_OVERLAY rendering mode.
*/
void 
SmSceneManager::setWireframeOverlayColor(const SbColor & color)
{
  PRIVATE(this)->overlaycolor = color;
}

/*!
  Returns the WIREFRAME_OVERLAY line color.
*/
const SbColor & 
SmSceneManager::getWireframeOverlayColor(void) const
{
  return PRIVATE(this)->overlaycolor;
}

// touch internal node (used when setting element values)
void 
SmSceneManager::touch(void)
{
  PRIVATE(this)->dummynode->touch();
  // do not trigger a redraw here. User might set the render mode
  // right before calling render().
}

// render once in correct draw style
void 
SmSceneManager::renderSingle(SoGLRenderAction * action,
                             SbBool initmatrices,
                             SbBool clearwindow,
                             SbBool clearzbuffer)
{
  SoState * state = action->getState();
  state->push();

  SoNode * node = PRIVATE(this)->dummynode;

  if (!PRIVATE(this)->texturesenabled) {
    SoTextureQualityElement::set(state, node, 0.0f);
    SoTextureOverrideElement::setQualityOverride(state, TRUE);
  }
  
  switch (PRIVATE(this)->rendermode) {
  case AS_IS:
    inherited::render(action, initmatrices, clearwindow, clearzbuffer);
    break;
  case WIREFRAME:
    SoDrawStyleElement::set(state, node, SoDrawStyleElement::LINES);
    SoLightModelElement::set(state, node, SoLightModelElement::BASE_COLOR);
    SoOverrideElement::setDrawStyleOverride(state, node, TRUE);
    SoOverrideElement::setLightModelOverride(state, node, TRUE);
    inherited::render(action, initmatrices, clearwindow, clearzbuffer);
    break;
  case POINTS:
    SoDrawStyleElement::set(state, node, SoDrawStyleElement::POINTS);
    SoLightModelElement::set(state, node, SoLightModelElement::BASE_COLOR);
    SoOverrideElement::setDrawStyleOverride(state, node, TRUE);
    SoOverrideElement::setLightModelOverride(state, node, TRUE);
    inherited::render(action, initmatrices, clearwindow, clearzbuffer);
    break;
  case HIDDEN_LINE:
    {
      // must clear before setting draw mask
      PRIVATE(this)->clearBuffers(TRUE, TRUE);

      // only draw into depth buffer
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      SoMaterialBindingElement::set(state, node, SoMaterialBindingElement::OVERALL);
      SoLightModelElement::set(state, node, SoLightModelElement::BASE_COLOR);
      SoPolygonOffsetElement::set(state, node, 1.0f, 1.0f,
                                  SoPolygonOffsetElement::FILLED, TRUE);
      SoOverrideElement::setPolygonOffsetOverride(state, node, TRUE);
      SoOverrideElement::setLightModelOverride(state, node, TRUE);
      SoOverrideElement::setMaterialBindingOverride(state, node, TRUE);
      inherited::render(action, initmatrices, FALSE, FALSE);

      // reenable draw masks
      glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
      SoPolygonOffsetElement::set(state, node, 0.0f, 0.0f,
                                  SoPolygonOffsetElement::FILLED, FALSE);
      SoDrawStyleElement::set(state, node, SoDrawStyleElement::LINES);
      SoOverrideElement::setDrawStyleOverride(state, node, TRUE);
      SoOverrideElement::setMaterialBindingOverride(state, node, FALSE);
      inherited::render(action, initmatrices, FALSE, FALSE);    
    }
    break;
  case WIREFRAME_OVERLAY:
      SoPolygonOffsetElement::set(state, node, 1.0f, 1.0f,
                                  SoPolygonOffsetElement::FILLED, TRUE);
      SoOverrideElement::setPolygonOffsetOverride(state, node, TRUE);
      inherited::render(action, initmatrices, clearwindow, clearzbuffer);
      SoPolygonOffsetElement::set(state, node, 0.0f, 0.0f,
                                  SoPolygonOffsetElement::FILLED, FALSE);
      
      SoLazyElement::setDiffuse(state, node, 1, &PRIVATE(this)->overlaycolor, 
                                &PRIVATE(this)->colorpacker);
      SoLightModelElement::set(state, node, SoLightModelElement::BASE_COLOR);
      SoMaterialBindingElement::set(state, node, SoMaterialBindingElement::OVERALL);
      SoDrawStyleElement::set(state, node, SoDrawStyleElement::LINES);
      SoOverrideElement::setLightModelOverride(state, node, TRUE);
      SoOverrideElement::setDiffuseColorOverride(state, node, TRUE);
      SoOverrideElement::setMaterialBindingOverride(state, node, TRUE);
      SoOverrideElement::setDrawStyleOverride(state, node, TRUE);
      inherited::render(action, initmatrices, FALSE, FALSE);    
    break;

  case BOUNDING_BOX:
    SoComplexityTypeElement::set(state, node, SoComplexityTypeElement::BOUNDING_BOX);
    SoOverrideElement::setComplexityTypeOverride(state, node, TRUE);
    inherited::render(action, initmatrices, clearwindow, clearzbuffer);
    break;
  default:
    assert(0 && "unknown rendering mode");
    break;
  }
  state->pop();
}


#undef PRIVATE

