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

#include "SmWellLogKit.h"
#include <SmallChange/nodes/UTMPosition.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoLOD.h>
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
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/elements/SoCacheElement.h>

#define DEFAULT_SIZE 100.0f

#define DEFAULT_TOPLOD_DISTANCE 10000.0f
#define DEFAULT_LOD_DISTANCE 4000.0f
#define EPS 0.1

typedef struct {
  SbVec3f pos;
  double mdepth;  // measured depth
  double tvdepth; // true vertical depth
  double gamma;
  double res;
  SbName lith;
  SbName fluid;
} well_pos;

class SmWellLogKitP {
public:  
  
  SmWellLogKitP(SmWellLogKit * master)
    : master(master) { }

  SmWellLogKit * master;

  // FIXME: consider dict
  SbList <SbName> cm_name;
  SbList <uint32_t> cm_col;

  SbList <float> gammaoffset;
  SbList <float> resoffset;

  SbList <well_pos> poslist;
  SbVec3f axis;

  uint32_t find_col(const SbName & name);
  int find_colidx(const SbName & name);
  int findIdx(const SbVec3f & pos);
  int findIdx(const double depth);
  void updateList(void);
  void buildGeometry(void);
  void generateFaces(const SbVec3f & axis);
  void mergeDepths(const SbList <double> & depthlist);
  void reduce(void);
  void setLithOrFluid(const SbList <double> & limits,
                      const SbList <SbName> & names,
                      const SbBool islith);
  void convertDepth(const SbList <double> & data);
  void fillInfo(SbString & str, int idx);
  static void callback_cb(void * userdata, SoAction * action);

  SoOneShotSensor * oneshot;
  
  static void oneshot_cb(void * closure, SoSensor * s);
};

SO_KIT_SOURCE(SmWellLogKit);

#define PRIVATE(obj) (obj)->pimpl

