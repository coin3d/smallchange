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

/*
  \class SmWellLogKit SmallChange/nodekits/SmWellLogKit.h
  \brief The SmWellLogKit class is used to visualize wells.
  
*/

/*!
  \var SoSFFloat SmWellLogKit::undefVal
  The value that is used for specifying undefined curve data. 
  Default value is -999.25.
*/

/*!  
  \var SoSFString SmWellLogKit::name The name of the
  Well. Currently not used, but we might add support for displaying
  this later.
*/

/*!
  \var SoMFVec3d SmWellLogKit::wellCoord
  The absolute (UTM) positions of each well position.
*/
  
/*!
  \var SoMFString SmWellLogKit::curveNames

  The names for each curve in the well log data. The first value
  should be the depth.

*/

/*!  
  \var SoMFString SmWellLogKit::curveUnits 
  
  The unit for each curve in the well log. Will soon be added as info
  in the tooltip.
*/

/*!
  \var SoMFFloat SmWellDataLogKit::curveData

  The raw well log data. Each log position will first have a depth
  value, then the rest of the curves values, repeated for each log
  position.
*/

/*!
  SoSFFloat SmWellLogKit::leftSize

  The (maximum) size of the left curve visualization.

*/

/*!
  SoSFFloat SmWellLogKit::rightSize

  The (maximum) size of the right curve visualization.

*/

/*!
  SoSFInt32 SmWellLogKit::leftCurveIndex

  The curve index of the left curve. Is used to select the curve on
  the left side of the wel. Use -1 to disable.
*/

/*!
  SoSFInt32 SmWellLogKit::rightCurveIndex

  The curve index of the right curve. Is used to select the curve on
  the right side of the well. Use -1 to disable.

*/

/*!  
  SoSFColor SmWellLogKit::leftColor

  The color of the left curve.
*/

/*!
  SoSFColor SmWellLogKit::rightColor

  The color of the right curve.
*/

/*!
  SoSFBool SmWellLogKit::leftUseLog

  Set to TRUE to use a logarithmic function for visualizing the left curve.
*/

/*!
  SoSFBool SmWellLogKit::rightUseLog;

  Set to TRUE to use a logarithmic function for visualizing the right curve.
*/

/*!
  SoSFFloat SmWellLogKit::lodDistance1

  The distance where the well will be rendered as a line, not a cylinder.
*/

/*!
  SoSFFloat SmWellLogKit::lodDistance2

  The distance where the well will not be rendered.
*/

/*!
  SoSFFloat SmWellLogKit::wellRadius

  The radius of the well.
*/

#include "SmWellLogKit.h"
#include <SmallChange/nodes/UTMPosition.h>
#include <SmallChange/nodes/SoLODExtrusion.h>
#include <SmallChange/nodekits/SmTooltipKit.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoLOD.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoPackedColor.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoPolygonOffset.h>
#include <Inventor/nodes/SoInfo.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/details/SoLineDetail.h>
#include <float.h>

// struct used for storing data for each depth value
typedef struct {
  SbVec3f pos;
  double mdepth;  // measured depth
  double tvdepth; // true vertical depth
  double left;  // curve data for the left side
  double right; // curve data for the right side
} well_pos;

class SmWellLogKitP {
public:  
  SmWellLogKitP(SmWellLogKit * master)
    : master(master) { }

  SmWellLogKit * master;

  // FIXME: consider dict
  SbList <SbName> cm_name;
  SbList <uint32_t> cm_col;

  SbList <float> leftoffset;
  SbList <float> rightoffset;

  SbList <well_pos> poslist;
  SbVec3f axis;
  SbVec3f prevaxis;
  SbBool processingoneshot;

  uint32_t find_col(const SbName & name);
  int find_colidx(const SbName & name);
  void updateList(void);
  void buildGeometry(void);
  void generateFaces(const SbVec3f & axis);
  void setLithOrFluid(const SbList <double> & limits,
                      const SbList <SbName> & names,
                      const SbBool islith);
  void convertDepth(const SbList <double> & data);
  static void callback_cb(void * userdata, SoAction * action);

