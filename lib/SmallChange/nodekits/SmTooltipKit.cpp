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
  \class SmTooltipKit SmTooltipKit.h
  \brief The SmTooltipKit class... 
  \ingroup nodekits

  FIXME: doc
*/

#include "SmTooltipKit.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/nodes/SoSeparator.h>
#include <SmallChange/nodes/DepthBuffer.h>
#include <Inventor/nodes/SoResetTransform.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SoFullPath.h>
#include <Inventor/elements/SoViewportRegionElement.h>

class SmTooltipKitP {
public:
  SmTooltipKitP(void) 
    : ba(SbViewportRegion(100,100)) { }
  SoFieldSensor * tooltipsensor;
  SoSearchAction sa;
  SoGetBoundingBoxAction ba;
  SbViewportRegion vp;
};

#define PRIVATE(obj) (obj)->pimpl

SO_KIT_SOURCE(SmTooltipKit);

/*!
  Constructor. 
*/
SmTooltipKit::SmTooltipKit(void)
{
  PRIVATE(this) = new SmTooltipKitP;

  SO_KIT_CONSTRUCTOR(SmTooltipKit);
  
  SO_KIT_ADD_FIELD(delayedRender, (TRUE));
  SO_KIT_ADD_FIELD(isActive, (FALSE));
  SO_KIT_ADD_FIELD(tooltipString, (""));
  SO_KIT_ADD_FIELD(frameSize, (3));
  
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(resetTransform, SoResetTransform, FALSE, topSeparator, depthBuffer, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(depthBuffer, DepthBuffer, TRUE, topSeparator, lightModel, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(lightModel, SoLightModel, FALSE, topSeparator, camera, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(camera, SoOrthographicCamera, FALSE, topSeparator, texture, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(texture, SoTexture2, FALSE, topSeparator, shapeHints, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(shapeHints, SoShapeHints, FALSE, topSeparator, pickStyle, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(pickStyle, SoPickStyle, TRUE, topSeparator, materialBinding, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(materialBinding, SoMaterialBinding, TRUE, topSeparator, backgroundColor, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(backgroundColor, SoBaseColor, TRUE, topSeparator, backgroundShape, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(backgroundShape, SoFaceSet, TRUE, topSeparator, position, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(position, SoTranslation, TRUE, topSeparator, textColor, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textColor, SoBaseColor, TRUE, topSeparator, textShape, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textShape, SoText2, TRUE, topSeparator, "", FALSE);

  SO_KIT_INIT_INSTANCE();

  SoOrthographicCamera * cam  = (SoOrthographicCamera*) this->getAnyPart("camera", TRUE);
  cam->height = 1.0f;
  cam->position = SbVec3f(0.5f, 0.5f, 0.0f);
  cam->viewportMapping = SoCamera::LEAVE_ALONE;
  cam->nearDistance = 0.0f;
  cam->farDistance = 2.0f;

  // disable picking on geometry below
  SoPickStyle * ps = (SoPickStyle*) this->getAnyPart("pickStyle", TRUE);
  ps->style = SoPickStyle::UNPICKABLE;

  // disable lighting
  SoLightModel * lm = (SoLightModel*)this->getAnyPart("lightModel", TRUE);
  lm->model = SoLightModel::BASE_COLOR;

  // disable depth buffer
  DepthBuffer * db = (DepthBuffer*) this->getAnyPart("depthBuffer", TRUE);
  db->enable = FALSE;
  
  // set justification
  SoText2 * text = (SoText2*) this->getAnyPart("textShape", TRUE);
  text->justification = SoText2::LEFT;

  SoBaseColor * col = (SoBaseColor*) this->getAnyPart("textColor", TRUE);
  col->rgb = SbColor(0.0f, 0.0f, 0.0f);

  col = (SoBaseColor*) this->getAnyPart("backgroundColor", TRUE);
  col->rgb = SbColor(1.0f, 1.0f, 0.0f);

  PRIVATE(this)->tooltipsensor = new SoFieldSensor(tooltip_changed_cb, this);
  PRIVATE(this)->tooltipsensor->attach(&this->tooltipString);
}

/*!
  Destructor.
*/
SmTooltipKit::~SmTooltipKit(void)
{
  delete PRIVATE(this)->tooltipsensor;
  delete PRIVATE(this);
}

// Documented in superclass
void
SmTooltipKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SmTooltipKit, SoBaseKit, "BaseKit");
  }
}

// Documented in superclass
void 
SmTooltipKit::GLRender(SoGLRenderAction * action)
{
  PRIVATE(this)->vp = SoViewportRegionElement::get(action->getState());
  if (!this->isActive.getValue()) return;

  if (this->delayedRender.getValue() && !action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());
    return;
  }
  inherited::GLRender(action);
}

/*!  
  Sets the current viewport region. The viewport region will also
  be picked up when the node is travered with SoGLRenderAction.
*/
void 
SmTooltipKit::setViewportRegion(const SbViewportRegion & vp)
{
  PRIVATE(this)->vp = vp;
  this->updateBackground();
}

/*!
  Convenience function that uses an SoPickedPoint to calculate the
  position of the tooltip.
*/
void 
SmTooltipKit::setPickedPoint(const SoPickedPoint * pp, const SbViewportRegion & vp)
{
  if (pp == NULL) {
    this->isActive = FALSE;
    return;
  }
  PRIVATE(this)->sa.setType(SoCamera::getClassTypeId());
  PRIVATE(this)->sa.setInterest(SoSearchAction::LAST);
  PRIVATE(this)->sa.setSearchingAll(FALSE);
  PRIVATE(this)->sa.apply(pp->getPath());

  if (PRIVATE(this)->sa.getPath()) {
    SoFullPath * p = (SoFullPath*) PRIVATE(this)->sa.getPath();
    SoCamera * cam = (SoCamera*) p->getTail();
    SbViewVolume vv;
    float aspectratio = vp.getViewportAspectRatio();
    switch (cam->viewportMapping.getValue()) {
    case SoCamera::CROP_VIEWPORT_FILL_FRAME:
    case SoCamera::CROP_VIEWPORT_LINE_FRAME:
    case SoCamera::CROP_VIEWPORT_NO_FRAME:
      vv = cam->getViewVolume(0.0f);
      break;
    case SoCamera::ADJUST_CAMERA:
      vv = cam->getViewVolume(aspectratio);
      if (aspectratio < 1.0f) vv.scale(1.0f / aspectratio);
      break;
    case SoCamera::LEAVE_ALONE:
      vv = cam->getViewVolume(0.0f);
      break;
    default:
      assert(0 && "unknown viewport mapping");
      break;
    }

    SbVec3f npt;
    vv.projectToScreen(pp->getPoint(), npt);
    npt[2] = -1.0f;

    SoTranslation * t = (SoTranslation*) this->getAnyPart("position", TRUE);
    t->translation = npt;
    
    this->updateBackground();
    this->isActive = TRUE;
  }
  else {
    this->isActive = FALSE;
  }
  PRIVATE(this)->sa.reset();
}


void 
SmTooltipKit::tooltip_changed_cb(void * closure, SoSensor * s)
{
  SmTooltipKit * thisp = (SmTooltipKit*) closure;
  SoText2 * text = (SoText2*) thisp->getAnyPart("textShape", TRUE);
  text->string = thisp->tooltipString;
  thisp->updateBackground();
}

void 
SmTooltipKit::updateBackground(void)
{
  SoPath * p = new SoPath(this->topSeparator.getValue());
  p->ref();
  p->append(this->textShape.getValue());

  SoFaceSet * fs = (SoFaceSet*) this->getAnyPart("backgroundShape", TRUE);
  fs->numVertices = 0;

  PRIVATE(this)->ba.setViewportRegion(PRIVATE(this)->vp);
  PRIVATE(this)->ba.apply(p);
  SbBox3f bb = PRIVATE(this)->ba.getBoundingBox();
  p->unref();

  SoVertexProperty * vp = (SoVertexProperty*) fs->vertexProperty.getValue();
  if (vp == NULL) {
    vp = new SoVertexProperty;
    fs->vertexProperty = vp;
  }
  if (fs->numVertices.getNum() != 1 || fs->numVertices[0] != 4) {
    fs->numVertices = 4;
  }
  
  float fx = this->frameSize.getValue() / float(PRIVATE(this)->vp.getViewportSizePixels()[0]);
  float fy = this->frameSize.getValue() / float(PRIVATE(this)->vp.getViewportSizePixels()[1]);
  
  SbVec3f bmin = bb.getMin();
  SbVec3f bmax = bb.getMax();

  bmin[0] -= fx;
  bmin[1] -= fy;
  bmax[0] += fx;
  bmax[1] += fy;

  SbVec3f varray[4];
  varray[0] = SbVec3f(bmin[0], bmin[1], 0.0f);
  varray[1] = SbVec3f(bmax[0], bmin[1], 0.0f);
  varray[2] = SbVec3f(bmax[0], bmax[1], 0.0f);
  varray[3] = SbVec3f(bmin[0], bmax[1], 0.0f);
  vp->vertex.setValues(0, 4, varray);
}

#undef PRIVATE

