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
  \class SmAxisDisplayKit SmAxisDisplayKit.h
  \brief The SmAxisDisplayKit is used to draw colored and annotated axes
  in the lower right corner of the display.
  \ingroup nodekits

  This class is organized as a nodekit, but should be used as a normal
  node (i.e. use public fields).
*/

/*!
  \var SoSFRotation SmAxisDisplayKit::orientation

  Specifies the orientation of the axes. This field should normally
  be connected to the current camera's orientation field.

  Default value is as for SoCamera::orientation.
*/

/*!
  \var SoMFVec3f SmAxisDisplayKit::axes

  Specifies which axes to display. The given axes are displayed as
  vectors originating in origo.

  Default value is empty.
*/

/*!
  \var SoMFColor SmAxisDisplayKit::colors

  Specifies which colors the axes should be drawn with.
  If not enough values are given, the last value will be reused for 
  the remaining axes.

  Default value is white - SbColor(1.0f, 1.0f, 1.0f).
*/

/*!
  \var SoMFBool SmAxisDisplayKit::enableArrows

  Specifies whether arrows should be displayed at the end of each axis.
  If not enough values are given, the last value will be reused for 
  the remaining axes.

  Default value is TRUE.
*/

/*!
  \var SoMFString SmAxisDisplayKit::annotations

  Specifies annotation for each axis. The annotation will be rendered
  as a SoText2 node.
  If not enough values are given, no annotations will be displayed for 
  the remaining axes.

  Default value is empty.
*/

#include "SmAxisDisplayKit.h"

#include <Inventor/SbRotation.h>
#include <Inventor/SbViewVolume.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoComplexityTypeElement.h>
#include <Inventor/elements/SoComplexityElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoInfo.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <SmallChange/nodes/ViewportRegion.h>
#include <SmallChange/nodes/SmHeadlight.h>

class SmAxisDisplayKitP {
public:
  SmAxisDisplayKitP(SmAxisDisplayKit * master) : master(master) { }

  SmAxisDisplayKit * master;

  SbBool processingoneshot;
  SoOneShotSensor * oneshot;
  static void oneshot_cb(void * closure, SoSensor * s);
  static void callback_cb(void * userdata, SoAction * action);
};

SO_KIT_SOURCE(SmAxisDisplayKit);

#define PRIVATE(obj) (obj)->pimpl

