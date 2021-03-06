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
  \class SmTooltipKit SmTooltipKit.h
  \brief The SmTooltipKit class... 
  \ingroup nodekits

  FIXME: doc
*/

/*!
  \var SoSFBool SmTooltipKit::autoTrigger
  
  When TRUE, will search for SmTooltip nodes in front of shapes and
  pop up a tooltip when the mouse pointer has been resting over an
  object for over autoTriggerTime seconds. Default value is FALSE.
*/

/*!
  \var SoSFBool SmTooltipKit::autoTriggerTime
  
  The time before the tooltip triggers. Default value is 1.0.
*/

/*!
  \var SoSFBool SmTooltipKit::isActive
  
  TRUE when tooltip is active, FALSE otherwise.
*/

/*!
  \var SoMFString SmTooltipKit::description

  The tooltip string.
*/

/*!
  \var SoSFInt32 SmTooltipKit::frameSize

  The amount of extra pixels around the frame. The default value is 3.

*/

#include "SmTooltipKit.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/sensors/SoAlarmSensor.h>
#include <Inventor/nodes/SoSeparator.h>
#include <SmallChange/nodes/SmDepthBuffer.h>
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
#include <SmallChange/nodes/SmTooltip.h>
#include <Inventor/elements/SoComplexityTypeElement.h>
#include <Inventor/elements/SoComplexityElement.h>

class SmTooltipKitP {
public:
  SmTooltipKitP(void) 
    : bba(SbViewportRegion(100,100)),
      rpa(SbViewportRegion(100,100))
  { }
  SoFieldSensor * tooltipsensor;
  SoSearchAction sa;
  SoGetBoundingBoxAction bba;
  SoRayPickAction rpa;
  SbViewportRegion vp;
  SoAlarmSensor * alarm;
  SoNode * alarm_root;
  SbVec2f alarm_pos;

  float bbw;
  float bbh;

  SbBool flipleftright;
  SbBool flipupdown;
  int fontsize;
  SbBool first;
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
  
  SO_KIT_ADD_FIELD(autoTrigger, (FALSE));
  SO_KIT_ADD_FIELD(autoTriggerTime, (1.0));
  SO_KIT_ADD_FIELD(isActive, (FALSE));
  SO_KIT_ADD_FIELD(description, (""));
  SO_KIT_ADD_FIELD(frameSize, (3));
  SO_KIT_ADD_FIELD(offset, (16, 0));
  
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(resetTransform, SoResetTransform, FALSE, topSeparator, depthBuffer, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(depthBuffer, SmDepthBuffer, TRUE, topSeparator, lightModel, TRUE);
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
  SmDepthBuffer * db = (SmDepthBuffer*) this->getAnyPart("depthBuffer", TRUE);
  db->enable = FALSE;
  
  // set justification
  SoText2 * text = (SoText2*) this->getAnyPart("textShape", TRUE);
  text->justification = SoText2::LEFT;

  SoBaseColor * col = (SoBaseColor*) this->getAnyPart("textColor", TRUE);
  col->rgb = SbColor(0.0f, 0.0f, 0.0f);

  col = (SoBaseColor*) this->getAnyPart("backgroundColor", TRUE);
  col->rgb = SbColor(1.0f, 1.0f, 0.0f);

  PRIVATE(this)->tooltipsensor = new SoFieldSensor(tooltip_changed_cb, this);
  PRIVATE(this)->tooltipsensor->attach(&this->description);
  PRIVATE(this)->tooltipsensor->setPriority(1);
  PRIVATE(this)->alarm = new SoAlarmSensor(alarm_cb, this);
  PRIVATE(this)->alarm_root = NULL;
  PRIVATE(this)->flipleftright = FALSE;
  PRIVATE(this)->flipupdown = FALSE;
  PRIVATE(this)->fontsize = 12; // FIXME: test in GLRender() for current font
  PRIVATE(this)->first = TRUE;
}

/*!
  Destructor.
*/
SmTooltipKit::~SmTooltipKit(void)
{
  delete PRIVATE(this)->alarm;
  if (PRIVATE(this)->alarm_root) {
    PRIVATE(this)->alarm_root->unref();
  }
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

  SoState *state = action->getState();

  // ignore when rendering wireframe overlay
  if (SoDrawStyleElement::get(state) != SoDrawStyleElement::FILLED) return;

  if (!this->isActive.getValue()) return;

  if (!action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());
    return;
  }
  state->push();
  SoDrawStyleElement::set(state, SoDrawStyleElement::FILLED);
  SoComplexityTypeElement::set(state, this, SoComplexityTypeElement::getDefault());
  SoComplexityElement::set(state, this, SoComplexityElement::getDefault());
  inherited::GLRender(action);
  state->pop();
}

SbBool 
SmTooltipKit::affectsState(void) const
{
  // important since we might add delayed paths to SoGLRenderAction
  return FALSE;
}

