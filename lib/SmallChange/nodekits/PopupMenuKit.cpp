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
  \var SoMFString SmPopupMenuKit::itemList

  The menu items
*/

/*!
  \var SoMFString SmPopupMenuKit::itemData

  Node data for each item. If an item contains an SmPopupMenuKit, a
  sub menu will be opened when the user selects this item.
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
#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoTransparencyType.h>
#include <SmallChange/nodes/DepthBuffer.h>
#include <Inventor/nodes/SoResetTransform.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SoFullPath.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoDrawStyleElement.h>
#include <Inventor/elements/SoOverrideElement.h>
#include <Inventor/elements/SoTextureOverrideElement.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/elements/SoComplexityTypeElement.h>
#include <Inventor/elements/SoComplexityElement.h>
#include <Inventor/nodes/SoMarkerSet.h>

static int (*schemescriptcb)(const char *);
static void (*schemefilecb)(const char *);

class SmOpenSubData {
public:
  SmPopupMenuKit * kit;
  SmPopupMenuKit * sub;
  SbVec2f np;
};

class SmPopupMenuKitP {
public:
  SmPopupMenuKitP(SmPopupMenuKit * master) 
    : master(master),
      bba(SbViewportRegion(100,100)),
      rpa(SbViewportRegion(100,100))
  { }
  SmPopupMenuKit * master;
  SmPopupMenuKit * parent;

  SoOneShotSensor * triggerscriptsensor;
  int triggeritem;

  SoFieldSensor * itemssensor;
  SoFieldSensor * isactivesensor;
  SoOneShotSensor * oneshot;
  SoOneShotSensor * activeitemchanged;
  SoSearchAction sa;
  SoGetBoundingBoxAction bba;
  SoRayPickAction rpa;
  SbViewportRegion vp;

  float bbw;
  float bbh;

  SbBool flipleftright;
  SbBool flipupdown;
  float fontsize;
  SbVec3f padding;

  float backgroundmenulow;
  float backgroundmenuhigh;
  float backgroundleft;
  float backgroundright;

  float activetransp;
  float inactivetransp;
  
  int activeitem;
  int submenumarkeridx;

  SmPopupMenuKit * getSubMenu(const int idx) {
    if (idx < master->itemData.getNum()) {
      SoNode * n = master->itemData[idx];
      if (n && n->isOfType(SmPopupMenuKit::getClassTypeId())) {
        return (SmPopupMenuKit*) n;
      }
    }
    return NULL;
  }

  void updateViewport(SoState * state) {
    const SbViewportRegion & vp = SoViewportRegionElement::get(state);
    
    if (vp.getViewportSizePixels() != this->vp.getViewportSizePixels()) {
      this->vp = vp;
      this->oneshot->schedule();
    }
  }

  void closeSubMenues(void);
  void buildTextScenegraph(void);
  static int addSubmenuMarker(); // Creates a right-pointing arrow

};

#define PRIVATE(obj) (obj)->pimpl
#define PUBLIC(obj) (obj)->master

SO_KIT_SOURCE(SmPopupMenuKit);

/*!
  Constructor. 
*/
SmPopupMenuKit::SmPopupMenuKit(void)
{
  PRIVATE(this) = new SmPopupMenuKitP(this);
  PRIVATE(this)->parent = NULL;

  SO_KIT_CONSTRUCTOR(SmPopupMenuKit);
  
  SO_KIT_ADD_FIELD(isActive, (FALSE));
  SO_KIT_ADD_FIELD(visible, (FALSE));
  SO_KIT_ADD_FIELD(itemList, (""));
  SO_KIT_ADD_FIELD(itemSchemeScript, (""));
  SO_KIT_ADD_FIELD(itemData, (NULL));
  SO_KIT_ADD_FIELD(itemDisabled, (FALSE));
  SO_KIT_ADD_FIELD(itemTagged, (FALSE));
  SO_KIT_ADD_FIELD(menuTitle, (""));
  SO_KIT_ADD_FIELD(frameSize, (4));
  SO_KIT_ADD_FIELD(offset, (0, 0));
  SO_KIT_ADD_FIELD(spacing, (1.5f));
  SO_KIT_ADD_FIELD(pickedItem, (-1));
  SO_KIT_ADD_FIELD(closeParent, (FALSE));
  
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(transparencyType, SoTransparencyType, TRUE, topSeparator, resetTransform, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(resetTransform, SoResetTransform, FALSE, topSeparator, depthBuffer, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(depthBuffer, DepthBuffer, TRUE, topSeparator, lightModel, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(lightModel, SoLightModel, FALSE, topSeparator, camera, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(camera, SoOrthographicCamera, FALSE, topSeparator, texture, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(texture, SoTexture2, FALSE, topSeparator, shapeHints, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(shapeHints, SoShapeHints, FALSE, topSeparator, pickStyle, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(pickStyle, SoPickStyle, TRUE, topSeparator, materialBinding, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(materialBinding, SoMaterialBinding, TRUE, topSeparator, backgroundMaterial, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(backgroundMaterial, SoMaterial, TRUE, topSeparator, backgroundTexture, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(backgroundTexture, SoTexture2, TRUE, topSeparator, justification, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(justification, SoTranslation, TRUE, topSeparator, backgroundShape, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(backgroundShape, SoFaceSet, TRUE, topSeparator, textSeparator, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(textSeparator, SoSeparator, TRUE, topSeparator, activeMaterial, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(position, SoTranslation, TRUE, textSeparator, textColor, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textColor, SoBaseColor, TRUE, textSeparator, textPickStyle, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textPickStyle, SoPickStyle, TRUE, textSeparator, itemSeparator, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textFont, SoFont, TRUE, textSeparator, itemSeparator, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(itemSeparator, SoSeparator, TRUE, textSeparator, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(activeMaterial, SoMaterial, TRUE, topSeparator, activeShape, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(activeShape, SoFaceSet, TRUE, topSeparator, borderShape, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(borderShape, SoIndexedFaceSet, TRUE, topSeparator, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(titleSeparator, SoSeparator, TRUE, textSeparator, "", FALSE);



  SO_KIT_INIT_INSTANCE();

  SoOrthographicCamera * cam  = (SoOrthographicCamera *) this->getAnyPart("camera", TRUE);
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

  // Set font, fetch size  
  SoFont * font = (SoFont *) this->getAnyPart("textFont", TRUE);  
  font->name.setValue("Verdana");
  font->size.setValue(12);
  PRIVATE(this)->fontsize = font->size.getValue();
  
  SoSeparator * itemsep = (SoSeparator *) this->getAnyPart("itemSeparator", TRUE);
  itemsep->ref();
  
  SoBaseColor * col = (SoBaseColor*) this->getAnyPart("textColor", TRUE);
  col->rgb = SbColor(0.0f, 0.0f, 0.0f);

  SoMaterial * amat = (SoMaterial*) this->getAnyPart("activeMaterial", TRUE);
  amat->diffuseColor.setNum(2);
  amat->diffuseColor.set1Value(0, SbColor(0.0f, 0.0f, 0.0f));
  amat->diffuseColor.set1Value(1, SbColor(1.0f, 1.0f, 1.0f));
  amat->transparency = 0.5f;

  SoTransparencyType * tt = (SoTransparencyType*) 
    this->getAnyPart("transparencyType", TRUE);
  tt->value = SoTransparencyType::BLEND;
  
  PRIVATE(this)->itemssensor = new SoFieldSensor(items_changed_cb, this);
  PRIVATE(this)->itemssensor->attach(&this->itemList);
  PRIVATE(this)->itemssensor->setPriority(1);
  PRIVATE(this)->flipleftright = FALSE;
  PRIVATE(this)->flipupdown = FALSE;

  PRIVATE(this)->backgroundmenulow = 0.0f;
  PRIVATE(this)->backgroundmenuhigh = 0.0f;
  PRIVATE(this)->backgroundleft = 0.0f;
  PRIVATE(this)->backgroundright = 0.0f;
  PRIVATE(this)->activeitem = -1;

  PRIVATE(this)->oneshot = new SoOneShotSensor(oneshot_cb, this);
  PRIVATE(this)->oneshot->setPriority(1);

  PRIVATE(this)->activeitemchanged = new SoOneShotSensor(activeitemchanged_cb, this);  

  PRIVATE(this)->isactivesensor = new SoFieldSensor(isactive_cb, this);
  PRIVATE(this)->isactivesensor->attach(&this->isActive);
  PRIVATE(this)->padding = SbVec3f(7, 7, 1);
  PRIVATE(this)->triggerscriptsensor = new SoOneShotSensor(trigger_cb, this);

  PRIVATE(this)->activetransp = 0.0f;
  PRIVATE(this)->inactivetransp = 0.1f;
  
  // Create a new better-looking submenu marker
  PRIVATE(this)->submenumarkeridx = SmPopupMenuKitP::addSubmenuMarker();

}

/*!
  Destructor.
*/
SmPopupMenuKit::~SmPopupMenuKit(void)
{
  this->setParent(NULL);
  delete PRIVATE(this)->triggerscriptsensor;
  delete PRIVATE(this)->isactivesensor;
  delete PRIVATE(this)->activeitemchanged;
  delete PRIVATE(this)->itemssensor;
  delete PRIVATE(this)->oneshot;
  delete PRIVATE(this);
}

void 
SmPopupMenuKit::setParent(SmPopupMenuKit * kit)
{
  if (PRIVATE(this)->parent) {
    PRIVATE(this)->parent->unref();
  }
  PRIVATE(this)->parent = kit;
  if (kit) kit->ref();
}

void 
SmPopupMenuKit::childFinished(SmPopupMenuKit * child)
{
  if (child->closeParent.getValue()) {
    this->visible = FALSE;
    this->isActive = FALSE;
    PRIVATE(this)->activeitem = -1;
    
    if (PRIVATE(this)->parent) {
      PRIVATE(this)->parent->childFinished(this);
      this->setParent(NULL);
    }
  }
  else {
    this->isActive = TRUE;
  }
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

SbBool 
SmPopupMenuKit::affectsState(void) const
{
  // important for delayed paths to work correctly
  return FALSE;
}


// Documented in superclass
void 
SmPopupMenuKit::GLRender(SoGLRenderAction * action)
{
  PRIVATE(this)->updateViewport(action->getState());
  if (!this->visible.getValue()) {
    return;
  }
  if (!action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());
    return;
  }
  
  SoState *state = action->getState();
  state->push();
  // set elements directly to overwrite overrided elements
  SoDrawStyleElement::set(state, SoDrawStyleElement::FILLED);
  SoComplexityTypeElement::set(state, this, SoComplexityTypeElement::getDefault());
  SoComplexityElement::set(state, this, SoComplexityElement::getDefault());
  SoOverrideElement::setDiffuseColorOverride(state, this, FALSE);
  SoOverrideElement::setMaterialBindingOverride(state, this, FALSE);
  SoOverrideElement::setPolygonOffsetOverride(state, this, FALSE);
  SoTextureOverrideElement::setImageOverride(state, FALSE);
  SoTextureOverrideElement::setQualityOverride(state, FALSE);
  inherited::GLRender(action);
  state->pop();
}

void 
SmPopupMenuKit::handleEvent(SoHandleEventAction * action)
{
  int activeitem = -1;
  
  SbBool handled = FALSE;

  if (this->isActive.getValue() && this->visible.getValue()) {  
    handled = TRUE;
    PRIVATE(this)->updateViewport(action->getState());
    
    const SoEvent * event = action->getEvent();
    PRIVATE(this)->rpa.setViewportRegion(PRIVATE(this)->vp);
    PRIVATE(this)->rpa.setPoint(event->getPosition());
    PRIVATE(this)->rpa.setPickAll(FALSE);
    PRIVATE(this)->rpa.apply(this->topSeparator.getValue());
    
    SoPickedPoint * pp = PRIVATE(this)->rpa.getPickedPoint();
    if (pp) {
      SbVec3f p = pp->getObjectPoint();
      p[1] -= PRIVATE(this)->backgroundmenulow;
      p[1] *= PRIVATE(this)->vp.getViewportSizePixels()[1];
      p[1] /= PRIVATE(this)->fontsize * this->spacing.getValue();
      
      if (this->itemList.getNum()) {
        int idx = (int) p[1];
        activeitem = (this->itemList.getNum()-1) - idx;
        if (activeitem >= this->itemList.getNum())
          activeitem = this->itemList.getNum()-1;
      }
    }

    if (activeitem >= 0 && SO_MOUSE_RELEASE_EVENT(event, ANY)) {
      SmPopupMenuKit * sub = PRIVATE(this)->getSubMenu(activeitem);
      
      if (sub) {
        this->isActive = FALSE;
        activeitem = -1;
        SbVec2f np;
        
        np[0] = PRIVATE(this)->backgroundright + (6.0f / PRIVATE(this)->vp.getViewportSizePixels()[0]);
        np[1] = event->getPosition()[1] / float (PRIVATE(this)->vp.getViewportSizePixels()[1]);

        SmOpenSubData * data = new SmOpenSubData;
        SoOneShotSensor * sensor = new SoOneShotSensor(opensub_cb, data);
        data->kit = this;
        data->sub = sub;
        data->np = np;
        sensor->schedule();
      }
      else {
        if ((this->itemList[activeitem] != "-separator") &&
            (!this->itemDisabled[activeitem])){          
          this->itemPicked(activeitem);
          if (SO_MOUSE_RELEASE_EVENT(event, BUTTON1)) {
            activeitem = -1;
            this->isActive = FALSE;
            this->visible = FALSE;
            if (PRIVATE(this)->parent) {
              PRIVATE(this)->parent->childFinished(this);
              this->setParent(NULL);
            }            
          }
        }
      }
    }
    else if (activeitem < 0 && SO_MOUSE_RELEASE_EVENT(event, ANY)) {
      handled = TRUE;
      this->isActive = FALSE;
      this->visible = FALSE;
      if (PRIVATE(this)->parent) {
        PRIVATE(this)->parent->isActive = TRUE;
        PRIVATE(this)->parent->visible = TRUE;
        // Check if the click missed the parent menu aswell
        PRIVATE(this)->parent->handleEvent(action);
        this->setParent(NULL);
      }           
    }
  }

  if (activeitem != PRIVATE(this)->activeitem) {
    PRIVATE(this)->activeitem = activeitem;
    PRIVATE(this)->activeitemchanged->schedule();
  }
  
  if (handled) action->setHandled();
  else inherited::handleEvent(action);
}

void 
SmPopupMenuKit::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SoCacheElement::invalidate(action->getState());
  SoNode::getBoundingBox(action);
}

void
SmPopupMenuKitP::closeSubMenues()
{
  for (int i=0;i<master->itemData.getNum();++i) {
    SoNode * n = master->itemData[i];
    if (n && n->isOfType(SmPopupMenuKit::getClassTypeId())) {
      ((SmPopupMenuKit*) n)->visible = FALSE;
      ((SmPopupMenuKit*) n)->isActive = FALSE;
    }
  }
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

void 
SmPopupMenuKit::itemPicked(const int idx)
{
  this->pickedItem = idx;
  
  if (schemescriptcb && schemefilecb) {
    if (!PRIVATE(this)->triggerscriptsensor->isScheduled()) {
      PRIVATE(this)->triggeritem = idx;
      PRIVATE(this)->triggerscriptsensor->schedule();
    }
  }
}

/*!  
  Sets the current viewport region. The viewport region will also
  be picked up when the node is travered with SoGLRenderAction.
*/
void 
SmPopupMenuKit::setViewportRegion(const SbViewportRegion & vp)
{
  PRIVATE(this)->vp = vp;
  this->updateBackground();
}

void
SmPopupMenuKit::setNormalizedPosition(const SbVec2f & npt)
{
  PRIVATE(this)->flipupdown = FALSE;
  PRIVATE(this)->flipleftright = FALSE;

  SbBool finished = FALSE;

  do {  
    SoTranslation * t = (SoTranslation*) this->getAnyPart("position", TRUE);
    t->translation = SbVec3f(npt[0], npt[1], 0.0f);
    
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

    t->translation = SbVec3f(npt[0], npt[1], 0.0f) + j;

    this->updateBackground();  
    
    // check if parts of the menu is outside the window
    finished = TRUE;

    if (PRIVATE(this)->backgroundmenuhigh > 1.0f) {
      t->translation = SbVec3f(npt[0], npt[1] - (PRIVATE(this)->backgroundmenuhigh - 1.0f), 0.0f) + j;
      this->updateBackground();  
    }        
    else if (PRIVATE(this)->backgroundmenulow < 0.0f) {
      t->translation = SbVec3f(npt[0], npt[1] - PRIVATE(this)->backgroundmenulow, 0.0f) + j;
      this->updateBackground();  
    }    

    if (!PRIVATE(this)->flipleftright && PRIVATE(this)->backgroundright > 1.0f) {
      PRIVATE(this)->flipleftright = TRUE;
      finished = FALSE;
    }
  } while (!finished);

}


/*!
  Convenience function that uses an SoPickedPoint to calculate the
  position of the tooltip. pp == NULL will deactivate the menu.
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
  thisp->updateBackground();
}

void 
SmPopupMenuKit::oneshot_cb(void * closure, SoSensor * s)
{
  SmPopupMenuKit * thisp = (SmPopupMenuKit*) closure;
  thisp->updateBackground();
}

void 
SmPopupMenuKit::trigger_cb(void * closure, SoSensor * s)
{
  SmPopupMenuKit * thisp = (SmPopupMenuKit*) closure;
  int idx = PRIVATE(thisp)->triggeritem;
  
  if (idx >= 0 && idx < thisp->itemSchemeScript.getNum()) {
    SbString s = thisp->itemSchemeScript[idx];
    if (s.getLength()) {        
      if (s[0] == '(' || s[0] == ';') {
        schemescriptcb(s.getString());
      }
      else {
        schemefilecb(s.getString());
      }
    }
  }
}

void 
SmPopupMenuKit::isactive_cb(void * closure, SoSensor * s)
{
  SmPopupMenuKit * thisp = (SmPopupMenuKit*) closure;
  SoMaterial * mat = (SoMaterial*) thisp->getAnyPart("backgroundMaterial", TRUE);
  
  if (thisp->isActive.getValue()) {
    mat->transparency = PRIVATE(thisp)->activetransp;
    mat->diffuseColor = SbColor(0.6f, 0.6f, 0.6f);
  }
  else {
    mat->transparency = PRIVATE(thisp)->inactivetransp;
    mat->diffuseColor = SbColor(0.5f, 0.5f, 0.5f);
  }
}

void
SmPopupMenuKit::opensub_cb(void * closure, SoSensor * s)
{
  SmOpenSubData * data = (SmOpenSubData*) closure;
  SmPopupMenuKit * thisp = data->kit;
  SmPopupMenuKit * sub = data->sub;
  
  sub->setNormalizedPosition(data->np);
  sub->setParent(thisp);
  sub->visible = TRUE;
  sub->isActive = TRUE;

  delete s;
  delete data;
}

void 
SmPopupMenuKit::setTransparencies(float active, float inactive)
{
  PRIVATE(this)->activetransp = active;
  PRIVATE(this)->inactivetransp = inactive;
}

void 
SmPopupMenuKit::activeitemchanged_cb(void * closure, SoSensor * s)
{
  SmPopupMenuKit * thisp = (SmPopupMenuKit*) closure;
  thisp->updateActiveItem();
}

void 
SmPopupMenuKit::updateActiveItem(void)
{
  SoFaceSet * fs = (SoFaceSet*) this->getAnyPart("activeShape", TRUE);
  
  if (PRIVATE(this)->activeitem < 0) {
    this->setAnyPart("activeShape", NULL);
    return;
  }

  SoVertexProperty * vp = (SoVertexProperty*) fs->vertexProperty.getValue();
  if (!vp) {
    vp = new SoVertexProperty;
    fs->vertexProperty = vp;
  }

  float vecdata[4][3];
  
  vecdata[0][2] = 0.0f;
  vecdata[1][2] = 0.0f;
  vecdata[2][2] = 0.0f;
  vecdata[3][2] = 0.0f;

  vecdata[0][0] = PRIVATE(this)->backgroundleft;
  vecdata[1][0] = PRIVATE(this)->backgroundright;
  vecdata[2][0] = PRIVATE(this)->backgroundright;
  vecdata[3][0] = PRIVATE(this)->backgroundleft;
  
  float size = (PRIVATE(this)->fontsize * this->spacing.getValue()) / PRIVATE(this)->vp.getViewportSizePixels()[1];

  float offset;
  int item = PRIVATE(this)->activeitem;
  if (this->menuTitle.getValue().getLength() != 0) { // Take the title into account?
    offset = size * (item + 1.1f);
    ++item;
  }
  else {
    offset = (item  * size);
  }

  offset += + ((PRIVATE(this)->padding[1] - 2.0f/2) / PRIVATE(this)->vp.getViewportSizePixels()[1]);

  vecdata[0][1] = PRIVATE(this)->backgroundmenuhigh - (offset+size);
  vecdata[1][1] = PRIVATE(this)->backgroundmenuhigh - (offset+size);
  vecdata[2][1] = PRIVATE(this)->backgroundmenuhigh - offset;
  vecdata[3][1] = PRIVATE(this)->backgroundmenuhigh - offset;

#if 0
  fprintf(stderr,"active: %g %g, %g %g, %g %g, %g %g\n",
          vecdata[0][0],
          vecdata[0][1],
          vecdata[1][0],
          vecdata[1][1],
          vecdata[2][0],
          vecdata[2][1],
          vecdata[3][0],
          vecdata[3][1]);
#endif
  vp->vertex.setValues(0, 4, vecdata);
  fs->numVertices = 4;

}

void
SmPopupMenuKitP::buildTextScenegraph()
{
  
  SoSeparator * sep = (SoSeparator *) PUBLIC(this)->getAnyPart("itemSeparator", TRUE);
  sep->removeAllChildren();

  const SbVec2s pixelsize = this->vp.getViewportSizePixels();
  const float spacing = (PUBLIC(this)->spacing.getValue() * this->fontsize) / pixelsize[1];
 
  SoBaseColor * textcolor = (SoBaseColor *) PUBLIC(this)->getAnyPart("textColor", TRUE);  
  SoMaterial * graymaterial = new SoMaterial;
  graymaterial->diffuseColor.setValue(textcolor->rgb[0]);
  graymaterial->transparency = 0.5;

  SoTranslation * texttrans = new SoTranslation;
  texttrans->translation.setValue(0, -spacing, 0);
  SoTranslation * tagtrans = new SoTranslation;
  tagtrans->translation.setValue(- 5.0f / pixelsize[0], ((this->fontsize-1) / 2.0f) / pixelsize[1], 0);
  SoMarkerSet * tagmarker = new SoMarkerSet;
  tagmarker->markerIndex.setValue(SoMarkerSet::SQUARE_FILLED_5_5);

  SbList <SoSeparator *> lineseparatorlist;
  SbList <SoSeparator *> submenuitemlist;
  
  SoText2 * dummyseparatortext = new SoText2;
  dummyseparatortext->string = " "; // An 'invisible' string. Used for creating a correct bbox

  int i;
  for (i=0;i<PUBLIC(this)->itemList.getNum();++i) {
    SoSeparator * itemsep = new SoSeparator;

    if (PUBLIC(this)->itemList[i] == "-separator") {      
      // This is a line-separator
      lineseparatorlist.append(itemsep);
      itemsep->addChild(dummyseparatortext); // Must add this or the bbox becomes wrong...
    }       
    else if (PUBLIC(this)->itemDisabled[i]) {
      // Disabled text item
      SoText2 * textitem = new SoText2;
      textitem->spacing.setValue(PUBLIC(this)->spacing.getValue());
      textitem->string = PUBLIC(this)->itemList[i];
      itemsep->addChild(graymaterial);
      itemsep->addChild(textitem);
      SmPopupMenuKit * submenu = this->getSubMenu(i);
      if (submenu) submenuitemlist.append(itemsep);
    }
    else {      
      // Regular text item
      SoText2 * textitem = new SoText2;
      textitem->spacing.setValue(PUBLIC(this)->spacing.getValue());
      textitem->string = PUBLIC(this)->itemList[i];      
      itemsep->addChild(textitem);     
      SmPopupMenuKit * submenu = this->getSubMenu(i);
      if (submenu) submenuitemlist.append(itemsep);
      // FIXME: Some items seems to be tagged several times (not
      // visible, though)... (20050209 handegar)
      if (!submenu && PUBLIC(this)->itemTagged[i]) {
        itemsep->addChild(tagtrans);
        itemsep->addChild(tagmarker);        
      }
    }     
    sep->addChild(itemsep);
    sep->addChild(texttrans);
  }

  SoSeparator * titlesep = (SoSeparator *) PUBLIC(this)->getAnyPart("titleSeparator", TRUE);
  titlesep->removeAllChildren();

  // Add a menu title
  if (PUBLIC(this)->menuTitle.getValue().getLength() != 0) {
    SoSeparator * textsep = new SoSeparator;

    SoFont * menufont = (SoFont *) PUBLIC(this)->getAnyPart("textFont", TRUE);
    SoFont * titlefont = new SoFont;
    titlefont->size.setValue((float) ((int) (menufont->size.getValue()*1.2f)));
    titlefont->name.setValue("Verdana");

    SoTranslation * trans = new SoTranslation;
    trans->translation.setValue(0, spacing, 0); 

    SoBaseColor * graycolor = new SoBaseColor;
    graycolor->rgb.setValue(0.2f, 0.2f, 0.2f);
    SoText2 * title = new SoText2;
    title->string = PUBLIC(this)->menuTitle.getValue();
    textsep->addChild(trans);
    textsep->addChild(titlefont);
    textsep->addChild(graycolor);
    textsep->addChild(title);

    SoBaseColor * whitecolor = new SoBaseColor;
    whitecolor->rgb.setValue(1.0f, 1.0f, 1.0f);
    SoTranslation * pixelshift = new SoTranslation;
    pixelshift->translation.setValue(-1.0f/pixelsize[0], 1.0f/pixelsize[1], 0);
    textsep->addChild(pixelshift);
    textsep->addChild(whitecolor);
    textsep->addChild(title);
    titlesep->addChild(textsep);
  }


  // Calculate boundingbox for the menu
  SoPath * p = new SoPath(PUBLIC(this)->topSeparator.getValue());
  p->ref();
  p->append(PUBLIC(this)->textSeparator.getValue());  
  SoFaceSet * fs = (SoFaceSet*) PUBLIC(this)->getAnyPart("backgroundShape", TRUE);
  fs->numVertices = 0;  
  SbVec3f submenubulletpad(0,0,0);
  if (submenuitemlist.getLength() > 0) submenubulletpad = SbVec3f(6.0f / pixelsize[0], 0.0f, 0.0f);  
  this->bba.setViewportRegion(this->vp);
  this->bba.apply(p);
  SbBox3f bb = this->bba.getBoundingBox();
  SbVec3f pad = SbVec3f(this->padding[0] / pixelsize[0], this->padding[1] / pixelsize[1], 1);  
  bb.extendBy(bb.getMax() + pad + submenubulletpad); 
  bb.extendBy(bb.getMin() - pad);  
  p->unref();
  
  const float sepwidth = (bb.getMax()[0] - bb.getMin()[0]);

  if (lineseparatorlist.getLength() > 0 || 
      submenuitemlist.getLength() > 0) {

    // Add line separators
    SoMaterial * sepmaterial = new SoMaterial;
    sepmaterial->diffuseColor.setValue(0.0f, 0.0f, 0.0f);
    sepmaterial->transparency.setValue(0.8f);

    for (i=0;i<lineseparatorlist.getLength();++i) {
      SoSeparator * itemsep = lineseparatorlist[i];
      SoCoordinate3 * lcoord = new SoCoordinate3;
      lcoord->point.set1Value(0, SbVec3f(0, ((this->fontsize - 1) / 2.0f) / pixelsize[1], 0));
      lcoord->point.set1Value(1, SbVec3f(sepwidth, ((this->fontsize-1) / 2.0f) / pixelsize[1], 0));
      SoLineSet * lset = new SoLineSet;
      lset->numVertices.setValue(2);
      itemsep->addChild(sepmaterial);
      itemsep->addChild(lcoord);
      itemsep->addChild(lset);  
    }
    
    // Add submenu markers
    SoMarkerSet * marker = new SoMarkerSet;
    marker->markerIndex.setValue(this->submenumarkeridx);
    SoTranslation * trans = new SoTranslation;
    trans->translation.setValue(sepwidth - (6.0f / pixelsize[0]), 
                                ((this->fontsize - 4.5f)/2.0f) / pixelsize[1], 
                                0);
    
    for (i=0;i<submenuitemlist.getLength();++i) {
      SoSeparator * itemsep = submenuitemlist[i];      
      itemsep->addChild(graymaterial);
      itemsep->addChild(trans);
      itemsep->addChild(marker);
    }

    lineseparatorlist.truncate(0);
    submenuitemlist.truncate(0);    
  }
    

  
}

void 
SmPopupMenuKit::updateBackground(void)
{

  PRIVATE(this)->buildTextScenegraph();

  SoPath * p = new SoPath(this->topSeparator.getValue());
  p->ref();
  p->append(this->textSeparator.getValue());

  SoFaceSet * fs = (SoFaceSet*) this->getAnyPart("backgroundShape", TRUE);
  fs->numVertices = 0;

  const SbVec2s pixelsize = PRIVATE(this)->vp.getViewportSizePixels();  
  PRIVATE(this)->bba.setViewportRegion(PRIVATE(this)->vp);
  PRIVATE(this)->bba.apply(p);
  SbBox3f bb = PRIVATE(this)->bba.getBoundingBox();
  SbVec3f pad = SbVec3f(PRIVATE(this)->padding[0] / pixelsize[0],
                        PRIVATE(this)->padding[1] / pixelsize[1],
                        1);  
  bb.extendBy(bb.getMax() + pad); 
  bb.extendBy(bb.getMin() - pad);  
  p->unref();

  SoVertexProperty * vp = (SoVertexProperty *) fs->vertexProperty.getValue();
  if (vp == NULL) {
    vp = new SoVertexProperty;
    fs->vertexProperty = vp;
  }
  if (fs->numVertices.getNum() != 1 || fs->numVertices[0] != 4) {
    fs->numVertices = 4;
  }
  
  float fx = this->frameSize.getValue() / float(pixelsize[0]);
  float fy = this->frameSize.getValue() / float(pixelsize[1]);
  
  SbVec3f bmin = bb.getMin();
  SbVec3f bmax = bb.getMax();

  PRIVATE(this)->backgroundmenulow = bmin[1];
  PRIVATE(this)->backgroundmenuhigh = bmax[1];
  PRIVATE(this)->backgroundmenulow += 3.0f / pixelsize[1];
  PRIVATE(this)->backgroundmenuhigh +=  3.0f / pixelsize[1];

  bmin[0] -= fx;
  bmin[1] -= fy;
  bmax[0] += fx;
  bmax[1] += fy;

  PRIVATE(this)->backgroundleft = bmin[0];
  PRIVATE(this)->backgroundright = bmax[0];

  PRIVATE(this)->bbw = bmax[0] - bmin[0];
  PRIVATE(this)->bbh = bmax[1] - bmin[1];

  SbVec3f varray[8];
  varray[0] = SbVec3f(bmin[0], bmin[1], 0.0f);
  varray[1] = SbVec3f(bmax[0], bmin[1], 0.0f);
  varray[2] = SbVec3f(bmax[0], bmax[1], 0.0f);
  varray[3] = SbVec3f(bmin[0], bmax[1], 0.0f);
  vp->vertex.setValues(0, 4, varray);

  // update border as well
  float bw = 2.0f / float(pixelsize[0]);
  float bh = 2.0f / float(pixelsize[1]);
  SoIndexedFaceSet * ifs = (SoIndexedFaceSet*) this->getAnyPart("borderShape", TRUE);
  vp = (SoVertexProperty*) ifs->vertexProperty.getValue();
  if (vp == NULL) {
    vp = new SoVertexProperty;
    vp->materialBinding = SoVertexProperty::PER_FACE_INDEXED;
    ifs->vertexProperty = vp;
    vp->orderedRGBA.setNum(2);
    vp->orderedRGBA.set1Value(0, 0x00000088);
    vp->orderedRGBA.set1Value(1, 0xffffff88);
  }
  varray[4] = SbVec3f(bmin[0]+bw, bmin[1]+bh, 0.0f);
  varray[5] = SbVec3f(bmax[0]-bw, bmin[1]+bh, 0.0f);
  varray[6] = SbVec3f(bmax[0]-bw, bmax[1]-bh, 0.0f);
  varray[7] = SbVec3f(bmin[0]+bw, bmax[1]-bh, 0.0f);
 
  vp->vertex.setValues(0,8,varray);

  const int32_t cidx[] = 
    {0,1,5,4,-1,
     1,2,6,5,-1,
     7,6,2,3,-1,
     0,4,7,3,-1 };

  const int32_t midx[] = {0,0,1,1};

  ifs->coordIndex.setValues(0,20, cidx);
  ifs->materialIndex.setValues(0,4,midx);

}

void 
SmPopupMenuKit::setSchemeEvalFunctions(int (*scriptcb)(const char *),
                                       void (*filecb)(const char *))
{
  schemescriptcb = scriptcb;
  schemefilecb = filecb;
}

int
SmPopupMenuKitP::addSubmenuMarker()
{

  const int WIDTH = 6;
  const int HEIGHT = 6;
  const int BYTEWIDTH = (WIDTH + 7) / 2;
  
  const char submenu_marker[WIDTH * HEIGHT + 1] = {
    "oo    "
    "oooo  "
    "ooooo*"
    "ooooo*"
    "oooo  "
    "oo    " };

  int byteidx = 0;
  unsigned char bitmapbytes[BYTEWIDTH * HEIGHT];
  for (int h = 0; h < HEIGHT; h++) {
    unsigned char bits = 0;
    for (int w = 0; w < WIDTH; w++) {
      if (submenu_marker[(h * WIDTH) + w] != ' ') { bits |= (0x80 >> (w % 8)); }
      if ((((w + 1) % 8) == 0) || (w == WIDTH - 1)) {
        bitmapbytes[byteidx++] = bits;
        bits = 0;
      }
    }
  }

  int MYAPP_ARROW_IDX = SoMarkerSet::getNumDefinedMarkers(); // add at end
  SoMarkerSet::addMarker(MYAPP_ARROW_IDX, SbVec2s(WIDTH, HEIGHT),
                         bitmapbytes, FALSE, TRUE);  
  return MYAPP_ARROW_IDX;
}



#undef PRIVATE
#undef PUBLIC