/*!
  Constructor.
*/
SmAxisDisplayKit::SmAxisDisplayKit(void) 
{
  PRIVATE(this) = new SmAxisDisplayKitP(this);
  PRIVATE(this)->oneshot = new SoOneShotSensor(SmAxisDisplayKitP::oneshot_cb, 
                                               PRIVATE(this));
  PRIVATE(this)->processingoneshot = FALSE;

  SO_KIT_CONSTRUCTOR(SmAxisDisplayKit);

  SO_KIT_ADD_FIELD(orientation, (SbRotation(SbVec3f(0.0f, 0.0f, 1.0f), 0.0f)));
  SO_KIT_ADD_FIELD(axes, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(colors, (1.0f, 1.0f, 1.0f));
  SO_KIT_ADD_FIELD(enableArrows, (TRUE));
  SO_KIT_ADD_FIELD(annotations, (""));
  SO_KIT_ADD_FIELD(headlight, (TRUE));

  this->axes.setNum(0);
  this->axes.setDefault(TRUE);
  this->annotations.setNum(0);
  this->annotations.setDefault(TRUE);

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(cameraCallback, SoCallback, FALSE, topSeparator, camera, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(camera, SoPerspectiveCamera, FALSE, topSeparator, viewportRegion, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(viewportRegion, ViewportRegion, FALSE, topSeparator, drawstyle, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(drawstyle, SoDrawStyle, FALSE, topSeparator, shapehints, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(shapehints, SoShapeHints, FALSE, topSeparator, headlightNode, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(headlightNode, SmHeadlight, FALSE, topSeparator, axessep, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(axessep, SoSeparator, FALSE, topSeparator, "", FALSE);

  SO_KIT_INIT_INSTANCE();

  // Setup default values
  SoCallback * cb = (SoCallback *)this->getAnyPart("cameraCallback", TRUE);
  cb->setCallback(SmAxisDisplayKitP::callback_cb, PRIVATE(this));

  ViewportRegion * vpr = (ViewportRegion *)this->getAnyPart("viewportRegion", TRUE);
  vpr->origin.setValue(1.0f, 0.0f);
  vpr->size.setValue(0.15f, 0.15f);
  vpr->clearDepthBuffer = TRUE;

  SoDrawStyle *drawstyle = (SoDrawStyle *)this->getAnyPart("drawstyle", TRUE);
  drawstyle->lineWidth = 2;

  SoShapeHints * sh = (SoShapeHints*) this->getAnyPart("shapehints", TRUE);
  sh->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
  sh->shapeType = SoShapeHints::SOLID;

  SmHeadlight * hl = (SmHeadlight*) this->getAnyPart("headlightNode", TRUE);
  hl->on.connectFrom(&this->headlight);
}

/*!
  Destructor.
*/
SmAxisDisplayKit::~SmAxisDisplayKit()
{
  delete PRIVATE(this)->oneshot;
  delete PRIVATE(this);
}

/*!
  Initializes this class. Call before using it.
*/
void
SmAxisDisplayKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SmAxisDisplayKit, SoBaseKit, "BaseKit");
  }
}

/*!
  Overloaded to perform delayed rendering and to force solid rendering mode.
*/
void 
SmAxisDisplayKit::GLRender(SoGLRenderAction *action)
{
  if (!action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());
    return;
  }
  else {
    SoState *state = action->getState();
    state->push();
    SoComplexityTypeElement::set(state, this, SoComplexityTypeElement::getDefault());
    SoComplexityElement::set(state, this, SoComplexityElement::getDefault());
    SoDrawStyleElement::set(state, SoDrawStyleElement::FILLED);
    SoLightModelElement::set(state, this, this->headlight.getValue() ? 
                             SoLightModelElement::PHONG :
                             SoLightModelElement::BASE_COLOR);
    inherited::GLRender(action);
    state->pop();
  }
}

SbBool 
SmAxisDisplayKit::affectsState(void) const
{
  // important since we might add delayed paths to SoGLRenderAction
  return FALSE;
}


void 
SmAxisDisplayKit::handleEvent(SoHandleEventAction * action)
{
  inherited::handleEvent(action);
}

void 
SmAxisDisplayKit::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SoCacheElement::invalidate(action->getState());
  SoNode::getBoundingBox(action);
}

void 
SmAxisDisplayKit::search(SoSearchAction * action)
{
  SoNode::search(action);
}

void 
SmAxisDisplayKit::callback(SoCallbackAction * action)
{
  SoNode::callback(action);
}

void 
SmAxisDisplayKit::getMatrix(SoGetMatrixAction * action) 
{ 
  SoNode::getMatrix(action);
}

void 
SmAxisDisplayKit::pick(SoPickAction * action) 
{
  SoNode::pick(action);
}

void
SmAxisDisplayKit::rayPick(SoRayPickAction * action) 
{ 
  SoNode::rayPick(action);
}

void 
SmAxisDisplayKit::audioRender(SoAudioRenderAction * action) 
{
  SoNode::audioRender(action);
}

void 
SmAxisDisplayKit::getPrimitiveCount(SoGetPrimitiveCountAction * action) 
{
  SoNode::getPrimitiveCount(action);
}

/*!
 Overloaded to detect when fields changes in order to correctly
 regenerate the private scene graph.
*/
void 
SmAxisDisplayKit::notify(SoNotList * l)
{
  if (!PRIVATE(this)->oneshot->isScheduled() && 
      !PRIVATE(this)->processingoneshot) {
    SoField * f = l->getLastField();
    if (f) {
      SoFieldContainer * c = f->getContainer();
      // test if a field in this nodekit changed, but only trigger on
      // normal fields, not on part fields.
      if (c == this && f->getTypeId() != SoSFNode::getClassTypeId()) {
        // trigger a sensor that will recalculate all internal data
        PRIVATE(this)->oneshot->schedule();
      }
    }
  }
  inherited::notify(l);
}

/*!
  Overloaded to set most node fields to default. The parts in this
  nodekit are mostly internal.
*/
void 
SmAxisDisplayKit::setDefaultOnNonWritingFields(void)
{
  SoFieldList fl;
  (void) this->getFields(fl);
  for (int i = 0; i < fl.getLength(); i++) {
    SoField * f = fl[i];
    if (f->isOfType(SoSFNode::getClassTypeId())) f->setDefault(TRUE);
  }  
  inherited::setDefaultOnNonWritingFields();
}

#undef PRIVATE
#define PUBLIC(obj) (obj)->master

/*!
  Called when something has changed and the private scenegraph 
  under the "axessep" part needs to be regenerated.
*/
void 
SmAxisDisplayKitP::oneshot_cb(void * closure, SoSensor * s)
{
  SmAxisDisplayKitP * thisp = (SmAxisDisplayKitP*) closure;
  thisp->processingoneshot = TRUE;

  // Update private scene graph
  
  SoSeparator *axessep = 
    (SoSeparator *)PUBLIC(thisp)->getAnyPart("axessep", TRUE);
  axessep->removeAllChildren();

  SoSeparator *linesep = NULL;
  SoSeparator *conesep = NULL;
  for (int i=0;i<PUBLIC(thisp)->axes.getNum();i++) {
    SbVec3f axis = PUBLIC(thisp)->axes[i];
    if (axis.length() == 0.0f) continue; // Don't render null vectors.
    SoSeparator *axissep = new SoSeparator; // One separator per axis
    SoBaseColor *axiscol = new SoBaseColor;
    if (PUBLIC(thisp)->colors.getNum() <= i)
      axiscol->rgb.setValue(PUBLIC(thisp)->colors[PUBLIC(thisp)->colors.getNum()-1]);
    else
      axiscol->rgb.setValue(PUBLIC(thisp)->colors[i]);

    // Rotate to correct orientation
    SbRotation rot(SbVec3f(0.0f, 1.0f, 0.0f), axis);
    SoRotation *axistrans = new SoRotation;
    axistrans->rotation = rot;

    // Vector
    if (!linesep) { // Reuse linesep for all axes
#if 0 // boring lines :)
      linesep = new SoSeparator;
      SoCoordinate3 *coord = new SoCoordinate3;
      SoLineSet *ls = new SoLineSet;

      coord->point.set1Value(0, 0.0f, 0.0f, 0.0f);
      coord->point.set1Value(1, 0.0f, 1.0f, 0.0f);
      ls->numVertices.setValue(2);

      linesep->addChild(coord);
      linesep->addChild(ls);
#else // better looking cylinders
      linesep = new SoSeparator;
      SoCylinder * cyl = new SoCylinder;
      cyl->radius = 0.02f;
      cyl->height = 1.0f;
      SoTranslation * t = new SoTranslation;
      t->translation = SbVec3f(0.0f, 0.5f, 0.0f);      
      linesep->addChild(t);
      linesep->addChild(cyl);
#endif // cylinders
    }
    axissep->addChild(axiscol);
    axissep->addChild(axistrans);
    axissep->addChild(linesep);
    SbBool enableAxis;
    if (PUBLIC(thisp)->enableArrows.getNum() <= i)
      enableAxis = PUBLIC(thisp)->enableArrows[PUBLIC(thisp)->enableArrows.getNum()-1];
    else
      enableAxis = PUBLIC(thisp)->enableArrows[i];

    // Arrow
    if (enableAxis) {
      if (!conesep) { // Reuse conesep for all axes
        conesep = new SoSeparator;
        SoTranslation *trans = new SoTranslation;
        SoCone *cone = new SoCone;
        
        trans->translation.setValue(0.0f, 1.125f, 0.0f);
        cone->height = 0.25f;
        cone->bottomRadius = 0.0833f;
        
        conesep->addChild(trans);
        conesep->addChild(cone);
      }
      axissep->addChild(conesep);
    }

    // Annotation
    SbString annot;
    if (PUBLIC(thisp)->annotations.getNum() > i)
      annot = PUBLIC(thisp)->annotations[i];
    if (annot.getLength() > 0) {
      SoSeparator *annotsep = new SoSeparator;
      SoTranslation *trans = new SoTranslation;
      SoText2 *text = new SoText2;

      trans->translation.setValue(0.0f, 1.25f, 0.0f);
      text->string.setValue(annot);
      
      annotsep->addChild(trans);
      annotsep->addChild(text);
      axissep->addChild(annotsep);
    }

    axessep->addChild(axissep);
  }

  // Camera settings
  SoPerspectiveCamera *camera = 
    (SoPerspectiveCamera *)PUBLIC(thisp)->getAnyPart("camera", TRUE);
  camera->orientation.connectFrom(&PUBLIC(thisp)->orientation);
  camera->nearDistance = 0.01f;
  camera->farDistance = 10.0f;
  camera->heightAngle = 22.5*M_PI/180; // Reduce perspective effect

  // FIXME: hack to avoid continuous redraws. Use engine-connections
  // instead, pederb, 2003-10-21
  camera->position.enableNotify(FALSE); 

  if (thisp->oneshot->isScheduled()) thisp->oneshot->unschedule();
  thisp->processingoneshot = FALSE;;
}

/*!
  Callback to be able to sync our camera position with movements of the
  main camera.
*/
void 
SmAxisDisplayKitP::callback_cb(void * userdata, SoAction * action)
{
  SmAxisDisplayKitP * thisp = (SmAxisDisplayKitP*) userdata;
  if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
    SoPerspectiveCamera *camera = 
      (SoPerspectiveCamera *)PUBLIC(thisp)->getAnyPart("camera", TRUE);
    
    SbRotation rot = PUBLIC(thisp)->orientation.getValue();
    SbVec3f dir;
    rot.multVec(SbVec3f(0.0f, 0.0f, 1.0f), dir);
    camera->position.setValue(7.0f*dir); // Just a good guess
  }
}