SmWellLogKit::SmWellLogKit(void)
{
  PRIVATE(this) = new SmWellLogKitP(this);
  PRIVATE(this)->axis = SbVec3f(0.0f, 0.0f, 0.0f);
  PRIVATE(this)->oneshot = new SoOneShotSensor(SmWellLogKitP::oneshot_cb, PRIVATE(this));

  SO_KIT_CONSTRUCTOR(SmWellLogKit);
  
  SO_KIT_ADD_FIELD(undefVal, (999999.0f));
  SO_KIT_ADD_FIELD(name,(""));
  SO_KIT_ADD_FIELD(wellCoord, (0.0, 0.0, 0.0));
  SO_KIT_ADD_FIELD(wellDepth, (0.0f));
  SO_KIT_ADD_FIELD(gammaSize, (100.0f));
  SO_KIT_ADD_FIELD(resSize, (100.0f));
  SO_KIT_ADD_FIELD(gamma, (0.0f));
  SO_KIT_ADD_FIELD(res, (0.0f));
  SO_KIT_ADD_FIELD(lodDistance1, (5000.0f));
  SO_KIT_ADD_FIELD(lodDistance2, (10000.0f));
  
  this->wellCoord.setNum(0);
  this->wellCoord.setDefault(TRUE);
  
  this->gamma.setNum(0);
  this->res.setNum(0);
  this->wellDepth.setNum(0);
  this->gamma.setDefault(TRUE);
  this->res.setDefault(TRUE);
  this->wellDepth.setDefault(TRUE);

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(utm, UTMPosition, FALSE, topSeparator, topLod, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(topLod, SoLOD, FALSE, topSeparator, "", TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(topLodGroup, SoSeparator, FALSE, topLod, topInfo, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(lightModel, SoLightModel, FALSE, topLodGroup, shapeHints, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(shapeHints, SoShapeHints, FALSE, topLodGroup, lineSetColor, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(lineSetColor, SoBaseColor, FALSE, topLodGroup, lineCoords, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(lineCoords, SoCoordinate3, FALSE, topLodGroup, lineSet, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(lineSet, SoLineSet, FALSE, topLodGroup, pickStyle, FALSE);
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

  SoShapeHints * sh = (SoShapeHints*) this->getAnyPart("shapeHints", TRUE);
  sh->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
  sh->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
  sh->faceType = SoShapeHints::CONVEX;
  
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
SmWellLogKit::notify(SoNotList * l)
{
  if (!PRIVATE(this)->oneshot->isScheduled()) {
    SoField * f = l->getLastField();
    if (f == &this->undefVal ||
        f == &this->name ||
        f == &this->wellCoord ||
        f == &this->wellDepth ||
        f == &this->gammaSize ||
        f == &this->resSize) {
      PRIVATE(this)->oneshot->schedule();
    }
  }
  inherited::notify(l);
}

#undef PRIVATE

#define PUBLIC(obj) (obj)->master

uint32_t 
SmWellLogKitP::find_col(const SbName & name) 
{
  int idx = find_colidx(name);
  if (idx < 0) {
    //    myfprintf(stderr,"undef col: \"%s\"\n", name.getString());
    return 0x000000ff;
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
    SoCacheElement::invalidate(action->getState());

    SbVec3f axis(1.0f, 0.0f, 0.0f);
    // FIXME!!!
//     SoCamera * cam = currviewer->getCamera();
//     cam->orientation.getValue().multVec(axis, axis);
    
    axis[2] = 0.0f;
    if (axis == SbVec3f(0.0f, 0.0f, 0.0f)) axis = SbVec3f(1.0f, 0.0f, 0.0f);
    else axis.normalize();
    thisp->generateFaces(axis);
  }
}


void
SmWellLogKitP::fillInfo(SbString & str, int idx)
{
  if (idx < 0 || idx >= this->poslist.getLength()) return;

  const well_pos & pos = this->poslist[idx];
  
  SbString gamma("UNDEFINED");
  SbString res("UNDEFINED");
  
  if (pos.gamma != PUBLIC(this)->undefVal.getValue()) {
    gamma.sprintf("%g", pos.gamma);
  }
  if (pos.res != PUBLIC(this)->undefVal.getValue()) {
    res.sprintf("%g", pos.res);
  }

  str.sprintf("%s\n"
              "Depth: %g\n"
              "TVDSS: %g\n"
              "Time:  %g\n"
              "Gamma: %s\n"
              "Res:   %s\n"
              "Lith:  %s\n"
              "Fluid: %s",
              PUBLIC(this)->name.getValue().getString(),
              pos.mdepth,
              -pos.tvdepth,
              -pos.pos[2],
              gamma.getString(),
              res.getString(),
              pos.lith.getString(),
              pos.fluid.getString());
            
}

int
SmWellLogKitP::findIdx(const SbVec3f & pos)
{
  int n = this->poslist.getLength();
  for (int i = 0; i < n; i++) {
    if (pos[2] > this->poslist[i].pos[2]) return i;
  }
  return n-1;
}

int
SmWellLogKitP::findIdx(const double depth)
{
  int idx = this->poslist.getLength() / 2;
  int len = this->poslist.getLength() / 2;

  const well_pos * ptr = this->poslist.getArrayPtr();

  while (len > 1) {
    assert(idx >= 0 && idx < this->poslist.getLength());
    len /= 2;
    if (depth == ptr[idx].tvdepth) return idx;
    if (depth < ptr[idx].tvdepth) {
      idx -= len;
    }
    else {
      if (idx == this->poslist.getLength()-1) return idx+1;
      else if (depth < ptr[idx+1].tvdepth) return idx;
      idx += len;
    }
  }
  return idx;
}


void
SmWellLogKitP::reduce(void)
{

  // FIXME: not properly implemented
  SbList <well_pos> newlist(this->poslist.getLength());

  int i, n = this->poslist.getLength();

  const well_pos * src = this->poslist.getArrayPtr();

  for (i = 0; i < n; i++) {
    SbBool doadd = FALSE;
    if (i == 0 || i == n-1) doadd = TRUE;
    else {
      if ((src[i-1].lith != src[i].lith) || (src[i].lith != src[i+1].lith)) doadd = TRUE;
      if ((src[i-1].fluid != src[i].fluid) || (src[i].fluid != src[i+1].fluid)) doadd = TRUE;
    }
  }
}


void 
SmWellLogKitP::setLithOrFluid(const SbList <double> & limits,
                            const SbList <SbName> & names,
                            const SbBool islith)
{
  int n = limits.getLength() / 2;

  for (int i = 0; i < n; i++) {
    int i0 = this->findIdx(limits[i*2]);
    int i1 = this->findIdx(limits[i*2+1] - EPS);

    if (i0 < 0 || i1 < 0 || i0 >= this->poslist.getLength()) {
      //      myfprintf(stderr,"oops\n");
    }

    for (;i0 <= i1 && i0 < this->poslist.getLength(); i0++) {
      if (islith) this->poslist[i0].lith = names[i];
      else this->poslist[i0].fluid = names[i];
    }
  }
}


void
SmWellLogKitP::generateFaces(const SbVec3f & newaxis)
{

  SoCoordinate3 * coord = (SoCoordinate3*) PUBLIC(this)->getAnyPart("coord", TRUE);
  
  int i, n = this->poslist.getLength();
  //  coord->point.setNum(3*n);
  SbVec3f * dst = coord->point.startEditing();
  
  for (i = 0; i < n; i++) {
    dst[n+i] = this->poslist[i].pos + newaxis * this->gammaoffset[i];
    dst[2*n+i] = this->poslist[i].pos - newaxis * this->resoffset[i];
  }
  coord->point.finishEditing();
}

static double log_func(double val)
{
  if (val < 0.0) return -1.0;
  return log10(1.0 + val);
}

void
SmWellLogKitP::buildGeometry(void)
{
  this->gammaoffset.truncate(0);
  this->resoffset.truncate(0);

  int i, n = this->poslist.getLength();

  double gammamax = 0.0f;
  double resmax = 0.0f;

  for (i = 0; i < n; i++) {
    if (this->poslist[i].gamma > gammamax &&
        this->poslist[i].gamma != PUBLIC(this)->undefVal.getValue()) gammamax = this->poslist[i].gamma;
    if (this->poslist[i].res > resmax &&
        this->poslist[i].res != PUBLIC(this)->undefVal.getValue()) resmax = this->poslist[i].res;
  }
  
  resmax = log_func(resmax);
  
  float gammascale = gammamax ? PUBLIC(this)->gammaSize.getValue() * 0.5f/ gammamax : 1.0f;
  float resscale = resmax ? PUBLIC(this)->resSize.getValue() * 0.5f / resmax : 1.0f;

  int numgamma = 0;
  int numres = 0;

  for (i = 0; i < n-1; i++) {
    if (this->poslist[i].gamma != PUBLIC(this)->undefVal.getValue() &&
        this->poslist[i+1].gamma != PUBLIC(this)->undefVal.getValue()) numgamma++;
    if (this->poslist[i].res != PUBLIC(this)->undefVal.getValue() &&
        this->poslist[i+1].res != PUBLIC(this)->undefVal.getValue()) numres++;
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

  SoCoordinate3 * lscoord = (SoCoordinate3*) PUBLIC(this)->getAnyPart("lineCoord", TRUE);
  
  lscoord->point.setNum(redlist.getLength());
  SbVec3f * lsdst = lscoord->point.startEditing();
  for (i = 0; i < redlist.getLength(); i++) {
    lsdst[i] = redlist[i];
  }
  lscoord->point.finishEditing();

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
    col[i] = find_col(this->poslist[i].lith);
    col[i+n] = find_col(this->poslist[i].fluid);
  }

  for (i = 0; i < n; i++) {
    float g = this->poslist[i].gamma;
    float r = this->poslist[i].res;
    this->gammaoffset.append(g < 0 ? 0.0f : gammascale * g + PUBLIC(this)->gammaSize.getValue()*0.5f);
    this->resoffset.append(r < 0.0f ? 0.0f : resscale * log_func(r) + PUBLIC(this)->resSize.getValue() * 0.5f);

    dst[n+i] = this->poslist[i].pos + 
      this->axis * (float)(gammascale * this->poslist[i].gamma + PUBLIC(this)->gammaSize.getValue()*0.5f);
    dst[2*n+i] = this->poslist[i].pos - 
      this->axis * (float) (resscale * this->poslist[i].res + PUBLIC(this)->resSize.getValue() * 0.5f);

//      fprintf(stderr,"pos: %g %g %g, %g %g %g\n",
//              dst[n+i][0],
//              dst[n+i][1],
//              dst[n+i][2],
//              dst[2*n+i][0],
//              dst[2*n+i][1],
//              dst[2*n+i][2]);
  }

  SoIndexedFaceSet * ifs = (SoIndexedFaceSet*) PUBLIC(this)->getAnyPart("faceSet", TRUE);

  ifs->coordIndex.setNum(numgamma * 5 + numres * 5);
  int32_t * dsti = ifs->coordIndex.startEditing();

  ifs->materialIndex.setNum(numgamma + numres);
  int32_t * dstm = ifs->materialIndex.startEditing();

  for (i = 0; i < n-1; i++) {
    if (this->poslist[i].gamma != PUBLIC(this)->undefVal.getValue() &&
        this->poslist[i+1].gamma != PUBLIC(this)->undefVal.getValue()) {
      assert(this->poslist[i].gamma >= 0.0f);
      assert(this->poslist[i+1].gamma >= 0.0f);

      *dsti++ = i;
      *dsti++ = n+i;
      *dsti++ = n+i+1;
      *dsti++ = i+1;
      *dsti++ = -1;
      *dstm++ = i;
    }
    if (this->poslist[i].res != PUBLIC(this)->undefVal.getValue() &&
        this->poslist[i+1].res != PUBLIC(this)->undefVal.getValue()) {
      assert(this->poslist[i].res >= 0.0f);
      assert(this->poslist[i+1].res >= 0.0f);

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
  p.gamma = prev.gamma; // FIXME: interpolate?
  p.res = prev.res;
  p.mdepth = prev.mdepth + (next.mdepth-prev.mdepth) * t;
  p.tvdepth = newdepth;

  if (t <= 0.5) {
    p.lith = prev.lith;
    p.fluid = prev.fluid;
  }
  else {
    p.lith = next.lith;
    p.fluid = next.fluid;
  }  
}

void 
SmWellLogKitP::mergeDepths(const SbList <double> & depthlist)
{
  SbList <well_pos> newlist(this->poslist.getLength() + depthlist.getLength());
  
  int i = 0;
  int j = 0;
  int numi = this->poslist.getLength();
  int numj = depthlist.getLength();

  int cnt = 0;
  //  myfprintf(stderr,"start merge: %s, %d %d...", this->name.getString(), numi, numj);
  
  while ((i < numi) && (j < numj)) {
    double i0 = this->poslist[i].tvdepth;
    double j0 = depthlist[j];

    if (SbAbs(i0-j0) < EPS) {
      if (i0 != j0) cnt++;
      newlist.append(this->poslist[i]);
      i++;
      j++;
    }
    else if (i0 < j0) {
      newlist.append(this->poslist[i]);
      i++;
    }
    else { // j0 < i0
      if (newlist.getLength() == 0) {
        well_pos p = this->poslist[0];
        p.tvdepth = depthlist[j];
        newlist.append(p);
      }
      else {
        well_pos prev = newlist[newlist.getLength()-1];
        well_pos next = this->poslist[i];
        well_pos p;

        interpolate(p, prev, next, depthlist[j]);
     
        newlist.append(p);
      }
      j++;
    } 
  }
  while (i < numi) {
    newlist.append(this->poslist[i]);
    i++;
  }
  
  if (j < numj) {
    //    myfprintf(stderr,"extra depth values found!\n");
  }

  this->poslist.truncate(0);
  SbVec3f prev(0.0f, 0.0f, 0.0f);
  double prevdepth = 0.0;
  for (i = 0; i < newlist.getLength(); i++) {
    if (i == 0 || (newlist[i].pos != prev && SbAbs(newlist[i].tvdepth-prevdepth) >= EPS)) {
      this->poslist.append(newlist[i]);
      prev = newlist[i].pos;
      prevdepth = newlist[i].tvdepth;
    }
    else cnt++;
  }
  //  myfprintf(stderr,"finished: %d (%d skipped)\n", newlist.getLength(), cnt);
}


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

void 
SmWellLogKitP::oneshot_cb(void * closure, SoSensor * s)
{
  SmWellLogKitP * thisp = (SmWellLogKitP*) closure;

  thisp->updateList();

  thisp->buildGeometry();
}

void
SmWellLogKitP::updateList(void)
{
  this->poslist.truncate(0);
  int n = PUBLIC(this)->wellCoord.getNum();
  if (n == 0) {
    SoIndexedFaceSet * fs = (SoIndexedFaceSet*) PUBLIC(this)->getPart("faceSet", TRUE);
    fs->coordIndex.setNum(0);

    SoLineSet * ls = (SoLineSet*) PUBLIC(this)->getPart("lineSet", TRUE);
    ls->numVertices = 0;
    return;
  }
  
  UTMPosition * utm = (UTMPosition*) PUBLIC(this)->getAnyPart("utm", TRUE);

  const SbVec3d * welldata = PUBLIC(this)->wellCoord.getValues(0);
  
  SbVec3d origin = SbVec3d(welldata[0][0], welldata[0][1], 0.0);
  utm->utmposition = origin;

  well_pos pos;
  pos.lith = SbName("UNDEFINED");
  pos.fluid = SbName("UNDEFINED");

  int cnt = 0;
  double undefval = PUBLIC(this)->undefVal.getValue();

  int numgamma = PUBLIC(this)->gamma.getNum();
  int numres = PUBLIC(this)->res.getNum();
  int numdepth = PUBLIC(this)->wellDepth.getNum();

  const float * gammaarray = numgamma ? PUBLIC(this)->gamma.getValues(0) : NULL;
  const float * resarray = numres ? PUBLIC(this)->res.getValues(0) : NULL;
  const float * deptharray = numdepth ? PUBLIC(this)->wellDepth.getValues(0) : NULL;

  double prevdepth = 0.0;
  SbVec3f prev(0.0f, 0.0f, 0.0f);
  
  for (int i = 0; i < n; i++) {
    SbVec3d t = SbVec3d(welldata[i][0],
                        welldata[i][1],
                        welldata[i][2]);
    if (t[0] == undefval || t[1] == undefval || t[2] == undefval) continue;
    t -= origin;

    pos.pos = SbVec3f((float) t[0], (float) t[1], (float) t[2]);
    
    pos.mdepth = i < numdepth ? deptharray[i] : pos.pos[2];
    pos.tvdepth = - t[2];
    pos.gamma = i < numgamma ? gammaarray[i] : -1.0f;
    pos.res = i < numres ? resarray[i] : -1.0f;
    
    if (pos.gamma < 0.0f) pos.gamma = undefval;
    if (pos.res < 0.0f) pos.res = undefval;

    if (!this->poslist.getLength() || SbAbs(pos.mdepth-prevdepth) >= EPS) {
      this->poslist.append(pos);
      prevdepth = pos.mdepth;
    }
    else {
      //myfprintf(stderr,"skipped depth becaused of EPS\n");
    }
  }
}


#undef PUBLIC
#undef EPS
#undef DEFAULT_SIZE
#undef DEFAULT_LOD_DISTANCE