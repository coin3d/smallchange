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
  \class SmPopupMenuKit SmPopupMenuKit.h
  \brief The SmPopupMenuKit class... 
  \ingroup nodekits

  FIXME: doc
*/


/*!
  \var SoSFBool SmPopupMenuKit::isActive
  
  TRUE when menu is active, FALSE otherwise.
*/

/*!
  \var SoMFString SmPopupMenuKit::items

  The menu items
*/

/*!
  \var SoSFInt32 SmPopupMenuKit::frameSize

  The amount of extra pixels around the frame. The default value is 3.

*/

#include "SmPopupMenuKit.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/sensors/SoOneShotSensor.h>
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
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoDrawStyleElement.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/elements/SoComplexityTypeElement.h>
#include <Inventor/elements/SoComplexityElement.h>

class SmPopupMenuKitP {
public:
  SmPopupMenuKitP(void) 
    : bba(SbViewportRegion(100,100)),
      rpa(SbViewportRegion(100,100))
  { }
  SoFieldSensor * itemssensor;
  SoOneShotSensor * oneshot;
  SoSearchAction sa;
  SoGetBoundingBoxAction bba;
  SoRayPickAction rpa;
  SbViewportRegion vp;

  float bbw;
  float bbh;

  SbBool flipleftright;
  SbBool flipupdown;
  int fontsize;

  void updateViewport(SoState * state) {
    const SbViewportRegion & vp = SoViewportRegionElement::get(state);
    
    if (vp.getViewportSizePixels() != this->vp.getViewportSizePixels()) {
      this->vp = vp;
      this->oneshot->schedule();
    }
  }
};

#define PRIVATE(obj) (obj)->pimpl

SO_KIT_SOURCE(SmPopupMenuKit);