  SoOneShotSensor * oneshot;
  
  static void oneshot_cb(void * closure, SoSensor * s);
};

SO_KIT_SOURCE(SmWellLogKit);

#define PRIVATE(obj) (obj)->pimpl

SmWellLogKit::SmWellLogKit(void)
{
  PRIVATE(this) = new SmWellLogKitP(this);
  PRIVATE(this)->axis = SbVec3f(1.0f, 0.0f, 0.0f);
  PRIVATE(this)->prevaxis = SbVec3f(0.0f, 0.0f, 0.0f);
  PRIVATE(this)->oneshot = new SoOneShotSensor(SmWellLogKitP::oneshot_cb, PRIVATE(this));
  PRIVATE(this)->processingoneshot = FALSE;

  SO_KIT_CONSTRUCTOR(SmWellLogKit);
  
  SO_KIT_ADD_FIELD(undefVal, (-999.25));
  SO_KIT_ADD_FIELD(name,("MyWell"));
  SO_KIT_ADD_FIELD(wellCoord, (0.0, 0.0, 0.0));

  SO_KIT_ADD_FIELD(curveNames, (""));
  SO_KIT_ADD_FIELD(curveUnits, (""));
  SO_KIT_ADD_FIELD(curveData, (0.0f));
  SO_KIT_ADD_FIELD(leftSize, (50.0f));
  SO_KIT_ADD_FIELD(rightSize, (50.0f));
  
  SO_KIT_ADD_FIELD(leftCurveIndex, (-1));
  SO_KIT_ADD_FIELD(rightCurveIndex, (-1));

  SO_KIT_ADD_FIELD(leftUseLog, (FALSE));
  SO_KIT_ADD_FIELD(rightUseLog, (FALSE));

  SO_KIT_ADD_FIELD(lodDistance1, (50000.0f));
  SO_KIT_ADD_FIELD(lodDistance2, (100000.0f));
  SO_KIT_ADD_FIELD(wellRadius,(1.0f));

  SO_KIT_ADD_FIELD(leftColor, (0.3, 0.6, 0.8));
  SO_KIT_ADD_FIELD(rightColor, (0.3, 0.6, 0.8));

  this->wellCoord.setNum(0);
  this->wellCoord.setDefault(TRUE);
  this->curveNames.setNum(0);
  this->curveNames.setDefault(TRUE);
  this->curveData.setNum(0);
  this->curveData.setDefault(TRUE);
  this->curveUnits.setNum(0);
  this->curveUnits.setDefault(TRUE);

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(tooltip, SmTooltipKit, FALSE, topSeparator, utm, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(utm, UTMPosition, FALSE, topSeparator, transform, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(transform, SoTransform, FALSE, topSeparator, topLod, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(topLod, SoLOD, FALSE, topSeparator, "", TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(topLodGroup, SoSeparator, FALSE, topLod, topInfo, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(shapeHints, SoShapeHints, FALSE, topLodGroup, wellMaterial, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(wellMaterial, SoMaterial, FALSE, topLodGroup, well, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(well, SoLODExtrusion, FALSE, topLodGroup, lightModel, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(lightModel, SoLightModel, FALSE, topLodGroup, pickStyle, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(pickStyle, SoPickStyle, FALSE, topLodGroup, lod, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(lod, SoLOD, FALSE, topLodGroup, "", TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(lodSeparator, SoSeparator, FALSE, lod, info, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(callback, SoCallback, FALSE, lodSeparator, materialBinding, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(materialBinding, SoMaterialBinding, FALSE, lodSeparator, faceSetColor, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(faceSetColor, SoPackedColor, FALSE, lodSeparator, coord, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(coord, SoCoordinate3, FALSE, lodSeparator, faceSet, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(faceSet, SoIndexedFaceSet, FALSE, lodSeparator, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(info, SoInfo, FALSE, lod, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(topInfo, SoInfo, FALSE, topLod, "", FALSE);

  SO_KIT_INIT_INSTANCE();

  // initialize parts to default values
  SoShapeHints * sh = (SoShapeHints*) this->getAnyPart("shapeHints", TRUE);
  sh->vertexOrdering = SoShapeHints::CLOCKWISE;
  sh->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
  sh->faceType = SoShapeHints::CONVEX;

  SoLODExtrusion * well = (SoLODExtrusion*) this->getAnyPart("well", TRUE);
  well->ccw = TRUE;
  well->lodDistance1.connectFrom(&this->lodDistance1);
  well->radius.connectFrom(&this->wellRadius);
  
  SoPickStyle * ps = (SoPickStyle*) this->getAnyPart("pickStyle", TRUE);
  ps->style = SoPickStyle::UNPICKABLE;

  SoMaterialBinding * mb = (SoMaterialBinding*) this->getAnyPart("materialBinding", TRUE);
  mb->value = SoMaterialBinding::PER_FACE_INDEXED;
  
  SoLightModel * lm = (SoLightModel*) this->getAnyPart("lightModel", TRUE);
  lm->model = SoLightModel::BASE_COLOR;

  SoLOD * toplod = (SoLOD*) this->getAnyPart("topLod", TRUE);
  toplod->range.connectFrom(&this->lodDistance1);

  SoLOD * facelod = (SoLOD*) this->getAnyPart("lod", TRUE);
  facelod->range.connectFrom(&this->lodDistance2);

  SoCallback * cb = (SoCallback*) this->getAnyPart("callback", TRUE);
  cb->setCallback(SmWellLogKitP::callback_cb, PRIVATE(this));
}

SmWellLogKit::~SmWellLogKit(void)
{
  delete PRIVATE(this)->oneshot;
  delete PRIVATE(this);
}

void
SmWellLogKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SmWellLogKit, SoBaseKit, "BaseKit");
  }
}

void 
SmWellLogKit::getBoundingBox(SoGetBoundingBoxAction * action)
{
  // the kit has changed but the sensor has not triggered
  // yet. Calculate manually and unschedule() so that we get the
  // correct bounding box.
  if (PRIVATE(this)->oneshot->isScheduled()) {
    PRIVATE(this)->oneshot->unschedule();
    SmWellLogKitP::oneshot_cb(PRIVATE(this), PRIVATE(this)->oneshot);
  }
  inherited::getBoundingBox(action);
}

void 
SmWellLogKit::handleEvent(SoHandleEventAction * action)
{
  // the kit has changed but the sensor has not triggered
  // yet. Calculate manually and unschedule() so that we get the
  // correct bounding box.
  if (PRIVATE(this)->oneshot->isScheduled()) {
    PRIVATE(this)->oneshot->unschedule();
    SmWellLogKitP::oneshot_cb(PRIVATE(this), PRIVATE(this)->oneshot);
  }

  SmTooltipKit * tooltip = (SmTooltipKit*)this->getAnyPart("tooltip", TRUE);
  const SoEvent * event = action->getEvent();

  SbBool handled = FALSE;
  
  if (SO_MOUSE_RELEASE_EVENT(event, BUTTON1)) {
    if (tooltip->isActive.getValue()) {
      tooltip->isActive = FALSE;
      handled = TRUE;
    }
  }

  // FIXME: investigate SoLocation2Event to see if left mouse button
  // has been released. Don't trust that we get a
  // SoMouseButtonReleaseEvent. pederb, 2003-10-16

  if ((!tooltip->isActive.getValue() && SO_MOUSE_PRESS_EVENT(event, BUTTON1)) ||
      (tooltip->isActive.getValue() && event->isOfType(SoLocation2Event::getClassTypeId()))) {
    
    const SoPickedPoint * pp = action->getPickedPoint();
    SoSeparator * topsep = (SoSeparator*) this->getAnyPart("topSeparator", TRUE);

    if (pp) {
      SoFullPath * path = (SoFullPath*) pp->getPath();
      int idx = path->findNode(topsep);
      if (idx >= 0) {
        const SoDetail * detail = pp->getDetail();
        if (detail && detail->isOfType(SoLineDetail::getClassTypeId())) {
          idx = this->findPickIdx(pp->getObjectPoint());
          if (this->setTooltipInfo(idx, tooltip)) {
            tooltip->setPickedPoint(pp, action->getViewportRegion());
            tooltip->isActive = TRUE;
          }
        }
      }
    }
  }  
  if (handled) action->setHandled();
  else inherited::handleEvent(action);
}



SbBool 
SmWellLogKit::readInstance(SoInput * in, unsigned short flags)
{
  SbBool ret = inherited::readInstance(in, flags);
  if (ret) {
    // only really needed to load old files...
    this->connectNodes(); // make connections from fields to nodes
  }
  return ret;
}

// linear search for depth index. Optimize?
int 
SmWellLogKit::findPickIdx(const SbVec3f & pos) const
{
  int n = PRIVATE(this)->poslist.getLength();
  for (int i = 0; i < n; i++) {
    if (pos[2] > PRIVATE(this)->poslist[i].pos[2]) return i;
  }
  return n-1;
}

// fill in tooltip info for a picked depth index
SbBool
SmWellLogKit::setTooltipInfo(const int idx, SmTooltipKit * tooltip)
{
  if (idx < 0 || idx >= PRIVATE(this)->poslist.getLength()) return FALSE;

  const well_pos & pos = PRIVATE(this)->poslist[idx];
  
  SbString l("UNDEFINED");
  SbString r("UNDEFINED");
  
  float undefval = this->undefVal.getValue();
  
  if (pos.left != undefval) {
    l.sprintf("%g", pos.left);
  }
  if (pos.right != undefval) {
    r.sprintf("%g", pos.right);
  }
  SbString str;

  int num = 3;
  int lidx = this->leftCurveIndex.getValue();
  if (lidx >= 0) num++;
  int ridx = this->rightCurveIndex.getValue();
  if (ridx >= 0) num++;

  tooltip->description.setNum(num);
  tooltip->description.set1Value(0, this->name.getValue());
  str.sprintf("Depth: %g", pos.mdepth);
  tooltip->description.set1Value(1, str);
  str.sprintf("TVDSS: %g", pos.tvdepth);
  tooltip->description.set1Value(2, str);

  if (lidx >= 0) {
    str.sprintf("%s: %s",
                this->curveNames[lidx].getString(), l.getString());
    tooltip->description.set1Value(3, str);
  }
  if (ridx >= 0) {
    str.sprintf("%s: %s",
                this->curveNames[ridx].getString(), r.getString());
    tooltip->description.set1Value(lidx >= 0 ? 4 : 3, str);    
  }
  return TRUE;
}

void 
SmWellLogKit::connectNodes(void)
{
  SoLODExtrusion * well = (SoLODExtrusion*) this->getAnyPart("well", TRUE);
  well->lodDistance1.connectFrom(&this->lodDistance1);
  well->radius.connectFrom(&this->wellRadius);

  SoLOD * toplod = (SoLOD*) this->getAnyPart("topLod", TRUE);
  toplod->range.connectFrom(&this->lodDistance1);

  SoLOD * facelod = (SoLOD*) this->getAnyPart("lod", TRUE);
  facelod->range.connectFrom(&this->lodDistance2);

  SoCallback * cb = (SoCallback*) this->getAnyPart("callback", TRUE);
  cb->setCallback(SmWellLogKitP::callback_cb, PRIVATE(this));
}

int 
SmWellLogKit::getNumCurves(void) const
{
  return this->curveNames.getNum();
}

int 
SmWellLogKit::getNumCurveValues(void) const
{
  int num = this->getNumCurves();
  if (num) {
    return this->curveData.getNum() / num;
  }
  return 0;
}

// returns the depth value for a depth index
float 
SmWellLogKit::getDepth(const int idx) const
{
  // FIXME: assumes depth is always first. Add depthIndex field?
  int num = this->getNumCurves();
  if (num) {
    int fidx = idx*num;
    assert(fidx < this->curveData.getNum());
    if (fidx < this->curveData.getNum()) {
      return this->curveData[fidx];
    }
  }
  return 0.0f;
}

// returns the left curve value for a depth index
float 
SmWellLogKit::getLeftCurveData(const int idx) const
{
  int num = this->getNumCurves();
  if (num && this->leftCurveIndex.getValue() >= 0) {
    int fidx = idx*num + this->leftCurveIndex.getValue();
    assert(fidx < this->curveData.getNum());
    if (fidx < this->curveData.getNum()) {
      return this->curveData[fidx];
    }
  }
  return this->undefVal.getValue();
}

// returns the right curve value for a depth index
float 
SmWellLogKit::getRightCurveData(const int idx) const
{
  int num = this->getNumCurves();
  if (num && this->rightCurveIndex.getValue() >= 0) {
    int fidx = idx*num + this->rightCurveIndex.getValue();
    assert(fidx < this->curveData.getNum());
    if (fidx < this->curveData.getNum()) {
      return this->curveData[fidx];
    }
  }
  return this->undefVal.getValue();
}

// overloader to test when stuff changes in the API fields
void 
SmWellLogKit::notify(SoNotList * l)
{
  if (!PRIVATE(this)->oneshot->isScheduled() && !PRIVATE(this)->processingoneshot) {
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
SmWellLogKit::setDefaultOnNonWritingFields(void)
{
  SoFieldList fl;
  (void) this->getFields(fl);
  for (int i = 0; i < fl.getLength(); i++) {
    SoField * f = fl[i];
    if (f->isOfType(SoSFNode::getClassTypeId()) &&
        f != &this->transform) {
      f->setDefault(TRUE);
    }
  }  
  inherited::setDefaultOnNonWritingFields();
}

#undef PRIVATE

#define PUBLIC(obj) (obj)->master

uint32_t 
SmWellLogKitP::find_col(const SbName & name) 
{
  if (name == "left") {
    return PUBLIC(this)->leftColor.getValue().getPackedValue();
  }
  if (name == "right") {
    return PUBLIC(this)->rightColor.getValue().getPackedValue();
  }
  int idx = find_colidx(name);
  if (idx < 0) {
    return 0x4477ccff;
  }
  return cm_col[idx];
}

int 
SmWellLogKitP::find_colidx(const SbName & name) 
{
  for (int i = 0; i < cm_name.getLength(); i++) {
    if (cm_name[i] == name) return i;
  }
  return -1;
}

void 
SmWellLogKitP::callback_cb(void * userdata, SoAction * action)
{
  SmWellLogKitP * thisp = (SmWellLogKitP*) userdata;
  if (action->isOfType(SoGetBoundingBoxAction::getClassTypeId())) {
    SoCacheElement::invalidate(action->getState());
  }
  else if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
    SoState * state = action->getState();
    SoCacheElement::invalidate(state);

    const SbViewVolume & vv = SoViewVolumeElement::get(state);
    SbVec3f Z = vv.getProjectionDirection();
    SbVec3f Y = vv.getViewUp();
    SbVec3f X = Y.cross(Z);
    
    // make log gfx face the viewer/camera
    SbMatrix m;
    m.makeIdentity();
    m[0][0] = X[0];
    m[0][1] = X[1];
    m[0][2] = X[2];

    m[1][0] = Y[0];
    m[1][1] = Y[1];
    m[1][2] = Y[2];

    m[2][0] = Z[0];
    m[2][1] = Z[1];
    m[2][2] = Z[2];
    
    // account for model matrix
    m.multRight(SoModelMatrixElement::get(state).inverse());
    
    SbVec3f axis(1.0f, 0.0f, 0.0f); // FIXME: possible to configure by user?
    m.multDirMatrix(axis, axis);
    
    axis[2] = 0.0f;
    if (axis == SbVec3f(0.0f, 0.0f, 0.0f)) axis = SbVec3f(1.0f, 0.0f, 0.0f);
    else axis.normalize();

    // FIXME: use an epsilon when comparing here?
    if (axis != thisp->prevaxis) {
      thisp->prevaxis = axis;
      thisp->generateFaces(axis);
    }
  }
}

// generate faces facing the viewer
void
SmWellLogKitP::generateFaces(const SbVec3f & newaxis)
{
  SoCoordinate3 * coord = (SoCoordinate3*) PUBLIC(this)->getAnyPart("coord", TRUE);
  
  int i, n = this->poslist.getLength();
  coord->point.setNum(3*n);
  SbVec3f * dst = coord->point.startEditing();
  
  for (i = 0; i < n; i++) {
    dst[n+i] = this->poslist[i].pos + newaxis * this->leftoffset[i];
    dst[2*n+i] = this->poslist[i].pos - newaxis * this->rightoffset[i];
  }
  coord->point.finishEditing();
}

// FIXME: should it be possible to configure the log function?
static double log_func(double val)
{
  if (val < 0.0) return -1.0;
  return log10(1.0 + val);
}

void
SmWellLogKitP::buildGeometry(void)
{
  this->leftoffset.truncate(0);
  this->rightoffset.truncate(0);

  int i, n = this->poslist.getLength();

  double leftmax = -DBL_MIN;
  double rightmax = -DBL_MAX;
  double leftmin = DBL_MAX;
  double rightmin = DBL_MAX;
  double undef = PUBLIC(this)->undefVal.getValue();

  for (i = 0; i < n; i++) {
    double l = this->poslist[i].left;
    double r = this->poslist[i].right;
    
    if (l != undef) {
      if (l < leftmin) leftmin = l;
      if (l > leftmax) leftmax = l;
    }
    if (r != undef) {
      if (r < rightmin) rightmin = r;
      if (r > rightmax) rightmax = r;
    }
  }
  
  double leftdiff = leftmax - leftmin;
  double rightdiff = rightmax - rightmin;
  
  double leftscale = leftdiff ? 1.0 / leftdiff : 1.0;
  double rightscale = rightdiff ? 1.0 / rightdiff : 1.0;

  if (leftdiff && PUBLIC(this)->leftUseLog.getValue()) {
    leftscale = 1.0 / log_func(leftdiff);
  }
  if (rightdiff && PUBLIC(this)->rightUseLog.getValue()) {
    rightscale = 1.0 / log_func(rightdiff);
  }

  leftscale *= PUBLIC(this)->leftSize.getValue() * 0.5;
  rightscale *= PUBLIC(this)->rightSize.getValue() * 0.5;

  int numleft = 0;
  int numright = 0;

  for (i = 0; i < n-1; i++) {
    if (this->poslist[i].left != PUBLIC(this)->undefVal.getValue() &&
        this->poslist[i+1].left != PUBLIC(this)->undefVal.getValue()) numleft++;
    if (this->poslist[i].right != PUBLIC(this)->undefVal.getValue() &&
        this->poslist[i+1].right != PUBLIC(this)->undefVal.getValue()) numright++;
  }

  SbList <SbVec3f> redlist(this->poslist.getLength());
  for (i = 0; i < n; i++) {
    if (i == 0 || i == n-1) redlist.append(this->poslist[i].pos);
    else {
      SbVec3f prev = this->poslist[i-1].pos;
      SbVec3f p = this->poslist[i].pos;
      SbVec3f next = this->poslist[i+1].pos;

      SbVec3f v0 = p - prev;
      SbVec3f v1 = next - p;
      
      v0.normalize();
      v1.normalize();

      if (SbAbs(1.0f-v0.dot(v1)) > 0.02f) {
        redlist.append(this->poslist[i].pos);
      } 
    }
  }

  SoLODExtrusion * well = (SoLODExtrusion*) PUBLIC(this)->getAnyPart("well", TRUE);
  well->spine.setNum(redlist.getLength());

  SbVec3f * lsdst = well->spine.startEditing();
  for (i = 0; i < redlist.getLength(); i++) {
    lsdst[i] = redlist[i];
  }
  well->spine.finishEditing();

  SoPackedColor * fscol = (SoPackedColor*) PUBLIC(this)->getAnyPart("faceSetColor", TRUE);
  SoCoordinate3 * coord = (SoCoordinate3*) PUBLIC(this)->getAnyPart("coord", TRUE);

  fscol->orderedRGBA.setNum(n*2);
  uint32_t * col = fscol->orderedRGBA.startEditing();
  
  coord->point.setNum(n * 3);
  SbVec3f * dst = coord->point.startEditing();
  SbVec3f prev;
  for (i = 0; i < n; i++) {
    dst[i] = this->poslist[i].pos;
    prev = dst[i];
    col[i] = find_col(SbName("left"));
    col[i+n] = find_col(SbName("right"));
  }

  for (i = 0; i < n; i++) {
    float l = this->poslist[i].left;
    float r = this->poslist[i].right;

    if (l == undef) {
      this->leftoffset.append(0.0);
    }
    else {
      l -= leftmin;
      if (PUBLIC(this)->leftUseLog.getValue()) {
        l = log_func(l);
      }
      l *= leftscale;
      this->leftoffset.append(l + PUBLIC(this)->leftSize.getValue() * 0.5f);
    }
    if (r == undef) {
      this->rightoffset.append(0.0);
    }
    else {
      r -= rightmin;
      if (PUBLIC(this)->rightUseLog.getValue()) {
        r = log_func(r);
      }
      r *= rightscale;
      this->rightoffset.append(r + PUBLIC(this)->rightSize.getValue() * 0.5f);
    }
  }

  SoIndexedFaceSet * ifs = (SoIndexedFaceSet*) PUBLIC(this)->getAnyPart("faceSet", TRUE);

  ifs->coordIndex.setNum(numleft * 5 + numright * 5);
  int32_t * dsti = ifs->coordIndex.startEditing();

  ifs->materialIndex.setNum(numleft + numright);
  int32_t * dstm = ifs->materialIndex.startEditing();

  for (i = 0; i < n-1; i++) {
    if (this->poslist[i].left != PUBLIC(this)->undefVal.getValue() &&
        this->poslist[i+1].left != PUBLIC(this)->undefVal.getValue()) {

      *dsti++ = i;
      *dsti++ = n+i;
      *dsti++ = n+i+1;
      *dsti++ = i+1;
      *dsti++ = -1;
      *dstm++ = i;
    }
    if (this->poslist[i].right != PUBLIC(this)->undefVal.getValue() &&
        this->poslist[i+1].right != PUBLIC(this)->undefVal.getValue()) {

      *dsti++ = i;
      *dsti++ = i+1;
      *dsti++ = 2*n+i+1;
      *dsti++ = 2*n+i;
      *dsti++ = -1;
      *dstm++ = i+n;
    }
  }

  coord->point.finishEditing();
  ifs->coordIndex.finishEditing();
  ifs->materialIndex.finishEditing();
}

static void
interpolate(well_pos & p, const well_pos & prev, const well_pos & next,
            double newdepth)
{
  double delta = next.tvdepth - prev.tvdepth;
  if (delta == 0.0) {
    p = prev;
    return;
  }
  double t = (newdepth - prev.tvdepth) / delta;

  if (t < 0.0 || t > 1.0) {
    //    myfprintf(stderr,"newdepth: %g\n", newdepth);
  }
  //  assert(t >= 0.0f && t <= 1.0);

  float ft = (float) t;

  SbVec3f vec = next.pos - prev.pos;
  p.pos = prev.pos + vec * ft;
  p.left = prev.left; // FIXME: interpolate?
  p.right = prev.right;
  p.mdepth = prev.mdepth + (next.mdepth-prev.mdepth) * t;
  p.tvdepth = newdepth;
}

// time/depth interpolation function. Used when converting between
// time and detph
static float 
time_depth_interpolate(double val, const SbList <double> & data)
{
  int i = 0 , n = data.getLength() / 2;

  while (i < n && val > data[i*2+1]) i++;
  
  int i0 = i;
  int i1 = i + 1;

  if (i1 >= n) {
    i0 = n-2;
    i1 = n-1;
  }

  double from[2];
  double to[2];

  from[0] = data[i0*2+1];
  from[1] = data[i1*2+1];

  to[0] = data[i0*2];
  to[1] = data[i1*2];

  double fd = from[1] - from[0];
  double td = to[1] - to[0];

  if (fd <= 0.0 || td <= 0.0) {
//     myfprintf(stderr,"error (%d %d, %d: %g %g %g %g\n",
//             i0, i1, i,
//            from[0], from[1], to[0], to[1]);
  }
  
  //assert(fd > 0.0 && td > 0.0);

  double t = (val - from[0]) / fd;

  //  fprintf(stderr,"convert: %g %g\n", val, (float) (to[0] + t * td)); 

  return (float) (to[0] + t * td);
}

// convert between time and depth
void 
SmWellLogKitP::convertDepth(const SbList <double> & data)
{
  int i, n = this->poslist.getLength();
  if (n < 2) return;

  for (i = 0; i < n; i++) {
    well_pos & pos = this->poslist[i];
    pos.pos[2] = - time_depth_interpolate((double) -pos.pos[2], data); 
  }
}

// called when something has changed and the internal list needs to be
// regenerated.
void 
SmWellLogKitP::oneshot_cb(void * closure, SoSensor * s)
{
  SmWellLogKitP * thisp = (SmWellLogKitP*) closure;
  thisp->processingoneshot = TRUE;

  thisp->prevaxis = SbVec3f(0.0f, 0.0f, 0.0f);
  thisp->updateList();
  thisp->buildGeometry();
  thisp->generateFaces(SbVec3f(1.0f, 0.0f, 0.0f));
  if (thisp->oneshot->isScheduled()) thisp->oneshot->unschedule();
  thisp->processingoneshot = FALSE;;
}

// need to be called when something changes in the input data. Will
// generate a new list of well_pos structures.
void
SmWellLogKitP::updateList(void)
{
  this->poslist.truncate(0);
  int n = PUBLIC(this)->wellCoord.getNum();
  if (n == 0) {
    SoIndexedFaceSet * fs = (SoIndexedFaceSet*) PUBLIC(this)->getPart("faceSet", TRUE);
    fs->coordIndex.setNum(0);

    SoLODExtrusion * lod = (SoLODExtrusion*) PUBLIC(this)->getPart("well", TRUE);
    lod->spine.setNum(0);
    return;
  }
  
  UTMPosition * utm = (UTMPosition*) PUBLIC(this)->getAnyPart("utm", TRUE);
  const SbVec3d * welldata = PUBLIC(this)->wellCoord.getValues(0);
  
  SbVec3d origin = SbVec3d(welldata[0][0], welldata[0][1], 0.0);

  // set origin to avoid floating point precision issues
  utm->utmposition = origin;

  well_pos pos;
  int cnt = 0;
  double undefval = PUBLIC(this)->undefVal.getValue();

  double prevdepth = 0.0;
  SbVec3f prev(0.0f, 0.0f, 0.0f);
  
  for (int i = 0; i < n; i++) {
    SbVec3d t = SbVec3d(welldata[i][0],
                        welldata[i][1],
                        welldata[i][2]);
    if (t[0] == undefval || t[1] == undefval) continue;
    pos.tvdepth = -t[2];
    t -= origin;

    pos.pos = SbVec3f((float) t[0], (float) t[1], (float) t[2]);
    
    pos.mdepth = PUBLIC(this)->getDepth(i);
    pos.left = PUBLIC(this)->getLeftCurveData(i);
    pos.right = PUBLIC(this)->getRightCurveData(i);
    
    // workaround for some LAS files that specifies an undef value,
    // but use some other value instead
    if (SbAbs(pos.left-undefval) < 1.0f) pos.left = undefval;
    if (SbAbs(pos.right-undefval) < 1.0f) pos.right= undefval;
    
    if (!this->poslist.getLength() || SbAbs(pos.mdepth-prevdepth) >= 0.0) {
      this->poslist.append(pos);
      prevdepth = pos.mdepth;
    }
  }
}


#undef PUBLIC