void 
SmTooltipKit::handleEvent(SoHandleEventAction * action)
{
  PRIVATE(this)->vp = SoViewportRegionElement::get(action->getState());
  if (this->autoTrigger.getValue()) {
    const SoEvent * ev = action->getEvent();
    if (ev->isOfType(SoLocation2Event::getClassTypeId())) {
      if (this->isActive.getValue()) this->isActive = FALSE;
      if (PRIVATE(this)->alarm->isScheduled()) {
        PRIVATE(this)->alarm->unschedule();
        assert(PRIVATE(this)->alarm_root);
        PRIVATE(this)->alarm_root->unref();
        PRIVATE(this)->alarm_root = NULL;
      }
      PRIVATE(this)->alarm_root = action->getPickRoot();
      PRIVATE(this)->alarm_root->ref();
      PRIVATE(this)->alarm->setTimeFromNow(SbTime(this->autoTriggerTime.getValue()));
      PRIVATE(this)->alarm_pos = ev->getNormalizedPosition(PRIVATE(this)->vp);
      PRIVATE(this)->alarm->schedule();
    }
  }
  inherited::handleEvent(action);
}

void 
SmTooltipKit::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SoCacheElement::invalidate(action->getState());
  SoNode::getBoundingBox(action);
}

void 
SmTooltipKit::search(SoSearchAction * action)
{
  // don't search under this nodekit
  SoNode::search(action);
}

void 
SmTooltipKit::callback(SoCallbackAction * action)
{
  SoNode::callback(action);
}

void 
SmTooltipKit::getMatrix(SoGetMatrixAction * action) 
{ 
  SoNode::getMatrix(action);
}

void 
SmTooltipKit::pick(SoPickAction * action) 
{
  SoNode::pick(action);
}

void
SmTooltipKit::rayPick(SoRayPickAction * action) 
{ 
  SoNode::rayPick(action);
}

void 
SmTooltipKit::audioRender(SoAudioRenderAction * action) 
{
  SoNode::audioRender(action);
}

void 
SmTooltipKit::getPrimitiveCount(SoGetPrimitiveCountAction * action) 
{
  SoNode::getPrimitiveCount(action);
}

/*!  
  Sets the current viewport region. The viewport region will also
  be picked up when the node is traversed with SoGLRenderAction.
*/
void 
SmTooltipKit::setViewportRegion(const SbViewportRegion & vp)
{
  PRIVATE(this)->vp = vp;
  SoText2 * t = (SoText2*) this->getAnyPart("textShape", FALSE);
  if (t && t->string.getNum() >= 1 && t->string[0].getLength()) { 
    this->updateBackground();
  }
}

void 
SmTooltipKit::setPickedPosition(const SbVec2f & np, const SbViewportRegion & vp)
{
  PRIVATE(this)->vp = vp;
  SbVec3f npt(np[0], np[1], 0.0f);

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
  if (PRIVATE(this)->first) {
    PRIVATE(this)->first = FALSE;
    this->setPickedPosition(np, vp);
  }
  this->isActive = TRUE;

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

    this->setPickedPosition(SbVec2f(npt[0], npt[1]), vp);
  }
  else {
    this->isActive = FALSE;
  }
  PRIVATE(this)->sa.reset();
  SoBaseKit::setSearchingChildren(oldsearch);

}

void 
SmTooltipKit::alarm_cb(void * closure, SoSensor * s)
{
  SmTooltipKit * thisp = (SmTooltipKit*) closure;
  assert(PRIVATE(thisp)->alarm_root);
  
  PRIVATE(thisp)->rpa.setViewportRegion(PRIVATE(thisp)->vp);
  PRIVATE(thisp)->rpa.setPickAll(FALSE);
  PRIVATE(thisp)->rpa.setNormalizedPoint(PRIVATE(thisp)->alarm_pos);
  PRIVATE(thisp)->rpa.apply(PRIVATE(thisp)->alarm_root);
  
  SoPickedPoint * pp = PRIVATE(thisp)->rpa.getPickedPoint();
  if (pp) {
    PRIVATE(thisp)->sa.setType(SmTooltip::getClassTypeId());
    PRIVATE(thisp)->sa.setInterest(SoSearchAction::LAST);
    PRIVATE(thisp)->sa.apply(pp->getPath());
    if (PRIVATE(thisp)->sa.getPath()) {
      SoFullPath * p = (SoFullPath*) PRIVATE(thisp)->sa.getPath();
      SmTooltip * tooltip = (SmTooltip*) p->getTail();
      thisp->description = tooltip->description;
      thisp->setPickedPoint(pp, PRIVATE(thisp)->vp);
    }
    PRIVATE(thisp)->sa.reset();
  }
 
  PRIVATE(thisp)->alarm_root->unref();
  PRIVATE(thisp)->alarm_root = NULL;
}

void 
SmTooltipKit::tooltip_changed_cb(void * closure, SoSensor * s)
{
  SmTooltipKit * thisp = (SmTooltipKit*) closure;
  thisp->updateBackground();
}

void 
SmTooltipKit::updateBackground(void)
{
  SoText2 * text = (SoText2*) this->getAnyPart("textShape", TRUE);
  text->string = this->description;

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