/*!
  Constructor. 
*/
SmPopupMenuKit::SmPopupMenuKit(void)
{
  PRIVATE(this) = new SmPopupMenuKitP;

  SO_KIT_CONSTRUCTOR(SmPopupMenuKit);
  
  SO_KIT_ADD_FIELD(isActive, (FALSE));
  SO_KIT_ADD_FIELD(items, (""));
  SO_KIT_ADD_FIELD(frameSize, (3));
  SO_KIT_ADD_FIELD(offset, (16, 0));
  
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(resetTransform, SoResetTransform, FALSE, topSeparator, depthBuffer, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(depthBuffer, DepthBuffer, TRUE, topSeparator, lightModel, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(lightModel, SoLightModel, FALSE, topSeparator, camera, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(camera, SoOrthographicCamera, FALSE, topSeparator, texture, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(texture, SoTexture2, FALSE, topSeparator, shapeHints, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(shapeHints, SoShapeHints, FALSE, topSeparator, pickStyle, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(pickStyle, SoPickStyle, TRUE, topSeparator, materialBinding, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(materialBinding, SoMaterialBinding, TRUE, topSeparator, backgroundColor, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(backgroundColor, SoBaseColor, TRUE, topSeparator, justification, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(justification, SoTranslation, TRUE, topSeparator, backgroundShape, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(backgroundShape, SoFaceSet, TRUE, topSeparator, position, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(position, SoTranslation, TRUE, topSeparator, textColor, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textColor, SoBaseColor, TRUE, topSeparator, textPickStyle, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textPickStyle, SoPickStyle, TRUE, topSeparator, textShape, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textShape, SoText2, TRUE, topSeparator, "", FALSE);

  SO_KIT_INIT_INSTANCE();

  SoOrthographicCamera * cam  = (SoOrthographicCamera*) this->getAnyPart("camera", TRUE);
  cam->height = 1.0f;
  cam->position = SbVec3f(0.5f, 0.5f, 0.0f);
  cam->viewportMapping = SoCamera::LEAVE_ALONE;
  cam->nearDistance = 0.0f;
  cam->farDistance = 2.0f;

  // disable picking on the text2-node
  SoPickStyle * ps = (SoPickStyle*) this->getAnyPart("textPickStyle", TRUE);
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
  col->rgb = SbColor(0.8f, 0.8f, 0.8f);
  
  PRIVATE(this)->itemssensor = new SoFieldSensor(items_changed_cb, this);
  PRIVATE(this)->itemssensor->attach(&this->items);
  PRIVATE(this)->itemssensor->setPriority(1);
  PRIVATE(this)->flipleftright = FALSE;
  PRIVATE(this)->flipupdown = FALSE;
  PRIVATE(this)->fontsize = 12; // FIXME: test in GLRender() for current font

  PRIVATE(this)->oneshot = new SoOneShotSensor(oneshot_cb, this);
  PRIVATE(this)->oneshot->setPriority(1);
}

/*!
  Destructor.
*/
SmPopupMenuKit::~SmPopupMenuKit(void)
{
  delete PRIVATE(this)->itemssensor;
  delete PRIVATE(this)->oneshot;
  delete PRIVATE(this);
}

// Documented in superclass
void
SmPopupMenuKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SmPopupMenuKit, SoBaseKit, "BaseKit");
  }
}

// Documented in superclass
void 
SmPopupMenuKit::GLRender(SoGLRenderAction * action)
{
  if (!this->isActive.getValue()) {
    return;
  }

  PRIVATE(this)->updateViewport(action->getState());

  if (!action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());
    return;
  }
  SoState *state = action->getState();
  state->push();
  SoDrawStyleElement::set(state, SoDrawStyleElement::FILLED);
  SoComplexityTypeElement::set(state, this, SoComplexityTypeElement::getDefault());
  SoComplexityElement::set(state, this, SoComplexityElement::getDefault());
  inherited::GLRender(action);
  state->pop();
}

void 
SmPopupMenuKit::handleEvent(SoHandleEventAction * action)
{
  PRIVATE(this)->updateViewport(action->getState());

#if 0 // testing menu picking
  const SoEvent * event = action->getEvent();
  PRIVATE(this)->rpa.setViewportRegion(PRIVATE(this)->vp);
  PRIVATE(this)->rpa.setPoint(event->getPosition());
  PRIVATE(this)->rpa.setPickAll(FALSE);
  PRIVATE(this)->rpa.apply(this->topSeparator.getValue());
  
  SoPickedPoint * pp = PRIVATE(this)->rpa.getPickedPoint();
  if (pp) {
    fprintf(stderr,"pp!!\n");
  }
#endif

  inherited::handleEvent(action);
}

void 
SmPopupMenuKit::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SoCacheElement::invalidate(action->getState());
  SoNode::getBoundingBox(action);
}

void 
SmPopupMenuKit::search(SoSearchAction * action)
{
  // don't search under this nodekit
  SoNode::search(action);
}

void 
SmPopupMenuKit::callback(SoCallbackAction * action)
{
  SoNode::callback(action);
}

void 
SmPopupMenuKit::getMatrix(SoGetMatrixAction * action) 
{ 
  SoNode::getMatrix(action);
}

void 
SmPopupMenuKit::pick(SoPickAction * action) 
{
  SoNode::pick(action);
}

void
SmPopupMenuKit::rayPick(SoRayPickAction * action) 
{ 
  SoNode::rayPick(action);
}

void 
SmPopupMenuKit::audioRender(SoAudioRenderAction * action) 
{
  SoNode::audioRender(action);
}

void 
SmPopupMenuKit::getPrimitiveCount(SoGetPrimitiveCountAction * action) 
{
  SoNode::getPrimitiveCount(action);
}

/*!  
  Sets the current viewport region. The viewport region will also
  be picked up when the node is travered with SoGLRenderAction.
*/
void 
SmPopupMenuKit::setViewportRegion(const SbViewportRegion & vp)
{
  PRIVATE(this)->vp = vp;
  SoText2 * t = (SoText2*) this->getAnyPart("textShape", FALSE);
  if (t && t->string.getNum() >= 1 && t->string[0].getLength()) { 
    this->updateBackground();
  }
}

/*!
  Convenience function that uses an SoPickedPoint to calculate the
  position of the tooltip.
*/
void 
SmPopupMenuKit::setPickedPoint(const SoPickedPoint * pp, const SbViewportRegion & vp)
{
  if (pp == NULL) {
    this->isActive = FALSE;
    return;
  }

  PRIVATE(this)->vp = vp;
  SbBool oldsearch = SoBaseKit::isSearchingChildren();
  SoBaseKit::setSearchingChildren(TRUE);

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
    npt[2] = 0.0f;

    PRIVATE(this)->flipupdown = FALSE;
    PRIVATE(this)->flipleftright = FALSE;
    if (npt[0] > 0.5) PRIVATE(this)->flipleftright = TRUE;
    if (npt[1] < 0.5) PRIVATE(this)->flipupdown = TRUE;
    
    SoTranslation * t = (SoTranslation*) this->getAnyPart("position", TRUE);
    t->translation = npt;
    this->updateBackground();

    SbVec2s of = this->offset.getValue();
    SbVec3f fof(0.0f, 0.0f, 0.0f);
    fof[0] = float(of[0]) / float(PRIVATE(this)->vp.getViewportSizePixels()[0]);
    fof[1] = float(of[1]) / float(PRIVATE(this)->vp.getViewportSizePixels()[1]);

    if (PRIVATE(this)->flipleftright) fof[0] = -fof[0];
    if (PRIVATE(this)->flipupdown) fof[1] = -fof[1];
    
    fof[1] -= float(PRIVATE(this)->fontsize) /  float(PRIVATE(this)->vp.getViewportSizePixels()[1]);

    SbVec3f j(0.0f, 0.0f, 0.0f);
    if (PRIVATE(this)->flipleftright) j[0] = -PRIVATE(this)->bbw;
    if (PRIVATE(this)->flipupdown) j[1] = PRIVATE(this)->bbh;

    j[0] += fof[0];
    j[1] += fof[1];
    t->translation = npt + j;    

    // calculate again to account for offset and justification
    this->updateBackground();

    this->isActive = TRUE;
  }
  else {
    this->isActive = FALSE;
  }
  PRIVATE(this)->sa.reset();
  SoBaseKit::setSearchingChildren(oldsearch);

}

void 
SmPopupMenuKit::items_changed_cb(void * closure, SoSensor * s)
{
  SmPopupMenuKit * thisp = (SmPopupMenuKit*) closure;
  SoText2 * text = (SoText2*) thisp->getAnyPart("textShape", TRUE);
  text->string = thisp->items;
  thisp->updateBackground();
}

void 
SmPopupMenuKit::oneshot_cb(void * closure, SoSensor * s)
{
  SmPopupMenuKit * thisp = (SmPopupMenuKit*) closure;
  thisp->updateBackground();
}

void 
SmPopupMenuKit::updateBackground(void)
{
  SoPath * p = new SoPath(this->topSeparator.getValue());
  p->ref();
  p->append(this->textShape.getValue());

  SoFaceSet * fs = (SoFaceSet*) this->getAnyPart("backgroundShape", TRUE);
  fs->numVertices = 0;

  PRIVATE(this)->bba.setViewportRegion(PRIVATE(this)->vp);
  PRIVATE(this)->bba.apply(p);
  SbBox3f bb = PRIVATE(this)->bba.getBoundingBox();
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

  PRIVATE(this)->bbw = bmax[0] - bmin[0];
  PRIVATE(this)->bbh = bmax[1] - bmin[1];

  SbVec3f varray[4];
  varray[0] = SbVec3f(bmin[0], bmin[1], 0.0f);
  varray[1] = SbVec3f(bmax[0], bmin[1], 0.0f);
  varray[2] = SbVec3f(bmax[0], bmax[1], 0.0f);
  varray[3] = SbVec3f(bmin[0], bmax[1], 0.0f);
  vp->vertex.setValues(0, 4, varray);
}

#undef PRIVATE

